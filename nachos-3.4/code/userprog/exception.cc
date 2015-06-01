// exception.cc
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "filesys.h"

void
readStrFromUsr(int usrAddr, char *outStr)
{
    int c;

    do {
        machine->ReadMem(usrAddr++, 1, &c);
        *outStr++ = c;
    } while (c != '\0');
}

void
readBuffFromUsr(int usrAddr, char *outBuff, int byteCount)
{
    int c, i;

    for (i = 0; i < byteCount; i++) {
        machine->ReadMem(usrAddr++, 1, &c);
        *outBuff++ = c;
    }
}

void
writeStrToUsr(char *str, int usrAddr)
{
    do {
        machine->WriteMem(usrAddr++, 1, *str++);
    } while (*str != '\0');
}

void
writeBuffToUsr(char *str, int usrAddr, int byteCount)
{
    int i;

    for (i = 0; i < byteCount; i++)
        machine->WriteMem(usrAddr++, 1, *str++);
}

void
incrementPCRegs()
{
    int pc;

    pc = machine->ReadRegister(PCReg);
    machine->WriteRegister(PrevPCReg, pc);
    pc = machine->ReadRegister(NextPCReg);
    machine->WriteRegister(PCReg, pc);
    machine->WriteRegister(NextPCReg, pc + 4);
}

void
startProcess(void *n)
{
    currentThread->space->InitRegisters();
    currentThread->space->RestoreState();

    machine->Run();
    ASSERT(false);
}

#define CODE_REG 2
#define ARG1_REG 4
#define ARG2_REG 5
#define ARG3_REG 6
#define ARG4_REG 7

void
handleSyscall(int type)
{
    int result = type;
    int arg1 = machine->ReadRegister(ARG1_REG);
    int arg2 = machine->ReadRegister(ARG2_REG);
    int arg3 = machine->ReadRegister(ARG3_REG);
    int arg4 = machine->ReadRegister(ARG4_REG);

    switch (type) {
    case SC_Halt: {
        DEBUG('c', "Shutdown, initiated by user program.\n");
        interrupt->Halt();
        break;
    }
    case SC_Exit: {
        currentThread->Finish(arg1);
        DEBUG('c', "Exiting thread with value %d\n", arg1);
        break;
    }
    case SC_Exec: {
        char *name = new char[1024];
        readStrFromUsr(arg2, name);
        OpenFile *f = fileSystem->Open(name);
        if (!f) {
            DEBUG('c', "Problem opening file %s for execution\n", name);
            delete[] name;
            break;
        }
        AddrSpace *s = new AddrSpace(f);
        Thread *t = new Thread(name, 1);
        t->space = s;
        t->Fork(startProcess, NULL);
        delete f;

        int pid = 999; //TODO
        result = pid;
        break;
    }
    case SC_Join: {
        break;
    }
    case SC_Create: {
        char name[1024];
        readStrFromUsr(arg1, name);
        result = fileSystem->Create(name, 0);
        DEBUG('c', "File creation with name %s was %s\n", name,
              result ? "successful" : "erroneous");
        break;
    }
    case SC_Open: {
        char name[1024];
        readStrFromUsr(arg1, name);
        OpenFile *f = fileSystem->Open(name);
        if (!f) {
            DEBUG('c', "Error opening file %s\n", name);
            result = -1;
            break;
        }
        result = currentThread->AddFile(f);
        DEBUG('c', "File with name %s opened on fd %d\n", name, result);
        break;
    }
    case SC_Read: {
        OpenFile *f = currentThread->GetFile(arg3);
        if (!f) {
            DEBUG('c', "Error opening file %d for reading\n", arg3);
            break;
        }
        char *buf = new char[arg2];
        result = f->Read(buf, arg2);
        writeBuffToUsr(buf, arg1, result);
        delete[] buf;
        DEBUG('c', "Read %d bytes (wanted %d) from file with fd %d\n",
              result, arg2, arg3);
        break;
    }
    case SC_Write: {
		OpenFile *f = currentThread->GetFile(arg3);
        if (!f) {
            DEBUG('c', "Error opening file %d for writing\n", arg3);
            break;
        }
        char *buf = new char[arg2];
        readBuffFromUsr(arg1, buf, arg2);
        result = f->Write(buf, arg2);
        delete[] buf;
        DEBUG('c', "Wrote %d bytes (wanted %d) from file with fd %d\n",
              result, arg2, arg3);
        break;
    }
    case SC_Close: {
        OpenFile *f = currentThread->RemoveFile(arg1);
        if (!f) {
            DEBUG('c', "Error closing file %d\n", arg1);
            break;
        }
        delete f;
        DEBUG('c', "Closed file with fd %d\n", arg1);
        break;
    }
    case SC_Fork:
    case SC_Yield: {
        DEBUG('c', "UNIMPLEMENTED SYSCALL\n");
        break;
    }
    default:
        DEBUG('c', "Unknown syscall of type %d\n", type);
        break;
    }

    incrementPCRegs();
    machine->WriteRegister(CODE_REG, result);
}

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2.
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions
//	are in machine.h.
//----------------------------------------------------------------------

void
ExceptionHandler(ExceptionType which)
{
    int code = machine->ReadRegister(CODE_REG);

    if (which == SyscallException) {
        handleSyscall(code);
    } else {
        printf("Unexpected user mode exception %d %d\n", which, code);
        ASSERT(false);
    }
}
