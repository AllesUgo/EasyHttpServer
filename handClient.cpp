#include <stdio.h>
#include <errno.h>
#include "ThreadInfo.h"
#include <string.h>
#include <fstream>
#include "cJSON.h"
#include "define.h"
#include "str.h"
#include "addone.h"
#include "log.h"
#include "ConnectionControl.h"

/*
errpr->0  正常
error->1  请求文件范围错误
error->2  文件不存在
*/

#define MAXSIZE 1024
extern LinkLib *Linklib;
extern cJSON *cjsonfile;
extern cJSON *UserAgentIndexPage;
extern char *HomeDir;
using namespace std;
void pthreadexit(ThreadInfo *);
Gp GET(SOCKET clientsocket, char *, Res res);
Gp POST(SOCKET clientsocket,char*buff,Res res);
size_t getFileSize(const char *); //获取文件大小


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

Gp GET(SOCKET clientsocket, char *buff, Res res)
{
	Gp r={0};
	char *p;
	res.RequestMethod=REQUEST_GET;
	p = strstr(buff, "GET /");
	if (0 >= sscanf(p, "GET /%s", res.filename))
	{
		printf("[%s] [WORNING] 请求格式错误，无法解析\n",gettime().time);
		return r;
	}
	//单独判断是否是直接请求

	//判断是否是执行程序
	if (strstr(res.filename, "app/"))
	{
		//printf("filename:%s\n", res.filename);
		res.buff=buff;
		char *p = strstr(res.filename, "app/") + 4;
		char appname[128] = {0}, funcname[128] = {0};
		int i = 0;
		while (p[i] != '.' && p[i] != '\r' && p[i] != '\0' && p[i] != '\n')
		{
			if (p[i] == '?')
			{
				//格式错误
				return addone_NoAddone(res);
			}
			else
			{
				appname[i] = p[i];
				i += 1;
			}
		}
		i++;
		int k = i;
		while (p[i] != '?' && p[i] != '\r' && p[i] != '\0' && p[i] != '\n')
		{
			funcname[i - k] = p[i];
			i += 1;
		}
		//插件名和类型名加载完成
		//printf("appname:%s funcname:%s\n", appname, funcname);
		return addone_RunAddoneAsApplication(Linklib, &res, appname, funcname);
	}
	else if (!strcmp(res.filename, "HTTP/1.1"))
	{
		char requesturl[128] = "index.html";
		char *p = strstr(buff, "User-Agent");
		if (p != NULL)
		{
			char *temp = strstr(p, "\n"); //查找该字段结束换行符
			p += strlen("User-Agent");
			if (temp != NULL && UserAgentIndexPage != NULL)
			{
				*temp = '\0';

				//去config.json文件检索是否有设置该UA的处理要求
				cJSON *child = UserAgentIndexPage->child;
				while (child)
				{
					//先判断当前字段是否存在"key"，如果不存在则证明配置文件错误，退出服务器
					if (!(cJSON_GetObjectItem(child, "key") && cJSON_GetObjectItem(child, "page")))
					{
						exit(1);
					}
					if (strstr(p, cJSON_GetObjectItem(child, "key")->valuestring))
					{
						strcpy(requesturl, cJSON_GetObjectItem(child, "page")->valuestring);
						break;
					}
					else
					{
						child = child->next;
					}
				}
				*temp = '\n'; //把刚才临时改的改回来
			}
			
		}
		Gp gp;
		gp.fp = NULL;
		gp.readsize = 0;
		gp.reshead = request302(requesturl, res);
		return gp;
	}
	else
	{
		char *temp_filename=(char*)malloc(strlen(HomeDir)+strlen(res.filename)+2);
		sprintf(temp_filename,"%s/%s",HomeDir,res.filename);
		strcpy(res.filename,temp_filename);
		free(temp_filename);
		//change name
		if (strstr(res.filename,"?"))
		{
			*(strstr(res.filename,"?"))=0;
		}
		//获取文件大小
		res.FileSize = getFileSize(res.filename);
		res.RangeStart = 0;
		res.RangeEnd = 0;
		res.info = 200;
		if (res.FileSize == 0)
		{
			res.error = 2; //文件不存在
			strcpy(res.filename, "404.html");
			res.FileSize = getFileSize(res.filename);
			res.info = 404;
		}
		res.RangeLength = res.FileSize;
		//查看是否要取得的文件的部分
		if (strstr(buff, "Range: bytes") && res.error == 0)
		{
			res.download = 1;
			res.info = 206;
			p = strstr(buff, "Range: bytes");
			int num = sscanf(p, "Range: bytes = %lu - %lu", &(res.RangeStart), &(res.RangeEnd));
			if (num == 2)
				res.RangeLength = res.RangeEnd - res.RangeStart + 1;
			else if (num == 1 && res.RangeEnd == 0)
			{
				res.RangeEnd = res.FileSize - 1;
				res.RangeLength = res.RangeEnd - res.RangeStart + 1;
			}
			else
			{
				res.error = 1; //设置为范围错误
				r.reshead = (char *)malloc(RESHEADSIZE);
				sprintf(r.reshead, "%s 416 Requested Range Not Satisfiable\r\n\r\n", res.version);
				return r;
			}
			if (((res.RangeEnd - res.RangeStart) <= 0) || (res.RangeEnd > res.FileSize) || (res.RangeStart > res.FileSize))
			{
				res.error = 1; //设置为范围错误
				r.reshead = (char *)malloc(RESHEADSIZE);
				sprintf(r.reshead, "%s 416 Requested Range Not Satisfiable\r\n\r\n", res.version);
				return r;
			}
		}
		else
		{
			res.download = 0;
		}
		//下来获取解析方式
		p = GetFileLastName(res.filename);
		if (p == NULL)
		{
			strcpy(res.mime, "application/octet-stream");
		}
		else
		{
			cJSON *p_json_temp = cJSON_GetObjectItem(cjsonfile, p);
			//判断mime能否解析这个文件
			if (p_json_temp == NULL)
			{
				//无法解析
				strcpy(res.mime, "application/octet-stream");
			}
			else
			{
				//可以解析
				strcpy(res.mime, p_json_temp->valuestring);
			}
		}
		//标明断开连接和状态码
		strcpy(res.connection, "close");

		//拼接处理完成的信息
		char *reshead = (char *)malloc(RESHEADSIZE);
		sprintf(reshead, "%s %d \r\nContent-Type: %s;charset=utf-8\r\nContent-Length: %lu\r\nAccept-Ranges: bytes\r\n", res.version, res.info, res.mime, res.RangeLength);
		if (res.download == 1)
		{
			p = reshead + strlen(reshead);
			sprintf(p, "Content-Range: bytes %lu-%lu/%lu\r\n", res.RangeStart, res.RangeEnd, res.FileSize);
		}
		strcat(reshead, "\r\n");

		//返回文件指针
		r.fp = fopen(res.filename, "rb");
		fseek((r.fp), (res.RangeStart), SEEK_SET);
		r.reshead = reshead;
		r.readsize = res.RangeLength;
		return r;
	}
	return r;
}

Gp  POST(SOCKET clientsocket,char*buff,Res res)
{
	res.RequestMethod=REQUEST_POST;
	Gp r;
	r.fp = NULL;
	r.readsize = 0;
	r.reshead = NULL;
	res.socket=clientsocket;
	char *p;
	p = strstr(buff, "POST /");
	if (0 >= sscanf(p, "POST /%s", res.filename))
	{
		printf("[%s] [WORNING] 请求格式错误，无法解析\n",gettime().time);
		return r;
	}

	//判断是否是执行程序
	if (strstr(res.filename, "app/"))
	{
		res.buff=buff;
		//printf("filename:%s\n", res.filename);
		char *p = strstr(res.filename, "app/") + 4;
		char appname[128] = {0}, funcname[128] = {0};
		int i = 0;
		while (p[i] != '.' && p[i] != '\r' && p[i] != '\0' && p[i] != '\n')
		{
			if (p[i] == '?')
			{
				//格式错误
				return addone_NoAddone(res);
			}
			else
			{
				appname[i] = p[i];
				i += 1;
			}
		}
		i++;
		int k = i;
		while (p[i] != '?' && p[i] != '\r' && p[i] != '\0' && p[i] != '\n')
		{
			funcname[i - k] = p[i];
			i += 1;
		}
		//插件名和类型名加载完成
		//printf("appname:%s funcname:%s\n", appname, funcname);
		return addone_RunAddoneAsApplication(Linklib, &res, appname, funcname);
	}
	else
	{
		//不支持的处理
		r.fp=fopen("404.html","r");
		r.readsize=getFileSize("404.html");
		r.reshead=(char*)malloc(1024);
		sprintf(r.reshead,"HTTP/1.1 404\r\nContent-Type: text/html;charset=utf-8\r\nContent-Length: %ld\r\nAccept-Ranges: bytes\r\n\r\n",r.readsize);
		return r;
	}

}
void pthreadexit(ThreadInfo *info)
{
	RemoveConnection(info->connectInfo.client_socket);
	close(info->connectInfo.client_socket);
	printf("[%s] [LOG] IP地址:%s,访问信息:%s,断开连接\n", gettime().time,info->connectInfo.address, info->connectInfo.filename);
	char *p=(char*)malloc(4500);
	snprintf(p,4500,"[%s] [%s] \"%s\"",gettime().time,info->connectInfo.address,info->connectInfo.filename);
	if (saveLog(p)!=0)
	{
		printf("[%s] [WARNING] 日志:%s无法记录\n",gettime().time,p);
	}
	free(p);
	free(info);
	pthread_exit(NULL);
}

void handClient(LPVOID inPut)
{

	ThreadInfo *myinfo = (ThreadInfo *)inPut;
	pthread_detach(myinfo->pid);
	SOCKET clientsocket = (myinfo->connectInfo.client_socket);
	Res res;
	strcpy(res.address, myinfo->connectInfo.address);
	printf("[%s] [LOG] 一个连接地址是:%s\n",gettime().time, myinfo->connectInfo.address);
	RegistConnection(myinfo);
	//strcpy("",);
	char *buff = (char *)malloc(MAXSIZE);
	int rcvcount = recv(clientsocket, buff, MAXSIZE, 0);
	UpdateConnection(clientsocket);
	buff[rcvcount] = '\0';
	//初始化错误信息为0,将文件指针链接
	res.error = 0;
	res.filename = myinfo->connectInfo.filename;

	//确定协议版本HTTP/1.1
	strcpy(res.version, "HTTP/1.1");
	//printf("-------------------------\n%s\n--------------------------\n",buff);

	//确定解析模式
	Gp p;
	//printf("==================\n%s\n=====================", buff);
	if (strstr(buff, "GET /"))
	{
		p = GET(clientsocket, buff, res);
		if (p.reshead != NULL)
		{
			send(clientsocket, p.reshead, strlen(p.reshead), MSG_NOSIGNAL);
			//////////////
			//printf("%s\n",p.reshead);
			if (p.databuff!=NULL&&p.databuff_size!=0)
			{
				send(clientsocket,p.databuff,p.databuff_size,MSG_NOSIGNAL);
				free(p.databuff);
			}
			if (p.readsize != 0 && p.fp != NULL)
			{

				char *temp = (char *)malloc(1024);
				while (p.readsize > 0 && !feof(p.fp))
				{
					long unsigned int i;
					if (p.readsize > 1024)
					{
						i = fread(temp, 1, 1024, p.fp);
						if (i != 1024)
						{
							printf("[%s] [WORNING] 错误:正在从文件请求1024字节数据，但只能从文件读取到%lu字节\n",gettime().time, i);
							break;
						}
					}
					else
					{
						i = fread(temp, 1, p.readsize, p.fp);
						if (i != p.readsize)
						{
							printf("[%s] [WORNING] 错误:正在从文件请求%lu字节数据，但只能从文件读取到%lu字节\n",gettime().time, p.readsize, i);
							break;
						}
					}
					if (0 > send(clientsocket, temp, i, MSG_NOSIGNAL))
					{
						//break;//连接错误
						printf("[%s] [WORNING] 一个连接因为异常原因断开\n",gettime().time);
						break;
					}
					UpdateConnection(clientsocket);
					p.readsize -= i;
				}
				fclose(p.fp);
				free(temp);
			}
			free(p.reshead);
		}
	}
	else if (strstr(buff, "POST /"))
	{
		p = POST(clientsocket, buff, res);
		if (p.reshead != NULL)
		{
			send(clientsocket, p.reshead, strlen(p.reshead), MSG_NOSIGNAL);
			//////////////
			//printf("%s\n",p.reshead);
			if (p.databuff!=NULL&&p.databuff_size!=0)
			{
				send(clientsocket,p.databuff,p.databuff_size,MSG_NOSIGNAL);
				free(p.databuff);
			}
			if (p.readsize != 0 && p.fp != NULL)
			{

				char *temp = (char *)malloc(1024);
				while (p.readsize > 0 && !feof(p.fp))
				{
					long unsigned int i;
					if (p.readsize > 1024)
					{
						i = fread(temp, 1, 1024, p.fp);
						if (i != 1024)
						{
							printf("[%s] [WORNING] 错误:正在从文件请求1024字节数据，但只能从文件读取到%lu字节\n",gettime().time, i);
							break;
						}
					}
					else
					{
						i = fread(temp, 1, p.readsize, p.fp);
						if (i != p.readsize)
						{
							printf("[%s] [WORNING] 错误:正在从文件请求%lu字节数据，但只能从文件读取到%lu字节\n",gettime().time, p.readsize, i);
							break;
						}
					}
					if (0 > send(clientsocket, temp, i, MSG_NOSIGNAL))
					{
						//break;//连接错误
						printf("[%s] [WORNING] 一个连接因为异常原因断开\n",gettime().time);
						break;
					}
					UpdateConnection(clientsocket);
					p.readsize -= i;
				}
				fclose(p.fp);
				free(temp);
			}
			free(p.reshead);
		}
	}
	free(buff);
	pthreadexit(myinfo);
}
