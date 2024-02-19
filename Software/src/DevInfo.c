#include <stdio.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/statfs.h>
#include "DevInfo.h"

#define TEMP_FILE_PATH  "/sys/class/thermal/thermal_zone0/temp"
#define MAX_SIZE        32

float cal_cpuoccupy(CPU_OCCUPY *o,CPU_OCCUPY *n)
{
    unsigned long od, nd;
    unsigned long id, sd;
    float cpu_use = 0;
    od = (unsigned long)(o->user + o->nice + o->system + o->idle);  //第一次(用户+优先级+系统+空闲)的时间再赋给od
    nd = (unsigned long)(n->user + n->nice + n->system + n->idle);  //第二次（用户+优先级+系统+空闲）的时间再赋给nd

    id = (unsigned long)(n->user - o->user); //用户第一次和第二次的时间之差再赋给id
    sd = (unsigned long)(n->system - o->system); //系统第一次和第二次的时间之差再赋给sd
    if((nd-od) != 0)
        cpu_use = ((sd+id)*100.0)/(nd-od); //((用户+系统)乘100)除(第一次和第二次的时间差)再赋给g_cpu_used
    else 
        cpu_use = 0;
    return cpu_use;
}

void get_cpuoccupy (CPU_OCCUPY *cpust)  //对无类型get函数含有一个形参结构体类弄的指针o
{
    FILE *fd;
    char buff[256] = "";
    CPU_OCCUPY *cpu_occupy = NULL;
    cpu_occupy = cpust;

    fd = fopen("/proc/stat","r");
    if (fd != NULL)
    {
        fgets(buff,sizeof(buff),fd);
        sscanf(buff,"%s %u %u %u %u",
                cpu_occupy->name,
                &cpu_occupy->user,
                &cpu_occupy->nice,
                &cpu_occupy->system,
                &cpu_occupy->idle);
        fclose(fd);
    }
}

float GetCpuUsage()
{
    CPU_OCCUPY cpu_stat1;
    CPU_OCCUPY cpu_stat2;
    float cpu;
    //第一次获取cpu使用情况
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat1);
    sleep(1);
    //第二次获取cpu使用情况
    get_cpuoccupy((CPU_OCCUPY *)&cpu_stat2);
    //计算cpu使用率
    cpu = cal_cpuoccupy((CPU_OCCUPY *)&cpu_stat1,(CPU_OCCUPY *)&cpu_stat2);
    return cpu;
}

unsigned long GetMemTotal()
{
    /*Byte*/
    struct sysinfo s_info;

    if(sysinfo(&s_info) == 0)
    {
        return s_info.totalram;
    }
    else
    {
        return -1;
    }
}

unsigned long GetMemFree()
{
    struct sysinfo s_info;
    if(sysinfo(&s_info) == 0)
    {
        return s_info.freeram;
    }
    else
    {
        return -1;
    }
}

float GetCpuTemp()
{   
    FILE* fp;
    char buf[MAX_SIZE] = "";
    double tempVal = 0.0;

    fp = fopen(TEMP_FILE_PATH, "r");
    if (fp == NULL)
    {
        printf("Fail to Read Temp...\n");
    }
    else
    {
        fread(buf, 1, MAX_SIZE, fp);
        tempVal = atof(buf) / 1000;
        fclose(fp);
    }
    
    return tempVal;
}

long long GetAvailDisk()
{
    struct statfs diskInfo;
    if(statfs("/opt", &diskInfo)==-1)
    {
        return 0;
    }
    unsigned long long availableDisk = diskInfo.f_bavail * diskInfo.f_bsize;
    return availableDisk >> 20;
}
