#ifndef CONNECTION_CONTROL
#define CONNECTION_CONTROL

// 下面是C++库
#include "define.h"
#include "ThreadInfo.h"

void RegistConnection(ThreadInfo *threadinfo);
int UpdateConnection(int sock);
int RemoveConnection(int sock);
int init_ConnectionControl();

#endif