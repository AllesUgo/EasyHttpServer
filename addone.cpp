#include "addone.h"
#include "str.h"
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
void printhelp_main();
void unmountall(LinkLib **head);
Gp addone_NoAddone(Res resbonse)
{
    //没有插件或没有注册
    Gp gp={0};
    gp.reshead = request302("../404.html", resbonse);
    return gp;
}
Gp addone_RunAddoneAsApplication(LinkLib *head, Res *resbonse, const char *AddoneName, const char *FuncName)
{
    typedef Gp (*application)(Res *);
    if (head == NULL)
    {
        //没有插件或没有注册
        printf("没有注册插件\n");
        return addone_NoAddone(*resbonse);
    }
    else
    {
        LinkLib *temp = head;
        while (temp)
        {
            if (!strcmp(temp->libname, AddoneName) && temp->enable == true)
            {
                Func *fn = temp->funcslist;
                while (fn)
                {
                    if (!strcmp(fn->funcname, FuncName) && fn->type == APPLICATION)
                    {
                        application func = (application)(fn->func);
                        return func(resbonse);
                    }
                    else
                    {
                        fn = fn->next;
                    }
                }
            }

            temp = temp->next;
        }
        return addone_NoAddone(*resbonse); //没有找到这个函数库
    }
}
int addone_RunAddoneAsCommand(LinkLib *head, const char *AddoneName, const char *FuncName, char *argv)
{
    typedef int (*application)(char *);
    if (head == NULL)
    {
        //没有插件或没有注册
        printf("没有注册插件\n");
        return 1;
    }
    else
    {
        LinkLib *temp = head;
        while (temp)
        {
            if (!strcmp(temp->libname, AddoneName))
            {
                Func *fn = temp->funcslist;
                while (fn)
                {
                    if (!strcmp(fn->funcname, FuncName) && fn->type == COMMAND)
                    {
                        application func = (application)(fn->func);
                        return func(argv);
                    }
                    else
                    {
                        fn = fn->next;
                    }
                }
            }

            temp = temp->next;
        }
        return 1; //没有找到这个函数库
    }
}
LinkLib *addone_MakeFuncLink(LinkLib **head, const char dir[128])
{
    printf("[log]设定的addones插件目录为%s\n", dir);
    printf("\n>>开始搜索目录<<\n");
    DIR *dp = opendir(dir);
    if (dp == NULL)
    {
        perror("[WARNING]目录打开失败");
        return NULL;
    }
    struct dirent *dir_entry = NULL;
    //获取，目录下有多少个文件
    int filenum = 0;
    while (1)
    {
        dir_entry = readdir(dp);
        if (dir_entry == NULL)
        {
            break;
        }
        printf("[log]发现文件%s\n", dir_entry->d_name);
        if (dir_entry->d_type == 8 && strstr(dir_entry->d_name, "_app"))
        {
            printf("[log]找到addone:%s\n", dir_entry->d_name);
            filenum++;
        }
    }
    //将目录指针返回开头
    seekdir(dp, 0);
    //定义一个二维数组存放文件名
    char filenames[filenum][128];
    int i = 0;
    //保存文件名
    while (1)
    {
        dir_entry = readdir(dp);
        if (dir_entry == NULL)
        {
            break;
        }
        if (dir_entry->d_type == 8 && strstr(dir_entry->d_name, "_app"))
        {
            strcpy(filenames[i], dir_entry->d_name);
            i++;
        }
    }
    closedir(dp);

    //尝试注册并尝试创建链表
    printf("\n>>开始注册插件<<\n");
    typedef Addone_Info (*_init_)(void);
    char strtemp[256];
    for (int i = 0; i < filenum; i++)
    {
        sprintf(strtemp, "%s/%s", dir, filenames[i]);
        void *lib = dlopen(strtemp, RTLD_LAZY);
        if (lib == NULL)
        {
            printf("[WARNING]打开%s错误,错误原因是:%s\n", strtemp, dlerror());
            continue;
        }
        _init_ init = (_init_)dlsym(lib, "_init_");
        if (init == NULL)
        {
            printf("[WARNING] Addone:%s无法注册，原因是%s，你可以尝试与开发者联系\n", filenames[i], dlerror());
            dlclose(lib);
        }
        else
        {
            if (*head == NULL)
            {
                Addone_Info addone_info = init();
                if (addone_info.return_size != sizeof(Gp))
                {
                    printf("[WARNING] Addone:%s无法被安全的注册，其协议版本可能与服务器版本不符，很可能发生数据异常或崩溃，你可以尝试与开发者联系\n", filenames[i]);
                }
                *head = (LinkLib *)malloc(sizeof(LinkLib));
                (*head)->handle = lib;
                strcpy((*head)->libname, filenames[i]);
                (*head)->funcslist = addone_info.func_list_head;
                (*head)->unmount = addone_info.unmount;
                (*head)->next = NULL;
                (*head)->enable = true;
                printf("[log]已注册Addone:%s,版本:%s\n", (*head)->libname, addone_info.addone_version);
            }
            else
            {
                Addone_Info addone_info = init();
                if (addone_info.return_size != sizeof(Gp))
                {
                    printf("[WARNING] Addone:%s [%ld,%ld]无法被安全的注册，其协议版本可能与服务器版本不符，很可能发生数据异常或崩溃，你可以尝试与开发者联系\n", filenames[i],addone_info.return_size,sizeof(Gp));
                }
                LinkLib *temp;
                temp = (LinkLib *)malloc(sizeof(LinkLib));
                temp->handle = lib;
                strcpy(temp->libname, filenames[i]);
                temp->funcslist = addone_info.func_list_head;
                temp->unmount = addone_info.unmount;
                temp->enable = true;
                temp->next = (*head);
                printf("[log]已注册Addone:%s,版本:%s\n", temp->libname, addone_info.addone_version);
                (*head) = temp;
            }
        }
    }
    putchar('\n');
    return (*head);
}

void addone_ConsoleControl(char *cmd, LinkLib **head)
{
    char argv1[1024] = {0};
    if (sscanf(cmd, "addone %s", argv1) != 1)
    {
        printhelp_main();
        return;
    }
    if (!(!strcmp(argv1, "enable") || !strcmp(argv1, "disable") || !strcmp(argv1, "unmount") ||
          !strcmp(argv1, "unmountall") || !strcmp(argv1, "reload") || !strcmp(argv1, "list") || !strcmp(argv1, "load")))
    {
        printf(">\n");
        printhelp_main();
        return;
    }
    if (!strcmp(argv1, "enable") || !strcmp(argv1, "disable") || !strcmp(argv1, "unmount"))
    {
        char addone_name[128];
        if (3 != sscanf(cmd, "%s %s %s", addone_name, addone_name, addone_name))
        {
            printhelp_main();
            return;
        }
        char type;
        if (!strcmp(argv1, "enable"))
            type = 1;
        else if (!strcmp(argv1, "disable"))
            type = 2;
        else if (!strcmp(argv1, "unmount"))
            type = 3;
        else
            return;
        LinkLib *temp = *head;
        char sign = 0;
        //对卸载节点单独处理
        if (type == 3)
        {
            if (*head == NULL)
            {
                printf("Addone:%s 没有找到该模块\n", addone_name);
                return;
            }
            //判断是否为头节点
            if (!strcmp(addone_name, temp->libname))
            {
                if (temp->unmount = false)
                {
                    printf("Addone:%s 不允许被动态卸载\n", addone_name);
                    return;
                }
                //清理内存资源
                Func *func_temp = temp->funcslist, *func_back;
                while (func_temp)
                {
                    func_back = func_temp->next;
                    free(func_temp);
                    func_temp = func_back;
                }
                //关闭句柄
                dlclose(temp->handle);
                //头指针向后移
                *head = temp->next;
                free(temp);
                printf("Addone:%s 已经被卸载\n", addone_name);
                return;
            }
            else
            {
                LinkLib *temp_back;
                while (temp->next != NULL)
                {
                    if (!strcmp(temp->next->libname, addone_name))
                    {
                        temp_back = temp->next;
                        temp->next = temp->next->next;
                        temp = temp_back;
                        //清理内存资源
                        Func *func_temp = temp->funcslist, *func_back;
                        while (func_temp)
                        {
                            func_back = func_temp->next;
                            free(func_temp);
                            func_temp = func_back;
                        }
                        //关闭句柄
                        dlclose(temp->handle);
                        free(temp);
                        printf("Addone:%s 已经被卸载\n", addone_name);
                        return;
                    }
                    temp=temp->next;
                }
                printf("Addone:%s 没有找到该模块\n", addone_name);
                return;
            }
        }

        while (temp != NULL)
        {
            if (!strcmp(temp->libname, addone_name))
            {
                sign = 1;
                if (type == 1)
                {
                    if (temp->enable == true)
                        printf("Addone:%s已经启用，无需重复启用\n", addone_name);
                    else
                    {
                        temp->enable = true;
                        printf("Addone:%s 启用成功\n", addone_name);
                    }
                    return;
                }
                else
                {
                    if (temp->enable == false)
                        printf("Addone:%s已经禁用，无需重复禁用\n", addone_name);
                    else
                    {
                        temp->enable = false;
                        printf("Addone:%s 禁用成功\n", addone_name);
                    }
                    return;
                }
            }
            temp = temp->next;
        }
        printf("Addone:%s 没有找到该模块\n", addone_name);
        return;
    }
    else if (!strcmp(argv1, "list"))
    {
        printf("\n==========Addone==========\n");
        LinkLib *temp = *head;
        Func *func;
        int i = 0, allowuse = 0, allowunmount = 0;
        while (temp)
        {
            i += 1;
            if (temp->enable == 1)
                allowuse += 1;
            if (temp->unmount == 1)
                allowunmount += 1;
            printf("Addone:%s 是否已启用:%d 可否被卸载:%d\n\t", temp->libname, temp->unmount, temp->enable);
            func = temp->funcslist;
            while (func)
            {
                printf("%s ", func->funcname);
                func = func->next;
            }
            printf("\n\n");
            temp = temp->next;
        }
        printf("共有%d个组件被加载，已启用%d个，可卸载%d个\n", i, allowuse, allowunmount);
        printf("==========================\n");
        return;
    }
    else if (!strcmp(argv1,"unmountall"))
    {
        unmountall(head);
        printf("Addone卸载完成\n\n");
        return;
    }
    else if (!strcmp(argv1,"reload"))
    {
        unmountall(head);
        *head = addone_MakeFuncLink(head, "app");
        printf("重新加载完成\n");
        return;
    }
    printf("命令异常\n");
}
void unmountall(LinkLib **head)
{
    char firist_sign = 0;
    LinkLib *temp = *head, *lib_back, *keep;
    Func *list = temp->funcslist, *func_back;
    while (temp)
    {
        if (temp->unmount == true)
        {
            list = temp->funcslist;
            while (list)
            {
                func_back = list->next;
                free(list);
                list = func_back;
            }
            dlclose(temp->handle);
            printf("Addone:%s 已卸载\n",temp->libname);
            lib_back=temp->next;
            free(temp);
            temp=lib_back;
        }
        else
        {
            if (firist_sign==0)
            {
                firist_sign=1;
                *head=temp;
                keep=temp;
            }
            else
            {
                keep->next=temp;
                keep=temp;
            }
            printf("Addone:%s 不允许被卸载,重新加载时该插件会被重复加载\n",temp->libname);
            temp=temp->next;
        }
    }
    *head=NULL;
}
void printhelp_main()
{
    printf("\n命令帮助:\n\taddone [enable/disable/unmount] [addone_name]\n\taddone [list/unmountall/reload/load]\n\n");
}
