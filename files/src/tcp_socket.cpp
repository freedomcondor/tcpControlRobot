/*-------------------------------------------------------------*/
/*
 * 	version 1.1:	add read time out
 */
/*-------------------------------------------------------------*/

#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "tcp_socket.h"

TCPSocket::TCPSocket(char address_ch[], int _port)
{
	strcpy(address,address_ch);
	port = _port;
}

int TCPSocket::Open()
{
	if (strcmp(address,"server") == 0)
	{
		struct sockaddr_in address;
		int addrlen = sizeof(address);
		int opt = 1;

		// a server, setup a server
		// Creating socket file descriptor
		if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
		{
			printf("socket failed");
			return -1;
		}
		// Forcefully attaching socket to the port 8080
		if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
					&opt, sizeof(opt)))
		{
			printf("setsockopt");
			return -1;
		}
		address.sin_family = AF_INET;
		address.sin_addr.s_addr = INADDR_ANY;
		address.sin_port = htons( port );

		// Forcefully attaching socket to the port 8080
		if (bind(server_fd, (struct sockaddr *)&address, 
					sizeof(address))<0)
		{
			printf("bind failed");
			return -1;
		}

		printf("tcp server is listening\n");
		if (listen(server_fd, 3) < 0)
		{
			printf("listening");
			return -1;
		}
		if ((sock = accept(server_fd, (struct sockaddr *)&address, 
						(socklen_t*)&addrlen))<0)
		{
			printf("accept");
			return -1;
		}
		printf("server:connection sock established\n");
		SERorCLI = 0;
	}
	else
	{
		struct sockaddr_in serv_addr;
		// a client, setup a client
		if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		{
			printf("\n Socket creation error \n");
			return -1;
		}

		memset(&serv_addr, '0', sizeof(serv_addr));

		serv_addr.sin_family = AF_INET;
		serv_addr.sin_port = htons(port);

		// Convert IPv4 and IPv6 addresses from text to binary form
		if(inet_pton(AF_INET, address, &serv_addr.sin_addr)<=0) 
		{
			printf("\nInvalid address/ Address not supported \n");
			return -1;
		}
  
		printf("tcp client is trying to connect server\n");
		if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		{
			printf("Connection Failed \n");
			return -1;
		}
		printf("client:connection sock established\n");
		SERorCLI = 1;
	}
	return 0;
}

int TCPSocket::Close()
{
	// something should be filled here
	return 0;
}

int TCPSocket::Read(char *buffer,int *len)
{
	int valread;
	valread = read(sock , buffer, 1024);
	*len = valread;

	return 0;
}
int TCPSocket::Read(char *buffer,int *len, int timeout)
{
	int valread;
	*len = 0;

	struct pollfd fd;
	int ret;
	fd.fd = sock;
	fd.events = POLLIN;
	ret = poll(&fd, 1, timeout);
	switch (ret)
	{
		case -1: return -1; break;	// error
		case 0: return -1; break;	// timeout
		default:
			valread = read(sock , buffer, 1024);
			*len = valread;
	}

	return 0;
}
int TCPSocket::Write(char *buffer, int len)
{
	send(sock , buffer , len , 0 );
	return 0;
}
