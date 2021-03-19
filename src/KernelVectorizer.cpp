#include <algorithm>
#include <iostream>
#include <string>
#include <utility>
#include <llvm/IR/Argument.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/IntrinsicsAArch64.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include "KernelVectorizer.h"

using namespace llvm;

static const unsigned UnitBitWidth = 128;

static Value *
getValueFromMap(ValueMapTy &Map, Value *V) {
  auto Iter = Map.find(V);
  if (Iter == Map.end()) {
    return nullptr;
  } else {
    return Iter->second;
  }
}

unsigned
KernelVectorizer::getNumSVEElmnts(Type *Ty) {
  switch (Ty->getTypeID()) {
    case Type::IntegerTyID: {
      IntegerType *IntTy = dyn_cast<IntegerType>(Ty);
      return UnitBitWidth / IntTy->getBitWidth();
    }
    case Type::HalfTyID:    return UnitBitWidth / 16;
    case Type::BFloatTyID:  return UnitBitWidth / 16;
    case Type::FloatTyID:   return UnitBitWidth / 32;
    case Type::DoubleTyID:  return UnitBitWidth / 64;
    case Type::FP128TyID:   return UnitBitWidth / 128;
    case Type::PointerTyID: return UnitBitWidth / 64;
    default:
      return 0;
  }
}

void
KernelVectorizer::collectSeedValues(Function &F) {
  for (auto &BB : F) {
    for (BasicBlock::iterator DI = BB.begin(); DI != BB.end(); ) {
      Instruction *Inst = &*DI++;
      if (dyn_cast<StoreInst>(Inst)) {
        SeedValues.push_back(Inst);
      }
    }
  }
}

unsigned
KernelVectorizer::calcNumSVEElmnts(Value *V) {
  if (isa<Argument>(V) || isa<Constant>(V)) {
    return getNumSVEElmnts(V->getType());
  } else if (isa<Instruction>(V)) {
    Instruction *I = dyn_cast<Instruction>(V);
    if (isa<StoreInst>(I)) {
      return calcNumSVEElmnts(I->getOperand(0));
    } else if (isa<LoadInst>(I)) {
      return getNumSVEElmnts(I->getType());
    }

    unsigned NumSVEElmnts = getNumSVEElmnts(V->getType());
    if (0 == NumSVEElmnts) {
      return 0;
    } else if (I->isUnaryOp()) {
      if (calcNumSVEElmnts(I->getOperand(0)) == NumSVEElmnts) {
        return NumSVEElmnts;
      }
    } else if (I->isBinaryOp()) {
      if ((calcNumSVEElmnts(I->getOperand(0)) == NumSVEElmnts) &&
          (calcNumSVEElmnts(I->getOperand(1)) == NumSVEElmnts)) {
        return NumSVEElmnts;
      }
    }
  }

  return 0;
}

void
KernelVectorizer::createMask(Function&F, unsigned NumSVEElmnts) {
  IRBuilder<> IRB(F.getContext());
  IRB.SetInsertPoint(&F.getEntryBlock().front());

  Module *M = F.getParent();

  Function *LSFn = M->getFunction("get_local_size");
  Value *UpperCI = IRB.CreateCall(LSFn->getFunctionType(), LSFn, {IRB.getInt32(0)});

  Function *LIFn = M->getFunction("get_local_id");
  Value *IndVarCI = IRB.CreateCall(LIFn->getFunctionType(), LIFn, {IRB.getInt32(0)});

  Type *VecTy = VectorType::get(IRB.getInt1Ty(), NumSVEElmnts, true);
  Function *IntrinFn = Intrinsic::getDeclaration(F.getParent(),
                                                 Intrinsic::aarch64_sve_whilelo,
                                                 {VecTy, IRB.getInt64Ty()});
  VecValueMap[&F] = IRB.CreateCall(IntrinFn->getFunctionType(), IntrinFn, {IndVarCI, UpperCI});
}

bool
KernelVectorizer::duplicateConstValue(Function &F, Value *V) {
  Type *Ty = V->getType();
  unsigned NumSVEElmnts = getNumSVEElmnts(Ty);
  if (0 == NumSVEElmnts) return false;

  Type *VecTy = VectorType::get(Ty, NumSVEElmnts, true);
  Function *IntrinFn = Intrinsic::getDeclaration(F.getParent(),
                                                 Intrinsic::aarch64_sve_dup_x,
                                                 {VecTy});
  IRBuilder<> IRB(F.getContext());
  IRB.SetInsertPoint(&F.getEntryBlock().front());
  VecValueMap[V] = IRB.CreateCall(IntrinFn->getFunctionType(), IntrinFn, {V});
  return true;
}

bool
KernelVectorizer::vectorizeLoad(Function &F, LoadInst *LI) {
  unsigned NumOperands = LI->getNumOperands();
  if (1 != NumOperands) return false;

  Value *LoadAddress = LI->getOperand(0);
  Type *Ty = dyn_cast<PointerType>(LoadAddress->getType())->getElementType();
  unsigned NumSVEElmnts = getNumSVEElmnts(Ty);
  if (0 == NumSVEElmnts) return false;

  Type *VecTy = VectorType::get(Ty, NumSVEElmnts, true);
  Function *IntrinFn = Intrinsic::getDeclaration(F.getParent(),
                                                 Intrinsic::aarch64_sve_ld1,
                                                 {VecTy});
  IRBuilder<> IRB(F.getContext());
  IRB.SetInsertPoint(LI);
  VecValueMap[LI] = IRB.CreateCall(IntrinFn->getFunctionType(), IntrinFn, {getValueFromMap(VecValueMap, &F),
                                                                           LoadAddress});
  EraseInstrs.push_back(LI);
  return true;
}

bool
KernelVectorizer::vectorizeStore(Function &F, StoreInst *SI) {
  Value *StoreValue = SI->getOperand(0);
  if (!vectorizeValue(F, StoreValue)) return false;

  Value *VecStoreValue = getValueFromMap(VecValueMap, StoreValue);
  Type *VecTy = VecStoreValue->getType();
  Function *IntrinFn = Intrinsic::getDeclaration(F.getParent(),
                                                 Intrinsic::aarch64_sve_st1,
                                                 {VecTy});
  IRBuilder<> IRB(F.getContext());
  IRB.SetInsertPoint(SI);
  IRB.CreateCall(IntrinFn->getFunctionType(), IntrinFn, {VecStoreValue,
                                                         getValueFromMap(VecValueMap, &F),
                                                         SI->getOperand(1)});
  EraseInstrs.push_back(SI);
  return true;
}

// not implemented yet
bool
KernelVectorizer::vectorizeBinaryArithmetic(Function &F, Instruction *I) {
  Value *OpL = I->getOperand(0);
  if (!vectorizeValue(F, OpL)) return false;

  Value *OpR = I->getOperand(1);
  if (!vectorizeValue(F, OpR)) return false;

  Value *VecOpL = getValueFromMap(VecValueMap, OpL);
  Value *VecOpR = getValueFromMap(VecValueMap, OpR);

  Type *Ty = I->getType();
  unsigned NumSVEElmnts = getNumSVEElmnts(Ty);
  Type *VecTy = VectorType::get(Ty, NumSVEElmnts, true);

  Function *IntrinFn = nullptr;
  switch (I->getOpcode()) {
    case Instruction::Add:
      IntrinFn = Intrinsic::getDeclaration(F.getParent(), Intrinsic::aarch64_sve_add, {VecTy});
      break;
    case Instruction::FAdd:
      IntrinFn = Intrinsic::getDeclaration(F.getParent(), Intrinsic::aarch64_sve_fadd, {VecTy});
      break;
    case Instruction::Sub:
      IntrinFn = Intrinsic::getDeclaration(F.getParent(), Intrinsic::aarch64_sve_sub, {VecTy});
      break;
    case Instruction::FSub:
      IntrinFn = Intrinsic::getDeclaration(F.getParent(), Intrinsic::aarch64_sve_fsub, {VecTy});
      break;
    case Instruction::Mul:
      IntrinFn = Intrinsic::getDeclaration(F.getParent(), Intrinsic::aarch64_sve_mul, {VecTy});
      break;
    case Instruction::FMul:
      IntrinFn = Intrinsic::getDeclaration(F.getParent(), Intrinsic::aarch64_sve_fmul, {VecTy});
      break;
    case Instruction::SDiv:
      IntrinFn = Intrinsic::getDeclaration(F.getParent(), Intrinsic::aarch64_sve_sdiv, {VecTy});
      break;
    case Instruction::UDiv:
      IntrinFn = Intrinsic::getDeclaration(F.getParent(), Intrinsic::aarch64_sve_udiv, {VecTy});
      break;
    case Instruction::FDiv:
      IntrinFn = Intrinsic::getDeclaration(F.getParent(), Intrinsic::aarch64_sve_fdiv, {VecTy});
      break;
    default:
      llvm_unreachable("unexpected binary instruction");
      return false;
  }

  IRBuilder<> IRB(F.getContext());
  IRB.SetInsertPoint(I);
  VecValueMap[I] = IRB.CreateCall(IntrinFn->getFunctionType(), IntrinFn, {getValueFromMap(VecValueMap, &F),
                                                                          VecOpL, VecOpR});
  EraseInstrs.push_back(I);
  return true;
}

bool
KernelVectorizer::vectorizeInstruction(Function &F, Instruction *I) {
  LoadInst *LI = dyn_cast<LoadInst>(I);
  if (LI) {
    return vectorizeLoad(F, LI);
  }

  StoreInst *SI = dyn_cast<StoreInst>(I);
  if (SI) {
    return vectorizeStore(F, SI);
  }

  switch (I->getOpcode()) {
    case Instruction::Add:
    case Instruction::FAdd:
    case Instruction::Sub:
    case Instruction::FSub:
    case Instruction::Mul:
    case Instruction::FMul:
    case Instruction::SDiv:
    case Instruction::UDiv:
    case Instruction::FDiv:
      return vectorizeBinaryArithmetic(F, I);
    default:
      break;
  }

  return false;
}

bool
KernelVectorizer::vectorizeValue(Function &F, Value *V) {
  auto Iter = VecValueMap.find(V);
  if (Iter != VecValueMap.end()) {
    return true;
  }

  if (isa<Argument>(V) || isa<Constant>(V)) {
    return duplicateConstValue(F, V);
  } else if (isa<Instruction>(V)) {
    return vectorizeInstruction(F, dyn_cast<Instruction>(V));
  } else {
    return false;
  }
}

bool
KernelVectorizer::doVectorization(Function &F, unsigned NumSVEElmnts) {
  ValueMapTy VecValueMap;

  createMask(F, NumSVEElmnts);

  for (Value *V : SeedValues) {
    if (!vectorizeValue(F, V)) return false;
  }

  std::reverse(EraseInstrs.begin(), EraseInstrs.end());
  for (Instruction *I : EraseInstrs) {
    I->eraseFromParent();
  }

  return true;
}

Function *
KernelVectorizer::getVectorizedKernel(llvm::Function *F) const {
  auto Iter = VecFuncMap.find(F);
  if (Iter == VecFuncMap.end()) {
    return nullptr;
  } else {
    return Iter->second;
  }
}

unsigned
KernelVectorizer::getSVEBitWidth(llvm::Function *F) const {
  auto Iter = SVEBitWidthMap.find(F);
  if (Iter == SVEBitWidthMap.end()) {
    return 0;
  } else {
    return Iter->second;
  }
}

bool
KernelVectorizer::vectorizeKernel(Function &F) {
  if (F.getCallingConv() != CallingConv::SPIR_KERNEL) return false;

  EraseInstrs.clear();
  SeedValues.clear();
  VecValueMap.clear();

  ValueToValueMapTy VMap;
  Function &ClonedFunc = *CloneFunction(&F, VMap);
  std::string VectorizedFuncName = "_SOCL_SVEK_" + F.getName().str();
  ClonedFunc.setName(VectorizedFuncName);
  ClonedFunc.setCallingConv(CallingConv::C);

  collectSeedValues(ClonedFunc);

  unsigned NumSVEElmnts = 0;
  for (Value *V : SeedValues) {
    unsigned Num = calcNumSVEElmnts(V);
    if (0 == Num) {
      NumSVEElmnts = 0;
      break;
    } else if (0 == NumSVEElmnts) {
      NumSVEElmnts = Num;
    } else if (Num != NumSVEElmnts) {
      NumSVEElmnts = 0;
      break;
    }
  }

  if (0 != NumSVEElmnts) {
    if (doVectorization(ClonedFunc, NumSVEElmnts)) {
      VecFuncMap[&F] = &ClonedFunc;
      SVEBitWidthMap[&ClonedFunc] = UnitBitWidth / NumSVEElmnts;
      return true;
    }
  }

  ClonedFunc.eraseFromParent();
  return false;
}
