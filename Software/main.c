#include <stdio.h>
#include <stdlib.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <stdint.h>
#include <unistd.h>
#include "NetState.h"
#include "SSD1306/SSD1306_IIC.h"
#include "CPU_Info.h"

#define TEMP_STR_LEN    64
#define IP_STR_LEN      20
#define WLAN_IF         "wlan0"
#define ETH_IF          "eth0"
#define TO_GB(b)        ((b) / pow(1024, 3))
#define CALC_NET_SPEED  WLAN_IF
#define REFRESH_TIME    1

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

int GetCurNetFlow(char* netname, long int* rx_rate, long int* tx_rate);
void Work();

int main()
{
    //printf("NET:%d\n", GetNetState());
    SSD1306_Init();
    while (1)
    {
        Work();
        GetCurNetFlow(CALC_NET_SPEED, &start_rcv_rates, &start_tx_rates);
        sleep(REFRESH_TIME);
        GetCurNetFlow(CALC_NET_SPEED, &end_rcv_rates, &end_tx_rates);
        rx_rates = (float)(end_rcv_rates - start_rcv_rates) / REFRESH_TIME;
        tx_rates = (float)(end_tx_rates - start_tx_rates) / REFRESH_TIME;
        //break;
    }
    
}

void Work()
{
    SSD1306_ClearScreen();
    /*显示网格*/
    SSD1306_DrawLine(0, 0, 127, 0, White);
    SSD1306_DrawLine(0, 15, 127, 15, White);
    SSD1306_DrawLine(82, 15, 82, 38, White);
    SSD1306_DrawLine(0, 38, 127, 38, White);
    SSD1306_DrawLine(0, 63, 127, 63, White);

    /*显示网络状态*/
    Net_State wlanState = GetWirelessState();
    Net_State ethState = GetEthernetState();

    if (wlanState == STATE_CONNECT || ethState == STATE_CONNECT)
    {
        SSD1306_DrawBitMap(0, 0, WIFI_CONNECT, 16, 16, White);
        memset(ipStr, 0, IP_STR_LEN);
        if (wlanState == STATE_CONNECT)
            GetLocalIP(WLAN_IF, ipStr);
        else if (ethState == STATE_CONNECT)
            GetLocalIP(ETH_IF, ipStr);
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
    sprintf(tempStr, " Up :%.1lf %s",
            tx_rates,
            netSpeedUnit[txUnitLevel]);
    SSD1306_PutString(0, 42, tempStr, MF_6x8, White);
    //显示Rx
    sprintf(tempStr, "Down:%.1lf %s",
            rx_rates,
            netSpeedUnit[rxUnitLevel]);
    SSD1306_PutString(0, 52, tempStr, MF_6x8, White);

    SSD1306_UpdateScreen();
}

int GetCurNetFlow(char* netname, long int* rx_rate,long int* tx_rate)
{
    FILE* net_dev_file;     //文件指针
    char buffer[1024];      //文件中的内容暂存在字符缓冲区里
    //size_t bytes_read;      //实际读取的内容大小
    char* match;            //用以保存所匹配字符串及之后的内容
    int counter = 0;
    int i = 0;
    char tmp_value[128];

    if((NULL == netname) || (NULL == rx_rate) || (NULL == tx_rate))
    {
        printf("bad param\n");
        return -1;    
    }

    if ((net_dev_file=fopen("/proc/net/dev", "r")) == NULL)     //打开文件/pro/net/dev/，我们要读取的数据就是它啦
    {
        printf("open file /proc/net/dev/ error!\n");
        return -1;
    }
    memset(buffer,0,sizeof(buffer));

    while(fgets(buffer,sizeof(buffer),net_dev_file) != NULL)
    {
        match = strstr(buffer,netname);

        if(NULL == match)
        {
            continue;
        }
        else
        {
            match = match + strlen(netname) + strlen(":");/*地址偏移到冒号*/
            sscanf(match,"%ld ",rx_rate);
            memset(tmp_value,0,sizeof(tmp_value));
            sscanf(match,"%s ",tmp_value);
            match = match + strlen(tmp_value);
            for(i=0;i<strlen(buffer);i++)
            {
                if(0x20 == *match)
                {
                    match ++;
                }
                else
                {
                    if(8 == counter)
                    {
                        sscanf(match,"%ld ",tx_rate);
                    }
                    memset(tmp_value,0,sizeof(tmp_value));
                    sscanf(match,"%s ",tmp_value);
                    match = match + strlen(tmp_value);
                    counter ++;
                }
            }
        }
    }

    return 0;/*返回成功*/
}
