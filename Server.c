#include"Server.h"

/**
 * @brief Prints usage Message to Console
 */
void usage(void)
{
	printf("Usage:\n");
	printf("/udp_chat_server -p <serv_port>\n");
	printf(
			"<serv_port>: The Port you want to use. Must be in betweeen 1024 and 65535\n");
}
/**
 * Routine for user thread.
 */
void thread_routine(void* args)
{
	//give some time to send connection reply
	sleep(1);
	int running = 1;
	uList* user = (uList*) args;
	char clientMessage[256];
	socklen_t slen = sizeof(user->clientAdress);
	uint8_t ping;
	int pingcnt = 0;

	uint8_t* svMsg, discRep;
	uint16_t namelen, namelenN;
	uint32_t msglen, msglenN;
	uList* n;

	//printf("Starting thread of <%s>.\n", user->name);

	fd_set fds;
	struct timeval timeout;

	while (running)
	{
		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		FD_ZERO(&fds);
		FD_SET(user->clientFD, &fds);
		FD_SET(0, &fds);
		if (select((user->clientFD) + 1, &fds, 0, 0, &timeout) < 0)
			printf("Error in select in thread of <%s>.\n", user->userName);

		if (FD_ISSET(user->clientFD, &fds))
		{
			if (recvfrom(user->clientFD, clientMessage, sizeof(clientMessage), 0,
					(struct sockaddr*) &(user->clientAdress), &slen) == -1)
				printf("Error receiving message from %s.\n", user->userName);


			switch (clientMessage[0])
			{
			// receive CL_MSG
			case CL_MSG:
				// send SV_AMSG
				namelen = (uint16_t) strlen(user->userName);
				namelenN = htons(namelen);
				memcpy(&msglenN, clientMessage + 1, sizeof(msglenN));
				msglen = ntohl(msglenN);
				svMsg = malloc(
						(1 + 2 + namelen + 4 + msglen) * sizeof(uint8_t));
				*svMsg = 0x05;
				memcpy(svMsg + 1, &namelenN, sizeof(namelenN));
				memcpy(svMsg + 1 + 2, user->userName, namelen * sizeof(char));
				memcpy(svMsg + 1 + 2 + namelen, &msglenN, sizeof(msglenN));
				memcpy(svMsg + 1 + 2 + namelen + 4,
						clientMessage + 1 + 2 + sizeof(namelen),
						msglen * sizeof(uint8_t));
				for (n = firstEntry; n != NULL; n = n->next)
				{
					if (sendto(n->clientFD, svMsg, 1 + 2 + namelen + 4 + msglen,
							0, (struct sockaddr *) &(n->clientAdress),
							sizeof(n->clientAdress)) == -1)
						printf(
								"Error sending user message from <%s> to <%s>.\n",
								user->userName, n->userName);
				}
				free(svMsg);
				break;
				// receive CL_DISC_REQ
			case CL_DISC_REQ:
				// send SV_DISC_REP
				discRep = 0x07;
				if (sendto(user->clientFD, &discRep, sizeof(discRep), 0,
						(struct sockaddr *) &(user->clientAdress),
						sizeof(user->clientAdress)) == -1)
					printf("Error sending disconnec reply to <%s>.\n",
							user->userName);
				running--;
				break;
				// receive CL_PING_REP
			case CL_PING_REP:
				pingcnt = 0;
				break;
			default:
				printf("Incoming message with unknown ID %d from <%s>.\n",
						*clientMessage, user->userName);
				break;
			}
		}
		else
		{
			++pingcnt;
			if (pingcnt > 1)
				printf("Unanswered ping (%d/3) from <%s>.\n", pingcnt - 1,
						user->userName);
			if (pingcnt > 3)
			{
				running--;
				printf("Lost connection to <%s>. Timeout.\n", user->userName);
			}
			else
			{
				// send SV_PING_REQ
				ping = 0x09;
				if (sendto(user->clientFD, &ping, sizeof(ping), 0,
						(struct sockaddr*) &(user->clientAdress),
						sizeof(user->clientAdress)) == -1)
					printf("Error sending ping to <%s>", user->userName);
			}
		}
	}
	//printf("Killing thread of <%s>.\n", user->name);
	printf("<%s> left the server.\n", user->userName);
	deleteUser((uList*) args);
	pthread_exit(NULL);
}

/**
 * @brief checks whether username is allreade used or noth
 * @return 1 if users in list
 * @return 0 if not
 */
int checkForUser(char* name)
{
	uList* user;
	for (user = firstEntry; user != NULL; user = user->next)
	{
		if (!strcmp(user->userName, name))
			return 1;
	}
	return 0;
}

/**
 *  Add new user: create data, socket and thread.
 *  @return 0 on success
 */
int addUser(char* buffer, struct sockaddr_in address, uint16_t port,
		int serverfd)
{

	struct sockaddr_in newClientSocket;
	newClientSocket.sin_family = AF_INET;
	newClientSocket.sin_port = htons(port);
	newClientSocket.sin_addr.s_addr = INADDR_ANY;
	socklen_t slen = sizeof(struct sockaddr_in);

	uint16_t userNameLength;
	memcpy(&userNameLength, buffer + sizeof(uint8_t), sizeof(uint16_t));
	userNameLength = ntohs(userNameLength);

	char* newUsersName = malloc(sizeof(char) * (userNameLength));
	memcpy(newUsersName, buffer + sizeof(uint8_t) + sizeof(uint16_t),
			userNameLength * sizeof(char));

	char* newUsersNameCopy = malloc(sizeof(char) * (userNameLength));
	memcpy(newUsersNameCopy, newUsersName, sizeof(char) * userNameLength);

	//socket fd
	int newClientFd = socket(AF_INET, SOCK_DGRAM, 0);
	if (bind(newClientFd, (struct sockaddr *) &newClientSocket,
			sizeof(newClientSocket)) == -1)
		printf("Error on bind.");

	uList* user = malloc(sizeof(uList));
	user->userName = newUsersNameCopy;
	user->clientAdress = address;
	user->clientFD = newClientFd;

	char* svMsg = malloc(sizeof(char) * 4);
	//List is Empty
	if (lastEntry == NULL)
	{
		firstEntry = lastEntry = user;
		if (pthread_create(&(user->thread), NULL, (void*) &thread_routine, user)
				!= 0)
		{
			printf("Error creating user thread.\n");
			exit(1);
		}

		svMsg[0] = SV_CON_REP;
		//We have to accept the first USer
		svMsg[1] = 0x00;
		if (sendto(serverfd, svMsg, 2, 0, (struct sockaddr *) &address,
				sizeof(address)) == -1)
			printf("Error sending connection reply to client.\n");

		printf("<%s> joined the server.\n", newUsersName);
	}

	/*
	 * List is not empty so we have to check if the username is already used
	 * then add him and inform the other
	 */
	else
	{
		//check if user name already exists
		if (checkForUser(newUsersName))
		{
			printf("Connection request of <%s> declined.\n", newUsersName);
			svMsg[0] = SV_CON_REP;
			svMsg[1] = 0x01;
			if (sendto(serverfd, svMsg, 2, 0, (struct sockaddr *) &address,
					sizeof(address)) == -1)
				printf("Error sending connection reply to client.\n");
			free(newUsersNameCopy);
		}
		//User doesnt exist so we can save him
		else
		{
			lastEntry->next = user;
			user->previous = lastEntry;
			lastEntry = user;
			if (pthread_create(&(user->thread), NULL, (void*) &thread_routine,
					user) != 0)
			{
				printf("Error creating user thread.\i");
				return -1;
			}

			svMsg[0] = SV_CON_REP;
			svMsg[1] = 0x00;

			memcpy(svMsg + 2, &(newClientSocket.sin_port), sizeof(uint16_t));
			if (sendto(serverfd, svMsg, sizeof(svMsg), 0,
					(struct sockaddr *) &address, sizeof(address)) == -1)
				printf("Error sending connection reply to client.\i");

			printf("<%s> joined the server.\n", newUsersName);
			uint16_t nameLengthToNetwork = htons(userNameLength);
			char* svMsgToAll = malloc(
					sizeof(uint8_t) + sizeof(uint16_t)
							+ userNameLength * sizeof(char));
			svMsgToAll[0] = SV_CON_AMSG;
			memcpy(svMsgToAll + sizeof(uint8_t), &nameLengthToNetwork,
					sizeof(uint16_t));
			memcpy(svMsgToAll + sizeof(uint8_t) + sizeof(uint16_t),
					newUsersName, userNameLength*sizeof(char));

			//Introduce the newly joined User to everybody else
			uList* i;
			for (i = firstEntry; i != NULL; i = i->next)
			{
				if (sendto(i->clientFD, svMsgToAll, 3 + userNameLength, 0,
						(struct sockaddr *) &(i->clientAdress),
						sizeof(i->clientAdress)) == -1)
					printf("Error sending new-user-svMsgToAll to %s.\n", i->userName);
			}
			free(svMsgToAll);
		}
	}
	free(newUsersName);
	return 0;
}

/**
 * @brief deletes the specified user struct out of list, cancels all connections, and sends msg
 * @param user - the user you wand to delete
 */
void deleteUser(uList* user)
{
	uint16_t userNameLength = strlen(user->userName);
	char* svMsg = malloc(
			(sizeof(uint8_t) + sizeof(uint16_t) + userNameLength)
					* sizeof(uint8_t));

	userNameLength = htons(userNameLength);

	svMsg[0] = (uint8_t) 8;

	memcpy(svMsg + 1, &userNameLength, sizeof(uint16_t));
	memcpy(svMsg + 1 + 2, user->userName, ntohs(userNameLength));

	uList* i;
	for (i = firstEntry; i != NULL; i = i->next)
	{
		if (sendto(i->clientFD, svMsg, 1 + 2 + ntohs(userNameLength), 0,
				(struct sockaddr *) &(i->clientAdress), sizeof(i->clientAdress))
				< 0)
			printf("Error sending disconnecting message from <%s> to <%s>.\n",
					user->userName, i->userName);

	}

	uList* next = user->next;
	uList* previous = user->previous;

	// User is in the middle of the list
	if (next != NULL && previous != NULL)
	{
		previous->next = next;
		next->previous = previous;
	}

	// User has no predecessor, therefore -> first element
	if (next != NULL && previous == NULL)
	{
		next->previous = NULL;
		firstEntry = next;
	}

	// User has no successor, therefore -> last element
	if (next == NULL && previous != NULL)
	{
		previous->next = NULL;
		lastEntry = previous;
	}

	// User has neither successor nor precedessor, therefore -> list is
	// empty after deletion
	if (next == NULL && previous == NULL)
	{
		firstEntry = NULL;
		lastEntry = NULL;
	}

	// free Buffer
	free(user->userName);
	free(svMsg);
	free(user);

	close(user->clientFD);
	pthread_detach(user->thread);
}



int checkPort(char* port)
{
	int tmp = atoi(port);

	if (tmp < 1024 || tmp > 65535)
	{
		usage();
		exit(1);
	}
	return tmp;
}
/**
 * main
 */
int main(int argc, char** argv)
{
	firstEntry = lastEntry = NULL;

	if (argc != 3)
	{
		usage();
		exit(1);
	}

	int opt;
	while ((opt = getopt(argc, argv, "p:")) != -1)
	{
		switch (opt)
		{
		case 'p':
			serverPort = checkPort(optarg);
			break;
		default:
			usage();
			exit(1);
		}
	}

	uint16_t currentPort = serverPort + 1;
	fd_set fds;
	clientLength = sizeof(client);
	char* clientMessage = (char*) malloc(256 * sizeof(char));

	if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
		printf("Error creating socket.\n");

	server.sin_family = AF_INET;
	server.sin_port = htons(serverPort);
	server.sin_addr.s_addr = htonl(INADDR_ANY);

	if (bind(fd, (struct sockaddr*) &server, sizeof(server)) == -1)
		printf("Error binding socket.\n");
	else
		printf("Server set up and ready to use.\n");

	// main loop for receiving new user requests.
	while (1)
	{
		FD_ZERO(&fds);
		FD_SET(fd, &fds);

		if (select(fd + 1, &fds, 0, 0, 0) < 0)
			printf("Error in select.\n");

		if (FD_ISSET(fd, &fds))
		{
			if (recvfrom(fd, clientMessage, sizeof(clientMessage), 0,
					(struct sockaddr*) &client, &clientLength) == -1)
				printf("Error receiving message from new user.\n");
			else
			{
				//receive CL_CON_REQ
				if (clientMessage[0] == CL_CON_REQ)
				{
					addUser(clientMessage, client, currentPort, fd);
					if (currentPort++ == 65535)
						currentPort = 1025;
				}
			}
		}
	}
	return 0;
}
