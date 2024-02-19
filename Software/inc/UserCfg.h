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
