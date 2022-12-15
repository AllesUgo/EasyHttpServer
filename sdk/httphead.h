#ifndef HTTPHEAD
#define HTTPHEAD
#include<string.h>
#include<stdio.h>
#define HH_MEMLACK 1
#define HH_SUCCESS 0
#define HH_NOTFOUND 2
#define HH_GET 3
#define HH_POST 4
#define HH_OTHERMETHOD 5
#define HH_ERRORHEAD 6
int HH_BuileHead(const char*state,char*outputstr,size_t maxsize);
int HH_AddKey(const char*keyname,const char*keyvalue,char*outputstr,size_t maxsize);
int HH_FinishBuild(char* outputstr,size_t maxsize);
int HH_RequestMethod(const char* buff);
int HH_GetValue(const char*buff,const char*key,char* output);

#endif