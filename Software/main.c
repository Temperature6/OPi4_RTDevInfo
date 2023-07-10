#include <stdio.h>
#include <stdlib.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include "NetTools.h"
#include "SSD1306/SSD1306_IIC.h"
#include "DevInfo.h"

#define TEMP_STR_LEN    64
#define IP_STR_LEN      20
#define WLAN_IF         "wlan0"
#define ETH_IF          "eth0"
#define TO_GB(b)        ((b) / pow(1024, 3))
#define CALC_NET_SPEED  WLAN_IF
#define REFRESH_TIME    1
#define NETFLOW_BADOPEN 0x08


int badOpen = 0;
extern uint8_t WIFI_CONNECT[32];
extern uint8_t WIFI_DISCONNECT[32];
extern uint8_t WIFI_ERROR[32];

char ipStr[IP_STR_LEN];
char tempStr[TEMP_STR_LEN];
unsigned int freeMem, totalMem;
long int start_rcv_rates = 0;   //保存开始时的流量计数
long int end_rcv_rates = 0;	    //保存结束时的流量计数
long int start_tx_rates = 0;    //保存开始时的流量计数
long int end_tx_rates = 0;      //保存结束时的流量计数
float tx_rates = 0;             //上传速度Bytes/s
float rx_rates = 0;             //下载速度Bytes/s
char netSpeedUnit[4][5] = {
    "B/s",
    "KB/s",
    "MB/s",
    "GB/s",
};

void Work();

int main(int argc, char* argv[])
{
    //printf("NET:%d\n", GetNetState());
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
        sleep(4);
    }
    
    badOpen = 0;
    while (1)
    {
        Work();
        GetCurNetFlow(CALC_NET_SPEED, &start_rcv_rates, &start_tx_rates);
        sleep(REFRESH_TIME);
        GetCurNetFlow(CALC_NET_SPEED, &end_rcv_rates, &end_tx_rates);
        rx_rates = (float)(end_rcv_rates - start_rcv_rates) / REFRESH_TIME;
        tx_rates = (float)(end_tx_rates - start_tx_rates) / REFRESH_TIME;

        if (badOpen != 0)
        {
            printf("error: BadOpen\n");
            SSD1306_ClearScreen();
            SSD1306_UpdateScreen();
            //char procCmd[255] = "";
            //sprintf(procCmd, "%s -r &", argv[0]);
            //system(procCmd);
            return badOpen;
        }
        //break;
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
    Net_State wlanState = GetWirelessState();
    Net_State ethState = GetEthernetState();

    if (wlanState == STATE_CONNECT || ethState == STATE_CONNECT)
    {
        SSD1306_DrawBitMap(0, 0, WIFI_CONNECT, 16, 16, White);

        memset(ipStr, 0, IP_STR_LEN);
        int ret = 0;

        if (wlanState == STATE_CONNECT)
            ret = GetLocalIP(WLAN_IF, ipStr);
        else if (ethState == STATE_CONNECT)
            ret = GetLocalIP(ETH_IF, ipStr);

        if (ret != 0)   memset(ipStr, 0, IP_STR_LEN);
        SSD1306_PutString(17, 4, ipStr, MF_7x10, White);
    }
    else if (wlanState == STATE_DISCONNECT && ethState == STATE_DISCONNECT)
    {
        SSD1306_DrawBitMap(0, 0, WIFI_DISCONNECT, 16, 16, White);
    }
    else
    {
        SSD1306_DrawBitMap(0, 0, WIFI_ERROR, 16, 16, White);
    }

    /*显示CPU利用率*/
    memset(tempStr, 0, TEMP_STR_LEN);
    sprintf(tempStr, "CPU: %.1f%%", GetCpuUsage());
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
