#include "minilib.h"

int main()
{
	int a = 1;
	int b = 2;

	char res[] = "0\n";

	res[0] = '0' + (a | b);

	puts("If you see a 3 below, OR is working alright\n");
	puts(res);

	Exit(0);
	return 0;
}
