// addrspace.cc
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "noff.h"

//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------

static void
SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------

AddrSpace::AddrSpace(OpenFile *executable, char **argv)
{
    NoffHeader noffH;
    unsigned int i, size;

    args = argv;

    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) &&
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFFMAGIC);

// how big is address space?
    size = noffH.code.size + noffH.initData.size + noffH.uninitData.size
			+ UserStackSize;	// we need to increase the size
						// to leave room for the stack
    numPages = divRoundUp(size, PageSize);
    size = numPages * PageSize;

    ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    ASSERT((int)numPages <= usedPages->NumClear());

    DEBUG('a', "Initializing address space, num pages %d, size %d\n",
					numPages, size);
// first, set up the translation
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
	pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
	pageTable[i].physicalPage = usedPages->Find();
	ASSERT(pageTable[i].physicalPage != -1);
	pageTable[i].valid = true;
	pageTable[i].use = false;
	pageTable[i].dirty = false;
	pageTable[i].readOnly = false;  // if the code segment was entirely on
					// a separate page, we could set its
					// pages to be read-only
    }

// zero out the entire address space, to zero the unitialized data segment
// and the stack segment
    for (i = 0; i < numPages; i++)
        bzero(&machine->mainMemory[pageTable[i].physicalPage * PageSize], PageSize);

// then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n",
			noffH.code.virtualAddr, noffH.code.size);

        for (int bytes = 0; bytes < noffH.code.size; bytes++) {
			int vaddr = noffH.code.virtualAddr + bytes;
            int vpage = vaddr / PageSize;
            int voffset = vaddr % PageSize;
            int paddr = pageTable[vpage].physicalPage * PageSize + voffset;
            executable->ReadAt(&(machine->mainMemory[paddr]), 1,
                noffH.code.inFileAddr + bytes);
        }
    }

    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n",
			noffH.initData.virtualAddr, noffH.initData.size);

        for (int bytes = 0; bytes < noffH.initData.size; bytes++) {
			int vaddr = noffH.initData.virtualAddr + bytes;
            int vpage = vaddr / PageSize;
            int voffset = vaddr % PageSize;
            int paddr = pageTable[vpage].physicalPage * PageSize + voffset;
            executable->ReadAt(&(machine->mainMemory[paddr]), 1,
                noffH.initData.inFileAddr + bytes);
        }
    }
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    for (unsigned int i = 0; i < numPages; i++)
        usedPages->Clear(pageTable[i].physicalPage);
    delete[] pageTable;
}

//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

static int
pushStrToStack(int sp, char *str)
{
    do {
        machine->WriteMem(sp++, 1, *str);
    } while (*str++ != '\0');

    return sp;
}

void
AddrSpace::SetArguments()
{
    int sp = machine->ReadRegister(StackReg);
    int argc = 0, argsize = 0;
    int uargbase, uargv;

    // Compute how much space are the arguments going to take
    for (int i = 0; args[i] != NULL; i++) {
        argc++;
        argsize += strlen(args[i]) + 1;
    }

    // Calculate addresses, while keeping alignment
    uargbase = (sp - argsize) & ~3;
    uargv = uargbase - (argc + 1) * sizeof(int);
    sp = uargv - 16; // MIPS ABI, space for register pushes

    // Configure main(...) arguments and new stack pointer
    machine->WriteRegister(4, argc);
    machine->WriteRegister(5, uargv);
    machine->WriteRegister(StackReg, sp);

    // Push the arguments and their addresses into the stack
    for (int i = 0; i < argc; i++) {
        machine->WriteMem(uargv, 4, uargbase);
        uargv += 4;
        uargbase = pushStrToStack(uargbase, args[i]);
    }
    machine->WriteMem(uargv, 4, 0);

    // Free now useless arguments
    for (int i = 0; args[i] != NULL; i++)
        delete[] args[i];
    delete[] args;
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState()
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState()
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}
