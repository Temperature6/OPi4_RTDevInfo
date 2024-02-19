# 实时设备信息显示（以OrangePi 4 LTS为例）

![](./rtdevinfo.jpg)

## 简介

> 把stm32上的SSD1306驱动移植到了OPi4上，然后百度还有学习着写了一些获取设备信息的代码
>

能显示的信息包括：

**网络是否链接**（图标）、**IP地址**、**CPU使用率**（CPU）、**已用内存和使用率**（Mem）**CPU温度**（Temp）**磁盘剩余容量**（Disk）**上下行网速**（Up/Down）



## 如何使用（以香橙派zero3为例）

**0）硬件准备**

​	1.一块Linux开发板，安装了香橙派的wiringOP或者树莓派的wiringPi

​	2.一个使用四线I2C通信的SSD1306驱动芯片的128*64像素OLED小屏幕

​	3.杜邦线若干，或使用本仓库提供的小转接板（在Hardware目录下，是一个立创EDA工程）

**1）检查一下I2C外设的位置**

```
user@orangepizero3:~$ gpio readall
 +------+-----+----------+------+---+   H616   +---+------+----------+-----+------+
 | GPIO | wPi |   Name   | Mode | V | Physical | V | Mode | Name     | wPi | GPIO |
 +------+-----+----------+------+---+----++----+---+------+----------+-----+------+
 |      |     |     3.3V |      |   |  1 || 2  |   |      | 5V       |     |      |
 |  229 |   0 |    SDA.3 | ALT5 | 0 |  3 || 4  |   |      | 5V       |     |      |
 |  228 |   1 |    SCL.3 | ALT5 | 0 |  5 || 6  |   |      | GND      |     |      |
 |   73 |   2 |      PC9 |  OFF | 0 |  7 || 8  | 0 | OFF  | TXD.5    | 3   | 226  |
 |      |     |      GND |      |   |  9 || 10 | 0 | OFF  | RXD.5    | 4   | 227  |
 |   70 |   5 |      PC6 | ALT5 | 0 | 11 || 12 | 0 | OFF  | PC11     | 6   | 75   |
 |   69 |   7 |      PC5 |  OUT | 1 | 13 || 14 |   |      | GND      |     |      |
 |   72 |   8 |      PC8 |  OUT | 1 | 15 || 16 | 0 | OFF  | PC15     | 9   | 79   |
 |      |     |     3.3V |      |   | 17 || 18 | 0 | OFF  | PC14     | 10  | 78   |
 |  231 |  11 |   MOSI.1 | ALT4 | 0 | 19 || 20 |   |      | GND      |     |      |
 |  232 |  12 |   MISO.1 | ALT4 | 0 | 21 || 22 | 1 | OUT  | PC7      | 13  | 71   |
 |  230 |  14 |   SCLK.1 | ALT4 | 0 | 23 || 24 | 0 | ALT4 | CE.1     | 15  | 233  |
 |      |     |      GND |      |   | 25 || 26 | 1 | OUT  | PC10     | 16  | 74   |
 |   65 |  17 |      PC1 |  OFF | 0 | 27 || 28 |   |      |          |     |      |
 |  272 |  18 |     PI16 | ALT2 | 0 | 29 || 30 |   |      |          |     |      |
 |  262 |  19 |      PI6 |  OFF | 0 | 31 || 32 |   |      |          |     |      |
 |  234 |  20 |     PH10 | ALT3 | 0 | 33 || 34 |   |      |          |     |      |
 +------+-----+----------+------+---+----++----+---+------+----------+-----+------+
 | GPIO | wPi |   Name   | Mode | V | Physical | V | Mode | Name     | wPi | GPIO |
 +------+-----+----------+------+---+   H616   +---+------+----------+-----+------+

```

**2）将四线I2C的OLED小屏幕连接到某个I2C外设上，例如香橙派Zero3的I2C.3**

> OLED   香橙派
>
> VCC    -   3.3V
>
> GND   -   GND
>
> SDA   -   SDA.3
>
> SCL    -   SCL.3

**3）检查设备是否打开 I2C.3**

```
user@orangepizero3:~$ ls /dev/i2c*
/dev/i2c-3  /dev/i2c-4  /dev/i2c-5
```

如果有/dev/i2c-3，说明已经开启

**4）检查屏幕是否正常连接**

安装i2c-tools，如果已安装可以跳过

```
sudo apt-get update
sudo apt-get install -y i2c-tools
```

屏幕连接的是i2c-3，所以检查i2c-3下面是否有设备

```
user@orangepizero3:~$ sudo i2cdetect -y 3
     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f
00:                         -- -- -- -- -- -- -- --
10: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
20: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
30: -- -- -- -- -- -- -- -- -- -- -- -- 3c -- -- --
40: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
50: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
60: -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
70: -- -- -- -- -- -- -- --
```

出现 3c 说明连接正常

**5）克隆本仓库**

```
git clone https://github.com/Temperature6/OPi4_RTDevInfo.git
```

**6）进入文件夹**

```
cd OPi4_RTDevInfo/
```

**7）编辑i2c配置**

一般来说，只需要更改i2c配置即可，其他由用户自行决定

```
vim Software/inc/UserCfg.h
```

文件内容如下

```C
/*无线网卡文件名*/
#define WLAN_IF         "wlan0"
/*有线网卡文件名*/
#define ETH_IF          "eth0"
/*i2c设备文件名*/
#define LINUX_IIC_FILE  "/dev/i2c-3"
/*刷新时间(s)*/
#define REFRESH_TIME    1

/*是否启用运行时间控制: 若启用，屏幕只会在固定时间点亮*/
#define ENABLE_RUNNING_PERIOD   0

#if ENABLE_RUNNING_PERIOD
#define BEG_H           7   //开始运行时
#define BEG_M           00  //开始运行分
#define END_H           23  //结束运行时
#define END_M           00  //结束运行分
#endif //ENABLE_RUNNING_PERIOD

#define WIRELESS    "/sys/class/net/"WLAN_IF"/operstate"
#define ETHERNET    "/sys/class/net/"ETH_IF"/operstate"

```

将

```
#define LINUX_IIC_FILE  "/dev/i2c-3"
```

改为需要的文件地址，由于本次连接的正好也是i2c3，不需要更改

如果需要更改其他值，更改完成后退出编辑器即可

**8）构建**

```
cd Software/
make
```

如果没有报错，此时ls一下就可以看到编译出来的文件

```
user@orangepizero3:~/OPi4_RTDevInfo/Software$ ls
inc  Makefile  obj  RTDevInfo  src
```

RTDevInfo就是可执行文件，可以运行测试，程序设计硬件操作，需要使用sudo权限

```
sudo ./RTDevInfo
```

没用问题的话，屏幕会成功点亮，程序默认有个开机画面，如果不需要，可以在main.c中删去

## 结束

我也是个小白，部分代码参考网上的文章，侵删。

感谢阅读
