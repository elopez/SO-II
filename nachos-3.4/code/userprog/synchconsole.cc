#include "synchconsole.h"
#include "synch.h"

SynchConsole::SynchConsole(const char *readf, const char *writef)
{
	console = new Console(readf, writef, SynchConsole::ReadAvail,
			      SynchConsole::WriteDone, this);
	readAvail = new Semaphore("read avail", 0);
	writeDone = new Semaphore("write done", 0);
	wlock = new Lock("synchronized console write lock");
	rlock = new Lock("synchronized console read lock");
}

SynchConsole::~SynchConsole()
{
	delete console;
	delete readAvail;
	delete writeDone;
	delete wlock;
	delete rlock;
}

void
SynchConsole::ReadAvail(void* arg)
{
	((SynchConsole*)arg)->readAvail->V();
}

void
SynchConsole::WriteDone(void* arg)
{
	((SynchConsole*)arg)->writeDone->V();
}

void
SynchConsole::PutChar(char c)
{
	wlock->Acquire();
	console->PutChar(c);
	writeDone->P();
	wlock->Release();
}

char
SynchConsole::GetChar()
{
	char c;

	rlock->Acquire();
	readAvail->P();
	c = console->GetChar();
	rlock->Release();

	return c;
}

int
SynchConsole::PutBuffer(char *buf, int size)
{
	wlock->Acquire();
	for (int i = 0; i < size; i++) {
		console->PutChar(buf[i]);
		writeDone->P();
	}
	wlock->Release();
	return size;
}

int
SynchConsole::GetBuffer(char *buf, int size)
{
	rlock->Acquire();
	for (int i = 0; i < size; i++) {
		readAvail->P();
		buf[i] = console->GetChar();
	}
	rlock->Release();
	return size;
}
