#include "syscall.h"

typedef unsigned int size_t;

#ifndef MINILIB_H
#define MINILIB_H

size_t strlen(const char *str);

#define puts(x) Write((x), strlen((x)), ConsoleOutput)

#endif
