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
char defFileName[cBUF_SIZE];

char chBuffIn[cBUF_SIZE];
int INBuffLen;

char chBuffOut[cBUF_SIZE];
int OUTBuffLen;

pthread_mutex_t mutex_quit_thread;
pthread_mutex_t OUTbufferLock;
pthread_mutex_t INbufferLock;
pthread_cond_t OUTbufferCond = PTHREAD_COND_INITIALIZER;

void* fileHandleThread (void *pInParam);
void* sCommThread(void *pInParam);
void* userInteractionThread (void *pInParam);


typedef struct tSerialData
// MOVE THIS TO HEADER FILE
{
    pthread_t ID;

    int hSerial;

    bool quit;

} tSerialData;

int main (int argc, char * argv[])
{
    if (argc <= 1)  /* check if parameter given */
    {
        fprintf(stderr, "MAIN: serial port path missing\n");
        exit(1);
    }
    pthread_mutex_init( &mutex_quit_thread, NULL );
    pthread_mutex_init( &INbufferLock, NULL );
    pthread_mutex_init( &OUTbufferLock, NULL );

    pthread_cond_init( &OUTbufferCond, NULL);

    int hSerial = serial_init(argv[1]);     /* open serial port, serial.c has error handler*/

    tSerialData universalThreadStruct = {.hSerial = hSerial};

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

    pthread_mutex_destroy( &mutex_quit_thread );
    pthread_mutex_destroy( &INbufferLock );
    pthread_mutex_destroy( &OUTbufferLock );

    pthread_cond_destroy( &OUTbufferCond);

    return EXIT_SUCCESS;
}

void* userInteractionThread (void *pInParam)
{
    tSerialData *userThreadData = (tSerialData *)pInParam;
    static int r = 0;   // return value

//    printf("Enter file name with drawings: ");

    pthread_mutex_lock(&mutex_quit_thread);
//    scanf("%s", defFileName);
    strcpy(defFileName, "main.txt");
    pthread_mutex_unlock(&mutex_quit_thread);

    fprintf(stderr, "Exiting user interaction thread with ID: %lu\n", (unsigned long)pthread_self());
    return &r;
}

void* fileHandleThread (void *pInParam)
{
    tSerialData *fileThreadData = (tSerialData *)pInParam;
    static int r = 0;

    tListItem * pFirstLI;   // Linked lists are represented by pointers to their first nodes or heads

    pFirstLI = L_loadFromFile (defFileName);
    (void) L_processIncludes(pFirstLI);

    while (pFirstLI != NULL)
    {
        pthread_mutex_lock( &pFirstLI->list_mutex );

//        printf("node: %s", pFirstLI->pLine);      // print list forward
        if (strlen(pFirstLI->pLine) >= 2 && pFirstLI->pLine [strlen(pFirstLI->pLine) - 1] == '\n') {
            pFirstLI->pLine[strlen(pFirstLI->pLine) - 1] = '\0';
        }
        if (strlen(pFirstLI->pLine) >= 2 && pFirstLI->pLine [strlen(pFirstLI->pLine) - 1] == '\r') {
            pFirstLI->pLine[strlen(pFirstLI->pLine) - 1] = '\0';
        }

        if (pFirstLI->pLine[0] == '/' && pFirstLI->pLine[1] == '/' )  // comments start with '//'
        {
            // printf("comment: %s\n", pFirstLI->pLine);
        }

        else if (pFirstLI->pLine[0] == '#')
        {   // instructions not for nucleo

//            // labels and includes are processed by functions in list.c and here, we process the other keywords
//
//            /* instruction "keywords" are: 'label', 'include', "wait_for_joystick", "if", "else", "exit", "goto"
//
//             * wait_for_joystick needs max 2, min 0 parameters       [wait_for_joystick:waitTime(mSecs):defaultJoySTATUS]
//                * first parameter is always integer milliseconds, second param is always char array joystick status
//                * if no event on joystick, then use, defaultJoySTATUS and go to its if case
//                * assume wait_for_joystick is always used with an if/else case
//                * if wait_for_joystick is used wrong, ignore if/else case below it
//
//             * if needs 3 parameters   (if:JOY_SEL:goto:LabelIntro:) [if:joystickEvent:goto:labelName:]
//             * else needs 2 parameters (else:goto:LabelIntro:)       [else:goto:labelName:]
//
//             * goto needs 1 parameter  (goto:LabelSplash:)           [goto:labelName:]
//                * can be used independently or within some if/else line
//            */
//
//            char *findKeyword;
//
//            if ( (findKeyword = strstr(pFirstLI->pLine, "#goto:")) != NULL)
//            {   // goto found in beginning of line
//
//                int iFirstColon = locFindColon(pFirstLI->pLine);
//                // iFirstColon is the colon in "#goto:"
//
//                if (iFirstColon > 0)
//                {
//                    int iSecondColon = locFindColon(&pFirstLI->pLine[iFirstColon + 1]);
//                    // iSecondColon is the last colon in "#goto:labelName:"
//                    if (iSecondColon > 0)
//                    {
//                        char strLocalLabel[255];
//                        memset(strLocalLabel, 0, 255);
//                        strncpy(strLocalLabel, &pFirstLI->pLine[iFirstColon+1],iSecondColon);    // strLocalLabel is the string between colons
//
//                        if ( L_findLabel(L_findBeg(pFirstLI), strLocalLabel) != NULL)
//                        {   // label is actually present in file
//
//                            // move to that element in list
//                            pFirstLI = L_findLabel(L_findBeg(pFirstLI), strLocalLabel);
//                        }
//                        else
//                            fprintf(stderr, "FILE: |%s| is not a valid label or parameter for |%s|\n", strLocalLabel, defFileName);
//                     }
//                }   // END if first colon found
//
//            }   // END if goto not found in file
//
//            if ( (findKeyword = strstr(pFirstLI->pLine, "#wait_for_joystick:")) != NULL)
//            {   // wait_for_joystick found in the beginning of line
//
//                if (strstr(pFirstLI->pN->pLine, "#if:") == NULL || strstr(pFirstLI->pN->pLine, "#if:") == NULL )
//                    // there is no if/else condition after the wait line
//                {continue;}
//
//                int timeout;
//                char *defaultState;
//                int iFirstColon = locFindColon(pFirstLI->pLine);
//                // iFirstColon is the colon in "#wait_for_joystick:"
//
//                if (iFirstColon > 0)
//                {
//                    int iSecondColon = locFindColon(&pFirstLI->pLine[iFirstColon + 1]);
//                    // iSecondColon is the last colon in "#wait_for_joystick:waitTime:"
//
//                    if (iSecondColon > 0)
//                    {   // one parameter found
//                        char strLocalLabel[255];
//                        memset(strLocalLabel, 0, 255);
//                        strncpy(strLocalLabel, &pFirstLI->pLine[iFirstColon+1], iSecondColon);   // strLocalLabel is wait time
//
//                        int iThirdColon = locFindColon(&pFirstLI->pLine[iSecondColon + 1]);
//                        // iThirdColon is the last colon in "#wait_for_joystick:waitTime:defaultState:"
//                        if (iThirdColon > 0)
//                        {   // 2 parameters found
//                            int ret = sscanf(pFirstLI->pLine, "#wait_for_joystick:%d:%s", timeout, &defaultState);
//                        }     // END two parameters found
//
//                    }     // END one parameter found
//
//                }
//                else
//                {    // no parameters = wait indefinitely
//
//                }
//                memset(chBuffIn, '\0', cBUF_SIZE);
//                INBuffLen = 0;
//                INBuffLen = serial_read(fileThreadData->hSerial, chBuffIn,
//                                        sizeof(chBuffIn));
//                usleep(500*1000);   // unimplemented wait time for joystick
//
//                if ( INBuffLen >= 2 && chBuffIn[INBuffLen - 2] == '\r' && chBuffIn[INBuffLen - 1] == '\n' )
//                {
//                    if (strstr(chBuffIn, "JOY") != NULL)
//                        strcpy(&defaultState, chBuffIn);
//                }
//
//            }
//
//            if ( (findKeyword = strstr(pFirstLI->pLine, "#if:")) != NULL)
//            {   // case for if
//                // PSEUDOCODE
//                if (three params found)
//                    check if joystick state variable from last if is same as first parameter
//                    if yes:
//                        pFirstLI = L_findLabel(L_findBeg(pFirstLI), last parameter);
//                    if no:
//                        continue to next line
//            }
//
//            if ( (findKeyword = strstr(pFirstLI->pLine, "#else:")) != NULL)
//            {   // case for else
//                // PSEUDOCODE
//                if (two params found)
//                    if first param is indeed goto
//                        pFirstLI = L_findLabel(L_findBeg(pFirstLI), last parameter);
//            }
//
//            printf("syntax: %s\n", pFirstLI->pLine);
        }

        else if ( pFirstLI->pLine[ strlen(pFirstLI->pLine) - 1 ] == '?' )   // single quotes are chars, double quotes are NULL terminated char arrays
        {   //  question commands expect response from nucleo

//            memset(chBuffOut, '\0', cBUF_SIZE);
//
//            sprintf(chBuffOut, "%s\r\n", pFirstLI->pLine);
//            OUTBuffLen = sizeof(chBuffOut) - 1;
//            printf("line being sent: |%s| with length |%d|", chBuffOut, OUTBuffLen);
//            int n_written = serial_write(fileThreadData->hSerial, chBuffOut, OUTBuffLen);
//            usleep(1000 * 1000);                        // (trying to) preventing clock skews
//            memset(chBuffOut, '\0', cBUF_SIZE);
//
//            memset(chBuffIn, '\0', cBUF_SIZE);
//            INBuffLen = 0;
//            INBuffLen = serial_read(fileThreadData->hSerial, chBuffIn, sizeof(chBuffIn));
//            usleep(500*1000);
//            if ( INBuffLen >= 2 && chBuffIn[INBuffLen - 2] == '\r' && chBuffIn[INBuffLen - 1] == '\n' )
//                printf("message from nucleo: |%s| with length |%d|", chBuffIn, INBuffLen);
//
//            printf("\nquestion: %s\n", pFirstLI->pLine);
        }

        else if ( strlen(pFirstLI->pLine) > 2 )   // ignore empty lines
        {   //  normal commands

            memset(chBuffOut, '\0', cBUF_SIZE);

            sprintf(chBuffOut, "%s\r\n", pFirstLI->pLine);
            OUTBuffLen = sizeof(chBuffOut) - 1;
            printf("line being sent: |%s| with length |%d|", chBuffOut, OUTBuffLen);
            int n_written = serial_write(fileThreadData->hSerial, chBuffOut, OUTBuffLen);
            usleep(1000 * 1000);                        // (trying to) preventing clock skews
            memset(chBuffOut, '\0', cBUF_SIZE);

            printf("\nnot question: %s\n", pFirstLI->pLine);
        }

        // since entire node is locked, it makes sense to unlock
        // before it changes to the next node and lock that one
        pthread_mutex_unlock( &pFirstLI->list_mutex );

        pFirstLI = pFirstLI->pN;     // move to next line
    }

    L_free(pFirstLI);
    return &r;
}

/*
 * @brief: handle serial communication
 * @param: universal structure with hSerial, buffers etc. and linked list
 */
void* sCommThread(void *pInParam)
{
    tSerialData *sCommData = (tSerialData *)pInParam;
    static int  r = 0;


    return &r;
}
