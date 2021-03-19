#ifndef __SOCL_TEST_SUITE
#define __SOCL_TEST_SUITE

#include <CL/cl.h>

#define NUM_ELMTS (4096*4*48)

extern cl_platform_id plt;
extern cl_device_id dev;
extern cl_context ctx;
extern cl_command_queue cq;
extern cl_mem in1, in2, inout;
extern cl_program prog;
extern cl_kernel ker_copy, ker_add, ker_saxpy;

typedef struct _test_struct_t {
  char name[32];
  void(* func)(void);
} test_struct_t;

// platform_layer.c
extern void test_platform(void);
extern void test_device(void);
extern void test_context(void);

// command_queue.c
extern void test_command_queue(void);

// buffer_object.c
extern void test_buffer_object(void);
extern void test_buffer_read(void);
extern void test_buffer_read_mem_flags(void);
extern void test_buffer_read_check(void);
extern void test_buffer_write(void);
extern void test_buffer_write_mem_flags(void);
extern void test_buffer_write_check(void);

// program.c
extern void test_program(void);
extern void test_program_build(void);

// kernel.c
extern void test_kernel(void);
extern void test_kernel_set_args(void);
extern void test_kernel_enqueue(void);
extern void test_kernel_enqueue_check_dim1(void);
extern void test_kernel_enqueue_check_dim2(void);
extern void test_kernel_enqueue_check_dim3(void);

#endif // __SOCL_TEST_SUITE
