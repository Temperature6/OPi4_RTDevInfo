#ifndef NET_STATE_H
#define NET_STATE_H

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

#endif //NET_STATE_H
