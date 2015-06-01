#ifndef SYNCHCONSOLE_H
#define SYNCHCONSOLE_H

#include "console.h"
#include "synch.h"

class SynchConsole {
  public:
    SynchConsole(const char *readf = NULL, const char *writef = NULL);
    ~SynchConsole();

    void PutChar(char ch);
    char GetChar();

    int PutBuffer(char *buf, int size);
    int GetBuffer(char *buf, int size);

  private:
    static void ReadAvail(void*);
    static void WriteDone(void*);
    Semaphore *readAvail;
    Semaphore *writeDone;
    Console *console;
    Lock *rlock;
    Lock *wlock;
};

#endif // SYNCHCONSOLE_H
