#include "syscall.h"
#include "minilib.h"

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
