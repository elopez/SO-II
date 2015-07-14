#include "system.h"
#include "addrspace.h"
#include "coremap.h"

CoreMap::CoreMap()
{
	physToSpace = new AddrSpace*[NumPhysPages]();
}

CoreMap::~CoreMap()
{
	delete[] physToSpace;
}

void
CoreMap::Map(unsigned int phys, AddrSpace *s)
{
	ASSERT(phys < NumPhysPages);
	ASSERT(physToSpace[phys] == NULL);
	physToSpace[phys] = s;
}

void
CoreMap::Unmap(unsigned int phys)
{
	ASSERT(phys < NumPhysPages);
	ASSERT(physToSpace[phys] != NULL);
	physToSpace[phys] = NULL;
}

int
CoreMap::Find()
{
	// See if there's free RAM
	int page = usedPages->Find();
	if (page != -1)
		return page;

	// No free RAM, make some room
	// TODO
	ASSERT(false);
}
