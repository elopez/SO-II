// processtable.h

#ifndef PROCESSTABLE_H
#define PROCESSTABLE_H

#include "thread.h"
#include "synch.h"

#define PROCESS_QTY 100

class ProcessTable {
 public:
    ProcessTable();
    ~ProcessTable();

    int AllocateId(Thread *t);
    Thread *GetProcess(int pid);
    Thread *DeleteProcess(int pid);

  private:
    Thread **processTable;
    Lock *lock;
};

#endif
