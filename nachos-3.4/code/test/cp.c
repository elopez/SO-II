/* cp.c
 * Simple program to copy files
 */

#include "syscall.h"
#include "minilib.h"

int main(int argc, char *argv[])
{
	char buf[BUF_SIZ];
	OpenFileId orig, dest;
	int readbytes;

	if (argc != 3) {
		puts("Usage: "); puts(argv[0]); puts(" origin destination\n");
		Exit(1);
	}

	orig = Open(argv[1]);
	if (orig == -1) {
		puts("Problem opening file \""); puts(argv[1]); puts("\"\n");
		Exit(2);
	}

	Create(argv[2]);
	dest = Open(argv[2]);

	while ((readbytes = Read(buf, BUF_SIZ, orig)) != 0)
		Write(buf, readbytes, dest);

	Close(orig);
	Close(dest);

	Exit(0);
	return 0;
}
