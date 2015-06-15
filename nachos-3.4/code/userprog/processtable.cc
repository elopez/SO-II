#include "processtable.h"
#include "thread.h"
#include "synch.h"

ProcessTable::ProcessTable()
{
	lock = new Lock("process table lock");
	processTable = new Thread*[PROCESS_QTY]();
}

ProcessTable::~ProcessTable()
{
	delete[] processTable;
	delete lock;
}

int
ProcessTable::AllocateId(Thread *t)
{
	int pid = -1;

	lock->Acquire();
	for (int i = 0; i < PROCESS_QTY; i++) {
		if (processTable[i] == NULL) {
			processTable[i] = t;
			pid = i;
			break;
		}
	}
	lock->Release();

	return pid;
}

Thread *
ProcessTable::GetProcess(int pid)
{
	if (pid < 0 || pid >= PROCESS_QTY)
		return NULL;

	lock->Acquire();
	Thread *t = processTable[pid];
	lock->Release();

	return t;
}

Thread *
ProcessTable::DeleteProcess(int pid)
{
	if (pid < 0 || pid >= PROCESS_QTY)
		return NULL;

	lock->Acquire();
	Thread *t = processTable[pid];
	processTable[pid] = NULL;
	lock->Release();

	return t;
}
