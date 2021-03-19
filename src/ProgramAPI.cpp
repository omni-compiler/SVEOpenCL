#include <cstdio>
#include <cstdlib>
#include "Impl.h"
#include "Util.h"

CL_API_ENTRY socl::Program * CL_API_CALL
clCreateProgramWithSource(socl::Context *Ctx,
                          cl_uint        Count,
                          const char   **Strings,
                          const size_t  *Lengths,
                          cl_int        *RetCode)
  CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == Ctx) {
    *RetCode = CL_INVALID_CONTEXT;
    return NULL;
  }

  if (0 == Count) {
    *RetCode = CL_INVALID_VALUE;
    return NULL;
  }

  if (NULL == Strings) {
    *RetCode = CL_INVALID_VALUE;
    return NULL;
  }

  for (int i = 0; i < Count; i++) {
    if (NULL == Strings[i]) {
      *RetCode = CL_INVALID_VALUE;
      return NULL;
    }
  }

  socl::Program *Prog = new socl::Program();
  for (int i = 0; i < Count; i++) {
    if (NULL == Lengths) {
      Prog->addString(Strings[i]);
    } else {
      Prog->addString(Strings[i], Lengths[i]);
    }
  }

  socl::ProgMap[Prog] = 1;

  *RetCode = CL_SUCCESS;
  return Prog;
}

CL_API_ENTRY socl::Program * CL_API_CALL
clCreateProgramWithBinary(socl::Context        *Ctx,
                          cl_uint               NumDevices,
                          const socl::Device   *Devices,
                          const size_t         *Lengths,
                          const unsigned char **Binaries,
                          cl_int               *BinaryStatus,
                          cl_int               *RetCode)
  CL_API_SUFFIX__VERSION_1_0 {
// FIXME implement this
  return NULL;
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainProgram(socl::Program *Prog)
  CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == Prog) return CL_INVALID_PROGRAM;

  if (socl::retain(Prog, socl::ProgMap)) {
    return CL_SUCCESS;
  } else {
    return CL_INVALID_PROGRAM;
  }
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseProgram(socl::Program *Prog)
  CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == Prog) return CL_INVALID_PROGRAM;

  if (socl::release(Prog, socl::ProgMap)) {
    return CL_SUCCESS;
  } else {
    return CL_INVALID_PROGRAM;
  }
}

CL_API_ENTRY cl_int CL_API_CALL
clBuildProgram(socl::Program        *Prog,
               cl_uint               NumDevices,
               socl::Device * const *Devices,
               const char           *Options,
               void (CL_CALLBACK    *PfnNotify)(socl::Program *Prog,
                                                void          *UserData),
                                                void          *UserData)
  CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == Prog) return CL_INVALID_PROGRAM;

  bool SupportedDeviceFound = false;
  if (NULL == Devices) {
    if (0 != NumDevices) return CL_INVALID_VALUE;

    SupportedDeviceFound = true;
  } else {
    if (0 == NumDevices) return CL_INVALID_VALUE;

    for (int i = 0; i < NumDevices; i++) {
      if (NULL == Devices[i]) return CL_INVALID_DEVICE;

      if (socl::Device::getSingleton() == Devices[i]) {
        SupportedDeviceFound = true;
      }
    }
  }

  if (SupportedDeviceFound) {
    Prog->build();
    return CL_SUCCESS;
  } else {
    return CL_INVALID_DEVICE;
  }
}
