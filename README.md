OpenCL Implementation for ARM SVE

I. How to build
1. clone OpenCL headers from GitHub
git clone https://github.com/KhronosGroup/OpenCL-Headers.git

2. clone this repository
git clone https://github.com/JinpilLee/SVEOpenCL.git

3. make a build directory
mkdir build
cd build

("OpenCL-Headers", "SVEOpenCL", "build" are placed in the same level)

4. cmake
cmake -DOCL_HEADER_ROOT=<OpenCL-Headers root> -DCMAKE_INSTALL_PREFIX=<dir to install> ../SVEOpenCL

II. How to use

