#include "minilib.h"

/* scratch buffer */
int buf[1024];
#define buflen (sizeof(buf)/sizeof(buf[0]))

/* random generator */
unsigned long int next = 1;
int rand(void)
{
    next = next * 1103515245 + 12345;
    return (unsigned int)(next / 65536) % 32768;
}

/* Message strings, we use these to avoid a compiler issue */
char err[] = "Error detected!\n";
char ok[] = "Alright!\n";

int main()
{
	int i, j;

	/* Fill with test values */
	for (i = 0; i < buflen; i++) {
		buf[i] = 0xDEADBEEF + i;
	}

	/* Verify if the values are read correctly
	 * in a random order */
	for (i = 0; i < buflen * 1000; i++) {
		j = rand() % buflen;
		if (buf[j] != (0xDEADBEEF + j)) {
			puts(err);
			Halt();
		}
	}

	puts(ok);
	Halt();
	return 0;
}
