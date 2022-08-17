/*
 * Display Device Information
 *
 * Script to print out some information about the OpenCL devices
 * and platforms available on your system
 *
 * History: C++ version written by Tom Deakin, 2012
 *          Ported to C by Tom Deakin, July 2013
 *          Updated by Tom Deakin, October 2014
*/

#include <stdio.h>
#include <stdlib.h>

#include <CL/cl.h>

int main(void)
{
    cl_int err;
    // Find the number of OpenCL platforms
    cl_uint num_platforms;
    err = clGetPlatformIDs(0, NULL, &num_platforms);
    //skipping error checking for now...
    if (num_platforms == 0)
    {
        printf("Found 0 platforms!\n");
        return EXIT_FAILURE;
    }
    // Create a list of platform IDs
    cl_platform_id platform[num_platforms];
    err = clGetPlatformIDs(num_platforms, platform, NULL);
    

    printf("\nNumber of OpenCL platforms: %d\n", num_platforms);
    printf("\n-------------------------\n");


    cl_uint num_devices;

    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, 1, NULL, &num_devices);
    if (num_devices == 0)
      {
	printf("Found 0 devices!\n");
	return EXIT_FAILURE;
      }
    printf("\nNumber of OpenCL Devices: %d\n", num_devices);
    printf("\n------------------------------\n");
    
    cl_device_id *devices = (cl_device_id *) malloc (sizeof(cl_device_id) * num_devices);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_ALL, num_devices, devices, NULL);
    
    cl_int ret;
    cl_uint ctx = clCreateContext(NULL, 1, &devices, NULL, NULL, &ret);
    if (NULL == ctx || CL_SUCCESS != ret) {
      printf("Sorry, error!\n");
      return -1;
    }
    else {
      printf("Successfully created context\n");
    }
    /*
    cq = clCreateCommandQueue(ctx, dev, 0, &ret);
    if (NULL == cq || CL_SUCCESS != ret) return -1;
    
  size_t bs = sizeof(int) * NUM_ELMTS;
  uint in1 = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, bs, NULL, &ret);
  if (NULL == in1 || CL_SUCCESS != ret) return -1;

  uint in2 = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, bs, NULL, &ret);
  if (NULL == in2 || CL_SUCCESS != ret) return -1;

  uint inout = clCreateBuffer(ctx, CL_MEM_READ_WRITE, bs, NULL, &ret);
  if (NULL == inout || CL_SUCCESS != ret) return -1;

  const char *srcs[4] = {
    "__kernel void\n\
copy(__global int *out, __global int *in) {\n	\
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

  uint prog = clCreateProgramWithSource(ctx, 3, srcs, lens, &ret);
  if ((NULL == prog) || (CL_SUCCESS != ret)) return -1;

  if (CL_SUCCESS != clBuildProgram(prog, 1, &dev, NULL, NULL, NULL)) return -1;

  uint ker_copy = clCreateKernel(prog, "copy", &ret);
  if (NULL == ker_copy || CL_SUCCESS != ret) return -1;

  uint ker_add = clCreateKernel(prog, "add", &ret);
  if (NULL == ker_add || CL_SUCCESS != ret) return -1;

  uint ker_saxpy = clCreateKernel(prog, "saxpy", &ret);
  if (NULL == ker_saxpy || CL_SUCCESS != ret) return -1;
    */

  return 0;
}
    
 


