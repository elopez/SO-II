#ifndef COREMAP_H
#define COREMAP_H

#include "addrspace.h"

class CoreMap {
  public:
    CoreMap();
    ~CoreMap();

    void Map(unsigned int phys, AddrSpace *s);
    void Unmap(unsigned int phys);

    int Find();

  private:
    AddrSpace **physToSpace;
};

#endif
