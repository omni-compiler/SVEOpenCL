#ifndef __SOCL_PLATFORM_H
#define __SOCL_PLATFORM_H

struct _cl_platform_id {
private:
  _cl_platform_id() = default;
  ~_cl_platform_id() = default;

public:
  _cl_platform_id(const _cl_platform_id &P) = delete;
  _cl_platform_id &operator=(const _cl_platform_id &P) = delete;

  _cl_platform_id(_cl_platform_id &P) = delete;
  _cl_platform_id &operator=(_cl_platform_id &P) = delete;

  static _cl_platform_id *getSingleton() {
    static _cl_platform_id Singleton;
    return &Singleton;
  }
};

namespace socl {

using Platform = struct _cl_platform_id;

}

#endif // __SOCL_PLATFORM_H
