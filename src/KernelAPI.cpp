#include <cstdlib>
#include "Impl.h"
#include "Util.h"

CL_API_ENTRY socl::Kernel * CL_API_CALL
clCreateKernel(socl::Program *Prog,
               const char    *KernelName,
               cl_int        *RetCode)
CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == Prog) {
    *RetCode = CL_INVALID_PROGRAM;
    return NULL;
  }

  if (NULL == KernelName) {
    *RetCode = CL_INVALID_VALUE;
    return NULL;
  }

  if (!Prog->hasKernel(KernelName)) {
    *RetCode = CL_INVALID_KERNEL_NAME;
    return NULL;
  }

  socl::Kernel *Ker = new socl::Kernel(Prog, KernelName);

  socl::KernelMap[Ker] = 1;

  *RetCode = CL_SUCCESS;
  return Ker;
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainKernel(socl::Kernel *Ker)
CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == Ker) return CL_INVALID_KERNEL;

  if (socl::retain(Ker, socl::KernelMap)) {
    return CL_SUCCESS;
  } else {
    return CL_INVALID_KERNEL;
  }
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseKernel(socl::Kernel *Ker)
CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == Ker) return CL_INVALID_KERNEL;

  if (socl::release(Ker, socl::KernelMap)) {
    return CL_SUCCESS;
  } else {
    return CL_INVALID_KERNEL;
  }
}

CL_API_ENTRY cl_int CL_API_CALL
clSetKernelArg(socl::Kernel *Ker,
               cl_uint       ArgIndex,
               size_t        ArgSize,
               const void   *ArgValue)
CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == Ker) return CL_INVALID_KERNEL;

  switch (Ker->setArg(ArgIndex, ArgSize, ArgValue)) {
    case socl::Success:             return CL_SUCCESS;
    case socl::InvalidMemoryObject: return CL_INVALID_MEM_OBJECT;
    case socl::InvalidParameter:    return CL_INVALID_ARG_VALUE;
    case socl::InvalidSize:         return CL_INVALID_ARG_SIZE;
    case socl::OutOfBound:          return CL_INVALID_ARG_INDEX;
    case socl::OutOfMemory:         return CL_INVALID_ARG_SIZE;
    default:
      socl::exitWithInternalError(Ker, "setArg() failed");
      return CL_INVALID_VALUE;
  }
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueNDRangeKernel(socl::CommandQueue *CQ,
                       socl::Kernel       *Ker,
                       cl_uint             WorkDim,
                       const size_t       *GlobalWorkOffsets,
                       const size_t       *GlobalWorkSizes,
                       const size_t       *LocalWorkSizes,
                       cl_uint             NumEventsInWaitList,
                       const cl_event     *EventWaitList,
                       cl_event           *Event)
CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == CQ) return CL_INVALID_COMMAND_QUEUE;
  if (NULL == Ker) return CL_INVALID_KERNEL;
  if (NULL == GlobalWorkSizes) return CL_INVALID_GLOBAL_WORK_SIZE;

  if (NULL != LocalWorkSizes) {
    for (int i = 0; i < WorkDim; i++) {
      if (0 != GlobalWorkSizes[i] % LocalWorkSizes[i]) return CL_INVALID_WORK_GROUP_SIZE;
    }
  }

  size_t *Offsets = NULL;
  if (NULL == GlobalWorkOffsets) {
    Offsets = new size_t[3];
    Offsets[0] = 0;
    Offsets[1] = 0;
    Offsets[2] = 0;
    GlobalWorkOffsets = Offsets;
  }

  cl_int RetCode = CL_SUCCESS;
  switch (Ker->enqueueKernel(CQ, WorkDim, GlobalWorkOffsets, GlobalWorkSizes, LocalWorkSizes)) {
    case socl::Success:    RetCode = CL_SUCCESS; break;
    case socl::OutOfBound: RetCode = CL_INVALID_WORK_DIMENSION; break;
    default:
      // FIXME implement errors;
      RetCode = CL_INVALID_VALUE;
  }

  if (NULL != Offsets) delete[] Offsets;

  return RetCode;
}

CL_API_ENTRY cl_int CL_API_CALL
clFinish(socl::CommandQueue *CQ)
CL_API_SUFFIX__VERSION_1_0 {
  return CL_SUCCESS;
}
