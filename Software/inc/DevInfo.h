#ifndef CPU_INFO_H
#define CPU_INFO_H

typedef struct
{
    char name[20];  
    unsigned int user;
    unsigned int nice;
    unsigned int system;
    unsigned int idle;
}CPU_OCCUPY;

float GetCpuUsage();
unsigned long GetMemFree();
unsigned long GetMemTotal();
float GetCpuTemp();
long long GetAvailDisk();
float cal_cpuoccupy(CPU_OCCUPY *o,CPU_OCCUPY *n);
void get_cpuoccupy (CPU_OCCUPY *cpust);

#endif //CPU_INFO_H
