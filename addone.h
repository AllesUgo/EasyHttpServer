#ifndef ADDONE
#define ADDONE

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
int addone_RunAddoneAsCommand(LinkLib *head, const char *AddoneName, const char *FuncName, char *argv);
Gp addone_NoAddone(Res resbonse);

LinkLib *addone_MakeFuncLink(LinkLib **, const char dir[128]);
Gp addone_RunAddoneAsApplication(LinkLib *head, Res *resbonse, const char *AddoneName, const char *FuncName);
void addone_ConsoleControl(char *cmd, LinkLib **head);

#endif
