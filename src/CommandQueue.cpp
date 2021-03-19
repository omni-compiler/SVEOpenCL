#include "CommandQueue.h"

namespace socl {

std::map<CommandQueue *, unsigned int> CQMap;

}

socl::RetCode
_cl_command_queue::enqueueKernel(KernelFuncTy KernelFuncPtr,
                                 void **ArgPtrs,
                                 unsigned int WorkDim,
                                 const size_t *GlobalWorkOffset,
                                 const size_t *GlobalWorkSize,
                                 const size_t *LocalWorkSize) {
  return Dev->runKernel(KernelFuncPtr, ArgPtrs,
                        WorkDim, GlobalWorkOffset, GlobalWorkSize, LocalWorkSize);
}
