// addrspace.h
//	Data structures to keep track of executing user programs
//	(address spaces).
//
//	For now, we don't keep any information about address spaces.
//	The user level CPU state is saved and restored in the thread
//	executing the user program (see thread.h).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#ifndef ADDRSPACE_H
#define ADDRSPACE_H

#include "copyright.h"
#include "filesys.h"
#include "noff.h"

class Thread;

#define UserStackSize		1024 	// increase this as necessary!

class AddrSpace {
  public:
    AddrSpace(OpenFile *executable, char **args = NULL, Thread *t = NULL);	// Create an address space,
					// initializing it with the program
					// stored in the file "executable"
    ~AddrSpace();			// De-allocate an address space

    void InitRegisters();		// Initialize user-level CPU registers,
					// before jumping to user code
    void SetArguments();		// Push arguments to thread stack and
					// reconfigure registers accordingly

    void SaveState();			// Save/restore address space-specific
    void RestoreState();		// info on a context switch

    bool LoadPageToTLB(unsigned int vpage);
    void StoreTLBFlags(unsigned int i);
    #if defined(VM) && defined(DEMAND_LOADING)
    void LoadPage(unsigned int vpage);
    #endif
    #ifdef VM
    void EvictPage(unsigned int vpage);
    void ReloadPage(unsigned int vpage);
    #endif

  private:
    TranslationEntry *pageTable;	// Assume linear page table translation
					// for now!
    unsigned int numPages;		// Number of pages in the virtual
					// address space
    char **args;			// Arguments for the new process, to be
					// allocated in the fresh stack
    int nextTLBIndex;
    int asid;
    OpenFile *swap;
    NoffHeader noffH;
    #if defined(VM) && defined(DEMAND_LOADING)
    OpenFile *binary;
    #endif
};

#endif // ADDRSPACE_H
