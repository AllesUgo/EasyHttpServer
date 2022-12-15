#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include "log.h"
#include "ConnectionControl.h"

struct ConnectNode
{
    ConnectInfo connectioninfo;
    time_t RegistTime;
    time_t LatestUpdateTime;
    pthread_t RegisterTid;
    int tryclose;
    struct ConnectNode *next;
};
struct ConnectNode *Head;
pthread_mutex_t Lock;
pthread_t ConnectionControlThreadTid;
void RegistConnection(ThreadInfo *threadinfo);
int UpdateConnection(int sock);
int RemoveConnection(int sock);
void *ConnectionControlThread(void *ptr);
int init_ConnectionControl();

int init_ConnectionControl()
{
    pthread_mutex_init(&Lock, NULL); // 初始化保护ConnectNode链表安全的互斥锁
    Head = NULL;                     // 将链表头设置为NULL
    //创建后台线程
    pthread_create(&ConnectionControlThreadTid,NULL,ConnectionControlThread,NULL);
    pthread_detach(ConnectionControlThreadTid);
    return 0;
}
void *ConnectionControlThread(void *ptr)
{
    while (1)
    {
        // 循环检查线程
        sleep(60); // 每次检查间隔60s

        pthread_mutex_lock(&Lock);
        struct ConnectNode *temp = Head;
        time_t nowtime = time(NULL);
        while (temp)
        {
            if (temp->tryclose == false && nowtime - temp->LatestUpdateTime > 60)
            {
                if (0 == shutdown(temp->connectioninfo.client_socket, SHUT_RDWR))
                {
                    temp->tryclose = true;
                }
                
            }
            temp = temp->next;
        }
        pthread_mutex_unlock(&Lock);
    }
}

void RegistConnection(ThreadInfo *threadinfo)
{
    struct ConnectNode *newnode = (struct ConnectNode *)malloc(sizeof(struct ConnectNode));
    newnode->connectioninfo = threadinfo->connectInfo;
    newnode->RegisterTid = pthread_self();
    newnode->RegistTime = time(NULL);
    newnode->tryclose = false;
    newnode->LatestUpdateTime = time(NULL);
    // 将这个节点添加至链表
    pthread_mutex_lock(&Lock);
    newnode->next = Head;
    Head = newnode;
    pthread_mutex_unlock(&Lock);
}
int UpdateConnection(int sock)
{
    // 刷新连接信息
    pthread_mutex_lock(&Lock);
    struct ConnectNode *temp = Head;
    char sign = false;
    while (temp)
    {
        if (temp->connectioninfo.client_socket == sock)
        {
            temp->LatestUpdateTime = time(NULL);
            sign = true;
        }
        temp=temp->next;
    }
    pthread_mutex_unlock(&Lock);
    if (sign == false)
        return -1;
    return 0;
}
int RemoveConnection(int sock)
{
    pthread_mutex_lock(&Lock);
    struct ConnectNode *temp = Head;
    if (temp == NULL)
    {
        pthread_mutex_unlock(&Lock);
        return -1; // 没有找到这一项
    }
    // 检查当前项是否为目标项
    if (temp->connectioninfo.client_socket == sock)
    {
        if (pthread_self() == temp->RegisterTid)
        {
            Head = temp->next;
            free(temp);
            pthread_mutex_unlock(&Lock);
            return 0;
        }
        else
        {
            printf("[%s] [ERROR] 找到套接字为%d的项目，但试图在其他线程释放它\n", gettime().time, temp->connectioninfo.client_socket);
            pthread_mutex_unlock(&Lock);
            return -1;
        }
    }
    // 检查除第一项之外的项目
    while (temp->next)
    {
        if (temp->next->connectioninfo.client_socket == sock)
        {
            if (pthread_self() == temp->next->RegisterTid)
            {
                struct ConnectNode *waitfree = temp->next;
                temp->next = waitfree->next;
                free(waitfree);
                pthread_mutex_unlock(&Lock);
                return 0;
            }
            else
            {
                printf("[%s] [ERROR] 找到套接字为%d的项目，但试图在其他线程释放它\n",gettime().time, temp->next->connectioninfo.client_socket);
                pthread_mutex_unlock(&Lock);
                return -1;
            }
        }
        temp=temp->next;
    }
    // 没有找到这一项
    printf("[%s] [WARING] 未能找到套接字为%d的连接\n",  gettime().time,temp->next->connectioninfo.client_socket);
    pthread_mutex_unlock(&Lock);
    return -2;
}
