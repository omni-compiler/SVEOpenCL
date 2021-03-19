#ifndef __SOCL_KERNEL_WRAPPER_H
#define __SOCL_KERNEL_WRAPPER_H

#include <vector>
#include <llvm/IR/IRBuilder.h>

// FIXME handling size_t as int64

class KernelVectorizer;

using ValueVectorTy = std::vector<llvm::Value *>;

class KernelWrapper {
private:
  KernelVectorizer *Vectorizer;

  ValueVectorTy GlobalOffsets;
  ValueVectorTy GlobalSizes;
  ValueVectorTy LocalSizes;
  ValueVectorTy WorkGroupIDs;
  ValueVectorTy LoopStartVals;
  ValueVectorTy LoopEndVals;

  llvm::Value *createArgRefValue(llvm::IRBuilder<> &IRB,
                                 llvm::Value *Arg, int Index, llvm::Type *RefType);
  llvm::CallInst *createLoop(uint64_t CurrentDim, llvm::IRBuilder<> &IRB,
                             llvm::LLVMContext &Context,
                             llvm::Function *F, ValueVectorTy &Args,
                             ValueVectorTy &IndValVector,
                             llvm::Value *VecStepVal);
  llvm::CallInst *createWrapperFunction(uint64_t WorkDim, llvm::Function &F,
                                        ValueVectorTy &IndValVector,
                                        bool isVectorized);
  void replaceIndexFunctionWith(uint64_t WorkDim, llvm::IRBuilder<> &IRB, llvm::CallInst *CI,
                                ValueVectorTy &Vec, uint64_t DefV);
  bool replaceIndexFunctions(uint64_t WorkDim, llvm::Function &F, ValueVectorTy &IndValVector);
  bool createExecutable(uint64_t WorkDim, llvm::Function &F, bool isVectorized);

public:
  KernelWrapper() = delete;
  KernelWrapper(KernelVectorizer *KV) : Vectorizer(KV) {}
  bool createWrapperFunction(llvm::Function &F);
};

#endif // __SOCL_KERNEL_WRAPPER_H
