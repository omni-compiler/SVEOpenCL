#include <cstdlib>
#include "Impl.h"
#include "Util.h"

extern "C" {

void *_SOCL_DEBUG_getPtr(socl::MemoryObject *Mem) {
  return Mem->getPtr();
}

}

// FIXME more condition check needed
static bool
checkFlags(cl_mem_flags &F, bool &R, bool &W) {
  R = false;
  W = false;

  if (0 == F) {
    F = CL_MEM_READ_WRITE;
  }

  if (F & CL_MEM_READ_WRITE ) {
    R = W = true;

    if (F & CL_MEM_READ_ONLY) return false;
    if (F & CL_MEM_WRITE_ONLY) return false;
  } else if (F & CL_MEM_READ_ONLY) {
    R = true;

    if (F & CL_MEM_WRITE_ONLY) return false;
  } else if (F & CL_MEM_WRITE_ONLY) {
    W = true;

    // if (F & CL_MEM_READ_ONLY) return false;
  } else {
    return false;
  }

  return true;
}

static socl::MemoryObject *
createMem(socl::Context *Ctx, cl_mem_flags Flags,
          const size_t Size, cl_int *RetCode) {
  if (0 == Size) {
    *RetCode = CL_INVALID_BUFFER_SIZE;
    return NULL;
  }

  bool R = false;
  bool W = false;
  if (!checkFlags(Flags, R, W)) {
    *RetCode = CL_INVALID_VALUE;
    return NULL;
  }

  socl::MemoryObject *Mem = new socl::MemoryObject(Ctx, Size, R, W);

  socl::MemObjMap[Mem] = 1;

  *RetCode = CL_SUCCESS;
  return Mem;
}

CL_API_ENTRY socl::MemoryObject * CL_API_CALL
clCreateBuffer(socl::Context *Ctx,
               cl_mem_flags   Flags,
               size_t         Size,
               void          *HostPtr,
               cl_int        *RetCode)
CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == Ctx) {
    *RetCode = CL_INVALID_CONTEXT;
    return NULL;
  }

  // FIXME implement error check, host ptr

  return createMem(Ctx, Flags, Size, RetCode);
}

static cl_int
checkReadWriteError(socl::CommandQueue *CQ,
                    socl::MemoryObject *Buffer) {
  if (NULL == CQ) return CL_INVALID_COMMAND_QUEUE;
  if (NULL == Buffer) return CL_INVALID_MEM_OBJECT;
  if (!socl::hasSameContext(CQ, Buffer)) return CL_INVALID_CONTEXT;

  return CL_SUCCESS;
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueReadBuffer(socl::CommandQueue *CQ,
                    socl::MemoryObject *Buffer,
                    cl_bool             BlockingRead,
                    size_t              Offset,
                    size_t              ReadByteSize,
                    void               *HostPtr,
                    cl_uint             NumEventsInWaitList,
                    const cl_event     *EventWaitList,
                    cl_event           *Event)
CL_API_SUFFIX__VERSION_1_0 {
  // FIXME implement event
  if (0 != NumEventsInWaitList) return CL_INVALID_EVENT;
  if (NULL != EventWaitList) return CL_INVALID_EVENT;
  if (NULL != Event) return CL_INVALID_EVENT;
// FIXME implement asynchronous read
//  if (!blocking_read) return CL_INVALID_VALUE;

  cl_int RetCode = checkReadWriteError(CQ, Buffer);
  if (RetCode != CL_SUCCESS) return RetCode;

  switch (Buffer->read(HostPtr, Offset, ReadByteSize)) {
    case socl::Success:          return CL_SUCCESS;
    case socl::InvalidOperation: return CL_INVALID_OPERATION;
    case socl::InvalidParameter: return CL_INVALID_VALUE;
    case socl::OutOfBound:       return CL_INVALID_VALUE;
    default:
      socl::exitWithInternalError(Buffer, "read failed in clEnqueueReadBuffer()");
      return CL_INVALID_VALUE;
  }
}

CL_API_ENTRY cl_int CL_API_CALL
clEnqueueWriteBuffer(socl::CommandQueue *CQ,
                     socl::MemoryObject *Buffer,
                     cl_bool             BlockingWrite,
                     size_t              Offset,
                     size_t              WriteByteSize,
                     const void         *HostPtr,
                     cl_uint             NumEventsInWaitList,
                     const cl_event     *EventWaitList,
                     cl_event           *Event)
CL_API_SUFFIX__VERSION_1_0 {
  // FIXME implement event
  if (0 != NumEventsInWaitList) return CL_INVALID_EVENT;
  if (NULL != EventWaitList) return CL_INVALID_EVENT;
  if (NULL != Event) return CL_INVALID_EVENT;
// FIXME implement asynchronous write
//  if (!blocking_write) return CL_INVALID_VALUE;

  cl_int RetCode = checkReadWriteError(CQ, Buffer);
  if (RetCode != CL_SUCCESS) return RetCode;

  switch (Buffer->write(HostPtr, Offset, WriteByteSize)) {
    case socl::Success:          return CL_SUCCESS;
    case socl::InvalidOperation: return CL_INVALID_OPERATION;
    case socl::InvalidParameter: return CL_INVALID_VALUE;
    case socl::OutOfBound:       return CL_INVALID_VALUE;
    default:
      socl::exitWithInternalError(Buffer, "write failed in clEnqueueWriteBuffer()");
      return CL_INVALID_VALUE;
  }
}

CL_API_ENTRY cl_int CL_API_CALL
clRetainMemObject(socl::MemoryObject *Mem)
CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == Mem) return CL_INVALID_MEM_OBJECT;

  if (socl::retain(Mem, socl::MemObjMap)) {
    return CL_SUCCESS;
  } else {
    return CL_INVALID_MEM_OBJECT;
  }
}

CL_API_ENTRY cl_int CL_API_CALL
clReleaseMemObject(socl::MemoryObject *Mem)
CL_API_SUFFIX__VERSION_1_0 {
  if (NULL == Mem) return CL_INVALID_MEM_OBJECT;

  if (socl::release(Mem, socl::MemObjMap)) {
    return CL_SUCCESS;
  } else {
    return CL_INVALID_MEM_OBJECT;
  }
}
