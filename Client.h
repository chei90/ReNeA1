/*
 * Client.h
 *
 *  Created on: 09.05.2013
 *      Author: Christoph
 */


#ifndef CLIENT_H_
#define CLIENT_H_

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
#include"unp_readline.c"
#include<errno.h>
#include<sys/select.h>
#include<sys/time.h>

	struct sockaddr_in server;
	int fd;
	socklen_t ssLength;
	pthread_t listener;
	fd_set fds;

	void printUsage();
	void checkInput();

#endif
