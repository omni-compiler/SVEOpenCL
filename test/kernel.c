#include <stdlib.h>
#include "CUnit/Basic.h"
#include "common.h"

void test_kernel(void) {
  cl_int ret;
  cl_kernel kernel;
  CU_ASSERT(NULL == clCreateKernel(NULL, "copy", &ret)); CU_ASSERT(CL_INVALID_PROGRAM     == ret);
  CU_ASSERT(NULL == clCreateKernel(prog, NULL,   &ret)); CU_ASSERT(CL_INVALID_VALUE       == ret);
  CU_ASSERT(NULL == clCreateKernel(prog, "null", &ret)); CU_ASSERT(CL_INVALID_KERNEL_NAME == ret);

  CU_ASSERT(NULL != (kernel = clCreateKernel(prog, "copy", &ret)));
  CU_ASSERT(CL_SUCCESS == ret);

  CU_ASSERT(CL_INVALID_KERNEL == clRetainKernel(NULL));
  CU_ASSERT(CL_SUCCESS        == clRetainKernel(kernel));
  CU_ASSERT(CL_SUCCESS        == clRetainKernel(kernel));
  CU_ASSERT(CL_SUCCESS        == clRetainKernel(kernel));
  CU_ASSERT(CL_SUCCESS        == clRetainKernel(kernel));
  
  CU_ASSERT(CL_INVALID_KERNEL == clReleaseKernel(NULL));
  CU_ASSERT(CL_SUCCESS        == clReleaseKernel(kernel));
  CU_ASSERT(CL_SUCCESS        == clReleaseKernel(kernel));
  CU_ASSERT(CL_SUCCESS        == clReleaseKernel(kernel));
  CU_ASSERT(CL_SUCCESS        == clReleaseKernel(kernel));
  CU_ASSERT(CL_SUCCESS        == clReleaseKernel(kernel));
  CU_ASSERT(CL_INVALID_KERNEL == clReleaseKernel(kernel));
}

void test_kernel_set_args(void) {
  cl_int ret;
  cl_kernel kernel;
  CU_ASSERT_FATAL(NULL != (kernel = clCreateKernel(prog, "saxpy", &ret)));
  CU_ASSERT_FATAL(CL_SUCCESS == ret);

  size_t bs = sizeof(int) * NUM_ELMTS;
  int *buffer = (int *)malloc(bs);
  for (int i = 0; i < NUM_ELMTS; i++) {
    buffer[i] = i;
  }

  CU_ASSERT(CL_INVALID_KERNEL     == clSetKernelArg(NULL,   0, sizeof(cl_mem), &inout));
  CU_ASSERT(CL_INVALID_ARG_INDEX  == clSetKernelArg(kernel, 3, sizeof(cl_mem), &inout));
  CU_ASSERT(CL_INVALID_ARG_SIZE   == clSetKernelArg(kernel, 0, sizeof(char),   &inout));
  CU_ASSERT(CL_INVALID_MEM_OBJECT == clSetKernelArg(kernel, 0, bs,             buffer));
  CU_ASSERT(CL_INVALID_MEM_OBJECT == clSetKernelArg(kernel, 0, sizeof(cl_mem), NULL));
  CU_ASSERT(CL_INVALID_MEM_OBJECT == clSetKernelArg(kernel, 0, sizeof(cl_mem), buffer));
  CU_ASSERT(CL_SUCCESS            == clSetKernelArg(kernel, 0, sizeof(cl_mem), &inout));

  CU_ASSERT(CL_INVALID_KERNEL    == clSetKernelArg(NULL,   2, sizeof(int), &(buffer[10])));
  CU_ASSERT(CL_INVALID_ARG_SIZE  == clSetKernelArg(kernel, 2, bs,          buffer));
  CU_ASSERT(CL_INVALID_ARG_VALUE == clSetKernelArg(kernel, 2, sizeof(int), NULL));
  CU_ASSERT(CL_INVALID_ARG_INDEX == clSetKernelArg(kernel, 3, sizeof(int), &(buffer[10])));
  CU_ASSERT(CL_SUCCESS           == clSetKernelArg(kernel, 2, sizeof(int), &(buffer[10])));

  free(buffer);
}

// FIXME not complete test
void test_kernel_enqueue(void) {
  size_t bs = sizeof(int) * NUM_ELMTS;
  int *cpu_in1 = (int *)malloc(bs);
  int *cpu_in2 = (int *)malloc(bs);
  int *cpu_inout = (int *)malloc(bs);

  for (size_t i = 0; i < NUM_ELMTS; i++) {
    cpu_in1[i] = i;
    cpu_inout[i] = 0;
  }

  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueWriteBuffer(cq, in1, 1, 0, bs, &(cpu_in1[0]), 0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueWriteBuffer(cq, inout, 1, 0, bs, &(cpu_inout[0]), 0, NULL, NULL));

  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_copy, 0, sizeof(cl_mem), &inout));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_copy, 1, sizeof(cl_mem), &in1));

  size_t goffs[4] = { 0, 0, 0, 0 };
  size_t gsizes[4] = { NUM_ELMTS,     1, 1, 1 };
  size_t lsizes[4] = { NUM_ELMTS / 4, 1, 1, 1 };
  CU_ASSERT(CL_INVALID_COMMAND_QUEUE    == clEnqueueNDRangeKernel(NULL, ker_copy, 1, NULL,  gsizes, lsizes, 0, NULL, NULL));
  CU_ASSERT(CL_INVALID_KERNEL           == clEnqueueNDRangeKernel(cq,   NULL,     1, NULL,  gsizes, lsizes, 0, NULL, NULL));
  CU_ASSERT(CL_INVALID_WORK_DIMENSION   == clEnqueueNDRangeKernel(cq,   ker_copy, 4, NULL,  gsizes, lsizes, 0, NULL, NULL));
  CU_ASSERT(CL_INVALID_GLOBAL_WORK_SIZE == clEnqueueNDRangeKernel(cq,   ker_copy, 1, NULL,  NULL,   lsizes, 0, NULL, NULL));
//  CU_ASSERT(CL_INVALID_WORK_SIZE        == clEnqueueNDRangeKernel(cq,   ker_copy, 1, NULL, gsizes, NULL,   0, NULL, NULL));
  CU_ASSERT(CL_SUCCESS                  == clEnqueueNDRangeKernel(cq,   ker_copy, 1, NULL,  gsizes, lsizes, 0, NULL, NULL));
  CU_ASSERT(CL_SUCCESS                  == clEnqueueNDRangeKernel(cq,   ker_copy, 1, goffs, gsizes, lsizes, 0, NULL, NULL));

  free(cpu_in1);
  free(cpu_in2);
  free(cpu_inout);
}

void test_kernel_enqueue_check_dim1(void) {
  size_t bs = sizeof(int) * NUM_ELMTS;
  int *cpu_in1 = (int *)malloc(bs);
  int *cpu_in2 = (int *)malloc(bs);
  int *cpu_inout = (int *)malloc(bs);

  for (size_t i = 0; i < NUM_ELMTS; i++) {
    cpu_in1[i] = i;
    cpu_in2[i] = i + 12345;
  }

  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueWriteBuffer(cq, in1,   1, 0, bs, &(cpu_in1[0]),   0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueWriteBuffer(cq, in2,   1, 0, bs, &(cpu_in2[0]),   0, NULL, NULL));

  size_t goffs[1] = { 0 };
  size_t gsizes[1] = { NUM_ELMTS };
  size_t lsizes[1] = { NUM_ELMTS / 4 };

  for (size_t i = 0; i < NUM_ELMTS; i++) cpu_inout[i] = 0;
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueWriteBuffer(cq, inout, 1, 0, bs, &(cpu_inout[0]), 0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_copy, 0, sizeof(cl_mem), &inout));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_copy, 1, sizeof(cl_mem), &in1));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueNDRangeKernel(cq, ker_copy, 1, NULL, gsizes, lsizes, 0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueReadBuffer(cq, inout, 1, 0, bs, &(cpu_inout[0]), 0, NULL, NULL));
  size_t err = 0;
  for (size_t i = 0; i < NUM_ELMTS; i++) {
    if (cpu_inout[i] != i) err++;
  }
  CU_ASSERT(0 == err);

  for (size_t i = 0; i < NUM_ELMTS; i++) cpu_inout[i] = 0;
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueWriteBuffer(cq, inout, 1, 0, bs, &(cpu_inout[0]), 0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_add, 0, sizeof(cl_mem), &inout));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_add, 1, sizeof(cl_mem), &in1));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_add, 2, sizeof(cl_mem), &in2));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueNDRangeKernel(cq, ker_add, 1, NULL, gsizes, lsizes, 0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueReadBuffer(cq, inout, 1, 0, bs, &(cpu_inout[0]), 0, NULL, NULL));
  err = 0;
  for (size_t i = 0; i < NUM_ELMTS; i++) {
    if (cpu_inout[i] != (i + i + 12345)) err++;
  }
  CU_ASSERT(0 == err);

  int a = 3;
  for (size_t i = 0; i < NUM_ELMTS; i++) cpu_inout[i] = 1001;
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueWriteBuffer(cq, inout, 1, 0, bs, &(cpu_inout[0]), 0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_saxpy, 0, sizeof(cl_mem), &inout));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_saxpy, 1, sizeof(cl_mem), &in1));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_saxpy, 2, sizeof(int),    &a));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueNDRangeKernel(cq, ker_saxpy, 1, NULL, gsizes, lsizes, 0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueReadBuffer(cq, inout, 1, 0, bs, &(cpu_inout[0]), 0, NULL, NULL));
  err = 0;
  for (size_t i = 0; i < NUM_ELMTS; i++) {
    if (cpu_inout[i] != (1001 + 3 * i)) err++;
  }
  CU_ASSERT(0 == err);

  free(cpu_in1);
  free(cpu_in2);
  free(cpu_inout);
}

void test_kernel_enqueue_check_dim2(void) {
  size_t bs = sizeof(int) * NUM_ELMTS;
  int *cpu_in1 = (int *)malloc(bs);
  int *cpu_in2 = (int *)malloc(bs);
  int *cpu_inout = (int *)malloc(bs);

  for (size_t i = 0; i < NUM_ELMTS; i++) {
    cpu_in1[i] = i;
    cpu_in2[i] = i + 12345;
  }

  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueWriteBuffer(cq, in1,   1, 0, bs, &(cpu_in1[0]),   0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueWriteBuffer(cq, in2,   1, 0, bs, &(cpu_in2[0]),   0, NULL, NULL));

  size_t goffs[2] = { 0, 0 };
  size_t gsizes[2] = { NUM_ELMTS / (4 * 48), (4 * 48) };
  size_t lsizes[2] = { NUM_ELMTS / (4 * 48), (    48) };

  for (size_t i = 0; i < NUM_ELMTS; i++) cpu_inout[i] = 0;
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueWriteBuffer(cq, inout, 1, 0, bs, &(cpu_inout[0]), 0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_copy, 0, sizeof(cl_mem), &inout));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_copy, 1, sizeof(cl_mem), &in1));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueNDRangeKernel(cq, ker_copy, 2, NULL, gsizes, lsizes, 0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueReadBuffer(cq, inout, 1, 0, bs, &(cpu_inout[0]), 0, NULL, NULL));
  size_t err = 0;
  for (size_t i = 0; i < NUM_ELMTS; i++) {
    if (cpu_inout[i] != i) err++;
  }
  CU_ASSERT(0 == err);

  for (size_t i = 0; i < NUM_ELMTS; i++) cpu_inout[i] = 0;
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueWriteBuffer(cq, inout, 1, 0, bs, &(cpu_inout[0]), 0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_add, 0, sizeof(cl_mem), &inout));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_add, 1, sizeof(cl_mem), &in1));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_add, 2, sizeof(cl_mem), &in2));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueNDRangeKernel(cq, ker_add, 2, NULL, gsizes, lsizes, 0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueReadBuffer(cq, inout, 1, 0, bs, &(cpu_inout[0]), 0, NULL, NULL));
  err = 0;
  for (size_t i = 0; i < NUM_ELMTS; i++) {
    if (cpu_inout[i] != (i + i + 12345)) err++;
  }
  CU_ASSERT(0 == err);

  int a = 3;
  for (size_t i = 0; i < NUM_ELMTS; i++) cpu_inout[i] = 1001;
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueWriteBuffer(cq, inout, 1, 0, bs, &(cpu_inout[0]), 0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_saxpy, 0, sizeof(cl_mem), &inout));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_saxpy, 1, sizeof(cl_mem), &in1));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_saxpy, 2, sizeof(int),    &a));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueNDRangeKernel(cq, ker_saxpy, 2, NULL, gsizes, lsizes, 0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueReadBuffer(cq, inout, 1, 0, bs, &(cpu_inout[0]), 0, NULL, NULL));
  err = 0;
  for (size_t i = 0; i < NUM_ELMTS; i++) {
    if (cpu_inout[i] != (1001 + 3 * i)) err++;
  }
  CU_ASSERT(0 == err);

  free(cpu_in1);
  free(cpu_in2);
  free(cpu_inout);
}

void test_kernel_enqueue_check_dim3(void) {
  size_t bs = sizeof(int) * NUM_ELMTS;
  int *cpu_in1 = (int *)malloc(bs);
  int *cpu_in2 = (int *)malloc(bs);
  int *cpu_inout = (int *)malloc(bs);

  for (size_t i = 0; i < NUM_ELMTS; i++) {
    cpu_in1[i] = i;
    cpu_in2[i] = i + 12345;
  }

  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueWriteBuffer(cq, in1,   1, 0, bs, &(cpu_in1[0]),   0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueWriteBuffer(cq, in2,   1, 0, bs, &(cpu_in2[0]),   0, NULL, NULL));

  size_t goffs[3] = { 0, 0, 0 };
  size_t gsizes[3] = { NUM_ELMTS / (32 * 4 * 48), 32, (4 * 48) };
  size_t lsizes[3] = { NUM_ELMTS / (32 * 4 * 48), 32, (    48) };

  for (size_t i = 0; i < NUM_ELMTS; i++) cpu_inout[i] = 0;
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueWriteBuffer(cq, inout, 1, 0, bs, &(cpu_inout[0]), 0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_copy, 0, sizeof(cl_mem), &inout));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_copy, 1, sizeof(cl_mem), &in1));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueNDRangeKernel(cq, ker_copy, 3, NULL, gsizes, lsizes, 0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueReadBuffer(cq, inout, 1, 0, bs, &(cpu_inout[0]), 0, NULL, NULL));
  size_t err = 0;
  for (size_t i = 0; i < NUM_ELMTS; i++) {
    if (cpu_inout[i] != i) err++;
  }
  CU_ASSERT(0 == err);

  for (size_t i = 0; i < NUM_ELMTS; i++) cpu_inout[i] = 0;
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueWriteBuffer(cq, inout, 1, 0, bs, &(cpu_inout[0]), 0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_add, 0, sizeof(cl_mem), &inout));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_add, 1, sizeof(cl_mem), &in1));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_add, 2, sizeof(cl_mem), &in2));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueNDRangeKernel(cq, ker_add, 3, NULL, gsizes, lsizes, 0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueReadBuffer(cq, inout, 1, 0, bs, &(cpu_inout[0]), 0, NULL, NULL));
  err = 0;
  for (size_t i = 0; i < NUM_ELMTS; i++) {
    if (cpu_inout[i] != (i + i + 12345)) err++;
  }
  CU_ASSERT(0 == err);

  int a = 3;
  for (size_t i = 0; i < NUM_ELMTS; i++) cpu_inout[i] = 1001;
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueWriteBuffer(cq, inout, 1, 0, bs, &(cpu_inout[0]), 0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_saxpy, 0, sizeof(cl_mem), &inout));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_saxpy, 1, sizeof(cl_mem), &in1));
  CU_ASSERT_FATAL(CL_SUCCESS == clSetKernelArg(ker_saxpy, 2, sizeof(int),    &a));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueNDRangeKernel(cq, ker_saxpy, 3, NULL, gsizes, lsizes, 0, NULL, NULL));
  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueReadBuffer(cq, inout, 1, 0, bs, &(cpu_inout[0]), 0, NULL, NULL));
  err = 0;
  for (size_t i = 0; i < NUM_ELMTS; i++) {
    if (cpu_inout[i] != (1001 + 3 * i)) err++;
  }
  CU_ASSERT(0 == err);

  free(cpu_in1);
  free(cpu_in2);
  free(cpu_inout);
}
