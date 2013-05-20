/*
 * Server.h
 *
 *  Created on: 16.05.2013
 *      Author: christoph
 */

#ifndef SERVER_H_
#define SERVER_H_

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<ctype.h>
#include<netinet/in.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include"Identifyers.h"
#include<errno.h>
#include<sys/select.h>
#include<sys/time.h>

#define portMin 1025
#define portMax 65535

typedef struct user
{
	char* userName;
	int clientFD;
	uint16_t port;
	struct sockaddr_in clientAdress;
	struct user* next;
	struct user* previous;
	pthread_t thread;
} uList;
uList* firstEntry;
uList* lastEntry;

static int maxFD;

struct sockaddr_in server;
struct sockaddr_in client;
socklen_t clientLength;

int fd, serverPort;
fd_set fds;

#endif
