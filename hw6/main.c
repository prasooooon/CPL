/*
 * comm function is responsible to receive and send data. Every time a request is made from the user thread, it
 * notifies the comm thread to send data and when the response is received, the communication thread notifies the
 * working thread to print the result of the operation.
*/

// run `make and then ./dwivepra with parameter which is the path to the USB/serial interface: such as “/dev/ttyACMx”

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "commands.h"
#include "serial.h"
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <termios.h>
#include <ctype.h>      // for isprint()
#include <signal.h>
#include <assert.h>
#include <semaphore.h>

void* comm (void *pInParam);
void* morse (void *fileName);
void print_menu ();
void call_termios(int reset);
void printSelection(char *strInfo);
float get_double(const char *str);

char * g_strArrMenu[] =
        {       "==Program Menu==",
                "Item 'o': LED ON",
                "Item 'f': LED OFF",
                "Item 'r': Button State",
                "Item 'l': Load morse code definition file",
                "Item 'c': Enter a custom command.",
                "Item 'e': Exit"
        };

#define cBUF_SIZE 255
char chBuffOut[cBUF_SIZE];
char chBuffIn [cBUF_SIZE];
char chCmd_CUSTOM_COMMAND[cBUF_SIZE];   // unlike other commands, this variable will be modified during execution,
                                        // and can not be stored in header file, which is read only memory

FILE *filePointer;                      // pointer of FILE type
char cancelMorseThread;                 // to terminate morse thread

pthread_mutex_t mtx;

typedef struct tSerialData
{
    char chBuffIn[cBUF_SIZE];
    int iBuffLen;

    char chCmdBuff[cBUF_SIZE];
    int cmdBuffLen;

    pthread_t oCom;

    int hSerial;

    // for morse thread
    float sigON;
    float sigOFF;

    char defFileName[cBUF_SIZE];

} tSerialData;

bool quit = false;

int main (int argc, char * argv[])
{
    if (argc <= 1)  /* check if parameter given */
    {
        fprintf(stderr, "Main: serial port path required\n");
        exit(1);
    }
    int hSerial = serial_init( argv[1] );     /* open serial port */

    pthread_mutex_init(&mtx, NULL);

    tSerialData oSerialData;
    oSerialData.hSerial = hSerial;
    pthread_create(&oSerialData.oCom, NULL, comm, (void *)&oSerialData);   // create communication thread

    int bContinue = 1;
    char strInput[255];
    int c;

    call_termios(0);        // call termios function to change console mode

    print_menu();

    while (bContinue)
    {
        c = getchar();
        switch(c)
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
            }   break;

            case 'c':
            {
                printf ("\n Enter your custom command: ");

                call_termios(1);            /* reset terminal to normal mode for scanf */

                scanf("%s", chCmd_CUSTOM_COMMAND);

                call_termios(0);            /* set terminal back to 'not waiting for enter' mode */

                int iBuffOutSize = 0;
                sprintf(chBuffOut, "%s\r\n", chCmd_CUSTOM_COMMAND);
                iBuffOutSize = strlen(chBuffOut);

                int n_written = write( hSerial, chBuffOut, iBuffOutSize);

                usleep (1000*1000); /* wait for nucleo to reply (time in nanoseconds) */

                memset (chBuffIn , '\0', cBUF_SIZE);
                int n = read( hSerial, chBuffIn , cBUF_SIZE );

                chBuffIn [n - 2] = 0;

                if (strcmp("Wrong command", chBuffIn) == 0) {
                    printf("Nucleo claims it does not know the command.\n");
                }

            }   break;
            case 'l':
            {
                tSerialData myMorseData;  // struct to pass into morse thread
                myMorseData.hSerial = hSerial;

                printf ("\nEnter name of file with morse definitions: ");
                call_termios(1);    // able to view filename
                scanf("%s", myMorseData.defFileName);
                pthread_create(&myMorseData.oCom, NULL, morse, (void *)&myMorseData);   // create morse thread

                call_termios(0);   // back to raw mode (not waiting for enter)

                pthread_join(myMorseData.oCom, NULL);      // wait for thread to finish

            }   break;

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
    call_termios(1);    //revert console to the original mode
}

void* morse (void *pInParam)
{
    struct tSerialData * myMorseData;
    myMorseData = (tSerialData *)pInParam;

    memset(myMorseData->chBuffIn, '\0', cBUF_SIZE);         // clear all buffers
    myMorseData->iBuffLen = 0;
    memset(myMorseData->chCmdBuff, '\0', cBUF_SIZE);
    myMorseData->cmdBuffLen = 0;

    if ((filePointer = fopen(myMorseData->defFileName, "r")) == NULL)
    {
        fprintf(stderr, "main: can't open file \"%s\".", myMorseData->defFileName);
        exit (-2);
    }

    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, filePointer)) != -1)
    {
        size_t i = 0;
        while(isspace(line[i]))         // find first non space index
        { ++i; }

        if (line[i] == '#')             // check if first non space char is #
        {
            continue;                   // skip comment line
        }

        else if(strstr(line, "sig_on")) // find signal on line
        { myMorseData->sigON = get_double(line); }

        else if(strstr(line, "sigoff")) // find signal off line
        {  myMorseData->sigOFF = get_double(line); }

        else if(line[i] == '1' || line[i] == '0')   // line with binary ish command
        {
            for (int iCnt = 0; iCnt <= strlen(line); iCnt++)
            {   // scan every character in command line of morse definition text
                int iBuffOutSize = 0;

                if (line[iCnt] == '1')
                {
                    sprintf(chBuffOut, "%s\r\n", chCmd_LED_ON);
                    iBuffOutSize = strlen(chBuffOut);
                    int n_ON = serial_write(myMorseData->hSerial, chBuffOut, iBuffOutSize);
                        usleep(3 * (myMorseData->sigON * 1000) * 1000);

                    sprintf(chBuffOut, "%s\r\n", chCmd_LED_OFF);
                    iBuffOutSize = strlen(chBuffOut);
                    int n_OFF= serial_write(myMorseData->hSerial, chBuffOut, iBuffOutSize);
                        usleep((myMorseData->sigOFF * 1000)* 1000);
                }

                else if (line[iCnt] == '0')
                {
                    sprintf(chBuffOut, "%s\r\n", chCmd_LED_ON);
                    iBuffOutSize = strlen(chBuffOut);
                    int n_ON = serial_write(myMorseData->hSerial, chBuffOut, iBuffOutSize);
                        usleep((myMorseData->sigON * 1000) * 1000);


                    sprintf(chBuffOut, "%s\r\n", chCmd_LED_OFF);
                    iBuffOutSize = strlen(chBuffOut);
                    int n_OFF= serial_write(myMorseData->hSerial, chBuffOut, iBuffOutSize);
                        usleep((myMorseData->sigOFF * 1000) * 1000);
                }

                else if (line[iCnt] == ' ')
                {
                    sprintf(chBuffOut, "%s\r\n", chCmd_LED_OFF);
                    iBuffOutSize = strlen(chBuffOut);
                    int n_OFF= serial_write(myMorseData->hSerial, chBuffOut, iBuffOutSize);
                        usleep(2 * (myMorseData->sigOFF * 1000) * 1000);
                }
            }
        }
        else // probably empty lines
        { continue; }

    }
    fclose(filePointer);
    if (line)
        free(line);
    return 0;
}

void* comm (void *pInParam)
{
    bool q = false;
    int iRecv;

    tSerialData * pSerialData;      // structure name * variable name
    pSerialData = (tSerialData *)pInParam;
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
                    if ((pSerialData->chCmdBuff[pSerialData->cmdBuffLen - 1] == '\n') &&
                        (pSerialData->chCmdBuff[pSerialData->cmdBuffLen - 2] == '\r'))
                    {
                        if (strstr(pSerialData->chCmdBuff, "Welcome to Nucleo") != NULL)
                        { printSelection("Nucleo reset"); }

                        if (strstr(pSerialData->chCmdBuff, "BUTTON:PRESSED\r\n") != NULL)
                        { printSelection("Button pressed."); }

                        if (strstr(pSerialData->chCmdBuff, "BUTTON:RELEASED\r\n") != NULL)
                        { printSelection("Button released."); }

                        fflush(stdout);
                        pSerialData->cmdBuffLen = 0;
                    }
                }
            }
        }
        q = quit;

    }
    return 0;
}


void printSelection(char *strInfo)
{
    char strLine[] = "Info:                         | Enter option: ";
    if (strInfo != NULL)
    {
        char cByte;
        for (int iCnt = 0; iCnt < 23; iCnt++)
        {
            cByte = strInfo[iCnt];
            if (isprint(cByte))
            {
                strLine[6 + iCnt] = cByte;
            }
            else { break; }
        }
    }
    printf("\r%s", strLine);
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

float get_double(const char *str)      // search float in string
{
    /* First skip non-digit characters */
    while (*str && !(isdigit(*str) || ((*str == '-' || *str == '+') && isdigit(*(str + 1)))))
        str++;

    /* Then parse to a double */
    return strtof(str, NULL);
}
void print_menu ()
{
    for (int iCnt = 0; iCnt < (sizeof(g_strArrMenu)/sizeof(char*)); iCnt++ )
    {
        printf("%s\n", g_strArrMenu[iCnt]);
    }
    printSelection(NULL);
}
