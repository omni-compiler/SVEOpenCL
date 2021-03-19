#ifndef __SOCL_RET_CODE_H
#define __SOCL_RET_CODE_H

namespace socl {

enum RetCode_ : unsigned {
  Success,
  InvalidMemoryObject,
  InvalidOperation,
  InvalidParameter,
  InvalidSize,
  OutOfBound,
  OutOfMemory,
  Unexpected,
  Unreachable,
};

using RetCode = enum RetCode_;

}

#endif // __SOCL_RET_CODE_H
