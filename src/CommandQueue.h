#ifndef __SOCL_COMMAND_QUEUE_H
#define __SOCL_COMMAND_QUEUE_H

#include <map>
#include "Context.h"
#include "Device.h"
#include "RetCode.h"
#include "Type.h"
#include "Util.h"

struct _cl_command_queue {
private:
  const socl::Context *Ctx;
  const socl::Device *Dev;

public:
  _cl_command_queue() = delete;
  _cl_command_queue(socl::Context *C, socl::Device *D)
    : Ctx(C), Dev(D) {}

  ~_cl_command_queue() = default;

  _cl_command_queue(const _cl_command_queue &C) = delete;
  _cl_command_queue &operator=(const _cl_command_queue &C) = delete;

  _cl_command_queue(_cl_command_queue &C) = delete;
  _cl_command_queue &operator=(_cl_command_queue &C) = delete;

  socl::RetCode enqueueKernel(KernelFuncTy KernelFuncPtr,
                              void **ArgPtrs,
                              unsigned int WorkDim,
                              const size_t *GlobalWorkOffset,
                              const size_t *GlobalWorkSize,
                              const size_t *LocalWorkSize);

  template <typename T1, typename T2>
  friend bool socl::hasSameContext(const T1 *C1, const T2 *C2);
};

namespace socl {

using CommandQueue = struct _cl_command_queue;
extern std::map<CommandQueue *, unsigned int> CQMap;

}

#endif // __SOCL_COMMAND_QUEUE_H
