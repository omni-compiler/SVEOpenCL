#ifndef __SOCL_DEVICE_H
#define __SOCL_DEVICE_H

#include "RetCode.h"
#include "Type.h"

namespace llvm {
class InitLLVM;
}

namespace socl {
class ThreadEngine;
}

struct _cl_device_id {
private:
// FIXME get real argc. argv
  int Argc;
  char **Argv;
  llvm::InitLLVM *LLVMRuntime;
  socl::ThreadEngine *TE;

  _cl_device_id();
  ~_cl_device_id();

public:
  _cl_device_id(const _cl_device_id &D) = delete;
  _cl_device_id &operator=(const _cl_device_id &D) = delete;

  _cl_device_id(_cl_device_id &D) = delete;
  _cl_device_id &operator=(_cl_device_id &D) = delete;

  static _cl_device_id *getSingleton();

  socl::RetCode runKernel(KernelFuncTy KernelFuncPtr,
                          void **ArgPtrs,
                          unsigned int WorkDim,
                          const size_t *GlobalWorkOffset,
                          const size_t *GlobalWorkSize,
                          const size_t *LocalWorkSize) const;

  std::string getClassName() const {
    return "cl_device";
  }
};

namespace socl {

using Device = struct _cl_device_id;

}

#endif // __SOCL_DEVICE_H
