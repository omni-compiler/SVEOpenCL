#ifndef __SOCL_CONSTANT_H
#define __SOCL_CONSTANT_H

namespace socl {

enum ArgKind_ {
  Global = 0,
  Local = 1,
  Constant = 2,
  Value = 15,
};

using ArgKind = enum ArgKind_;

}

#endif // __SOCL_CONSTANT_H
