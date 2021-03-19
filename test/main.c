#include <string.h>
#include "CUnit/Basic.h"
#include "common.h"

cl_platform_id plt;
cl_device_id dev;
cl_context ctx;
cl_command_queue cq;
cl_mem in1, in2, inout;
cl_program prog;
cl_kernel ker_copy, ker_add, ker_saxpy;

test_struct_t test_func_list[] = {
// platform_layer.c
  {"platform API test", test_platform},
  {"device API test", test_device},
  {"context API test", test_context},
// command_queue.c
  {"command queue API test", test_command_queue},
// buffer_object.c
  {"buffer object API test", test_buffer_object},
  {"buffer read API test", test_buffer_read},
  {"read from write only buffer", test_buffer_read_mem_flags},
  {"read value check", test_buffer_read_check},
  {"buffer write API test", test_buffer_write},
  {"write from read only buffer", test_buffer_write_mem_flags},
  {"write value check", test_buffer_write_check},
// program.c
  {"program API test", test_program},
  {"program build test", test_program_build},
// kernel.c
  {"kernel API test", test_kernel},
  {"set kernel arguments", test_kernel_set_args},
  {"enqueue kernel", test_kernel_enqueue},
  {"enqueue result check (1D)", test_kernel_enqueue_check_dim1},
  {"enqueue result check (2D)", test_kernel_enqueue_check_dim2},
  {"enqueue result check (3D)", test_kernel_enqueue_check_dim3},
};

int init_socl_test_suite(void) {
  if (CL_SUCCESS != clGetPlatformIDs(1, &plt, NULL)) return -1;
  if (CL_SUCCESS != clGetDeviceIDs(plt, CL_DEVICE_TYPE_CPU, 1, &dev, NULL)) return -1;

  cl_int ret;
  ctx = clCreateContext(NULL, 1, &dev, NULL, NULL, &ret);
  if (NULL == ctx || CL_SUCCESS != ret) return -1;

  cq = clCreateCommandQueue(ctx, dev, 0, &ret);
  if (NULL == cq || CL_SUCCESS != ret) return -1;

  size_t bs = sizeof(int) * NUM_ELMTS;
  in1 = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, bs, NULL, &ret);
  if (NULL == in1 || CL_SUCCESS != ret) return -1;

  in2 = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, bs, NULL, &ret);
  if (NULL == in2 || CL_SUCCESS != ret) return -1;

  inout = clCreateBuffer(ctx, CL_MEM_READ_WRITE, bs, NULL, &ret);
  if (NULL == inout || CL_SUCCESS != ret) return -1;

  const char *srcs[4] = {
"__kernel void\n\
copy(__global int *out, __global int *in) {\n\
  size_t i0 = get_global_id(0);\n\
  size_t i1 = get_global_id(1);\n\
  size_t i2 = get_global_id(2);\n\
  size_t s0 = get_global_size(0);\n\
  size_t s1 = get_global_size(1);\n\
  size_t idx = i2 * s1 * s0 + i1 * s0 + i0;\n\
  out[idx] = in[idx];\n\
}",

"__kernel void\n\
add(__global int *out, __global int *in1, __global int *in2) {\n\
  size_t i0 = get_global_id(0);\n\
  size_t i1 = get_global_id(1);\n\
  size_t i2 = get_global_id(2);\n\
  size_t s0 = get_global_size(0);\n\
  size_t s1 = get_global_size(1);\n\
  size_t idx = i2 * s1 * s0 + i1 * s0 + i0;\n\
  out[idx] = in1[idx] + in2[idx];\n\
}",

"__kernel void\n\
saxpy(__global int *y, __global int *x,\n\
      int a) {\n\
  size_t i0 = get_global_id(0);\n\
  size_t i1 = get_global_id(1);\n\
  size_t i2 = get_global_id(2);\n\
  size_t s0 = get_global_size(0);\n\
  size_t s1 = get_global_size(1);\n\
  size_t idx = i2 * s1 * s0 + i1 * s0 + i0;\n\
  int temp = y[idx];\n\
  y[idx] = a * x[idx] + temp;\n\
}",
"__kernel void\n\
sgemm(__global float *c, __global float *a, __global float *b\n\
      float alpha, float beta, int n) {\n\
  size_t i0 = get_global_id(0);\n\
  size_t i1 = get_global_id(1);\n\
  size_t s0 = get_global_size(0);\n\
  size_t temp = 0;\n\
  a = a + (i1 * s0);\n\
  b = b + i0;\n\
  for (int i = 0; i < n; i++) {\n\
    temp += (alpha * (*a) * (*b));\n\
    a++;\n\
    b += s0;\n\
  }\n\
  size_t idx = i1 * s0 + i0;\n\
  c[idx] = beta * c[idx] + temp;\n\
}",
  };

  size_t lens[4] = {
    strlen(srcs[0]),
    strlen(srcs[1]),
    strlen(srcs[2]),
    strlen(srcs[3]),
  };

  prog = clCreateProgramWithSource(ctx, 3, srcs, lens, &ret);
  if ((NULL == prog) || (CL_SUCCESS != ret)) return -1;

  if (CL_SUCCESS != clBuildProgram(prog, 1, &dev, NULL, NULL, NULL)) return -1;

  ker_copy = clCreateKernel(prog, "copy", &ret);
  if (NULL == ker_copy || CL_SUCCESS != ret) return -1;

  ker_add = clCreateKernel(prog, "add", &ret);
  if (NULL == ker_add || CL_SUCCESS != ret) return -1;

  ker_saxpy = clCreateKernel(prog, "saxpy", &ret);
  if (NULL == ker_saxpy || CL_SUCCESS != ret) return -1;

  return 0;
}

int clean_socl_test_suite(void) {
  if (CL_SUCCESS != clReleaseProgram(prog)) return -1;
  if (CL_SUCCESS != clReleaseCommandQueue(cq)) return -1;
  if (CL_SUCCESS != clReleaseContext(ctx)) return -1;

  return 0;
}

int main(void) {
  if (CUE_SUCCESS != CU_initialize_registry()) {
    return CU_get_error();
  }

  CU_pSuite pSuite = CU_add_suite("SOCL_test_suite",
                                  init_socl_test_suite,
                                  clean_socl_test_suite);
  if (NULL == pSuite) {
    CU_cleanup_registry();
    return CU_get_error();
  }

  size_t n = sizeof(test_func_list) / sizeof(test_struct_t);
  for (size_t i = 0; i < n; i++) {
    if (NULL == CU_add_test(pSuite, test_func_list[i].name,
                            test_func_list[i].func)) {
      CU_cleanup_registry();
      return CU_get_error();
    }
  }

  CU_basic_set_mode(CU_BRM_VERBOSE);
  CU_basic_run_tests();
  CU_cleanup_registry();
  return CU_get_error();
}
