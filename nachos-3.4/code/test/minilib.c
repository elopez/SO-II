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

int strcmp(const char *a, const char *b)
{
	while (*a != '\0' && *b != '\0') {
		if (*a - *b == 0) {
			a++, b++;
			continue;
		} else {
			break;
		}
	}

	return *a - *b;
}
