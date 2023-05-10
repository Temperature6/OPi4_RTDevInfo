#include "NetState.h"
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <fcntl.h>  
#include <sys/types.h>  
#include <sys/stat.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <features.h>
//#define __USE_MISC

#define MAX_LEN     32
#define WIRELESS    "/sys/class/net/wlan0/operstate"
#define ETHERNET    "/sys/class/net/eth0/operstate"
#define CON_STR     "up\n"
#define DIS_STR     "down\n"
#define IF_NAME     "eth0"

char tempBuf[MAX_LEN];

void ReadContext(const char* filePath, char* buf, int bufLen);

Net_State GetWirelessState()
{
    char state[MAX_LEN] = "";
    ReadContext(WIRELESS, state, MAX_LEN);

    if (!strcmp(state, CON_STR))
    {
        return STATE_CONNECT;
    }
    else if (!strcmp(state, DIS_STR))
    {
        return STATE_DISCONNECT;
    }
    else
    {
        return STATE_FAIL;
    }
}

Net_State GetEthernetState()
{
    char state[MAX_LEN] = "";
    ReadContext(ETHERNET, state, MAX_LEN);

    if (!strcmp(state, CON_STR))
    {
        return STATE_CONNECT;
    }
    else if (!strcmp(state, DIS_STR))
    {
        return STATE_DISCONNECT;
    }
    else
    {
        return STATE_FAIL;
    }
}

Net_State GetNetState()
{
    if (GetWirelessState() == STATE_CONNECT || GetEthernetState() == STATE_CONNECT)
    {
        return STATE_CONNECT;
    }
    else
    {
        return STATE_DISCONNECT;
    }
}

void ReadContext(const char* filePath, char* buf, int bufLen)
{
    FILE* fp;
    fp = fopen(filePath, "r");
    if (fp == NULL)
    {
#ifdef ENABLE_LOG
        printf("Can't Open File \"%s\"\n", filePath);
#endif //ENABLE_LOG
    }
    else
    {
        memset(tempBuf, 0, MAX_LEN);
        fread(tempBuf, 1, MAX_LEN, fp);

        if (strlen(tempBuf) < bufLen)
        {
            memcpy(buf, tempBuf, strlen(tempBuf));
        }
        else
        {
#ifdef ENABLE_LOG
            printf("Dst Buffer Too Small\n");
#endif //ENABLE_LOG
        }
        
    }
    fclose(fp);
}

int GetLocalIP(char * ifname, char * ip)
{
    char *temp = NULL;
    int inet_sock;
    struct ifreq ifr;

    inet_sock = socket(AF_INET, SOCK_DGRAM, 0); 

    memset(ifr.ifr_name, 0, sizeof(ifr.ifr_name));
    memcpy(ifr.ifr_name, ifname, strlen(ifname));

    if(0 != ioctl(inet_sock, SIOCGIFADDR, &ifr)) 
    {   
        perror("ioctl error");
        return -1;
    }

    temp = inet_ntoa(((struct sockaddr_in*)&(ifr.ifr_addr))->sin_addr);     
    memcpy(ip, temp, strlen(temp));

    close(inet_sock);

    return 0;
}
