#include "addonesdk.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <string.h>
#include "httphead.h"
Addone_Info _init_(void);
Gp diskuse(Res *resbonse);
size_t getFileSize(const char *filename);
Addone_Info _init_(void)
{
    Addone_Info info=AO_SDK_Create(1,"1.0.0","查看磁盘剩余空间");
    AO_SDK_AddFunction(&info,"diskuse",diskuse);
    return info;
}
size_t getFileSize(const char *filename)
{
    struct stat statbuf;
    if (-1 == stat(filename, &statbuf))
    {
        return 0;
    }
    size_t size = statbuf.st_size;

    return size;
}
Gp diskuse(Res *resbonse)
{
    Gp a={0};
    FILE*fp=popen("df -h","r");
    char *str=(char*)malloc(1024);
    fread(str,1024,1,fp);
    pclose(fp);
    char *p = (char *)malloc(1024);
    HH_BuileHead("200 ok",p,1024);
    HH_AddKey("Content-Type","text/plain;charset=UTF-8",p,1024);
    char temp[128];
    *temp=0;
    sprintf(temp,"%d",strlen(str));
    HH_AddKey("Content-Length",temp,p,1024);
    HH_FinishBuild(p,1024);
    //strcpy(p,"HTTP/1.1 200\r\nContent-Type: text/plain; charset=UTF-8\r\n\r\n");
    a.databuff=str;
    a.databuff_size=strlen(a.databuff);
    a.reshead = p;
    return a;
}
