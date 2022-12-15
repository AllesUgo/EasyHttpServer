#include "addonesdk.h"
#include <stdlib.h>
#include <iostream>
#include <list>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;

Gp upload(Res *res);
Gp SetValue(Res *res);
Gp GetDevices(Res *res);
Gp GetValue(Res *res);

class DevicesInfo
{
public:
    std::string devices_name;
    int value;
    int needvalue;
    time_t online_time;
    time_t latest_time;
};
std::list<DevicesInfo> Devices;
extern "C"
{
    void*thread(void*);
    Addone_Info _init_()
    {
        Addone_Info info = AO_SDK_Create(0, "1.0", "一个控制计算机音量的服务端程序");
        AO_SDK_AddFunction(&info, "upload", upload);
        AO_SDK_AddFunction(&info,"SetValue",SetValue);
        AO_SDK_AddFunction(&info,"GetDevices",GetDevices);
        AO_SDK_AddFunction(&info,"GetValue",GetValue);
        pthread_t tid;
        pthread_create(&tid,NULL,thread,NULL);
        //pthread_detach(tid);
        return info;
    }
    void*thread(void*)
    {
        while (1)
        {
            //检查设备超时未连接
            sleep(5);
            pthread_mutex_lock(&lock);
            time_t nowtime=time(NULL);
            retry:
            for (auto& it:Devices)
            {
                if (nowtime-it.latest_time>20)
                {
                    //移除这一项
                    auto name=it.devices_name;
                    Devices.remove_if([name](DevicesInfo& a){return (a.devices_name==name);});
                    goto retry;
                }
            }
            pthread_mutex_unlock(&lock);
        }
    }
}
Gp GetValue(Res *res)
{
    Gp a={0};
    //获取要查询的设备名
    //解析HTTP请求
    DevicesInfo info;
    char *buff = (char *)malloc(1024);
    if (HH_SUCCESS != HH_GetValue(res->buff, "Name", buff))
    {
        free(buff);
        return a;
    }
    info.devices_name = buff;
    //查询该设备
    pthread_mutex_lock(&lock);
    for (auto& it:Devices)
    {
        if (it.devices_name==info.devices_name)
        {
            //构造HTTP标头
            HH_BuileHead("200 ok",buff,1024);
            HH_AddKey("Value",std::to_string(it.value).c_str(),buff,1024);
            HH_AddKey("OnlineTime",std::to_string(it.online_time).c_str(),buff,1024);
            pthread_mutex_unlock(&lock);
            HH_FinishBuild(buff,1024);
            a.reshead=buff;
            return a;
        }
    }
    pthread_mutex_unlock(&lock);
    HH_BuileHead("404 not found",buff,1024);
    HH_FinishBuild(buff,1024);
    a.reshead=buff;
    return a;
}
Gp GetDevices(Res *res)
{
    Gp a={0};
    std::string d;
    int sign=0;
    pthread_mutex_lock(&lock);
    for (auto it:Devices)
    {
        d=d+it.devices_name+"\r\n";
        sign=1;
    }
    pthread_mutex_unlock(&lock);
    if (sign==0)
    {
        d="\r\n";
    }
    char *buff=(char*)malloc(d.length()+1);
    strcpy(buff,d.c_str());
    a.databuff=buff;
    a.databuff_size=strlen(buff);
    char *head=(char*)malloc(1024);
    HH_BuileHead("200 ok",head,1024);
    HH_AddKey("Content-Length",std::to_string(a.databuff_size).c_str(),head,1024);
    HH_FinishBuild(head,1024);
    a.reshead=head;
    return a;

}
Gp SetValue(Res *res)
{
    Gp a;
    a.fp = NULL;
    a.readsize = 0;
    a.reshead = NULL;
    a.databuff_size = 0;
    //解析HTTP请求
    DevicesInfo info;
    char *buff = (char *)malloc(1024);
    if (HH_SUCCESS != HH_GetValue(res->buff, "Name", buff))
    {
        free(buff);
        return a;
    }
    info.devices_name = buff;
    if (HH_SUCCESS != HH_GetValue(res->buff, "Value", buff))
    {
        free(buff);
        return a;
    }
    int value;
    if (1 != sscanf(buff, "%d", &value))
    {
        free(buff);
        return a;
    }
    pthread_mutex_lock(&lock);
    for (auto &it:Devices)
    {
        if (it.devices_name==std::string(info.devices_name))
        {
            //找到该设备
            it.needvalue=value;
            pthread_mutex_unlock(&lock);
            HH_BuileHead("200 ok",buff,1024);
            HH_FinishBuild(buff,1024);
            a.reshead=buff;
            return a;
        }
    }
    pthread_mutex_unlock(&lock);
    HH_BuileHead("404 not found",buff,1024);
    HH_FinishBuild(buff,1024);
    a.reshead=buff;
    return a;
    
}
Gp upload(Res *res)
{
    Gp a;
    a.fp = NULL;
    a.readsize = 0;
    a.reshead = NULL;
    a.databuff_size = 0;
    //解析HTTP请求
    DevicesInfo info;
    char *buff = (char *)malloc(1024);
    if (HH_SUCCESS != HH_GetValue(res->buff, "Name", buff))
    {
        free(buff);
        return a;
    }
    info.devices_name = buff;
    if (HH_SUCCESS != HH_GetValue(res->buff, "Value", buff))
    {
        free(buff);
        return a;
    }
    int value;
    if (1 != sscanf(buff, "%d", &value))
    {
        free(buff);
        return a;
    }
    info.value = value;
    info.online_time = time(NULL);
    //检查是否重复
    pthread_mutex_lock(&lock);
    for (auto &it : Devices)
    {
        if (it.devices_name == info.devices_name)
        {
            //有重复
            it.value = info.value;
            it.latest_time=time(NULL);
            HH_BuileHead("200 ok", buff, 1024);
            if (it.needvalue != info.value&&it.needvalue!=-1)
            {
                HH_AddKey("Value",std::to_string(it.needvalue).c_str(),buff,1024);
                it.needvalue=-1;
            }
            HH_FinishBuild(buff,1024);
            a.reshead=buff;
            pthread_mutex_unlock(&lock);
            return a;
        }
    }
    //没有重复
    info.needvalue=-1;
    info.latest_time=time(NULL);
    Devices.push_back(info);
    pthread_mutex_unlock(&lock);
    HH_BuileHead("200 ok",buff,1024);
    HH_FinishBuild(buff,1024);
    a.reshead=buff;
    return a;
}
