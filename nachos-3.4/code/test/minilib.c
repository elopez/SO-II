#include "minilib.h"
#include "syscall.h"

/* String length */
size_t strlen(const char *str)
{
	int count = 0;
	while (*str++ != '\0')
		count++;
	return count;
}
