#include<stdio.h>
#include<errno.h>
#include"ThreadInfo.h"

ConnectInfo* conneectClient(const SOCKET server_socket,ConnectInfo*info)
{
	struct sockaddr_in ClientAddr;
	SOCKET ClientSocket;
	unsigned int AddrLen =sizeof(ClientAddr);
	ClientSocket = accept(server_socket, (struct sockaddr*)&ClientAddr, &AddrLen);
	if (ClientSocket == -1)
	{
		perror("acceptError");
		exit(1);
	}
	info->sockinfo = ClientAddr;
	info->client_socket = ClientSocket;
	strcpy(info->address, inet_ntoa(ClientAddr.sin_addr));

	return info;
}
