#include <string.h>
#include "CUnit/Basic.h"
#include "common.h"

static const char *srcs[3] = {
"__kernel void\n\
copy(__global float *out, __global float *in) {\n\
  size_t index = get_local_id(0);\n\
  out[index] = in[index];\n\
}",

NULL,

"__kernel void\n\
saxpy(__global float *y, __global float *x,\n\
      float a) {\n\
  size_t index = get_global_id(0);\n\
  float temp = y[index];\n\
  y[index] = a * x[index] + temp;\n\
}",
};

void test_program(void) {
  size_t lens[3] = {
    strlen(srcs[0]),
    0,
    strlen(srcs[2]),
  };

  cl_int ret;
  cl_program program;
  CU_ASSERT(NULL == clCreateProgramWithSource(NULL, 3, srcs, lens, &ret));  CU_ASSERT(CL_INVALID_CONTEXT == ret);
  CU_ASSERT(NULL == clCreateProgramWithSource(ctx,  0, srcs, lens, &ret));  CU_ASSERT(CL_INVALID_VALUE   == ret);
  CU_ASSERT(NULL == clCreateProgramWithSource(ctx,  3, NULL, lens, &ret));  CU_ASSERT(CL_INVALID_VALUE   == ret);
  CU_ASSERT(NULL == clCreateProgramWithSource(ctx,  3, srcs, lens, &ret));  CU_ASSERT(CL_INVALID_VALUE   == ret);

  srcs[1] =
"__kernel void\n\
add(__global float *out, __global float *in1, __global float *in2) {\n\
  size_t index = get_global_id(0);\n\
  out[index] = in1[index] + in2[index];\n\
}";

  lens[1] = strlen(srcs[1]);

  CU_ASSERT(NULL != (program = clCreateProgramWithSource(ctx, 3, srcs, lens, &ret)));
  CU_ASSERT(CL_SUCCESS == ret);

  CU_ASSERT(CL_INVALID_PROGRAM == clRetainProgram(NULL));
  CU_ASSERT(CL_SUCCESS         == clRetainProgram(program));
  CU_ASSERT(CL_SUCCESS         == clRetainProgram(program));
  CU_ASSERT(CL_SUCCESS         == clRetainProgram(program));
  CU_ASSERT(CL_SUCCESS         == clRetainProgram(program));

  CU_ASSERT(CL_INVALID_PROGRAM == clReleaseProgram(NULL));
  CU_ASSERT(CL_SUCCESS         == clReleaseProgram(program));
  CU_ASSERT(CL_SUCCESS         == clReleaseProgram(program));
  CU_ASSERT(CL_SUCCESS         == clReleaseProgram(program));
  CU_ASSERT(CL_SUCCESS         == clReleaseProgram(program));
  CU_ASSERT(CL_SUCCESS         == clReleaseProgram(program));
  CU_ASSERT(CL_INVALID_PROGRAM == clReleaseProgram(program));
}

void test_program_build(void) {
  size_t lens[] = {
    strlen(srcs[0]),
  };

  cl_int ret;
  cl_program program;

  CU_ASSERT_FATAL(NULL != (program = clCreateProgramWithSource(ctx, 1, srcs, lens, &ret)));
  CU_ASSERT_FATAL(CL_SUCCESS == ret);

  CU_ASSERT(CL_INVALID_PROGRAM == clBuildProgram(NULL,    1, &dev, NULL, NULL, NULL));
  CU_ASSERT(CL_INVALID_VALUE   == clBuildProgram(program, 0, &dev, NULL, NULL, NULL));
  CU_ASSERT(CL_INVALID_VALUE   == clBuildProgram(program, 1, NULL, NULL, NULL, NULL));
  CU_ASSERT(CL_SUCCESS         == clBuildProgram(program, 1, &dev, NULL, NULL, NULL));

  CU_ASSERT_FATAL(CL_SUCCESS == clReleaseProgram(program));
}
