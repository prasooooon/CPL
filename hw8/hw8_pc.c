// Prasoon Dwivedi on 12 December 2020

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>      // for isalpha()
#include <string.h>

#include <stdbool.h>

#include "serial.h"
#include "commands.h"

#include <pthread.h>

#include <termios.h>
#include <unistd.h>     // for STDIN_FILENO (from termios function)

#define cBUF_SIZE 255
char chCmdOut[cBUF_SIZE];

FILE *filePointer;

pthread_mutex_t mutex_quit_thread;

void* fileHandleThread (void *pInParam);
void* sCommThread(void *pInParam);
void* userInteractionThread (void *pInParam);
void call_termios(int reset);

char q;

typedef struct tSerialData
{
    char chBuffIn[cBUF_SIZE];
    int INBuffLen;

    char chBuffOut[cBUF_SIZE];
    int OUTBuffLen;

    pthread_t ID;

    int hSerial;

    // for file handling thread
    char defFileName[cBUF_SIZE];

    bool quit;

} tSerialData;


int main (int argc, char * argv[])
{
    if (argc <= 1)  /* check if parameter given */
    {
        fprintf(stderr, "Main: serial port path required\n");
        exit(1);
    }
    int hSerial = serial_init(argv[1]);     /* open serial port, serial.c has error handler*/

    tSerialData universalThreadStruct; //= {.hSerial = hSerial};

    enum {user, file, serial, NUM_THREADS};
    const char *threadNames[] = {"user", "file", "serial"};

    void* (*thr_functions[])(void*) = {userInteractionThread, fileHandleThread, sCommThread};

    pthread_t threads[NUM_THREADS];     // for references to IDs of created thread
    pthread_mutex_init(&mutex_quit_thread, NULL);

    call_termios(0);             // 0 is not-waiting-for-input mode

    for (int i = 0; i < NUM_THREADS; i++)
    {
        int r = pthread_create(&threads[i], NULL, thr_functions[i], &universalThreadStruct);
        fprintf(stderr, "MAIN: Creating thread '%s' %s\n", threadNames[i], ( r == 0 ? "OK" : "FAILED") );
    }

    int *ex;
    for (int i = 0; i < NUM_THREADS; ++i) {
        fprintf(stderr, "MAIN: Called join for thread: %s\n", threadNames[i]);
        int r = pthread_join(threads[i], (void*)&ex);
        fprintf(stderr, "MAIN: Joining the thread %s has been %s with exit value %i\n", threadNames[i], (r == 0 ? "OK" : "FAILED"), *ex);
    }

    call_termios(1);
    return EXIT_SUCCESS;
}

void* userInteractionThread (void *pInParam)
{
    tSerialData *userThreadData = (tSerialData *)pInParam;
    static int r = 0;   // return value
    int c;

    printf("Enter file name with drawings: ");
    call_termios(1);

    pthread_mutex_lock(&mutex_quit_thread);
    scanf("%s", userThreadData->defFileName);
    pthread_mutex_unlock(&mutex_quit_thread);

    call_termios(0);

//    while (( c = getchar()) != 'q')   // user output loop
//    {
//
//    }

    r = 1;
    pthread_mutex_lock(&mutex_quit_thread);
//    userThreadData->quit = true;                // if q pressed
    pthread_mutex_unlock(&mutex_quit_thread);

    fprintf(stderr, "Exiting user interaction thread with ID: %lu\n", (unsigned long)pthread_self());
    return &r;
}

void* fileHandleThread (void *pInParam)
{
    tSerialData *fileThreadData = (tSerialData *)pInParam;
    static int r = 0;
    bool q = false;

    memset(fileThreadData->chBuffIn, '\0', cBUF_SIZE);         // clear all buffers
    fileThreadData->INBuffLen = 0;
    memset(fileThreadData->chBuffOut, '\0', cBUF_SIZE);
    fileThreadData->OUTBuffLen = 0;

    while (!q)
    {
        pthread_mutex_lock(&mutex_quit_thread);
        q = fileThreadData->quit;
        fflush(stdout);
        pthread_mutex_unlock(&mutex_quit_thread);

        if ((filePointer = fopen(fileThreadData->defFileName, "r")) == NULL)
        {
            fprintf(stderr, "File thread: can't open file \"%s\".", fileThreadData->defFileName);
            r = 1;
        }

        char * line = NULL;
        size_t len = 0;
        ssize_t read;

        while ((read = getline(&line, &len, filePointer)) != 1)
        {
            size_t i = 0;

            while(isspace(line[i])) { ++i; }        // find first non space index

            if (line[i] == '#') {continue;}         // if first non space character is #, then ignore line

            else if (strstr(line, "*IDN?") == 0)    // identification command
            {
                printf("file thread: found IDN command\n %s", line);
                sprintf(fileThreadData->chBuffOut, "%s\r\n", line);
                int iBuffOutsize = strlen(fileThreadData->chBuffOut);
                int n_written = serial_write(fileThreadData->hSerial, fileThreadData->chBuffOut, iBuffOutsize);
                return 0;
            }



        }



    }

    return 0;
}

void* sCommThread(void *pInParam)
{
    tSerialData *sCommData = (tSerialData *)pInParam;
    static int  r = 0;

    pthread_mutex_lock(&mutex_quit_thread);
    bool q = sCommData->quit;
    pthread_mutex_unlock(&mutex_quit_thread);

    memset (sCommData->chBuffIn, '\0', cBUF_SIZE);  // clear communcation in/out buffers
    sCommData->INBuffLen = 0;
    memset(sCommData->chBuffOut, '\0', cBUF_SIZE);
    sCommData->OUTBuffLen = 0;

    int iRecv;

//    while (!q)
//    {
//        iRecv = serial_read(sCommData->hSerial, sCommData->chBuffIn, cBUF_SIZE);
//        if (iRecv > 0)
//        {
//
//        }
//    }





    return 0;
}

void call_termios(int reset)
{
    static struct termios tio, tioOld;      // tioOld = tio"OLD"
    tcgetattr(STDIN_FILENO, &tio);
    if (reset) {
        tcsetattr(STDIN_FILENO, TCSANOW, &tioOld);
    } else {
        tioOld = tio; //backup
        cfmakeraw(&tio);
        tio.c_oflag |= OPOST; // enable output postprocessing
        tcsetattr(STDIN_FILENO, TCSANOW, &tio);
    }
}
