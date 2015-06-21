#include "syscall.h"

int strlen(char *str)
{
	int count = 0;
	while (*str++ != '\0')
		count++;
	return count;
}

#define puts(x) Write((x), strlen((x)), ConsoleOutput)

int main(int argc, char *argv[])
{
	int i;

	puts("ArgumentPrinter starting...\n");

	for (i = 0; i < argc; i++) {
		puts("Argument: ");
		puts(argv[i]);
		puts("\n");
	}

	puts("ArgumentPrinter finishing...\n");
	Exit(0);

	return 0;
}
