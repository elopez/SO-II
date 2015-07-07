/* cat.c
 * Simple program to concatenate files
 */

#include "syscall.h"
#include "minilib.h"

int main(int argc, char *argv[])
{
	char buf[BUF_SIZ];
	OpenFileId file;
	int i, readbytes;

	if (argc < 2) {
		puts("Usage: "); puts(argv[0]); puts(" [file]...\n");
		Exit(1);
	}

	for (i = 1; i < argc; i++) {
		if (!strcmp("-", argv[i])) {
			file = ConsoleInput;
		} else {
			file = Open(argv[i]);
			if (file == -1) {
				puts("Problem opening file \""); puts(argv[i]); puts("\"\n");
				continue;
			}
		}

		while ((readbytes = Read(buf, BUF_SIZ, file)) != 0)
			Write(buf, readbytes, ConsoleOutput);

		if (file != ConsoleInput)
			Close(file);
	}

	Exit(0);
	return 0;
}
