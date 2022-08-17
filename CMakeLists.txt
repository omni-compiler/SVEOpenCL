cmake_minimum_required(VERSION 3.10)

project(sve_opencl CXX)

include_directories(/home/ec2-user/OpenCL-Headers)
include_directories(/home/ec2-user/argobots/src/include/)
include_directories(/usr/local/include/llvm)

add_subdirectory(src)
