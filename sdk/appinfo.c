#include "appinfo.h"
#include <string.h>
#include <stdlib.h>

Addone_Info AO_SDK_Create(char able_unmount, const char *version, const char *describe);

Addone_Info AO_SDK_Create(char able_unmount, const char *version, const char *describe)
{
    Addone_Info info;
    info.addone_version = (char*)version;
    info.describe = (char*)describe;
    info.func_list_head = NULL;
    info.lib_version = NULL;
    info.return_size = sizeof(Gp);
    info.unmount = able_unmount;
    return info;
}
void AO_SDK_AddFunction(Addone_Info *info, const char *name, Gp (*func)(Res *))
{
    Func *temp = info->func_list_head;
    if (temp == NULL)
    {
        info->func_list_head = (Func *)malloc(sizeof(Func));
        strcpy(info->func_list_head->funcname, name);
        info->func_list_head->func=(void*)func;
        info->func_list_head->next=NULL;
        info->func_list_head->type=APPLICATION;
        return;
    }
    while (temp->next!=NULL)
    {
        temp=temp->next;
    }
    temp->next = (Func *)malloc(sizeof(Func));
        strcpy(temp->next->funcname, name);
        temp->next->func=(void*)func;
        temp->next->next=NULL;
        temp->next->type=APPLICATION;
}