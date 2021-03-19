#include <cstdlib>
#include <cstring>
#include "MemoryObject.h"

namespace socl {

std::map<MemoryObject *, unsigned int> MemObjMap;

}

_cl_mem::_cl_mem(socl::Context *C, size_t S, bool R, bool W)
    : Ctx(C), ByteSize(S), Readable(R), Writable(W) {
  Ptr = std::malloc(S);
/* FIXME how to catch this error: CL_MEM_OBJECT_ALLOCATION_FAILURE
  if (NULL == Ptr) {
    return NULL;
  }
*/
}

_cl_mem::~_cl_mem() {
  std::free(Ptr);
}

socl::RetCode
_cl_mem::read(void *HostPtr, size_t Offset, size_t ReadByteSize) const {
  if (!Readable) return socl::InvalidOperation;
  if (NULL == HostPtr) return socl::InvalidParameter;
  if (0 == ReadByteSize) return socl::InvalidParameter;
  if (Offset + ReadByteSize > ByteSize) return socl::OutOfBound;

  std::memcpy(HostPtr, (char *)Ptr + Offset, ReadByteSize);

  return socl::Success;
}

socl::RetCode
_cl_mem::write(const void *HostPtr, size_t Offset, size_t WriteByteSize) {
  if (!Writable) return socl::InvalidOperation;
  if (NULL == HostPtr) return socl::InvalidParameter;
  if (0 == WriteByteSize) return socl::InvalidParameter;
  if (Offset + WriteByteSize > ByteSize) return socl::OutOfBound;

  std::memcpy((char *)Ptr + Offset, HostPtr, WriteByteSize);

  return socl::Success;
}
