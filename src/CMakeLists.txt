option(ABT_ROOT "Path to Argobots")
include_directories(${ABT_ROOT}/include)
link_directories(${ABT_ROOT}/lib)

find_package(LLVM REQUIRED CONFIG)
include_directories(${LLVM_INCLUDE_DIRS})

option(OCL_HEADER_ROOT "Path to OpenCL-Headers on GitHub")

set(CMAKE_CXX_FLAGS "-O2 -fno-rtti")

add_library(socl STATIC
  BufferObjectAPI.cpp
  CommandQueueAPI.cpp
  KernelAPI.cpp
  PlatformLayerAPI.cpp
  ProgramAPI.cpp

  CommandQueue.cpp
  Context.cpp
  Device.cpp
  Kernel.cpp
  KernelVectorizer.cpp
  KernelWrapper.cpp
  MemoryObject.cpp
  Program.cpp
  Thread.cpp
  Util.cpp
)


target_include_directories(socl PUBLIC
  ${OCL_HEADER_ROOT}
)

target_compile_definitions(socl PRIVATE CL_TARGET_OPENCL_VERSION=120)

target_link_libraries(socl abt)

install(TARGETS socl DESTINATION lib)
install(DIRECTORY ${OCL_HEADER_ROOT}/CL DESTINATION include)
