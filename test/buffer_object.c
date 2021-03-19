#include "CUnit/Basic.h"
#include "common.h"
#include "stdlib.h"
#include "time.h"

extern void *_SOCL_DEBUG_getPtr(cl_mem Mem);

int hbuf[NUM_ELMTS];
int vbuf[NUM_ELMTS];

void test_buffer_object(void) {
  size_t bsize = sizeof(int) * NUM_ELMTS;
  cl_int ret;
  cl_mem mem;
  CU_ASSERT(NULL == clCreateBuffer(NULL, 0,   bsize, NULL, &ret));  CU_ASSERT(CL_INVALID_CONTEXT     == ret);
  CU_ASSERT(NULL == clCreateBuffer(ctx,  CL_MEM_READ_WRITE | CL_MEM_READ_ONLY,
                                              bsize, NULL, &ret));  CU_ASSERT(CL_INVALID_VALUE       == ret);
  CU_ASSERT(NULL == clCreateBuffer(ctx,  CL_MEM_READ_WRITE | CL_MEM_WRITE_ONLY,
                                              bsize, NULL, &ret));  CU_ASSERT(CL_INVALID_VALUE       == ret);
  CU_ASSERT(NULL == clCreateBuffer(ctx,  CL_MEM_READ_ONLY  | CL_MEM_WRITE_ONLY,
                                              bsize, NULL, &ret));  CU_ASSERT(CL_INVALID_VALUE       == ret);
  CU_ASSERT(NULL == clCreateBuffer(ctx,  CL_MEM_READ_WRITE | CL_MEM_READ_ONLY,
                                              bsize, NULL, &ret));  CU_ASSERT(CL_INVALID_VALUE       == ret);
  CU_ASSERT(NULL == clCreateBuffer(ctx,  0,   0,     NULL, &ret));  CU_ASSERT(CL_INVALID_BUFFER_SIZE == ret);
  CU_ASSERT(NULL != (mem =
                    clCreateBuffer(ctx,  0,
                                              bsize, NULL, &ret))); CU_ASSERT(CL_SUCCESS             == ret);

  CU_ASSERT(CL_INVALID_MEM_OBJECT == clRetainMemObject(NULL));
  CU_ASSERT(CL_SUCCESS            == clRetainMemObject(mem));
  CU_ASSERT(CL_SUCCESS            == clRetainMemObject(mem));
  CU_ASSERT(CL_SUCCESS            == clRetainMemObject(mem));
  CU_ASSERT(CL_SUCCESS            == clRetainMemObject(mem));

  CU_ASSERT(CL_INVALID_MEM_OBJECT == clReleaseMemObject(NULL));
  CU_ASSERT(CL_SUCCESS            == clReleaseMemObject(mem));
  CU_ASSERT(CL_SUCCESS            == clReleaseMemObject(mem));
  CU_ASSERT(CL_SUCCESS            == clReleaseMemObject(mem));
  CU_ASSERT(CL_SUCCESS            == clReleaseMemObject(mem));
  CU_ASSERT(CL_SUCCESS            == clReleaseMemObject(mem));
  CU_ASSERT(CL_INVALID_MEM_OBJECT == clReleaseMemObject(mem));
}

void test_buffer_read(void) {
  size_t bs = sizeof(int) * NUM_ELMTS;
  cl_int ret;
  cl_mem mem = clCreateBuffer(ctx, CL_MEM_READ_WRITE, bs, NULL, &ret);
  CU_ASSERT_FATAL(NULL != mem);
  CU_ASSERT_FATAL(CL_SUCCESS == ret);

  CU_ASSERT(CL_INVALID_COMMAND_QUEUE == clEnqueueReadBuffer(NULL, mem,  1, 0, bs,     &(hbuf[0]), 0, NULL, NULL));
  CU_ASSERT(CL_INVALID_MEM_OBJECT    == clEnqueueReadBuffer(cq,   NULL, 1, 0, bs,     &(hbuf[0]), 0, NULL, NULL));
  CU_ASSERT(CL_INVALID_VALUE         == clEnqueueReadBuffer(cq,   mem,  1, 9, bs,     &(hbuf[0]), 0, NULL, NULL));
  CU_ASSERT(CL_INVALID_VALUE         == clEnqueueReadBuffer(cq,   mem,  1, 0, bs * 2, &(hbuf[0]), 0, NULL, NULL));
  CU_ASSERT(CL_INVALID_VALUE         == clEnqueueReadBuffer(cq,   mem,  1, 0, bs,     NULL,       0, NULL, NULL));

  CU_ASSERT_FATAL(CL_SUCCESS == clReleaseMemObject(mem));
}

void test_buffer_read_mem_flags(void) {
  size_t bs = sizeof(int) * NUM_ELMTS;
  cl_int ret;
  cl_mem mem = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, bs, NULL, &ret);
  CU_ASSERT_FATAL(NULL != mem);
  CU_ASSERT_FATAL(CL_SUCCESS == ret);

  CU_ASSERT(CL_INVALID_OPERATION == clEnqueueReadBuffer(cq, mem, 1, 0, bs, &(hbuf[0]), 0, NULL, NULL));

  CU_ASSERT_FATAL(CL_SUCCESS == clReleaseMemObject(mem));
}

void test_buffer_read_check(void) {
  size_t bs = sizeof(int) * NUM_ELMTS;
  cl_int ret;
  cl_mem mem = clCreateBuffer(ctx, CL_MEM_READ_WRITE, bs, NULL, &ret);
  CU_ASSERT_FATAL(NULL != mem);
  CU_ASSERT_FATAL(CL_SUCCESS == ret);

  srand((unsigned)time(NULL));
  int *dptr = (int *)_SOCL_DEBUG_getPtr(mem);
  for (size_t i = 0; i < NUM_ELMTS; i++) {
    hbuf[i] = -1;
    vbuf[i] = dptr[i] = rand() % 10000;
  }

  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueReadBuffer(cq, mem, 1, 0, bs, &(hbuf[0]), 0, NULL, NULL));
 
  int err = 0; 
  for (size_t i = 0; i < NUM_ELMTS; i++) {
    if (vbuf[i] != hbuf[i]) err++;
  }

  CU_ASSERT(0 == err);

  CU_ASSERT_FATAL(CL_SUCCESS == clReleaseMemObject(mem));
}

void test_buffer_write(void) {
  size_t bs = sizeof(int) * NUM_ELMTS;
  cl_int ret;
  cl_mem mem = clCreateBuffer(ctx, CL_MEM_READ_WRITE, bs, NULL, &ret);
  CU_ASSERT_FATAL(NULL != mem);
  CU_ASSERT_FATAL(CL_SUCCESS == ret);

  CU_ASSERT(CL_INVALID_COMMAND_QUEUE == clEnqueueWriteBuffer(NULL, mem,  1, 0, bs,     &(hbuf[0]), 0, NULL, NULL));
  CU_ASSERT(CL_INVALID_MEM_OBJECT    == clEnqueueWriteBuffer(cq,   NULL, 1, 0, bs,     &(hbuf[0]), 0, NULL, NULL));
  CU_ASSERT(CL_INVALID_VALUE         == clEnqueueWriteBuffer(cq,   mem,  1, 9, bs,     &(hbuf[0]), 0, NULL, NULL));
  CU_ASSERT(CL_INVALID_VALUE         == clEnqueueWriteBuffer(cq,   mem,  1, 0, bs * 2, &(hbuf[0]), 0, NULL, NULL));
  CU_ASSERT(CL_INVALID_VALUE         == clEnqueueWriteBuffer(cq,   mem,  1, 0, bs,     NULL,       0, NULL, NULL));

  CU_ASSERT_FATAL(CL_SUCCESS == clReleaseMemObject(mem));
}

void test_buffer_write_mem_flags(void) {
  size_t bs = sizeof(int) * NUM_ELMTS;
  cl_int ret;
  cl_mem mem = clCreateBuffer(ctx, CL_MEM_READ_ONLY, bs, NULL, &ret);
  CU_ASSERT_FATAL(NULL != mem);
  CU_ASSERT_FATAL(CL_SUCCESS == ret);

  CU_ASSERT(CL_INVALID_OPERATION == clEnqueueWriteBuffer(cq, mem, 1, 0, bs, &(hbuf[0]), 0, NULL, NULL));

  CU_ASSERT_FATAL(CL_SUCCESS == clReleaseMemObject(mem));
}

void test_buffer_write_check(void) {
  size_t bs = sizeof(int) * NUM_ELMTS;
  cl_int ret;
  cl_mem mem = clCreateBuffer(ctx, CL_MEM_READ_WRITE, bs, NULL, &ret);
  CU_ASSERT_FATAL(NULL != mem);
  CU_ASSERT_FATAL(CL_SUCCESS == ret);

  srand((unsigned)time(NULL));
  int *dptr = (int *)_SOCL_DEBUG_getPtr(mem);
  for (size_t i = 0; i < NUM_ELMTS; i++) {
    vbuf[i] = hbuf[i] = rand() % 10000;
    dptr[i] = -1;
  }

  CU_ASSERT_FATAL(CL_SUCCESS == clEnqueueWriteBuffer(cq, mem, 1, 0, bs, &(hbuf[0]), 0, NULL, NULL));

  int err = 0; 
  for (size_t i = 0; i < NUM_ELMTS; i++) {
    if (vbuf[i] != dptr[i]) err++;
  }

  CU_ASSERT(0 == err);

  CU_ASSERT_FATAL(CL_SUCCESS == clReleaseMemObject(mem));
}
