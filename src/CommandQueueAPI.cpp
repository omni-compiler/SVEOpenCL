#include <cstdlib>
#include "Impl.h"
#include "Util.h"

static socl::CommandQueue *
createCommandQueue(socl::Context *Ctx,
                   socl::Device  *Dev,
                   cl_int        *RetCode) {
  if (NULL == Ctx) {
    *RetCode = CL_INVALID_CONTEXT;
    return NULL;
  }

  if (NULL == Dev) {
    *RetCode = CL_INVALID_DEVICE;
    return NULL;
  }

  if (socl::Device::getSingleton() != Dev) {
    *RetCode = CL_INVALID_DEVICE;
    return NULL;
  }

  socl::CommandQueue *CQ = new socl::CommandQueue(Ctx, Dev);

  socl::CQMap[CQ] = 1;

  *RetCode = CL_SUCCESS;
  return CQ;
}                

CL_API_ENTRY CL_API_PREFIX__VERSION_1_2_DEPRECATED socl::CommandQueue * CL_API_CALL
clCreateCommandQueue(socl::Context              *Ctx,
                     socl::Device               *Dev,
                     cl_command_queue_properties Properties,
                     cl_int                     *RetCode)
CL_API_SUFFIX__VERSION_1_2_DEPRECATED {
  if (0 != Properties) {
    *RetCode = CL_INVALID_QUEUE_PROPERTIES;
    return NULL;
  }

  return createCommandQueue(Ctx, Dev, RetCode);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainCommandQueue(socl::CommandQueue *CQ)
CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == CQ) return CL_INVALID_COMMAND_QUEUE;

  if (socl::retain(CQ, socl::CQMap)) {
    return CL_SUCCESS;
  } else {
    return CL_INVALID_COMMAND_QUEUE;
  }
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseCommandQueue(socl::CommandQueue *CQ)
CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == CQ) return CL_INVALID_COMMAND_QUEUE;

  if (socl::release(CQ, socl::CQMap)) {
    return CL_SUCCESS;
  } else {
    return CL_INVALID_COMMAND_QUEUE;
  }
}
