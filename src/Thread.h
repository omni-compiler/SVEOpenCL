#ifndef __SOCL_THREAD_IMPL_H
#define __SOCL_THREAD_IMPL_H

#include <abt.h>
#include <vector>
#include "Type.h"

namespace socl {

class ComputeUnit {
private:
  ABT_xstream ES;
  ABT_pool Pool;

  unsigned int ID;

  ComputeUnit() = delete;

public:
  ComputeUnit(unsigned int ID) : ID(ID) {}

  ComputeUnit(const ComputeUnit &T) = delete;
  ComputeUnit &operator=(const ComputeUnit &T) = delete;

  ComputeUnit(ComputeUnit &T) = delete;
  ComputeUnit &operator=(ComputeUnit &T) = delete;

  unsigned int getID() { return ID; }
  ABT_xstream *getES() { return &ES; }
  ABT_pool *getPool() { return &Pool; }
};

class Thread {
private:
  ABT_thread Thd;

  size_t *Args;

  Thread() = delete;

public:
  Thread(ComputeUnit *CU, size_t WGID,
         size_t ThreadID);
  Thread(ComputeUnit *CU, size_t WGID0, size_t WGID1,
         size_t ThreadID);
  Thread(ComputeUnit *CU, size_t WGID0, size_t WGID1, size_t ID2,
         size_t ThreadID);
  ~Thread();

  Thread(const Thread &T) = delete;
  Thread &operator=(const Thread &T) = delete;

  Thread(Thread &T) = delete;
  Thread &operator=(Thread &T) = delete;
};

class ThreadEngine {
private:
  unsigned int NumCUs;
  unsigned int NumNodes;
  unsigned int NumCoresPerNode;
  ComputeUnit **CUs;
  std::vector<Thread *> ThreadGroup;

  ThreadEngine();
  ~ThreadEngine();

public:
  ThreadEngine(const ThreadEngine &T) = delete;
  ThreadEngine &operator=(const ThreadEngine &T) = delete;

  ThreadEngine(ThreadEngine &T) = delete;
  ThreadEngine &operator=(ThreadEngine &T) = delete;

  static ThreadEngine *getSingleton() {
    static ThreadEngine Singleton;
    return &Singleton;
  }

  void setupCommonArgs(KernelFuncTy KernelFuncPtr,
                       void **ArgPtrs,
                       const size_t *GlobalWorkOffset,
                       const size_t *GlobalWorkSize,
                       const size_t *LocalWorkSize);

  void createThreadGroup(size_t NumWorkGroups);
  void createThreadGroup(size_t NumWorkGroups0, size_t NumWorkGroups1);
  void createThreadGroup(size_t NumWorkGroups0, size_t NumWorkGroups1, size_t NumWorkGroups2);

  void finishExecution();
};

}

#endif // __SOCL_THREAD_IMPL_H
