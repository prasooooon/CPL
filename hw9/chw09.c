#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>      // for isalpha()
#include <string.h>

#include <stdbool.h>

#include "serial.h"
#include "commands.h"
#include "list.h"

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

    //----------------------------

    // create threads
    enum {
        user, file, serial, NUM_THREADS
    };
    const char *threadNames[] = {"user", "file", "serial"};

    void *(*thr_functions[])(void *) = {userInteractionThread, fileHandleThread, sCommThread};

    pthread_t threads[NUM_THREADS];     // for references to IDs of created thread
    pthread_mutex_init(&mutex_quit_thread, NULL);

    for (int i = 0; i < NUM_THREADS; i++) {
        int r = pthread_create(&threads[i], NULL, thr_functions[i], &universalThreadStruct);
        fprintf(stderr, "MAIN: Creating thread '%s' %s\n", threadNames[i], (r == 0 ? "OK" : "FAILED"));
    }

    //----------------------------

    // kill threads
    int *ex;
    for (int i = 0; i < NUM_THREADS; ++i) {
        fprintf(stderr, "MAIN: Called join for thread: %s\n", threadNames[i]);
        int r = pthread_join(threads[i], (void*)&ex);
        fprintf(stderr, "MAIN: Joining the thread %s has been %s with exit value %i\n", threadNames[i], (r == 0 ? "OK" : "FAILED"), *ex);
    }
    //----------------------------

    return EXIT_SUCCESS;
}

void* userInteractionThread (void *pInParam)
{
    tSerialData *userThreadData = (tSerialData *)pInParam;
    static int r = 0;   // return value

//    printf("Enter file name with drawings: ");

    pthread_mutex_lock(&mutex_quit_thread);
//    sscanf("%s", userThreadData->defFileName);
    strcpy(userThreadData->defFileName, "main.txt");
    pthread_mutex_unlock(&mutex_quit_thread);

    fprintf(stderr, "Exiting user interaction thread with ID: %lu\n", (unsigned long)pthread_self());
    return &r;
}

void* fileHandleThread (void *pInParam)
{
    tSerialData *fileThreadData = (tSerialData *)pInParam;
    static int r = 0;
    bool q = false;

    tListItem * pFirstLI;   // Linked lists are represented by pointers to their first nodes or heads

    pthread_mutex_lock( &pFirstLI->list_mutex );

    pFirstLI = L_loadFromFile (fileThreadData->defFileName);
    L_processIncludes(pFirstLI);

    pthread_mutex_unlock( &pFirstLI->list_mutex );

    L_PrintForward(pFirstLI);


    return &r;
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
