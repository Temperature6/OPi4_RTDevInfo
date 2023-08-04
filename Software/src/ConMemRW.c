#include <wiringPiI2C.h>
#include <sys/ioctl.h>
#include <asm/ioctl.h>
#include <stdint.h>
#include <fcntl.h>
#include "ConMemRW.h"



#define I2C_SMBUS_READ      1
#define I2C_SMBUS_WRITE     0
#define I2C_SMBUS_I2C_BLOCK_DATA    8
#define I2C_SMBUS_BLOCK_MAX    32          /* As specified in SMBus standard */
#define I2C_SMBUS           0x0720      /* SMBus-level access */
 
 
union i2c_smbus_data
{
    uchar  byte ;
    ushort word ;
    uchar  block [I2C_SMBUS_BLOCK_MAX + 2] ;    // block [0] is used for length + one more for PEC
};
struct i2c_smbus_ioctl_data
{
    char read_write ;
    uchar command ;
    int size ;
    union i2c_smbus_data *data ;
} ;
 
static inline int i2c_smbus_access (int fd, char rw, uchar command, int size, union i2c_smbus_data *data)
{
  struct i2c_smbus_ioctl_data args ;
 
  args.read_write = rw ;
  args.command    = command ;
  args.size       = size ;
  args.data       = data ;
 
  return ioctl (fd, I2C_SMBUS, &args) ;
}
 
int wiringPiI2CRedRegs (int fd, int reg, uchar *buf, int size)
{
    union i2c_smbus_data data ;
 
    data.block[0] = size ;
    if (i2c_smbus_access (fd, I2C_SMBUS_READ, reg, I2C_SMBUS_I2C_BLOCK_DATA, &data))
        return -1 ;
    else {
        for (int i =1; i<size+1;i++) {
            buf[i-1] = data.block[i];
        }
        return size;
    }
}

int wiringPiI2CWirteRegs (int fd, int reg, uchar *buf, int size)
{
    union i2c_smbus_data data ;
 
    data.block[0] = size ;
    for (int i =1; i<size+1;i++) {
        data.block[i] = buf[i-1];
    }
    return i2c_smbus_access (fd, I2C_SMBUS_WRITE, reg, I2C_SMBUS_I2C_BLOCK_DATA, &data) ;
}