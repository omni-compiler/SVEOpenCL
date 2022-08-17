#ifndef __SOCL_KERNEL_VECTORIZER_H
#define __SOCL_KERNEL_VECTORIZER_H

#include <map>
#include <vector>

namespace llvm {
  class Constant;
  class Function;
  class Instruction;
  class StoreInst;
  class Type;
  class Value;
  class LoadInst;
}

using FuncMapTy = std::map<llvm::Function *, llvm::Function *>;
using IntMapTy = std::map<llvm::Function *, unsigned>;
using InstrVectorTy = std::vector<llvm::Instruction *>;
using ValueVectorTy = std::vector<llvm::Value *>;
using ValueMapTy = std::map<llvm::Value *, llvm::Value *>;

class KernelVectorizer {
private:
  FuncMapTy VecFuncMap;
  IntMapTy SVEBitWidthMap;

  InstrVectorTy EraseInstrs;
  ValueVectorTy SeedValues;
  ValueMapTy VecValueMap;

  unsigned getNumSVEElmnts(llvm::Type *Ty);
  void collectSeedValues(llvm::Function &F);
  unsigned calcNumSVEElmnts(llvm::Value *V);
  void createMask(llvm::Function&F, unsigned NumSVEElmnts);
  bool duplicateConstValue(llvm::Function &F, llvm::Value *V);
  bool vectorizeLoad(llvm::Function &F, llvm::LoadInst *LI);
  bool vectorizeStore(llvm::Function &F, llvm::StoreInst *SI);
  bool vectorizeBinaryArithmetic(llvm::Function &F, llvm::Instruction *I);
  bool vectorizeInstruction(llvm::Function &F, llvm::Instruction *I);
  bool vectorizeValue(llvm::Function &F, llvm::Value *V);
  bool doVectorization(llvm::Function &F, unsigned NumSVEElmnts);

public:
  KernelVectorizer() {
    VecFuncMap.clear();
    SVEBitWidthMap.clear();
  }

  llvm::Function *getVectorizedKernel(llvm::Function *F) const;
  unsigned getSVEBitWidth(llvm::Function *F) const;

  bool vectorizeKernel(llvm::Function &F);
};

#endif // __SOCL_KERNEL_VECTORIZER_H
