#ifndef __SOCL_MEMORY_OBJECT_H
#define __SOCL_MEMORY_OBJECT_H

#include <map>
#include <string>
#include "Context.h"
#include "RetCode.h"
#include "Util.h"

struct _cl_kernel;

struct _cl_mem {
private:
  const socl::Context *Ctx;
  size_t ByteSize;
  void *Ptr;

  bool Readable;
  bool Writable;

public:
  _cl_mem() = delete;
  _cl_mem(socl::Context *C, size_t S, bool R, bool W);
  ~_cl_mem();

  _cl_mem(const _cl_mem &C) = delete;
  _cl_mem &operator=(const _cl_mem &C) = delete;

  _cl_mem(_cl_mem &C) = delete;
  _cl_mem &operator=(_cl_mem &C) = delete;

  socl::RetCode read(void *HostPtr, size_t Offset, size_t ReadByteSize) const;
  socl::RetCode write(const void *HostPtr, size_t Offset, size_t WriteByteSize);

  std::string getClassName() const {
    return "cl_mem";
  }

  template <typename T1, typename T2>
  friend bool socl::hasSameContext(const T1 *C1, const T2 *C2);

  friend struct _cl_kernel;

// FIXME for debug
  void *getPtr() { return Ptr; }
};

namespace socl {

using MemoryObject = struct _cl_mem;
extern std::map<MemoryObject *, unsigned int> MemObjMap;

}

#endif // __SOCL_MEMORY_OBJECT_H
