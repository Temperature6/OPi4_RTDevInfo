#include <wiringPiI2C.h>
#include <sys/ioctl.h>
#include <asm/ioctl.h>
#include <stdint.h>
#include <fcntl.h>
 
typedef unsigned char uchar;
typedef unsigned short ushort;

int wiringPiI2CRedRegs (int fd, int reg, uchar *buf, int size);
int wiringPiI2CWirteRegs (int fd, int reg, uchar *buf, int size);