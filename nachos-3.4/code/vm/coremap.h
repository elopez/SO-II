#ifndef COREMAP_H
#define COREMAP_H

#include "addrspace.h"
#include "list.h"

class CoreMap {
  public:
    CoreMap();
    ~CoreMap();

    void Map(unsigned int phys, TranslationEntry *virt, AddrSpace *s);
    void Unmap(unsigned int phys);

    int Find();

  private:
    int evictPage();
    AddrSpace **physToSpace;
    TranslationEntry **physToVirt;
#ifdef CLOCK_EVICT
    int clockRound(bool dirty);
    List<TranslationEntry*> *clock;
#endif
};

#endif
