#ifndef TH
#define TH
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
//下面是C++库
#include"define.h"

typedef void* LPVOID;
typedef int SOCKET;
typedef struct connect_info
{
	SOCKET client_socket;
	struct sockaddr_in sockinfo;
	char address[IP_SIZE];
	char filename[FILENAME_MAX];
}ConnectInfo;
typedef struct thread_info
{
	pthread_t pid;
	int runinfo;
	ConnectInfo connectInfo;
}ThreadInfo;

#endif