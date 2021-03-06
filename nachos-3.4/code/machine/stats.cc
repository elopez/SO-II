// stats.h
//	Routines for managing statistics about Nachos performance.
//
// DO NOT CHANGE -- these stats are maintained by the machine emulation.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "utility.h"
#include "stats.h"

//----------------------------------------------------------------------
// Statistics::Statistics
// 	Initialize performance metrics to zero, at system startup.
//----------------------------------------------------------------------

Statistics::Statistics()
{
    totalTicks = idleTicks = systemTicks = userTicks = 0;
    numDiskReads = numDiskWrites = 0;
    numConsoleCharsRead = numConsoleCharsWritten = 0;
    numPageFaults = numPacketsSent = numPacketsRecvd = 0;
    numTLBHits = numTLBMisses = 0;
    numSwapReads = numSwapWrites = 0;
#ifdef DFS_TICKS_FIX
    numBugFix = 0;
#endif

}

//----------------------------------------------------------------------
// Statistics::Print
// 	Print performance metrics, when we've finished everything
//	at system shutdown.
//----------------------------------------------------------------------

void
Statistics::Print()
{
#ifdef DFS_TICKS_FIX
    printf("Ticks bug fixed %llu times!\n", numBugFix);
    printf("WARNING: The next statistics may be invalid...\n\n");
#endif
    printf("Ticks: total %d, idle %d, system %d, user %d\n", totalTicks,
	idleTicks, systemTicks, userTicks);
    printf("Disk I/O: reads %d, writes %d\n", numDiskReads, numDiskWrites);
    printf("Console I/O: reads %d, writes %d\n", numConsoleCharsRead,
	numConsoleCharsWritten);
    printf("Paging: faults %d\n", numPageFaults);
    printf("Network I/O: packets received %d, sent %d\n", numPacketsRecvd,
	numPacketsSent);

#ifdef USE_TLB
    unsigned long long total = numTLBHits + numTLBMisses;
    double hitspct = (100.0 * numTLBHits) / total;
    double misspct = (100.0 * numTLBMisses) / total;
    printf("TLB Hits: %llu (%.2f%%)\n", numTLBHits, hitspct);
    printf("TLB Misses: %llu (%.2f%%)\n", numTLBMisses, misspct);
#endif

#ifdef VM
    printf("Swap reads: %llu\n", numSwapReads);
    printf("Swap writes: %llu\n", numSwapWrites);
#endif
}
