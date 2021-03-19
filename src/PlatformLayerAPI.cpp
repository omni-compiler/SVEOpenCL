#include <cstdlib>
#include "Impl.h"
#include "Util.h"

CL_API_ENTRY cl_int CL_API_CALL
clGetPlatformIDs(cl_uint          NumEntries,
                 socl::Platform **Platforms,
                 cl_uint         *NumPlatforms)
CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == Platforms && NULL == NumPlatforms) return CL_INVALID_VALUE;

  if (NULL != Platforms) {
    if (0 == NumEntries) return CL_INVALID_VALUE;

    Platforms[0] = socl::Platform::getSingleton();
  }

  if (NULL != NumPlatforms) *NumPlatforms = 1;

  return CL_SUCCESS;
}

static cl_int
checkDeviceType(cl_device_type DeviceType) {
  // FIXME inplement CL_INVALID_DEVICE_TYPE
  if (!((DeviceType & CL_DEVICE_TYPE_CPU) || 
        (DeviceType & CL_DEVICE_TYPE_DEFAULT))) return CL_DEVICE_NOT_FOUND;

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clGetDeviceIDs(socl::Platform *Platform,
               cl_device_type  DeviceType,
               cl_uint         NumEntries,
               socl::Device  **Devices,
               cl_uint        *NumDevices)
CL_API_SUFFIX__VERSION_1_0 {
  if (socl::Platform::getSingleton() != Platform) return CL_INVALID_PLATFORM;
  if (NULL == Devices && NULL == NumDevices) return CL_INVALID_VALUE;

  cl_int DeviceCheck = checkDeviceType(DeviceType);
  if (CL_SUCCESS != DeviceCheck) return DeviceCheck;

  if (NULL != Devices) {
    if (0 == NumEntries) return CL_INVALID_VALUE;

    Devices[0] = socl::Device::getSingleton();
  }

  if (NULL != NumDevices) *NumDevices = 1;

  return CL_SUCCESS;
}

static cl_context
createContext(cl_int *RetCode) {
  cl_context Ctx = new socl::Context();

  socl::ContextMap[Ctx] = 1;

  *RetCode = CL_SUCCESS;
  return Ctx;  
}

// FIXME implement readContextProperties(), other error check
CL_API_ENTRY cl_context CL_API_CALL
clCreateContext(const cl_context_properties *Properties,
                cl_uint                      NumDevices,
                socl::Device * const        *Devices,
                void (CL_CALLBACK *PfnNotify)(const char *ErrInfo,
                                              const void *PrivateInfo,
                                              size_t      Cb,
                                              void       *UserData),
                void   *UserData,
                cl_int *RetCode)
CL_API_SUFFIX__VERSION_1_0 {
  if (0 == NumDevices) {
    *RetCode = CL_INVALID_VALUE;
    return NULL;
  }

  if (NULL == Devices) {
    *RetCode = CL_INVALID_VALUE;
    return NULL;
  }

  // readContextProperties(properties);

  return createContext(RetCode);
}

// FIXME implement readContextProperties(), other error check
CL_API_ENTRY cl_context CL_API_CALL
clCreateContextFromType(const cl_context_properties *Properties,
                        cl_device_type               DeviceType,
                        void (CL_CALLBACK *PfnNotify)(const char *ErrInfo,
                                                      const void *PrivateInfo,
                                                      size_t      Cb,
                                                      void       *UserData),
                        void   *UserData,
                        cl_int *RetCode)
CL_API_SUFFIX__VERSION_1_0 {
  // readContextProperties(properties);

  cl_int DeviceCheck = checkDeviceType(DeviceType);
  if (CL_SUCCESS != DeviceCheck) {
    *RetCode = DeviceCheck;
    return NULL;
  }

  return createContext(RetCode);
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainContext(socl::Context *Ctx)
CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == Ctx) return CL_INVALID_CONTEXT;

  if (socl::retain(Ctx, socl::ContextMap)) {
    return CL_SUCCESS;
  } else {
    return CL_INVALID_CONTEXT;
  }
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseContext(socl::Context *Ctx)
CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == Ctx) return CL_INVALID_CONTEXT;

  if (socl::release(Ctx, socl::ContextMap)) {
    return CL_SUCCESS;
  } else {
    return CL_INVALID_CONTEXT;
  }
}
