#include "NetTools.h"
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
#include "UserCfg.h"

#define MAX_LEN             32
#define CON_STR             "up\n"
#define DIS_STR             "down\n"
#define BUFFER_LEN          1024

char tempBuf[MAX_LEN];
char buffer[BUFFER_LEN] = "";   //文件中的内容暂存在字符缓冲区里

int ReadContext(const char* filePath, char* buf, int bufLen);

Net_State GetWirelessState()
{
    char state[MAX_LEN] = "";
    if (ReadContext(WIRELESS, state, MAX_LEN) == 0)
    {
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
    return STATE_FAIL;
}

Net_State GetEthernetState()
{
    char state[MAX_LEN] = "";
    if (ReadContext(ETHERNET, state, MAX_LEN) == 0)
    {
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

    return STATE_FAIL;
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

int ReadContext(const char* filePath, char* buf, int bufLen)
{
    FILE* fp;
    fp = fopen(filePath, "r");
    if (fp == NULL)
    {
#ifdef ENABLE_LOG
        printf("Can't Open File \"%s\"\n", filePath);  
#endif //ENABLE_LOG
        return -1;
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
            return -1;
        }
        fclose(fp);
    }
    
    return 0;
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

int GetCurNetFlow(char* netname, long int* rx_rate, long int* tx_rate)
{
    FILE* fp = NULL;     //文件指针
    memset(buffer, 0, BUFFER_LEN);
    char* match = NULL;
    
    //错误参数检查
    if((NULL == netname) || (NULL == rx_rate) || (NULL == tx_rate))
    {
        printf("bad param\n");
        return -1;    
    }
    //文件打开失败检查
    if ((fp = fopen("/proc/net/dev", "r")) == NULL)
    {
        printf("Can't Open File /proc/net/dev/\n");
        return -1;
    }
    //读文件
    if (fread(buffer, 1, BUFFER_LEN, fp) < 200)
    {
        fclose(fp);
        return -1;
    }
    else
    {
        fclose(fp);
        match = strstr(buffer, netname);
        
        if (match == NULL)
        {
            return -1;
        }
        else
        {
            match += (strlen(netname) + 1);
            sscanf(match, "%ld %*d %*d %*d %*d %*d %*d %*d %ld", rx_rate, tx_rate);
        }
    }
    return 0;/*返回成功*/
}
