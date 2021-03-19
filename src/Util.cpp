#include <cstdlib>
#include "Util.h"

namespace socl {

std::string
genRandomStr(const int Len) {
  static const char AlphaNum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

  std::string Str;
  for (int i = 0; i < Len; i++) {
    Str += AlphaNum[rand() % (sizeof(AlphaNum) - 1)];
  }

  return Str;
}

}
