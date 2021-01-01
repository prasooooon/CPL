#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>      // for isalpha()
#include <string.h>

#include <stdbool.h>

#include "serial.h"
#include "commands.h"


//#include <termios.h>
#include <unistd.h>     // for STDIN_FILENO (from termios function)

#define cBUF_SIZE 255

FILE *filePointer;

char chBuffIn[cBUF_SIZE];

char chBuffOut[cBUF_SIZE];
int OUTBuffLen;

int hSerial;

char defFileName[cBUF_SIZE] = "intro.txt";

int main (int argc, char * argv[])
{
    if (argc <= 1)  /* check if parameter given */
    {
        fprintf(stderr, "Main: serial port path required\n");
        return 1;
    }
    int hSerial = serial_init(argv[1]);     /* open serial port, serial.c has error handler*/

    if ((filePointer = fopen(defFileName, "r")) == NULL)
    {
        fprintf(stderr, "Main: can't open file \"%s\".", defFileName);
        return 2;
    }

    fflush(stdout);
    fflush(stdin);

    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    sprintf(chBuffOut, "\r\n");
    OUTBuffLen = sizeof(chBuffOut) - 1;
    int n_written = serial_write(hSerial, chBuffOut, OUTBuffLen);   // empty command
    int n = serial_read(hSerial, chBuffIn, sizeof(chBuffIn));


    while ( (read = getline(&line, &len, filePointer)) != -1 )
    {
        size_t i = 0;
        while(isspace(line[i])) { ++i; }        // find first non space index

        if (line[i] == '#') {continue;}         // if first non space character is #, then ignore line

        memset(chBuffOut, '\0', cBUF_SIZE);
        OUTBuffLen = 0;

        sprintf(chBuffOut, "%s\r\n", line);
        OUTBuffLen = sizeof(chBuffOut) - 1;
        printf("line being sent: |%s| with length |%d|", chBuffOut, OUTBuffLen);
        int n_written = serial_write(hSerial, chBuffOut, OUTBuffLen);
        usleep(500);        // preventing clock skews

        memset(chBuffIn, '\0', cBUF_SIZE);                          // clear all buffers
        int n = serial_read(hSerial, chBuffIn, sizeof(chBuffIn));
        printf("message from nucleo: |%s| with length |%d|", chBuffIn, n);
    }

    fclose(filePointer);
    if(line) {
        free(line);
    }

    fflush(stdout);
    fflush(stdin);

    serial_close(hSerial);
    return EXIT_SUCCESS;
}
