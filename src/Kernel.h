#ifndef __SOCL_KERNEL_H
#define __SOCL_KERNEL_H

#include <map>
#include "CommandQueue.h"
#include "Constant.h"
#include "Program.h"
#include "RetCode.h"

#define _SOCL_CL_DEVICE_MAX_PARAMETER_SIZE 256

struct _cl_kernel {
private:
  KernelFuncTy WrapperFuncPtr1;
  KernelFuncTy WrapperFuncPtr2;
  KernelFuncTy WrapperFuncPtr3;

  size_t NumArgs;

  socl::ArgKind *ArgKinds;

  size_t TotalArgSize;
  void **ArgPtrs;

public:
  _cl_kernel() = delete;
  _cl_kernel(socl::Program *P, const char *N);
  ~_cl_kernel();

  _cl_kernel(const _cl_kernel &C) = delete;
  _cl_kernel &operator=(const _cl_kernel &C) = delete;

  _cl_kernel(_cl_kernel &C) = delete;
  _cl_kernel &operator=(_cl_kernel &C) = delete;

  socl::RetCode setArg(unsigned int Index, size_t Size, const void *Value);

  socl::RetCode enqueueKernel(socl::CommandQueue *CQ,
                              unsigned int WorkDim,
                              const size_t *GlobalWorkOffset,
                              const size_t *GlobalWorkSize,
                              const size_t *LocalWorkSize);

  std::string getClassName() const {
    return "cl_kernel";
  }
};

namespace socl {

using Kernel = struct _cl_kernel;
extern std::map<Kernel *, unsigned int> KernelMap;

}

#endif // __SOCL_KERNEL_H
