#include <cstdlib>
#include <unistd.h>
#include <llvm/ExecutionEngine/Orc/LLJIT.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IRReader/IRReader.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/SourceMgr.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Utils.h>
#include "Constant.h"
#include "KernelVectorizer.h"
#include "KernelWrapper.h"
#include "Program.h"
#include "Util.h"

namespace socl {

std::map<Program *, unsigned int> ProgMap;

}

_cl_program::_cl_program()
    : JIT(nullptr), Ctx(nullptr), M(nullptr) {
  std::string NamePrefix("_SOCL_TEMP_FILE_");

  std::string RandomStr = socl::genRandomStr(8);

  int count = 100;
  while (-1 != access((NamePrefix + RandomStr + ".cl").c_str(), F_OK) ||
         -1 != access((NamePrefix + RandomStr + ".ll").c_str(), F_OK)) {
    count--;
    if (0 == count) {
      socl::exitWithInternalError(this, "cannot create a temporary .cl file");
    }

    RandomStr = socl::genRandomStr(8);
  }

  TempFileName = NamePrefix + RandomStr;

  TempFile.open(TempFileName + ".cl");
  if (!TempFile) {
    socl::exitWithInternalError(this, "cannot create a temporary .cl file");
  }

// FIXME how to add this information?
// intrinsic functions have dummy implementations to prevent inlining
  TempFile << "#include<stddef.h>\n\n";
  TempFile << "#define __global __attribute__((address_space(" << socl::Global << ")))\n";
  TempFile << "#define __local __attribute__((address_space(" << socl::Local << ")))\n\n";
  TempFile << "#define __constant __attribute__((address_space(" << socl::Constant << ")))\n\n";
  TempFile << "size_t get_global_id(unsigned int dim) { return 0; } \n";
  TempFile << "size_t get_global_size(unsigned int dim) { return 0; } \n";
  TempFile << "size_t get_local_id(unsigned int dim) { return 0; } \n";
  TempFile << "size_t get_local_size(unsigned int dim) { return 0; } \n\n";
}

_cl_program::~_cl_program() {
  if (TempFile.is_open()) {
    TempFile.close();
  }

// FIXME remove temporary files
}

void
_cl_program::addString(const char *Str, size_t StrLen) {
  if (0 == StrLen) {
    TempFile << Str;
  } else {
    TempFile.write(Str, StrLen);
  }

  TempFile << std::endl;
  TempFile << std::endl;
}

void
_cl_program::build() {
  TempFile.close();

// FIXME parse source file directly from string buffer
// instead of using clang command
  std::string Cmd("clang -mcpu=native+sve -O0 -Xclang -disable-O0-optnone -S -emit-llvm ");
  Cmd += TempFileName + ".cl";

  if (0 != std::system(Cmd.c_str())) {
    socl::exitWithInternalError(this, "parse error");
  }

  llvm::ExitOnError EOE;
// FIXME is this safe?
  JIT = EOE(llvm::orc::LLJITBuilder().create());

  auto CPtr = std::make_unique<llvm::LLVMContext>();

  llvm::SMDiagnostic Err;
  auto MPtr = llvm::parseIRFile(TempFileName + ".ll", Err, *CPtr);
  llvm::Module &Mod = *MPtr;

  auto PreFPM = std::make_unique<llvm::legacy::FunctionPassManager>(&Mod);
  PreFPM->add(llvm::createPromoteMemoryToRegisterPass());
  PreFPM->doInitialization();
  for (auto &F : Mod) {
    PreFPM->run(F);
  }

  KernelVectorizer Vectorizer;
  KernelWrapper Wrapper(&Vectorizer);
  for (auto &F : Mod) {
    Vectorizer.vectorizeKernel(F);
    Wrapper.createWrapperFunction(F);
  }

  for (auto &F : Mod) {
    if (F.getCallingConv() == llvm::CallingConv::SPIR_KERNEL) {
      F.setCallingConv(llvm::CallingConv::C);
    }
  }

  auto PostFPM = std::make_unique<llvm::legacy::FunctionPassManager>(&Mod);
  PostFPM->add(llvm::createDeadCodeEliminationPass());
// FIXME what kind of passes are needed?
//
//  PostFPM->add(llvm::createConstantHoistingPass());
  PostFPM->doInitialization();
  for (auto &F : Mod) {
    PostFPM->run(F);
  }

  auto TSM = llvm::orc::ThreadSafeModule(std::move(MPtr), std::move(CPtr));
  EOE(JIT->addIRModule(std::move(TSM)));

// FIXME duplicated data
  Ctx = std::make_unique<llvm::LLVMContext>();
  M = llvm::parseIRFile(TempFileName + ".ll", Err, *Ctx);
}

bool
_cl_program::hasKernel(const char *KernelName) const {
  if (M->getFunction(KernelName)) {
    return true;
  } else {
    return false;
  }
}

KernelFuncTy
_cl_program::getKernelPtr(const char *KernelName) const {
  llvm::ExitOnError EOE;
  auto FuncSym = EOE(JIT->lookup(KernelName));

  KernelFuncTy FuncPtr = (KernelFuncTy)FuncSym.getAddress();
  return FuncPtr;
}
