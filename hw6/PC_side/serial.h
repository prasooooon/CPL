
#ifndef __SERIAL_H__
#define __SERIAL_H__

int serial_init ( char * strDevName);
int serial_close ( int hSerial);
int serial_write ( int hSerial, char * chBuff, int iLen);
int serial_read ( int hSerial, char * chBuff, int iLen);

#endif
