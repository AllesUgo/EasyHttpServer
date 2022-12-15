#include "addonesdk.h"
#include "httphead.h"
#include <stdlib.h>

Gp getip(Res *res)
{
    char *str=(char*)malloc(1024);
    HH_BuileHead("200 ok",str,1024);
    HH_AddKey("Content-Type","text/html; charset=UTF-8",str,1024);
    Gp ret={0};
    ret.databuff=(char*)malloc(1024);
    sprintf(ret.databuff,"<h1>你的IP地址是%s</h1>",res->address);
    HH_FinishBuild(str,1024);
    ret.databuff_size=strlen(ret.databuff);
    ret.reshead=str;
    return ret;
}

Addone_Info _init_()
{
    Addone_Info info= AO_SDK_Create(1,"1.0","获取用户IP");
    AO_SDK_AddFunction(&info,"getip",getip);
    return info;
}

