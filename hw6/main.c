// comm function is responsible to receive and send data. Every time a request is made from the user thread, it
// notifies the comm thread to send data and when the response is received, the communication thread notifies the
// working thread to print the result of the operation.

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "commands.h"
#include "serial.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

void* comm (void *pInParam);
void print_menu (int * pSelection);
void printBuffer(char *str, int iLen);

char * g_strArrMenu[] =
        {       "Item 'o': LED ON",
                "Item 'f': LED OFF",
                "Item 'r': Button State",
                "Item 'w': Read Buffer",
                "Item 'c': Enter a custom command.",
                "Item 'e': Exit"
        };

#define cBUF_SIZE 255
char chBuffOut[cBUF_SIZE];
char chBuffIn [cBUF_SIZE];

typedef struct tSerialData
{
    char chBuffIn[cBUF_SIZE];
    int iBuffLen;

    char chCmdBuff[cBUF_SIZE];
    int cmdBuffLen;

    pthread_t oCom;

    int hSerial;

}   tSerialData;

bool quit = false;

int main (int argc, char * argv[])
{
    int hSerial = serial_init("/dev/ttyACM0");
    if (hSerial <= 0) {  printf("Error from open: %s\n", strerror(errno)); return -1; } /* check for errors in opening */

    tSerialData oSerialData;
    oSerialData.hSerial = hSerial;
    pthread_create(&oSerialData.oCom, NULL, comm, (void *)&oSerialData );

    int bContinue = 1;
    char strInput[255];
    int iSelection;

    while (bContinue)
    {
        print_menu(&iSelection);
        switch(iSelection)
        {
            case 'o':
            {
                int iBuffOutSize = 0;
                sprintf(chBuffOut, "%s\r\n", chCmd_LED_ON);
                iBuffOutSize = strlen(chBuffOut);

                int n_written = serial_write(hSerial, chBuffOut, iBuffOutSize);
            }   break;

            case 'f':
            {
                int iBuffOutSize = 0;
                sprintf(chBuffOut, "%s\r\n", chCmd_LED_OFF);
                iBuffOutSize = strlen(chBuffOut);

                int n_written = serial_write(hSerial, chBuffOut, iBuffOutSize);
            }   break;

            case 'r':
            {
                int iBuffOutSize = 0;
                sprintf(chBuffOut, "%s\r\n", chCmd_BUTTON_STATUS);
                iBuffOutSize = strlen(chBuffOut);

                int n_written = serial_write( hSerial, chBuffOut, iBuffOutSize);

                usleep (1000*1000);                             // wait for nucleo to reply (time in nanoseconds)

                memset (chBuffIn , '\0', cBUF_SIZE);            // set chBuffIn to NULL (entire array is NULL chars)
                int n = serial_read( hSerial, chBuffIn , cBUF_SIZE );  // n is number of bytes read
                printf ("Received data (%d) %s\n", n, chBuffIn);
            }   break;

            case 'w':
            {
                memset(chBuffIn, '\0', cBUF_SIZE);
                int n = serial_read( hSerial, chBuffIn , cBUF_SIZE );
                printf ("Received data (%d) %s\n", n, chBuffIn);
            }

            case 'e':
            {
                bContinue = 0;
                quit = true;
            }   break;

            default:
            printf ("Wrong option\n");
            break;
        }
    }
    pthread_join(oSerialData.oCom, NULL);
    close(hSerial);
}

void print_menu (int * pSelection)
{
    char chArrSelection[cBUF_SIZE];
    int iSelection;
    printf("== program menu ==\n");

    for (int iCnt = 0; iCnt < (sizeof(g_strArrMenu)/sizeof(char*)); iCnt++ )
    {
        printf("%s\n", g_strArrMenu[iCnt]);
    }

    printf("Selection: ");
    scanf("%s", chArrSelection);
    *pSelection = chArrSelection[0];
    printf("\n");

}
void printBuffer(char *str, int iLen)
{
    for (int iCnt = 0; iCnt < iLen; iCnt++)
    {
        printf(" %02X ", str[iCnt]);
    }
    printf("\n");
}

void* comm (void *pInParam)
{
    bool q = false;
    int iRecv;

    tSerialData * pSerialData;
    pSerialData = (tSerialData   *)pInParam;
    memset(pSerialData->chBuffIn, '\0', cBUF_SIZE);
    pSerialData->iBuffLen = 0;

    memset(pSerialData->chCmdBuff, '\0', cBUF_SIZE);
    pSerialData->cmdBuffLen = 0;

    while (!q)
    {
        iRecv = serial_read( pSerialData->hSerial, pSerialData->chBuffIn , cBUF_SIZE );
        if (iRecv > 0 )
        {
//            printf ("Received data (%d) %s\n", iRecv, pSerialData->chBuffIn);

            for (int iCnt = 0; iCnt < iRecv; iCnt++ )
            {
                pSerialData->chCmdBuff[pSerialData->cmdBuffLen] = pSerialData->chBuffIn[iCnt];
                pSerialData->cmdBuffLen++ ;

                if (pSerialData->cmdBuffLen >= cBUF_SIZE)
                {
                    pSerialData->cmdBuffLen = 0;
                }
                if ( pSerialData->cmdBuffLen > 1)
                {
//                    printf ("Command buffer detected(%d) %s\n", pSerialData->cmdBuffLen, pSerialData->chCmdBuff);
//                    printBuffer(pSerialData->chCmdBuff, pSerialData->cmdBuffLen);

                    if ((pSerialData->chCmdBuff[pSerialData->cmdBuffLen - 1] == '\n') &&
                        (pSerialData->chCmdBuff[pSerialData->cmdBuffLen - 2] == '\r'))
                    {
                        printf ("Command detected(%d) %s\n", pSerialData->cmdBuffLen, pSerialData->chCmdBuff);

                        if (strstr(pSerialData->chCmdBuff, "Welcome to Nucleo") != NULL)
                        {
                            printf ("Nucleo was reset\n");
                        }
                            pSerialData->cmdBuffLen = 0;
                    }
                }
            }
        }

        q = quit;

//        pthread_mutex_lock(&mtx);
//            pthread_cond_wait(&condvar, &mtx);
//            printf("\rCounter %10i", counter);
//            fflush(stdout);
//             q = quit;
//        pthread_mutex_unlock(&mtx);
    }
    return 0;
}