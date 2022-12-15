#include "httphead.h"
#include <stdlib.h>
int HH_BuileHead(const char *state, char *outputstr, size_t maxsize)
{
    //检查需要的空间大小
    size_t needsize = 0;
    needsize += strlen("HTTP/1.1 ");
    needsize += (strlen(outputstr) + 1 + 1 + 1);
    if (needsize > maxsize)
    {
        return HH_MEMLACK;
    }
    sprintf(outputstr, "HTTP/1.1 %s\r\n", state);
    return HH_SUCCESS;
}

int HH_AddKey(const char *keyname, const char *keyvalue, char *outputstr, size_t maxsize)
{
    size_t needsize = strlen(outputstr) + strlen(keyname) + 2 + strlen(keyvalue) + 3;
    if (needsize > maxsize)
    {
        return HH_MEMLACK;
    }
    strcat(outputstr, keyname);
    strcat(outputstr, ": ");
    strcat(outputstr, keyvalue);
    strcat(outputstr, "\r\n");
    return HH_SUCCESS;
}

int HH_FinishBuild(char *outputstr, size_t maxsize)
{
    size_t needsize = strlen(outputstr) + 3;
    if (needsize > maxsize)
    {
        return HH_MEMLACK;
    }
    strcat(outputstr, "\r\n");
    return HH_SUCCESS;
}

int HH_RequestMethod(const char *buff)
{
    if (strstr(buff, "GET") == buff)
    {
        return HH_GET;
    }
    else if (strstr(buff, "POST") == buff)
    {
        return HH_POST;
    }
    else
    {
        return HH_OTHERMETHOD;
    }
}
int HH_GetValue(const char *buff, const char *key, char *output)
{
    char *p = strstr((char*)buff, "\r\n");
    if (p == NULL)
    {
        return HH_ERRORHEAD;
    }
    p += 2;

    while (1)
    {
        p = strstr(p, key);
        if (p == NULL)
        {
            return HH_NOTFOUND;
        }
        else
        {
            if (*(p - 1) != '\n' || *(p + strlen(key)) != ':')
            {
                p++;
                continue;
            }
            else
            {
                p = p + strlen(key) + 2;
                char *temp = (char *)malloc(strlen(p) + 1);
                strcpy(temp, p);
                char *a = strstr(temp, "\r\n");
                if (a == NULL || temp == p)
                {
                    free(temp);
                    return HH_ERRORHEAD;
                }
                else
                {
                    *a = 0;
                    strcpy(output, temp);
                    free(temp);
                    return HH_SUCCESS;
                }
            }
        }
    }
}