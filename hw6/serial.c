#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

int serial_init ( char * strDevName)
{
    int hSerial = open(strDevName, O_RDWR | O_NONBLOCK | O_NDELAY);
    fcntl(hSerial, F_SETFL, 0);

    if (hSerial <= 0) /* check for opening errors */
    {
        printf("Error from open: %s\n", strerror(errno));
        return -1;
    }

    struct termios o_tty;                       /* structure calls information about serial port settings */
    memset (&o_tty, 0, sizeof(o_tty) );
    int iRetVal = tcgetattr (hSerial , &o_tty); /* write the existing configuration of the serial port */

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

    return hSerial;
}

int serial_close ( int hSerial)
{
    close(hSerial);
    return 0;
}

int serial_write ( int hSerial, char * chBuff, int iLen)
{
//    printf("LED ON size (8) %d %s\n", iLen, chBuff);
    int n_written = write(hSerial, chBuff, iLen);
    return n_written;
}

int serial_read ( int hSerial, char * chBuff, int iLen)
{
    int n = read( hSerial, chBuff , iLen );  // number of bytes read
    return n;
}
