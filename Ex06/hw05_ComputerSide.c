#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <termios.h>    // POSIX terminal control definitions
#include <fcntl.h>      // File controls like O_RDWR from open function
#include <unistd.h>     // write(), read(), open() etc.

#define cBUF_SIZE 255
void call_termios(int reset);

char * g_strArrMenu[] =
        {       "==Program Menu==",
                "Item 'o': LED ON",
                "Item 'f': LED OFF",
                "Item 'b': Button State",
                "Item 'c': Enter a custom command.",
                "Item 'e': Exit"
        };

char chCmd_LED_ON[]  = {'L', 'E', 'D', ' ', 'O', 'N', '\0'};    // both ways get same result bc strings are 1D arrays in C
char chCmd_LED_OFF[] = "LED OFF";                               // '\0' character will automatically be added by compiler
char chCmd_BUTTON_STATUS[] = "BUTTON:STATUS?";
char chCmd_CUSTOM_COMMAND[cBUF_SIZE];

char chBuffOut[cBUF_SIZE];
char chBuffIn[cBUF_SIZE];


void print_menu(void)
{
    for (int iCnt = 0; iCnt < (sizeof(g_strArrMenu)/sizeof(char*)); iCnt++ )
    {
        printf("%s\n", g_strArrMenu[iCnt]);
    }
    printf("Selection: \n");
}

int main(int argc, char *argv[])
{
    if (argc > 1) {
            char *port = argv[1];
            printf("%s\n", port);
        }
    else {
        fprintf(stderr, "Main: serial port path required\n");
        exit(1);
    }

    int hSerial = open( "/dev/ttyACM0", O_RDWR| O_NONBLOCK | O_NDELAY );    // serial port open
    if (hSerial < 0) {  printf("tcgetattr: %s\n", strerror(errno)); }

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

    if ( tcsetattr(hSerial, TCSANOW, &o_tty) != 0 ) { printf("tcsetattr: %s\n", strerror(errno)); }
    tcsetattr(hSerial, TCSANOW, &o_tty);

    int bContinue = 1;
    char strInput[cBUF_SIZE];
    int iSelection, c;

    void (*set)(int reset) = 0;
    set = call_termios;
    if (set) { (*set)(0); }

    print_menu();

    while (bContinue)
    {
        c = getchar();
        switch (c) {
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

            case 'b':
            {
                int iBuffOutSize = 0;
                sprintf(chBuffOut, "%s\r\n", chCmd_BUTTON_STATUS);
                iBuffOutSize = strlen(chBuffOut);

                int n_written = write( hSerial, chBuffOut, iBuffOutSize);

                usleep (1000*1000); // time in nanoseconds

                memset (chBuffIn , '\0', cBUF_SIZE);
                int n = read( hSerial, chBuffIn , cBUF_SIZE );
                printf("Recieved data: %s\n", chBuffIn);

            }
                break;

            case 'c':
            {       // set chBuffIn to NULL (entire array is NULL chars) // n is number of bytes read

                printf ("Enter your custom command: ");
                scanf("%s", chCmd_CUSTOM_COMMAND);

                int iBuffOutSize = 0;
                sprintf(chBuffOut, "%s\r\n", chCmd_CUSTOM_COMMAND);
                iBuffOutSize = strlen(chBuffOut);

                int n_written = write( hSerial, chBuffOut, iBuffOutSize);

                usleep (1000*1000); /* wait for nucleo to reply (time in nanoseconds) */

                memset (chBuffIn , '\0', cBUF_SIZE);
                int n = read( hSerial, chBuffIn , cBUF_SIZE );
                printf("%s\n", chBuffIn);

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

    if (set) {
        (*set)(1); //revert to the original mode
    }
    return 0;
}

void call_termios(int reset)
{
    static struct termios tio, tioOld;
    tcgetattr(STDIN_FILENO, &tio);
    if (reset) {
        tcsetattr(STDIN_FILENO, TCSANOW, &tioOld);
    } else {
        tioOld = tio; //backup
        cfmakeraw(&tio);
        tio.c_lflag &= ~ECHO; // assure echo is disabled
        tio.c_oflag |= OPOST; // enable output postprocessing
        tcsetattr(STDIN_FILENO, TCSANOW, &tio);
    }
}