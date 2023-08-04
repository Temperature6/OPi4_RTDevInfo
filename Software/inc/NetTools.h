#ifndef NET_TOOLS_H
#define NET_TOOLS_H

#define ENABLE_LOG

typedef enum
{
    STATE_CONNECT = 0,
    STATE_DISCONNECT,
    STATE_FAIL,
}Net_State;

Net_State GetWirelessState();
Net_State GetEthernetState();
Net_State GetNetState();
int GetLocalIP(char * ifname, char * ip);
int GetCurNetFlow(char* netname, long int* rx_rate, long int* tx_rate);

#endif //NET_TOOLS_H
