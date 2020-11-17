#include "stdlib.h"
#include "stdio.h"
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>


int main(int argc, char *argv[])
{

        int hSerial = open( "/dev/ttyACM0", O_RDWR| O_NONBLOCK | O_NDELAY );
        fcntl(hSerial, F_SETFL, 0);
        // h = handle

        int iRetVal;

        struct termios o_tty;
        // o = object
        memset (&o_tty, 0, sizeof(o_tty) );
        iRetVal = tcgetattr (hSerial , &o_tty);

        cfsetispeed(&o_tty, B9600);
        cfsetospeed(&o_tty, B9600);

        /* set raw input, 1 second timeout */
        o_tty.c_cflag |= (CLOCAL | CREAD);
        o_tty.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
        o_tty.c_oflag &= ~OPOST;
        o_tty.c_cc[VMIN] = 0;
        o_tty.c_cc[VTIME] = 10;

        o_tty.c_cflag &= ~PARENB;
        o_tty.c_cflag &= ~CSTOPB;
        o_tty.c_cflag &= ~CSIZE;


        /* set the options */
        tcsetattr(hSerial, TCSANOW, &o_tty);

        int bContinue = 1;
        char strInput[255];
        while (bContinue)
        {
                printf("Selection: ");
                scanf ("%s", strInput);
                printf("\n");

                char chSendChar = 'o';
                int n_written = write( hSerial, &chSendChar, 1);
        }
        

        close(hSerial);

        return 0;
}

