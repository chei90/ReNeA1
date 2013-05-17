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

#define portMin 1025
#define portMax 65535

typedef struct user
{
	char* userName;
	int port;
	struct user* next;
	struct user* previous;
} uList;
uList* firstEntry;

static int userCount = 0;

struct sockaddr_in server;
socklen_t ssLength;

int fd;


void addNewPlayer(char*, int);
int checkForPlayer(char*);
void initializeList();
int deletePlayer(char*);

#endif
