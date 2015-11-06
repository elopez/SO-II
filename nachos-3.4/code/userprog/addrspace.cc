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
#include "memaccess.h"
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

AddrSpace::AddrSpace(OpenFile *executable, char **argv, Thread *t)
{
    unsigned int i, size;

    args = argv;
    t->space = this;
    asid = t->pid;

#ifdef VM
    // Open swap storage
    char name[50] = "SWAP.12345";
    sprintf(name, "SWAP.%d", asid);
    fileSystem->Create(name, numPages * PageSize);
    swap = fileSystem->Open(name);
#endif

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

#ifndef VM
    ASSERT(numPages <= NumPhysPages);		// check we're not trying
						// to run anything too big --
						// at least until we have
						// virtual memory

    ASSERT((int)numPages <= usedPages->NumClear());
#endif

    DEBUG('a', "Initializing address space, num pages %d, size %d\n",
					numPages, size);
// first, set up the translation
    pageTable = new TranslationEntry[numPages];
    for (i = 0; i < numPages; i++) {
	pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
	pageTable[i].valid = true;
	pageTable[i].use = false;
	pageTable[i].dirty = true;
	pageTable[i].readOnly = false;  // if the code segment was entirely on
					// a separate page, we could set its
					// pages to be read-only
#if defined(VM) && defined(DEMAND_LOADING)
	pageTable[i].physicalPage = -1;
	pageTable[i].valid = false;
#elif defined(VM)
	pageTable[i].physicalPage = coreMap->Find();
	bzero(&machine->mainMemory[pageTable[i].physicalPage * PageSize], PageSize);
	coreMap->Map(pageTable[i].physicalPage, &pageTable[i], this);
#else
	pageTable[i].physicalPage = usedPages->Find();
	ASSERT(pageTable[i].physicalPage != -1);
	bzero(&machine->mainMemory[pageTable[i].physicalPage * PageSize], PageSize);
#endif
    }

#ifdef DEMAND_LOADING
    // If we're doing on-demand loading of pages, we don't need to read them now
    // Just keep the executable at hand
    binary = executable;
    return;
#endif

// then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n",
			noffH.code.virtualAddr, noffH.code.size);

        for (int bytes = 0; bytes < noffH.code.size; bytes++) {
            int vaddr = noffH.code.virtualAddr + bytes;
            int vpage = vaddr / PageSize;
            int voffset = vaddr % PageSize;
#ifdef VM
            ReloadPage(vpage);
#endif
            int paddr = pageTable[vpage].physicalPage * PageSize + voffset;
            executable->ReadAt(&(machine->mainMemory[paddr]), 1,
                noffH.code.inFileAddr + bytes);
#ifdef VM
            // The page must not be swapped while writing
            ASSERT(pageTable[vpage].valid);
            pageTable[vpage].dirty = true;
#endif
        }
    }

    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n",
			noffH.initData.virtualAddr, noffH.initData.size);

        for (int bytes = 0; bytes < noffH.initData.size; bytes++) {
            int vaddr = noffH.initData.virtualAddr + bytes;
            int vpage = vaddr / PageSize;
            int voffset = vaddr % PageSize;
#ifdef VM
            ReloadPage(vpage);
#endif
            int paddr = pageTable[vpage].physicalPage * PageSize + voffset;
            executable->ReadAt(&(machine->mainMemory[paddr]), 1,
                noffH.initData.inFileAddr + bytes);
#ifdef VM
            // The page must not be swapped while writing
            ASSERT(pageTable[vpage].valid);
            pageTable[vpage].dirty = true;
#endif
        }
    }
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    for (unsigned int i = 0; i < numPages; i++) {
#ifdef VM
        if (pageTable[i].valid)
            coreMap->Unmap(pageTable[i].physicalPage);
#else
        usedPages->Clear(pageTable[i].physicalPage);
#endif
    }
    delete[] pageTable;
    delete swap;
#ifdef DEMAND_LOADING
    delete binary;
#endif

    interrupt->SetLevel(oldLevel);
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
        writeMem(sp++, 1, *str);
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
        writeMem(uargv, 4, uargbase);
        uargv += 4;
        uargbase = pushStrToStack(uargbase, args[i]);
    }
    writeMem(uargv, 4, 0);

    // Free now useless arguments
    for (int i = 0; args[i] != NULL; i++)
        delete[] args[i];
    delete[] args;
}

void AddrSpace::StoreTLBFlags(unsigned int i)
{
    if (!machine->tlb[i].valid)
        return;

    int virt = machine->tlb[i].virtualPage;
    pageTable[virt].use = machine->tlb[i].use;
    pageTable[virt].dirty |= machine->tlb[i].dirty;
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState()
{
    #ifdef USE_TLB
    // Save useful data from the TLB
    for (int i = 0; i < TLBSize; i++)
        StoreTLBFlags(i);
    #endif
}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState()
{
    #ifdef USE_TLB
    // Invalidate the TLB
    for (int i = 0; i < TLBSize; i++)
        machine->tlb[i].valid = false;
    // Start loading from scratch
    nextTLBIndex = 0;
    #else
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
    #endif
}

bool
AddrSpace::LoadPageToTLB(unsigned int vpage)
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    if (vpage >= numPages)
        return false;

    #if defined(VM) && defined(DEMAND_LOADING)
    // Load the page from the binary if needed
    LoadPage(vpage);
    #endif

    #ifdef VM
    // Reload the page from swap if necessary
    ReloadPage(vpage);
    #endif

    StoreTLBFlags(nextTLBIndex);
    machine->tlb[nextTLBIndex++] = pageTable[vpage];
    nextTLBIndex %= TLBSize;

    interrupt->SetLevel(oldLevel);

    return true;
}

#if defined(VM) && defined(DEMAND_LOADING)
void
AddrSpace::LoadPage(unsigned int vpage)
{
    // Not loaded pages are set to phys -1
    if (pageTable[vpage].valid || pageTable[vpage].physicalPage != -1)
	return;

    int phys = coreMap->Find();

    DEBUG('v', "Loading page %d asid %d (phys %d)\n", vpage, asid, phys);

    for (int bytes = 0; bytes < PageSize; bytes++) {
	int paddr = phys * PageSize + bytes;
        int vaddr = vpage * PageSize + bytes;

        if (vaddr >= noffH.code.virtualAddr &&
            vaddr < noffH.code.virtualAddr + noffH.code.size) {
            binary->ReadAt(&(machine->mainMemory[paddr]), 1,
                noffH.code.inFileAddr + vaddr - noffH.code.virtualAddr);
        } else if (vaddr >= noffH.initData.virtualAddr &&
            vaddr < noffH.initData.virtualAddr + noffH.initData.size) {
            binary->ReadAt(&(machine->mainMemory[paddr]), 1,
                noffH.initData.inFileAddr + vaddr - noffH.initData.virtualAddr);
        } else {
            machine->mainMemory[paddr] = 0;
        }
    }

    pageTable[vpage].physicalPage = phys;
    pageTable[vpage].valid = true;
    pageTable[vpage].dirty = true;
    coreMap->Map(phys, &pageTable[vpage], this);
}
#endif

#ifdef VM
void
AddrSpace::EvictPage(unsigned int vpage)
{
    ASSERT(pageTable[vpage].valid == true);

    // Get latest dirty bits if applicable
    if (currentThread->space == this)
        SaveState();

    int phys = pageTable[vpage].physicalPage;
    int paddr = phys * PageSize;
    int vaddr = vpage * PageSize;

    DEBUG('v', "Evicting page %d asid %d (phys %d)\n", vpage, asid, phys);
    if (pageTable[vpage].dirty) {
        swap->WriteAt(&(machine->mainMemory[paddr]), PageSize, vaddr);
        pageTable[vpage].dirty = false;
        DEBUG('v', "Page was dirty, wrote to swap\n");
        stats->numSwapWrites++;
    }

    pageTable[vpage].valid = false;
    coreMap->Unmap(phys);

    // Process is not active, no need to invalidate TLB
    if (currentThread->space != this)
        return;

    // Invalidate any potential TLB entry involving this page
    for (int i = 0; i < TLBSize; i++) {
        if (!machine->tlb[i].valid)
            continue;
        if (machine->tlb[i].virtualPage == (int)vpage) {
            machine->tlb[i].valid = false;
            break;
        }
    }
}

void
AddrSpace::ReloadPage(unsigned int vpage)
{
    // Do not reload pages in memory
    if (pageTable[vpage].valid)
        return;

    int phys = coreMap->Find();
    int paddr = phys * PageSize;
    int vaddr = vpage * PageSize;

    stats->numSwapReads++;

    DEBUG('v', "Reloading page %d asid %d (phys %d)\n", vpage, asid, phys);
    swap->ReadAt(&(machine->mainMemory[paddr]), PageSize, vaddr);
    pageTable[vpage].physicalPage = phys;
    pageTable[vpage].valid = true;
    coreMap->Map(phys, &pageTable[vpage], this);
}
#endif
