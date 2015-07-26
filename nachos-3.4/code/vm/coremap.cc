#include "system.h"
#include "addrspace.h"
#include "coremap.h"

#ifdef VM

CoreMap::CoreMap()
{
	physToSpace = new AddrSpace*[NumPhysPages]();
	physToVirt = new int[NumPhysPages]();
}

CoreMap::~CoreMap()
{
	delete[] physToSpace;
	delete[] physToVirt;
}

void
CoreMap::Map(unsigned int phys, unsigned int virt, AddrSpace *s)
{
	ASSERT(phys < NumPhysPages);
	ASSERT(physToSpace[phys] == NULL);

	DEBUG('v', "Mapping phys %d to virt %d\n", phys, virt);
	physToSpace[phys] = s;
	physToVirt[phys] = virt;
}

void
CoreMap::Unmap(unsigned int phys)
{
	ASSERT(phys < NumPhysPages);
	ASSERT(physToSpace[phys] != NULL);

	DEBUG('v', "Unmapping phys %d\n", phys);
	physToSpace[phys] = NULL;
	physToVirt[phys] = -1;
	usedPages->Clear(phys);
}

int
CoreMap::evictPage()
{
	static int nextEviction = 0;
	int evict = nextEviction;
	nextEviction = (nextEviction+1) % NumPhysPages;

	DEBUG('v', "Evicting phys %d\n", evict);
	AddrSpace *s = physToSpace[evict];
	s->EvictPage(physToVirt[evict]);
	usedPages->Mark(evict);

	return evict;
}

int
CoreMap::Find()
{
	// See if there's free RAM
	int page = usedPages->Find();
	if (page != -1)
		return page;

	// No free RAM, make some room
	return evictPage();
}

#endif
