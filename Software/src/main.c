#include <stdio.h>
#include <stdlib.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include "NetTools.h"
#include "SSD1306_IIC.h"
#include "DevInfo.h"
#include "UserCfg.h"

#define TEMP_STR_LEN    64
#define IP_STR_LEN      20
#define TO_GB(b)        ((b) / pow(1024, 3))
#define TO_SEC(h, m)    ((h) * 3600 + (m) * 60)


extern uint8_t WIFI_CONNECT[32];
extern uint8_t ETH_CONNECT[32];
extern uint8_t NET_ERROR[32];

time_t timep;
struct tm* myTm;
uint8_t inited = 1;

#if ENABLE_RUNNING_PERIOD
int BEG_SEC = TO_SEC(BEG_H, BEG_M);
int END_SEC = TO_SEC(END_H, END_M);
int CUR_SEC = 0;
#endif //ENABLE_RUNNING_PERIOD

char ipStr[IP_STR_LEN];
char tempStr[TEMP_STR_LEN];
unsigned int freeMem, totalMem;
long int start_rcv_rates = 0;   //保存开始时的流量计数
long int end_rcv_rates = 0;	    //保存结束时的流量计数
long int start_tx_rates = 0;    //保存开始时的流量计数
long int end_tx_rates = 0;      //保存结束时的流量计数
float tx_rates = 0;             //上传速度Bytes/s
float rx_rates = 0;             //下载速度Bytes/s
CPU_OCCUPY cpu_stat1;           //用于CPU利用率计算
CPU_OCCUPY cpu_stat2;
float cpuUsage;
Net_State wlanState;
Net_State ethState;
char netSpeedUnit[4][5] = {
    "B/s",
    "KB/s",
    "MB/s",
    "GB/s",
};

void Work();

int main(int argc, char* argv[])
{
    SSD1306_Init();

    /*显示开机画面, 重启不显示*/
    if (!(argc >= 2 && !strcmp(argv[1], "-r")))
    {
        SSD1306_FillRect2(2, 0, 5, 128, White);
        SSD1306_PutString(21, 2, "OrangePi 4 LTS", MF_6x8, White);
        SSD1306_PutString(15, 16, "AQin", MF_16x26, White);
        SSD1306_PutString(90, 25, "</>", MF_11x18, White);
        SSD1306_PutString(15, 52, "Ubuntu 20.04.6", MF_6x8, White);
        SSD1306_UpdateScreen();
        sleep(3);
    }
    
    while (1)
    {
#if ENABLE_RUNNING_PERIOD
        //是否需要停止显示
        time(&timep);
        myTm = localtime(&timep);
        CUR_SEC = TO_SEC(myTm->tm_hour, myTm->tm_min);

        //printf("BEG: %d\tCUR: %d\tEND: %d\n", BEG_SEC, CUR_SEC, END_SEC);

        if (BEG_SEC <= CUR_SEC && CUR_SEC <= END_SEC)
        {
            inited = 1;
        }
        else if (inited)
        {
            //不在规定时间内，且初始化的，说明屏幕亮着，该关了
            inited = 0;
            SSD1306_ClearScreen();
            SSD1306_UpdateScreen();
        }
        

        if (inited)
        {
#endif //ENABLE_RUNNING_PERIOD
            Work();
            GetCurNetFlow(wlanState == STATE_CONNECT ? WLAN_IF : (ethState == STATE_CONNECT ? ETH_IF : NULL),
                        &start_rcv_rates, &start_tx_rates);
            get_cpuoccupy((CPU_OCCUPY *)&cpu_stat1);
            sleep(REFRESH_TIME);
            get_cpuoccupy((CPU_OCCUPY *)&cpu_stat2);
            GetCurNetFlow(wlanState == STATE_CONNECT ? WLAN_IF : (ethState == STATE_CONNECT ? ETH_IF : NULL),
                            &end_rcv_rates, &end_tx_rates);

            //计算CPU利用率
            cpuUsage = cal_cpuoccupy((CPU_OCCUPY *)&cpu_stat1,(CPU_OCCUPY *)&cpu_stat2);
            //计算网速
            rx_rates = (float)(end_rcv_rates - start_rcv_rates) / REFRESH_TIME;
            tx_rates = (float)(end_tx_rates - start_tx_rates) / REFRESH_TIME;
#if ENABLE_RUNNING_PERIOD
        }
        else
        {
            sleep(10);
        }
#endif //ENABLE_RUNNING_PERIOD
        
    }
    
}

void Work()
{
    SSD1306_ClearScreen();
    /*显示网格*/
    SSD1306_DrawLine(0, 0, 127, 0, White);      //顶部横线
    SSD1306_DrawLine(0, 15, 127, 15, White);    //IP地址下面的线
    SSD1306_DrawLine(82, 15, 82, 38, White);    //CPU信息和温度分割线
    SSD1306_DrawLine(0, 38, 127, 38, White);    //蓝色区域中心分割线
    SSD1306_DrawLine(0, 63, 127, 63, White);    //最底下的线
    SSD1306_DrawLine(37, 39, 37, 62, White);    //网络信息和磁盘信息分割线

    /*显示网络状态*/
    wlanState = GetWirelessState();
    ethState = GetEthernetState();

    if (wlanState == STATE_CONNECT)
    {
        SSD1306_DrawBitMap(0, 0, WIFI_CONNECT, 16, 16, White);
        memset(ipStr, 0, IP_STR_LEN);
        int ret = 0;
        ret = GetLocalIP(WLAN_IF, ipStr);
        if (ret != 0)   memset(ipStr, 0, IP_STR_LEN);
        SSD1306_PutString(17, 4, ipStr, MF_7x10, White);
    }
    else if (ethState == STATE_CONNECT)
    {
        SSD1306_DrawBitMap(0, 0, ETH_CONNECT, 16, 16, White);
        memset(ipStr, 0, IP_STR_LEN);
        int ret = 0;
        ret = GetLocalIP(ETH_IF, ipStr);
        if (ret != 0)   memset(ipStr, 0, IP_STR_LEN);
        SSD1306_PutString(17, 4, ipStr, MF_7x10, White);
    }
    else
    {
        SSD1306_DrawBitMap(0, 0, NET_ERROR, 16, 16, White);
    }

    /*显示CPU利用率*/
    memset(tempStr, 0, TEMP_STR_LEN);
    sprintf(tempStr, "CPU: %.1f%%", cpuUsage);
    SSD1306_PutString(0, 17, tempStr, MF_7x10, White);

    /*显示内存利用率*/
    freeMem = GetMemFree();
    totalMem = GetMemTotal();

    memset(tempStr, 0, TEMP_STR_LEN);
    sprintf(tempStr, "Mem:%.1lf %d%%",
            TO_GB(totalMem - freeMem),
            (int)((totalMem - freeMem) * 1.0 / totalMem * 100));
    SSD1306_PutString(0, 28, tempStr, MF_7x10, White);

    /*显示温度*/
    memset(tempStr, 0, TEMP_STR_LEN);
    SSD1306_PutString(91, 17, "Temp", MF_7x10, White);
    sprintf(tempStr, "%.2f", GetCpuTemp());
    SSD1306_PutString(88, 28, tempStr, MF_7x10, White);

    /*显示网速*/
    uint8_t rxUnitLevel = 0;
    uint8_t txUnitLevel = 0;
    while (tx_rates >= 1000)
    {
        tx_rates /= 1000;
        txUnitLevel++;
    }
    while (rx_rates >= 1000)
    {
        rx_rates /= 1000;
        rxUnitLevel++;
    }
    //显示Tx
    memset(tempStr, 0, TEMP_STR_LEN);
    sprintf(tempStr, " Up :%.1lf %s",
            tx_rates,
            netSpeedUnit[txUnitLevel]);
    SSD1306_PutString(40, 42, tempStr, MF_6x8, White);
    //显示Rx
    memset(tempStr, 0, TEMP_STR_LEN);
    sprintf(tempStr, "Down:%.1lf %s",
            rx_rates,
            netSpeedUnit[rxUnitLevel]);
    SSD1306_PutString(40, 52, tempStr, MF_6x8, White);

    /*显示磁盘剩余容量*/
    SSD1306_PutString(7, 42, "Disk", MF_6x8, White);
    memset(tempStr, 0, TEMP_STR_LEN);
    sprintf(tempStr, "%.2f", (float)GetAvailDisk() / 1000);
    SSD1306_PutString(4, 52, tempStr, MF_6x8, White);

    SSD1306_UpdateScreen();
}
