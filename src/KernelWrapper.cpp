#include <iostream>
#include <string>
#include <utility>
#include <llvm/IR/IntrinsicsAArch64.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/Utils/Cloning.h>
#include "KernelWrapper.h"
#include "KernelVectorizer.h"

// FIXME change error handling (replace llvm_unreachable())

using namespace llvm;

Value *
KernelWrapper::createArgRefValue(IRBuilder<> &IRB, Value *Arg, int Index, Type *RefType) {
  Type *RefPtrType = RefType->getPointerTo();
// FIXME why not use TypeID?
  if (Arg->getType() != RefPtrType) {
    Arg = IRB.CreatePointerCast(Arg, RefPtrType);
  }

  Value *GEP = IRB.CreateInBoundsGEP(RefType, Arg, IRB.getInt64(Index));
  return IRB.CreateLoad(RefType, GEP);
}

CallInst *
KernelWrapper::createLoop(uint64_t CurrentDim, IRBuilder<> &IRB, LLVMContext &Context,
                          Function *F, std::vector<Value *> &Args,
                          ValueVectorTy &IndValVector,
                          Value *VecStepVal) {
  Value *StartVal = LoopStartVals[CurrentDim];
  Value *EndVal = LoopEndVals[CurrentDim];

  BasicBlock *PreheaderBB = IRB.GetInsertBlock();
  Function *PreheaderBBFunc = PreheaderBB->getParent();

  BasicBlock *LoopEntryBB = BasicBlock::Create(Context, "loop_entry", PreheaderBBFunc);
  IRB.CreateBr(LoopEntryBB);
  IRB.SetInsertPoint(LoopEntryBB);
  PHINode *IndVar = IRB.CreatePHI(Type::getInt64Ty(Context), 2, "i");
  IndVar->addIncoming(StartVal, PreheaderBB);

  Value *NextVal = nullptr;
  CallInst *Instr = nullptr;
  if (0 == CurrentDim) {
    if (nullptr == VecStepVal) {
      NextVal = IRB.CreateAdd(IndVar, IRB.getInt64(1));
    } else {
      NextVal = IRB.CreateAdd(IndVar, VecStepVal);;
    }

    Instr = IRB.CreateCall(F->getFunctionType(), F, Args);
  } else {
    NextVal = IRB.CreateAdd(IndVar, IRB.getInt64(1));
    Instr = createLoop(CurrentDim - 1, IRB, Context, F, Args, IndValVector, VecStepVal);
  }

  BasicBlock *LoopExitBB = IRB.GetInsertBlock();
  BasicBlock *PostLoopBB = BasicBlock::Create(Context, "post_loop", PreheaderBBFunc);
  IRB.CreateCondBr(IRB.CreateICmpULT(NextVal, EndVal), LoopEntryBB, PostLoopBB);
  IRB.SetInsertPoint(PostLoopBB);
  IndVar->addIncoming(NextVal, LoopExitBB);

  IndValVector.push_back(IndVar);
  return Instr;
}

CallInst *
KernelWrapper::createWrapperFunction(uint64_t WorkDim, Function &F, ValueVectorTy &IndValVector,
                                     bool isVectorized) {
  Module *M = F.getParent();
  LLVMContext &Context = F.getContext();
  IRBuilder<> IRB(Context);

  // FIXME make more maintainalbe function name
  std::string WrapperFuncName = "_SOCL_WPFN";
  if (!isVectorized) {
    WrapperFuncName += "_SOCL_SCAK_";
  } // XXX SVE: _SOCL_SVEK_

  WrapperFuncName += F.getName().str() + "_" + std::to_string(WorkDim);
  FunctionCallee WrapperFC = M->getOrInsertFunction(WrapperFuncName,
                                                    IRB.getVoidTy(),
                                                    IRB.getInt8PtrTy()->getPointerTo(),
                                                    IRB.getInt8PtrTy(),
                                                    IRB.getInt8PtrTy(),
                                                    IRB.getInt8PtrTy(),
                                                    IRB.getInt8PtrTy());
  Function *WrapperFn = cast<Function>(WrapperFC.getCallee());

  BasicBlock *WrapperFnEntry = BasicBlock::Create(Context, "entry", WrapperFn);
  IRB.SetInsertPoint(WrapperFnEntry);

  CallInst *CI = nullptr;
  if (isVectorized) {
    Function *IntrinFn = nullptr;
    unsigned SVEBitWidth = Vectorizer->getSVEBitWidth(&F);
    switch (SVEBitWidth) {
      case 8:
        IntrinFn = Intrinsic::getDeclaration(F.getParent(), Intrinsic::aarch64_sve_cntb);
        break;
      case 16:
        IntrinFn = Intrinsic::getDeclaration(F.getParent(), Intrinsic::aarch64_sve_cnth);
        break;
      case 32:
        IntrinFn = Intrinsic::getDeclaration(F.getParent(), Intrinsic::aarch64_sve_cntw);
        break;
      case 64:
        IntrinFn = Intrinsic::getDeclaration(F.getParent(), Intrinsic::aarch64_sve_cntd);
        break;
      default:
        llvm_unreachable("unexpected bit width");
    }

    // FIXME 31 is a magic number for SV_ALL
    CI = IRB.CreateCall(IntrinFn->getFunctionType(), IntrinFn, {IRB.getInt32(31)});
  }

  Argument *WrapperArg = WrapperFn->getArg(0);

  std::vector<Value *> CallerArgs;
  for (Argument &Arg : F.args()) {
    // FIXME current implementation create addresscast for __local
    // OpenCL kernel preprocessing required
    // - removing address space and implement __local memoryi
    Type *ArgType = Arg.getType();
    Type *ArgPtrType = ArgType->getPointerTo();
    Value *V = createArgRefValue(IRB, WrapperArg, Arg.getArgNo(), IRB.getInt8PtrTy());
    V = IRB.CreatePointerCast(V, ArgPtrType);
    V = IRB.CreateLoad(ArgType, V);
    CallerArgs.push_back(V);
  }

  Argument *GlobalOffsetArg = WrapperFn->getArg(1);
  for (uint64_t i = 0; i < WorkDim; i++) {
    GlobalOffsets.push_back(createArgRefValue(IRB, GlobalOffsetArg, i, IRB.getInt64Ty()));
  }

  Argument *GlobalSizeArg = WrapperFn->getArg(2);
  for (uint64_t i = 0; i < WorkDim; i++) {
    GlobalSizes.push_back(createArgRefValue(IRB, GlobalSizeArg, i, IRB.getInt64Ty()));
  }

  Argument *LocalSizeArg = WrapperFn->getArg(3);
  for (uint64_t i = 0; i < WorkDim; i++) {
    LocalSizes.push_back(createArgRefValue(IRB, LocalSizeArg, i, IRB.getInt64Ty()));
  }

  Argument *KernelArg = WrapperFn->getArg(4);
  for (uint64_t i = 0; i < WorkDim; i++) {
    WorkGroupIDs.push_back(createArgRefValue(IRB, KernelArg, i, IRB.getInt64Ty()));
  }

  for (uint64_t i = WorkDim; i < WorkDim * 3; i += 2) {
    LoopStartVals.push_back(createArgRefValue(IRB, KernelArg, i, IRB.getInt64Ty()));
    LoopEndVals.push_back(createArgRefValue(IRB, KernelArg, i + 1, IRB.getInt64Ty()));
  }

  CallInst *Instr = createLoop(WorkDim - 1, IRB, Context, &F, CallerArgs, IndValVector, CI);

  IRB.CreateRetVoid();

  return Instr;
}

void
KernelWrapper::replaceIndexFunctionWith(uint64_t WorkDim, IRBuilder<> &IRB, CallInst *CI,
                                        ValueVectorTy &Vec, uint64_t DefV) {
  uint64_t Index = dyn_cast<ConstantInt>(CI->getOperand(0))->getZExtValue();
  if (Index < WorkDim) {
    CI->replaceAllUsesWith(Vec[Index]);
  } else {
    CI->replaceAllUsesWith(IRB.getInt64(DefV));
  }

  CI->eraseFromParent();
}

bool
KernelWrapper::replaceIndexFunctions(uint64_t WorkDim, Function &F, ValueVectorTy &IndValVector) {
  LLVMContext &Context = F.getContext();
  IRBuilder<> IRB(Context);

  bool Changed = false;
  for (auto &BB : F) {
    for (BasicBlock::iterator DI = BB.begin(); DI != BB.end(); ) {
      Instruction *Inst = &*DI++;
      if (isa<CallInst>(Inst)) {
        CallInst *CI = dyn_cast<CallInst>(Inst);
        Function *Func = CI->getCalledFunction();
        if (Func->getName().equals("get_global_id")) {
          if (isa<ConstantInt>(CI->getOperand(0))) {
            uint64_t Index = dyn_cast<ConstantInt>(CI->getOperand(0))->getZExtValue();
            if (Index < WorkDim) {
              IRB.SetInsertPoint(CI);
              Value *V = IRB.CreateMul(WorkGroupIDs[Index], LocalSizes[Index]);
              V = IRB.CreateAdd(V, IndValVector[Index]);
              V = IRB.CreateAdd(V, GlobalOffsets[Index]);
              CI->replaceAllUsesWith(V);
            } else {
              CI->replaceAllUsesWith(IRB.getInt64(0));
            }

            CI->eraseFromParent();
            Changed = true;
          } else {
            llvm_unreachable("variable index for get_global_id() is not implemented yet");
          }
        } else if (Func->getName().equals("get_global_size")) {
          if (isa<ConstantInt>(CI->getOperand(0))) {
            replaceIndexFunctionWith(WorkDim, IRB, CI, GlobalSizes, 1);
            Changed = true;
          } else {
            llvm_unreachable("variable index for get_global_size() is not implemented yet");
          }
        } else if (Func->getName().equals("get_local_id")) {
          if (isa<ConstantInt>(CI->getOperand(0))) {
            replaceIndexFunctionWith(WorkDim, IRB, CI, IndValVector, 0);
            Changed = true;
          } else {
            llvm_unreachable("variable index for get_local_id() is not implemented yet");
          }
        } else if (Func->getName().equals("get_local_size")) {
          if (isa<ConstantInt>(CI->getOperand(0))) {
            replaceIndexFunctionWith(WorkDim, IRB, CI, LocalSizes, 1);
            Changed = true;
          } else {
            llvm_unreachable("variable index for get_local_size() is not implemented yet");
          }
        }
      }
    }
  }

  return Changed;
}

bool
KernelWrapper::createExecutable(uint64_t WorkDim, Function &F, bool isVectorized) {
  ValueVectorTy IndValVector;
  CallInst *Instr = createWrapperFunction(WorkDim, F, IndValVector, isVectorized);

  Function *WrapperFn = Instr->getParent()->getParent();

  InlineFunctionInfo IFI;
  InlineResult IR = InlineFunction(*Instr, IFI);
  if (!IR.isSuccess()) {
// FIXME generate warning message
    llvm_unreachable("kernel inlining failed");
  }

  bool Changed = replaceIndexFunctions(WorkDim, *WrapperFn, IndValVector);

  return Changed;
}

// FIXME create args for work item arrays
bool
KernelWrapper::createWrapperFunction(Function &F) {
  if (F.getCallingConv() != CallingConv::SPIR_KERNEL) return false;

  bool Changed = false;
  for (uint64_t i = 1; i <= 3; i++) {
    GlobalOffsets.clear();
    GlobalSizes.clear();
    LocalSizes.clear();
    WorkGroupIDs.clear();
    LoopStartVals.clear();
    LoopEndVals.clear();
    Changed |= createExecutable(i, F, false);
  }

  Function *VectorizedFunc = Vectorizer->getVectorizedKernel(&F);
  if (nullptr != VectorizedFunc) {
    for (uint64_t i = 1; i <= 3; i++) {
      GlobalOffsets.clear();
      GlobalSizes.clear();
      LocalSizes.clear();
      WorkGroupIDs.clear();
      LoopStartVals.clear();
      LoopEndVals.clear();
      Changed |= createExecutable(i, *VectorizedFunc, true);
    }
  }

  return Changed;
}
