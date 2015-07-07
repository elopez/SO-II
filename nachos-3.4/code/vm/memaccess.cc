#include "memaccess.h"
#include "system.h"

int
readMem(int usrAddr, int c, int *val)
{
	int result = machine->ReadMem(usrAddr, c, val);

#ifdef USE_TLB
	// When using TLB, read can fail due to TLB miss
	if (!result)
		result = machine->ReadMem(usrAddr, c, val);
	ASSERT(result);
#endif

	return result;
}

int
writeMem(int usrAddr, int c, int val)
{
	int result = machine->WriteMem(usrAddr, c, val);

#ifdef USE_TLB
	// When using TLB, write can fail due to TLB miss
	if (!result)
		result = machine->WriteMem(usrAddr, c, val);
	ASSERT(result);
#endif

	return result;
}
