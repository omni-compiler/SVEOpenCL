#include <memory>
#include <llvm/Support/InitLLVM.h>
#include <llvm/Support/TargetSelect.h>
#include "Device.h"
#include "Thread.h"
#include "Util.h"

_cl_device_id::_cl_device_id() {
  Argc = 1;
  Argv = new char*[1];
  Argv[0] = new char[8];

  llvm::InitLLVM *LLVMRuntime = new llvm::InitLLVM(Argc, Argv);
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();

  TE = socl::ThreadEngine::getSingleton();
}

_cl_device_id::~_cl_device_id() {
  delete[] Argv[0];
  delete[] Argv;
  delete LLVMRuntime;
}

_cl_device_id *
_cl_device_id::getSingleton() {
  static _cl_device_id Singleton;
  return &Singleton;
}

// FIXME how to implement GlobalWorkOffset?
socl::RetCode
_cl_device_id::runKernel(KernelFuncTy KernelFuncPtr,
                         void **ArgPtrs,
                         unsigned int WorkDim,
                         const size_t *GlobalWorkOffset,
                         const size_t *GlobalWorkSize,
                         const size_t *LocalWorkSize) const {
  if (WorkDim > 3) return socl::OutOfBound;

  TE->setupCommonArgs(KernelFuncPtr, ArgPtrs,
                      GlobalWorkOffset, GlobalWorkSize, LocalWorkSize);

  std::unique_ptr<size_t[]> NumWorkGroups(new size_t[WorkDim]);
  for (int i = 0; i < WorkDim; i++) {
    NumWorkGroups[i] = GlobalWorkSize[i] / LocalWorkSize[i];
  }

  switch (WorkDim) {
    case 1:
      TE->createThreadGroup(NumWorkGroups[0]);
      break;
    case 2:
      TE->createThreadGroup(NumWorkGroups[0], NumWorkGroups[1]);
      break;
    case 3:
      TE->createThreadGroup(NumWorkGroups[0], NumWorkGroups[1], NumWorkGroups[2]);
      break;
    default:
      return socl::OutOfBound;
  }

  TE->finishExecution();

  return socl::Success;
}
