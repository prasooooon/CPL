#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <termios.h>    // POSIX terminal control definitions
#include <fcntl.h>      // File controls like O_RDWR from open function
#include <unistd.h>     // write(), read(), open() etc.

char * g_strArrMenu[] =
        {
                "Item 'o': LED ON",
                "Item 'f': LED OFF",
                "Item 'r': Button State",
                "Item 'c': Enter a custom command.",
                "Item 'e': Exit"
        };

char chCmd_LED_ON[]  = {'L', 'E', 'D', ' ', 'O', 'N', '\0'};    // both ways get same result bc strings are 1D arrays in C
char chCmd_LED_OFF[] = "LED OFF";                               // '\0' character will automatically be added by compiler
char chCmd_BUTTON_STATUS[] = "BUTTON:STATUS?";

#define cBUF_SIZE 255
char chBuffOut[cBUF_SIZE];
char chBuffIn[cBUF_SIZE];

void print_menu(int * pSelection)
{
    char chArrSelection[BUFSIZ];    // BUFSIZ is recommended size of input buffer defined in stdio.h
    int iSelection;

    printf("==Program Menu==\n");
    for (int iCnt = 0; iCnt < (sizeof(g_strArrMenu)/sizeof(char*)); iCnt++ )
    {
        printf("%s\n", g_strArrMenu[iCnt]);
    }
    printf("Selection: ");
    scanf("%s", chArrSelection);
    *pSelection = chArrSelection[0];
    printf(" \n");
}

int main(int argc, char *argv[])
{
    int hSerial = open( "/dev/ttyACM0", O_RDWR| O_NONBLOCK | O_NDELAY );    // serial port open
//    if (hSerial < 0) {  printf("tcgetattr: %s\n", strerror(errno)); }

    fcntl(hSerial, F_SETFL, 0);

    int iRetVal;
    struct termios o_tty;   // structure calls information about serial port settings
    memset (&o_tty, 0, sizeof(o_tty) );
    iRetVal = tcgetattr (hSerial , &o_tty);

    /* set in/out baud rate non-integer is unix-compliant*/
    cfsetispeed(&o_tty, B9600);
    cfsetospeed(&o_tty, B9600);

    /* check man termios for definitions */
    o_tty.c_cflag |= (CLOCAL | CREAD);                  /* control modes */
    o_tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);   /* local modes */
    o_tty.c_oflag &= ~OPOST;                            /* output modes */

    /* read() will block until either any amount of data is given, or the timeout (VTIME = 10 deciSeconds) occurs */
    o_tty.c_cc[VMIN] = 0;                               /* special characters */
    o_tty.c_cc[VTIME] = 10;

    o_tty.c_cflag &= ~PARENB;
    o_tty.c_cflag &= ~CSTOPB;
    o_tty.c_cflag &= ~CSIZE;
    o_tty.c_cflag |= CS8;

//    if ( tcsetattr(hSerial, TCSANOW, &o_tty) != 0 ) { printf("tcsetattr: %s\n", strerror(errno)); }
    tcsetattr(hSerial, TCSANOW, &o_tty);

    int bContinue = 1;
    char strInput[cBUF_SIZE];
    int iSelection;

    while (bContinue)
    {
        print_menu(&iSelection);
        switch (iSelection) {
            case 'o':
            {
                int iBuffOutSize = 0;
                sprintf(chBuffOut, "%s\r\n", chCmd_LED_ON);
                iBuffOutSize = strlen(chBuffOut);

                // printf("LED ON size (8)  %d  %s\n", iBuffOutSize, chBuffOut);
                int n_written = write( hSerial, chBuffOut, iBuffOutSize);
            }
                break;

            case 'f':
            {
                int iBuffOutSize = 0;
                sprintf(chBuffOut, "%s\r\n", chCmd_LED_OFF);
                iBuffOutSize = strlen(chBuffOut);

                int n_written = write( hSerial, chBuffOut, iBuffOutSize);

            }
                break;

            case 'r':
            {
                int iBuffOutSize = 0;
                sprintf(chBuffOut, "%s\r\n", chCmd_BUTTON_STATUS);
                iBuffOutSize = strlen(chBuffOut);

                int n_written = write( hSerial, chBuffOut, iBuffOutSize);

                usleep (1000*1000); // time in nanoseconds

                char chArrBuf [256];
                memset (chBuffIn , '\0', cBUF_SIZE);
                int n = read( hSerial, chBuffIn , cBUF_SIZE );
                printf("Recieved data (%d) %s\n", n, chBuffIn);

            }
                break;

            case 'w':
            {
                memset (chBuffIn, '\0', cBUF_SIZE);
                int n = read( hSerial, chBuffIn , cBUF_SIZE);
                printf("Recieved data (%d) %s\n", n, chBuffIn);
            }
                break;

            case 'e':
            {
                bContinue = 0;
            }
                break;

            default:
            {
                printf("wrong option\n");
            }
                break;
        }   // end switch case
    }       // end while loop

    close(hSerial);  // close serial port

    return 0;
}

