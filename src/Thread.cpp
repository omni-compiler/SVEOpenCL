#include <cstdlib>
#include <numa.h>
#include "Thread.h"

extern "C" {
typedef struct {
  KernelFuncTy KernelFuncPtr;
  void **ArgPtrs;
  const size_t *GlobalWorkOffset;
  const size_t *GlobalWorkSize;
  const size_t *LocalWorkSize;
} ThreadArg;

static ThreadArg CommonArgs;
static size_t ThreadSize;

static void _SOCL_thread_wrapper_1D(void *Args) {
  size_t *NewArgs = (size_t *)Args;

  size_t LocalWorkSize = CommonArgs.LocalWorkSize[0];
  size_t ThreadID = NewArgs[1];
  size_t ChunkSize = LocalWorkSize / ThreadSize;
  if (LocalWorkSize % ThreadSize != 0) {
    ChunkSize++;
  }

  // lower bound
  NewArgs[1] = ThreadID * ChunkSize;

  // upper bound + 1
  NewArgs[2] = NewArgs[1] + ChunkSize;
  if (NewArgs[2] > LocalWorkSize) {
    NewArgs[2] = LocalWorkSize;
  }

  CommonArgs.KernelFuncPtr(CommonArgs.ArgPtrs,
                           CommonArgs.GlobalWorkOffset,
                           CommonArgs.GlobalWorkSize,
                           CommonArgs.LocalWorkSize,
                           NewArgs);
}

static void _SOCL_thread_wrapper_2D(void *Args) {
  size_t *NewArgs = (size_t *)Args;

  size_t LocalWorkSize = CommonArgs.LocalWorkSize[1];
  size_t ThreadID = NewArgs[4];
  size_t ChunkSize = LocalWorkSize / ThreadSize;
  if (LocalWorkSize % ThreadSize != 0) {
    ChunkSize++;
  }

  // lower bound
  NewArgs[4] = ThreadID * ChunkSize;

  // upper bound + 1
  NewArgs[5] = NewArgs[4] + ChunkSize;
  if (NewArgs[5] > LocalWorkSize) {
    NewArgs[5] = LocalWorkSize;
  }

  CommonArgs.KernelFuncPtr(CommonArgs.ArgPtrs,
                           CommonArgs.GlobalWorkOffset,
                           CommonArgs.GlobalWorkSize,
                           CommonArgs.LocalWorkSize,
                           NewArgs);
}

static void _SOCL_thread_wrapper_3D(void *Args) {
  size_t *NewArgs = (size_t *)Args;

  size_t LocalWorkSize = CommonArgs.LocalWorkSize[2];
  size_t ThreadID = NewArgs[7];
  size_t ChunkSize = LocalWorkSize / ThreadSize;
  if (LocalWorkSize % ThreadSize != 0) {
    ChunkSize++;
  }

  // lower bound
  NewArgs[7] = ThreadID * ChunkSize;

  // upper bound + 1
  NewArgs[8] = NewArgs[7] + ChunkSize;
  if (NewArgs[8] > LocalWorkSize) {
    NewArgs[8] = LocalWorkSize;
  }

  CommonArgs.KernelFuncPtr(CommonArgs.ArgPtrs,
                           CommonArgs.GlobalWorkOffset,
                           CommonArgs.GlobalWorkSize,
                           CommonArgs.LocalWorkSize,
                           NewArgs);
}
}

namespace socl {

Thread::Thread(ComputeUnit *CU,
               size_t WGID,
	       size_t ThreadID) {
  Args = new size_t[3];
  Args[0] = WGID;
  Args[1] = ThreadID;
  // Args[1] = 0;
  // Args[2] = CommonArgs.LocalWorkSize[0];
  ABT_thread_create(*(CU->getPool()), _SOCL_thread_wrapper_1D, Args,
                    ABT_THREAD_ATTR_NULL, &Thd);
}

Thread::Thread(ComputeUnit *CU,
               size_t WGID0, size_t WGID1,
	       size_t ThreadID) {
  Args = new size_t[6];
  Args[0] = WGID0;
  Args[1] = WGID1;
  Args[2] = 0;
  Args[3] = CommonArgs.LocalWorkSize[0];
  Args[4] = ThreadID;
  // Args[4] = 0;
  // Args[5] = CommonArgs.LocalWorkSize[1];
  ABT_thread_create(*(CU->getPool()), _SOCL_thread_wrapper_2D, Args,
                    ABT_THREAD_ATTR_NULL, &Thd);
}

Thread::Thread(ComputeUnit *CU,
               size_t WGID0, size_t WGID1, size_t WGID2,
	       size_t ThreadID) {
  Args = new size_t[9];
  Args[0] = WGID0;
  Args[1] = WGID1;
  Args[2] = WGID2;
  Args[3] = 0;
  Args[4] = CommonArgs.LocalWorkSize[0];
  Args[5] = 0;
  Args[6] = CommonArgs.LocalWorkSize[1];
  Args[7] = ThreadID;
  // Args[7] = 0;
  // Args[8] = CommonArgs.LocalWorkSize[2];
  ABT_thread_create(*(CU->getPool()), _SOCL_thread_wrapper_3D, Args,
                    ABT_THREAD_ATTR_NULL, &Thd);
}

Thread::~Thread() {
  ABT_thread_free(&Thd);
  delete[] Args;
}

ThreadEngine::ThreadEngine() {
  // FIXME not a general implementation
  // assumes that
  // 1. each numa node has the same number of cores
  // 2. core is distributed in a block manner
  NumCUs = numa_num_configured_cpus();
  NumNodes = numa_max_node() + 1;
  NumCoresPerNode = NumCUs / NumNodes;
  ThreadSize = NumCoresPerNode;

  CUs = new ComputeUnit*[NumCUs];
  for (int i = 0; i < NumCUs; i++) {
    CUs[i] = new ComputeUnit(i);
  }

  ABT_init(0, NULL);
  ABT_xstream_self(CUs[0]->getES());

  for (int i = 1; i < NumCUs; i++) {
    ABT_xstream_create(ABT_SCHED_NULL, CUs[i]->getES());
    ABT_xstream_set_cpubind(*(CUs[i]->getES()), i);
  }

  for (int i = 0; i < NumCUs; i++) {
    ABT_xstream_get_main_pools(*(CUs[i]->getES()), 1, CUs[i]->getPool());
  }

  ThreadGroup.clear();
}

ThreadEngine::~ThreadEngine() {
  for (int i = 1; i < NumCUs; i++) {
    ABT_xstream_join(*(CUs[i]->getES()));
    ABT_xstream_free(CUs[i]->getES());
  }

  ABT_finalize();

  for (int i = 0; i < NumCUs; i++) {
    delete CUs[i];
  }

  delete[] CUs;
}

void
ThreadEngine::setupCommonArgs(KernelFuncTy KernelFuncPtr,
                              void **ArgPtrs,
                              const size_t *GlobalWorkOffset,
                              const size_t *GlobalWorkSize,
                              const size_t *LocalWorkSize) {
  CommonArgs.KernelFuncPtr = KernelFuncPtr;
  CommonArgs.ArgPtrs = ArgPtrs;
  CommonArgs.GlobalWorkOffset = GlobalWorkOffset;
  CommonArgs.GlobalWorkSize = GlobalWorkSize;
  CommonArgs.LocalWorkSize = LocalWorkSize;
}

void
ThreadEngine::createThreadGroup(size_t NumWorkGroups) {
  for (size_t i = 0; i < NumWorkGroups; i++) {
    unsigned int NodeNum = i % NumNodes;
    unsigned int CoreNum = NodeNum * NumCoresPerNode;
    for (unsigned int ThreadID = 0; ThreadID < NumCoresPerNode; ThreadID++, CoreNum++) {
      Thread *T = new Thread(CUs[CoreNum], i, ThreadID);
      ThreadGroup.push_back(T);
    }
  }
}

void
ThreadEngine::createThreadGroup(size_t NumWorkGroups0, size_t NumWorkGroups1) {
  size_t Idx = 0;
  for (size_t j = 0; j < NumWorkGroups1; j++) {
    for (size_t i = 0; i < NumWorkGroups0; i++) {
      unsigned int NodeNum = Idx % NumNodes;
      unsigned int CoreNum = NodeNum * NumCoresPerNode;
      for (unsigned int ThreadID = 0; ThreadID < NumCoresPerNode; ThreadID++, CoreNum++) {
        Thread *T = new Thread(CUs[CoreNum], i, j, ThreadID);
        ThreadGroup.push_back(T);
      }
      Idx++;
    }
  }
}

void
ThreadEngine::createThreadGroup(size_t NumWorkGroups0, size_t NumWorkGroups1, size_t NumWorkGroups2) {
  size_t Idx = 0;
  for (size_t k = 0; k < NumWorkGroups2; k++) {
    for (size_t j = 0; j < NumWorkGroups1; j++) {
      for (size_t i = 0; i < NumWorkGroups0; i++) {
        unsigned int NodeNum = Idx % NumNodes;
        unsigned int CoreNum = NodeNum * NumCoresPerNode;
        for (unsigned int ThreadID = 0; ThreadID < NumCoresPerNode; ThreadID++, CoreNum++) {
          Thread *T = new Thread(CUs[CoreNum], i, j, k, ThreadID);
          ThreadGroup.push_back(T);
        }
        Idx++;
      }
    }
  }
}

void
ThreadEngine::finishExecution() {
  for (Thread *T : ThreadGroup) {
    delete T;
  }

  ThreadGroup.clear();
}

}
