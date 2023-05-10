#include <stdio.h>
#include <stdlib.h>
#include <wiringPiI2C.h>
#include <wiringPi.h>
#include <stdint.h>
#include <unistd.h>
#include "NetState.h"
#include "SSD1306/SSD1306_IIC.h"
#include "CPU_Info.h"

#define TEMP_STR_LEN    32
#define IP_STR_LEN      20
#define WLAN_IF         "wlan0"
#define ETH_IF          "eth0"
#define TO_GB(b)        ((b) / pow(1024, 3))


extern uint8_t WIFI_CONNECT[32];
extern uint8_t WIFI_DISCONNECT[32];
extern uint8_t WIFI_ERROR[32];
char ipStr[IP_STR_LEN];
char tempStr[TEMP_STR_LEN];
unsigned int freeMem, totalMem;



void Work();

int main()
{
    //printf("NET:%d\n", GetNetState());
    SSD1306_Init();
    while (1)
    {
        Work();
        sleep(1);
        //break;
    }
    
}

void Work()
{
    SSD1306_ClearScreen();
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
    sprintf(tempStr, "CPU: %.1f %%", GetCpuUsage());
    SSD1306_PutString(0, 17, tempStr, MF_7x10, White);

    /*显示内存利用率*/
    freeMem = GetMemFree();
    totalMem = GetMemTotal();

    memset(tempStr, 0, TEMP_STR_LEN);
    sprintf(tempStr, "Mem:%.1lf/%.1lfGB %d%%",
            TO_GB(totalMem - freeMem),
            TO_GB(totalMem),
            (int)((totalMem - freeMem) * 1.0 / totalMem * 100));
    SSD1306_PutString(0, 17 * 2, tempStr, MF_7x10, White);

    /*显示温度*/
    memset(tempStr, 0, TEMP_STR_LEN);
    sprintf(tempStr, "Temp: %.2f C", GetCpuTemp());
    SSD1306_PutString(0, 17 * 3, tempStr, MF_7x10, White);

    SSD1306_UpdateScreen();
}
