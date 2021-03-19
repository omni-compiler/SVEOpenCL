#ifndef __SOCL_PROGRAM_H
#define __SOCL_PROGRAM_H

#include <map>
#include <memory>
#include <string>
#include <fstream>
#include "Type.h"

namespace llvm {
  class LLVMContext;
  class Module;

  namespace orc {
    class LLJIT;
  }
}

struct _cl_kernel;

struct _cl_program {
private:
  std::ofstream TempFile;
  std::string TempFileName;

  std::unique_ptr<llvm::orc::LLJIT> JIT;

  std::unique_ptr<llvm::LLVMContext> Ctx;
  std::unique_ptr<llvm::Module> M;

  KernelFuncTy getKernelPtr(const char *KernelName) const;

public:
  _cl_program();
  ~_cl_program();

  _cl_program(const _cl_program &C) = delete;
  _cl_program &operator=(const _cl_program &C) = delete;

  _cl_program(_cl_program &C) = delete;
  _cl_program &operator=(_cl_program &C) = delete;

  void addString(const char *Str, size_t StrLen = 0);

  void build();

  bool hasKernel(const char *KernelName) const;

  std::string getClassName() const {
    return "cl_program";
  }

  friend struct _cl_kernel;
};

namespace socl {

using Program = struct _cl_program;
extern std::map<Program *, unsigned int> ProgMap;

}

#endif // __SOCL_PROGRAM_H
