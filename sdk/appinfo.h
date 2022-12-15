#ifndef APPINFO
#define APPINFO
#include "addonesdk.h"
#include "define.h"
#define APPLICATION 0
#define COMMAND 1
#define LIB_VERSION __TIME__
typedef void *HANDLE;
typedef struct FUNC
{
    char funcname[128];
    int type;
    void *func;
    struct FUNC *next;
} Func;
typedef struct ADDONE_INFO
{
    char unmount;
    unsigned long return_size;
    char *addone_version;
    char *lib_version;
    char *describe;
    struct FUNC *func_list_head;
} Addone_Info;

typedef struct LINKLIB
{
    char enable;
    char unmount;
    char libname[128];
    HANDLE handle;
    Func *funcslist;
    struct LINKLIB *next;
} LinkLib;


Addone_Info AO_SDK_Create(char able_unmount,const char*version,const char*describe);
void AO_SDK_AddFunction(Addone_Info *info, const char *name, Gp (*func)(Res *));
#endif