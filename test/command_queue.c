#include "CUnit/Basic.h"
#include "common.h"

void test_command_queue(void) {
  cl_int ret;
  cl_command_queue command_queue;
  CU_ASSERT(NULL == clCreateCommandQueue(NULL, dev,  0, &ret));  CU_ASSERT(CL_INVALID_CONTEXT == ret);
  CU_ASSERT(NULL == clCreateCommandQueue(ctx,  NULL, 0, &ret));  CU_ASSERT(CL_INVALID_DEVICE  == ret);
  CU_ASSERT(NULL != (command_queue =
                    clCreateCommandQueue(ctx,  dev,  0, &ret))); CU_ASSERT(CL_SUCCESS         == ret);

  CU_ASSERT(CL_INVALID_COMMAND_QUEUE == clRetainCommandQueue(NULL));
  CU_ASSERT(CL_SUCCESS               == clRetainCommandQueue(command_queue));
  CU_ASSERT(CL_SUCCESS               == clRetainCommandQueue(command_queue));
  CU_ASSERT(CL_SUCCESS               == clRetainCommandQueue(command_queue));
  CU_ASSERT(CL_SUCCESS               == clRetainCommandQueue(command_queue));

  CU_ASSERT(CL_INVALID_COMMAND_QUEUE == clReleaseCommandQueue(NULL));
  CU_ASSERT(CL_SUCCESS               == clReleaseCommandQueue(command_queue));
  CU_ASSERT(CL_SUCCESS               == clReleaseCommandQueue(command_queue));
  CU_ASSERT(CL_SUCCESS               == clReleaseCommandQueue(command_queue));
  CU_ASSERT(CL_SUCCESS               == clReleaseCommandQueue(command_queue));
  CU_ASSERT(CL_SUCCESS               == clReleaseCommandQueue(command_queue));
  CU_ASSERT(CL_INVALID_COMMAND_QUEUE == clReleaseCommandQueue(command_queue));
}
