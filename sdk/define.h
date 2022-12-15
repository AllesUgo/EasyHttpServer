#ifndef DEFINE
#define DEFINE
#define IP_SIZE 16
#define RESHEADSIZE 2048
#define MAX_PATH 255
#define REQUEST_GET 1
#define REQUEST_POST 2
#include <stdio.h>
typedef struct gp
{
	char *reshead;
	char *databuff;
	long unsigned int databuff_size;
	FILE *fp;
	long unsigned int readsize;
} Gp;
typedef struct res
{
	char *filename;//浏览器请求的文件名称
	char version[9];//HTTP协议版本
	int info; //状态码
	int download;//是否为下载文件
	long unsigned int RangeLength;
	long unsigned int RangeStart;
	long unsigned int RangeEnd;
	long unsigned int FileSize;
	char location[128];
	char mime[128];//解析方式,如text/plain
	char connection[32];//连接状态
	char address[IP_SIZE];//IP地址
	int error;//错误码
	int socket;
	char *buff;
	char RequestMethod;
} Res;

#endif