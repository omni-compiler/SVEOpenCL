#include "CUnit/Basic.h"
#include "common.h"

void test_platform(void) {
  cl_platform_id platforms[8];
  cl_uint num_platforms;

  CU_ASSERT(CL_INVALID_VALUE == clGetPlatformIDs(0, &(platforms[0]), &num_platforms));
  CU_ASSERT(CL_INVALID_VALUE == clGetPlatformIDs(8, NULL,            NULL));
  CU_ASSERT(CL_INVALID_VALUE == clGetPlatformIDs(0, NULL,            NULL));
  CU_ASSERT(CL_SUCCESS       == clGetPlatformIDs(8, &(platforms[0]), &num_platforms));
  CU_ASSERT(1 == num_platforms);
}

void test_device(void) {
  cl_device_id devices[8];
  cl_uint num_devices;

  CU_ASSERT(CL_INVALID_PLATFORM == clGetDeviceIDs(NULL, CL_DEVICE_TYPE_CPU, 8, &(devices[0]), &num_devices));
  CU_ASSERT(CL_DEVICE_NOT_FOUND == clGetDeviceIDs(plt,  CL_DEVICE_TYPE_GPU, 8, &(devices[0]), &num_devices));
  CU_ASSERT(CL_INVALID_VALUE    == clGetDeviceIDs(plt,  CL_DEVICE_TYPE_CPU, 0, &(devices[0]), &num_devices));
  CU_ASSERT(CL_INVALID_VALUE    == clGetDeviceIDs(plt,  CL_DEVICE_TYPE_CPU, 8, NULL,          NULL));
  CU_ASSERT(CL_INVALID_VALUE    == clGetDeviceIDs(plt,  CL_DEVICE_TYPE_CPU, 0, NULL,          NULL));
  CU_ASSERT(CL_SUCCESS          == clGetDeviceIDs(plt,  CL_DEVICE_TYPE_CPU, 8, &(devices[0]), &num_devices));
  CU_ASSERT(1 == num_devices);
}

void test_context(void) {
  cl_context context;
  cl_int ret;

  CU_ASSERT(NULL == clCreateContext(NULL, 0, &dev, NULL, NULL, &ret));  CU_ASSERT(CL_INVALID_VALUE == ret);
  CU_ASSERT(NULL == clCreateContext(NULL, 1, NULL, NULL, NULL, &ret));  CU_ASSERT(CL_INVALID_VALUE == ret);
  CU_ASSERT(NULL != (context =
                    clCreateContext(NULL, 1, &dev, NULL, NULL, &ret))); CU_ASSERT(CL_SUCCESS       == ret);

  CU_ASSERT(CL_INVALID_CONTEXT == clRetainContext(NULL));
  CU_ASSERT(CL_SUCCESS         == clRetainContext(context));
  CU_ASSERT(CL_SUCCESS         == clRetainContext(context));
  CU_ASSERT(CL_SUCCESS         == clRetainContext(context));
  CU_ASSERT(CL_SUCCESS         == clRetainContext(context));

  CU_ASSERT(CL_INVALID_CONTEXT == clReleaseContext(NULL));
  CU_ASSERT(CL_SUCCESS         == clReleaseContext(context));
  CU_ASSERT(CL_SUCCESS         == clReleaseContext(context));
  CU_ASSERT(CL_SUCCESS         == clReleaseContext(context));
  CU_ASSERT(CL_SUCCESS         == clReleaseContext(context));
  CU_ASSERT(CL_SUCCESS         == clReleaseContext(context));
  CU_ASSERT(CL_INVALID_CONTEXT == clReleaseContext(context));
}
