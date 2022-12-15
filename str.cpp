#include "str.h" 
#include <string.h>
char *request302(const char *url, Res res)
{
	char *reshead = (char *)malloc(1024);
	sprintf(reshead, "%s 302 \r\nContent-Length: 0\r\nLocation: %s\r\n", res.version, url);
	return reshead;
}
char * GetFileLastName(char*filename)
{
	if (filename==NULL) return NULL;
	for (int i=strlen(filename)-1;i>0&&filename[i]!='/';i--)
	{
		if (filename[i]=='.')
		{
			//检查是否是路径的开始
			if (i!=0)
			{
				if (filename[i-1]=='/') return NULL;
			}
			return filename+i;

		}
	}
	return NULL;
}