#ifndef _SOCL_UTIL_H
#define _SOCL_UTIL_H

#include <iostream>
#include <map>
#include <string>

namespace socl {

extern std::string genRandomStr(const int Len);

template <typename T>
void exitWithInternalError(const T *C, const char *Msg) {
  std::cerr << "[SOCL]" << C->getClassName() + ": " + Msg << std::endl;
  exit(1);
}

template <typename T1, typename T2>
bool hasSameContext(const T1 *C1, const T2 *C2) {
  if (C1->Ctx == C2->Ctx) {
    return true;
  } else {
    return false;
  }
}

template <typename T>
bool retain(const T &Ptr, std::map<T, unsigned int> &Map) {
  auto Iter = Map.find(Ptr);
  if (Iter == Map.end()) {
    return false;
  } else {
    (Iter->second)++;
  }

  return true;
}

template <typename T>
bool release(const T &Ptr, std::map<T, unsigned int> &Map) {
  auto Iter = Map.find(Ptr);
  if (Iter == Map.end()) {
    return false;
  } else {
    if (Iter->second == 1) {
      delete Ptr;
      Map.erase(Iter);
    } else {
      (Iter->second)--;
    }

    return true;
  }
}

}

#endif // _SOCL_UTIL_H
