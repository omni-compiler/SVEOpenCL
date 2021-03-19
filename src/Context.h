#ifndef __SOCL_CONTEXT_H
#define __SOCL_CONTEXT_H

#include <map>

struct _cl_context {
private:

public:
  _cl_context() = default;
  ~_cl_context() = default;

  _cl_context(const _cl_context &C) = delete;
  _cl_context &operator=(const _cl_context &C) = delete;

  _cl_context(_cl_context &C) = delete;
  _cl_context &operator=(_cl_context &C) = delete;
};

namespace socl {

using Context = struct _cl_context;
extern std::map<Context *, unsigned int> ContextMap;

}

#endif // __SOCL_CONTEXT_H
