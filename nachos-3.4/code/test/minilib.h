#include "syscall.h"

typedef unsigned int size_t;

#ifndef MINILIB_H
#define MINILIB_H

#define BUF_SIZ 32

size_t strlen(const char *str);
int strcmp(const char *a, const char *b);

#define puts(x) Write((x), strlen((x)), ConsoleOutput)

#endif
