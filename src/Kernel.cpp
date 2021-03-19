#include <cstring>
#include <llvm/IR/Module.h>
#include "Kernel.h"
#include "MemoryObject.h"
#include "Util.h"

namespace socl {

std::map<Kernel *, unsigned int> KernelMap;

}

_cl_kernel::_cl_kernel(socl::Program *P, const char *N) {
  std::string FuncName = std::string(N);
  // FIXME select sve / scalar wrapper
  // rename wrapper functions
  WrapperFuncPtr1 = P->getKernelPtr(("_SOCL_WPFN_SOCL_SVEK_" + FuncName + "_1").c_str());
  WrapperFuncPtr2 = P->getKernelPtr(("_SOCL_WPFN_SOCL_SVEK_" + FuncName + "_2").c_str());
  WrapperFuncPtr3 = P->getKernelPtr(("_SOCL_WPFN_SOCL_SVEK_" + FuncName + "_3").c_str());

  llvm::Function *F = P->M->getFunction(FuncName);
  NumArgs = F->arg_size();
  ArgKinds = new socl::ArgKind[NumArgs];
  ArgPtrs = new void *[NumArgs];
  for (size_t i = 0; i < NumArgs; i++) {
    ArgPtrs[i] = NULL;
    llvm::Type *Ty = F->getArg(i)->getType();
    if (Ty->isPointerTy()) {
      llvm::PointerType *PtrTy = llvm::dyn_cast<llvm::PointerType>(Ty);
      switch (PtrTy->getAddressSpace()) {
        case 0: ArgKinds[i] = socl::Global; break;
        case 1: ArgKinds[i] = socl::Local; break;
        case 2: ArgKinds[i] = socl::Constant; break;
        default:
          socl::exitWithInternalError(this, "invalid address space");
      }
    }
    else {
      ArgKinds[i] = socl::Value;
    }
  }

  TotalArgSize = 0;
}

_cl_kernel::~_cl_kernel() {
  for (size_t i = 0; i < NumArgs; i++) {
    if (NULL != ArgPtrs[i]) std::free(ArgPtrs[i]);
  }

  delete[] ArgPtrs;
  delete[] ArgKinds;
}

socl::RetCode
_cl_kernel::setArg(unsigned int Index, size_t Size, const void *Value) {
  if (Index >= NumArgs) return socl::OutOfBound;

  char *SrcPtr = NULL;
  switch (ArgKinds[Index]) {
    case socl::Global: {
      if (NULL == Value) return socl::InvalidMemoryObject;

      socl::MemoryObject *Mem = *((socl::MemoryObject **)Value);
      auto Iter = socl::MemObjMap.find(Mem);
      if (Iter == socl::MemObjMap.end()) {
        return socl::InvalidMemoryObject;
      }

      if (sizeof(socl::MemoryObject *) != Size) return socl::InvalidSize;

      SrcPtr = (char *)&(Mem->Ptr);
    } break;
    case socl::Local: {
      // FIXME check NULL
    } break;
    case socl::Constant: {
      // FIXME implement
    } break;
    case socl::Value: {
      if (NULL == Value) return socl::InvalidParameter;

      SrcPtr = (char *)Value;
    } break;
    default:
      // FIXME error check;
      return socl::InvalidParameter;
  }

  if (TotalArgSize + Size > _SOCL_CL_DEVICE_MAX_PARAMETER_SIZE) return socl::OutOfMemory;

// FIXME slow implementation
  if (NULL != ArgPtrs[Index]) std::free(ArgPtrs[Index]);

  char *DstPtr = (char *)std::malloc(Size);
  ArgPtrs[Index] = DstPtr;

  std::memcpy(DstPtr, SrcPtr, Size);
  TotalArgSize += Size;

  return socl::Success;
}

socl::RetCode
_cl_kernel::enqueueKernel(socl::CommandQueue *CQ,
                          unsigned int WorkDim,
                          const size_t *GlobalWorkOffset,
                          const size_t *GlobalWorkSize,
                          const size_t *LocalWorkSize) {
  KernelFuncTy FuncPtr = nullptr;
  switch (WorkDim) {
    case 1: FuncPtr = WrapperFuncPtr1; break;
    case 2: FuncPtr = WrapperFuncPtr2; break;
    case 3: FuncPtr = WrapperFuncPtr3; break;
    default:
      return socl::OutOfBound;
  }

  return CQ->enqueueKernel(FuncPtr, ArgPtrs,
                           WorkDim, GlobalWorkOffset, GlobalWorkSize, LocalWorkSize);
}
