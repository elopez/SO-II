#include "syscall.h"

#define NULL ((void*)0)

void bufferToArgv(char *buf, char *argv[])
{
    int i = 0;

    while (*buf != '\0') {
        argv[i++] = buf;
        while(*buf != ' ' && *buf != '\0')
            buf++;
        if (*buf == '\0') {
            argv[i] = NULL;
            break;
        } else {
            *buf++ = '\0';
        }
    }
}

int
main()
{
    SpaceId newProc;
    OpenFileId input = ConsoleInput;
    OpenFileId output = ConsoleOutput;
    char prompt[2], ch, buffer[60];
    char *argv[10];
    int i;

    prompt[0] = '-';
    prompt[1] = '-';

    while( 1 )
    {
	Write(prompt, 2, output);

	i = 0;

	do {

	    Read(&buffer[i], 1, input);

	} while( buffer[i++] != '\n' );

	buffer[--i] = '\0';

	if( i > 0 ) {
		if (*buffer == '&') {
			bufferToArgv(buffer+1, argv);
			Exec(buffer+1, argv);
		} else {
			bufferToArgv(buffer, argv);
			newProc = Exec(buffer, argv);
			Join(newProc);
		}
	}
    }
}

