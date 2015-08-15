#include "system.h"
#include "addrspace.h"
#include "list.h"
#include "coremap.h"

#ifdef VM

CoreMap::CoreMap()
{
	physToSpace = new AddrSpace*[NumPhysPages]();
	physToVirt = new TranslationEntry*[NumPhysPages]();
#ifdef CLOCK_EVICT
	clock = new List<TranslationEntry*>();
#endif
}

CoreMap::~CoreMap()
{
	delete[] physToSpace;
	delete[] physToVirt;
#ifdef CLOCK_EVICT
	delete clock;
#endif
}

void
CoreMap::Map(unsigned int phys, TranslationEntry *virt, AddrSpace *s)
{
	ASSERT(phys < NumPhysPages);
	ASSERT(physToSpace[phys] == NULL);

	DEBUG('v', "CoreMap: Mapping phys %d to virt %d\n", phys, virt->virtualPage);
	physToSpace[phys] = s;
	physToVirt[phys] = virt;

#ifdef CLOCK_EVICT
	clock->Append(virt);
#endif
}

void
CoreMap::Unmap(unsigned int phys)
{
	ASSERT(phys < NumPhysPages);
	ASSERT(physToSpace[phys] != NULL);

	DEBUG('v', "CoreMap: Unmapping phys %d\n", phys);
#ifdef CLOCK_EVICT
	clock->RemoveItem(physToVirt[phys]);
#endif
	physToSpace[phys] = NULL;
	physToVirt[phys] = NULL;
	usedPages->Clear(phys);
}

#ifdef CLOCK_EVICT
int
CoreMap::clockRound(bool dirty)
{
	TranslationEntry *head, *next;
	head = next = clock->Remove();

	do {
		clock->Append(next);

		if (next->use == false && next->dirty == dirty)
			return next->physicalPage;

		next->use = false;
		next = clock->Remove();
	} while (next != head);

	clock->Prepend(head);

	return -1;
}
#endif

int
CoreMap::evictPage()
{
#ifdef CLOCK_EVICT
	int evict = clockRound(false);
	if (evict == -1)
		evict = clockRound(true);
	if (evict == -1)
		evict = clockRound(false);
	if (evict == -1)
		evict = clockRound(true);
	ASSERT(evict != -1);
#else
	static int nextEviction = 0;
	int evict = nextEviction;
	nextEviction = (nextEviction+1) % NumPhysPages;
#endif

	DEBUG('v', "CoreMap: Evicting phys %d\n", evict);
	AddrSpace *s = physToSpace[evict];
	s->EvictPage(physToVirt[evict]->virtualPage);
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
