// communication thread: send instructions over serial line, (yes: send, no: continue (is there incoming data (yes: process that, no: check sending instructions again)))
// and
// file handling thread: read instructions
// and
// user interaction thread: handle file info, handler for cancel current file execution, exit, custom commands, read buffer

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


void* comm (void *pInParam);
void* morse (void *fileName);
void print_menu ();
void call_termios(int reset);
void printSelection(char *strInfo);
float get_double(const char *str);

#define cBUF_SIZE

bool quit = false;

int main (int argc, char * argv[])
{
    if (argc <= 1)  /* check if parameter given */
    {
        fprintf(stderr, "Main: serial port path required\n");
        exit(1);
    }
    int hSerial = serial_init( argv[1] );     /* open serial port */

    tSerialData myMorseData;  // struct to pass into morse thread
    myMorseData.hSerial = hSerial;

    printf ("\nEnter name of file with morse definitions: ");
    call_termios(1);    // able to view filename
    scanf("%s", myMorseData.defFileName);
    pthread_create(&myMorseData.oCom, NULL, morse, (void *)&myMorseData);   // create morse thread

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
