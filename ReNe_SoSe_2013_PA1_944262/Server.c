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
/*
 * @brief method that is utilized by every thread.
 * 			it handles the input sent from every user, so for
 * 			every new user, the server starts an own thread
 * @param currentUser, a struct that contains references to every other user and
 * 			their informations
 */
void clientHandling(void* currentUser)
{
	//Counts the pingreplies of every user, when 0 the thread stops
	int repliedPings = 3;
	uList* user = (uList*) currentUser;
	char* clientMessage = malloc(256*sizeof(char));


	socklen_t currentClientSocketLength = sizeof(user->clientAdress);
	struct timeval timeout;


	while (repliedPings)
	{

		timeout.tv_sec = 5;
		timeout.tv_usec = 0;
		FD_ZERO(&fds);
		FD_SET(user->clientFD, &fds);
		FD_SET(0, &fds);
		if (select((user->clientFD) + 1, &fds, 0, 0, &timeout) < 0)
			printf("Error in select in thread of <%s>.\n", user->userName);

		// if there is a new message incoming...
		if (FD_ISSET(user->clientFD, &fds))
		{
			if (recvfrom(user->clientFD, clientMessage, sizeof(char)*256,
					0, (struct sockaddr*) &(user->clientAdress), &currentClientSocketLength) == -1)
				printf("Error at receiving Message from %s.\n", user->userName);

			//handle it depending on the identifyer
			switch (clientMessage[0])
			{
			//... here a chat method is incoming...
			case CL_MSG:
			{
				uint16_t userNameLength = strlen(user->userName);
				uint16_t userNameLengthToNetwork = htons(userNameLength);
				uint32_t messageLength;
				uint32_t messageLengthToNetwork;
				memcpy(&messageLengthToNetwork, clientMessage + 1,
						sizeof(messageLengthToNetwork));
				messageLength = ntohl(messageLengthToNetwork);

				char* messageToAll = malloc(
						sizeof(uint8_t) + sizeof(uint16_t)
								+ userNameLength * sizeof(char)
								+ sizeof(uint32_t)
								+ messageLength * sizeof(char));

				messageToAll[0] = SV_AMSG;
				memcpy(messageToAll + sizeof(uint8_t), &userNameLengthToNetwork,
						sizeof(userNameLengthToNetwork));
				memcpy(messageToAll + sizeof(uint8_t) + sizeof(uint16_t),
						user->userName, userNameLength * sizeof(char));
				memcpy(
						messageToAll + sizeof(uint8_t) + sizeof(uint16_t)
								+ userNameLength * sizeof(char),
						&messageLengthToNetwork, sizeof(uint32_t));
				memcpy(
						messageToAll + sizeof(uint8_t) + sizeof(uint16_t)
								+ userNameLength * sizeof(char)
								+ sizeof(uint32_t),
						clientMessage + sizeof(uint8_t) + sizeof(uint16_t)
								+ sizeof(userNameLength) * sizeof(char),
						messageLength * sizeof(uint8_t));

				uList* i;

				//...and spread to every other registered client...
				for (i = firstEntry; i != NULL; i = i->next)
				{
					if (sendto(i->clientFD, messageToAll,
							sizeof(uint8_t) + sizeof(uint16_t)
									+ userNameLength * sizeof(char)
									+ sizeof(uint32_t)
									+ messageLength * sizeof(char), 0,
							(struct sockaddr *) &(i->clientAdress),
							sizeof(i->clientAdress)) == -1)
						printf(
								"Error at sending message.(Client Handler)\n");
				}
				free(messageToAll);
				break;
			}
			// In case a user wants to disconnect...
			case CL_DISC_REQ:
			{
				//we reply to him and...
				uint8_t discRep = SV_DISC_REP;
				if (sendto(user->clientFD, &discRep, sizeof(discRep), 0,
						(struct sockaddr *) &(user->clientAdress),
						sizeof(user->clientAdress)) == -1)
					printf("Error sending disconnec reply to <%s>.\n",
							user->userName);
				//... set his pingreply to 0, so loop ends and we delete him
				repliedPings=0;
				break;
			}
			// If the user replies to our pingrequest, his counter is reset to initial value
			case CL_PING_REP:
				repliedPings = 3;
				break;
			default:
				printf("Couldnt compute the incoming message in Thread of User %s\n",
						user->userName);
				break;
			}
		}
		else
		{
			repliedPings--;
			if (repliedPings < 2)
				printf("%s doesn't answer for the %d time\n", user->userName, 3 - repliedPings);
			// send SV_PING_REQ
			uint8_t ping = SV_PING_REQ;
			if (sendto(user->clientFD, &ping, sizeof(uint8_t), 0,
					(struct sockaddr*) &(user->clientAdress),
					sizeof(user->clientAdress)) == -1)
				printf("Error sending ping to <%s>", user->userName);

		}
	}
	printf("<%s> disconnected.\n", user->userName);
	deleteUser((uList*) currentUser);
	pthread_exit(NULL);
}

/**
 * @brief checks whether username is allready used or not
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
 * @brief Adds user to the list
 * @param buffer - the received servermessage, containing all information
 * @param address - the address of the User
 * @param port - the port of the new user
 */
void addUser(char* buffer, struct sockaddr_in address, uint16_t port,
		int serverfd)
{
	struct sockaddr_in newClientSocket;
	newClientSocket.sin_family = AF_INET;
	newClientSocket.sin_port = htons(port);
	newClientSocket.sin_addr.s_addr = INADDR_ANY;
	socklen_t newClientSocketLength = sizeof(struct sockaddr_in);

	uint16_t userNameLength;

	memcpy(&userNameLength, buffer + sizeof(uint8_t), sizeof(uint16_t));
	userNameLength = ntohs(userNameLength);

	char* newUsersName = malloc(sizeof(char) * userNameLength);
	memcpy(newUsersName, buffer + sizeof(uint8_t) + sizeof(uint16_t),
			userNameLength * sizeof(char));

	char* newUsersNameCopy = malloc(sizeof(char) * (userNameLength));
	memcpy(newUsersNameCopy, newUsersName, sizeof(char) * userNameLength);

	int newClientFd = socket(AF_INET, SOCK_DGRAM, 0);
	if (bind(newClientFd, (struct sockaddr *) &newClientSocket,
			sizeof(newClientSocket)) == -1)
		printf("Error at socketbinding (addUser).");

	uList* user = malloc(sizeof(uList));
	user->userName = newUsersNameCopy;
	user->clientAdress = address;
	user->clientFD = newClientFd;
	user->port = port;

	char* svMsg = malloc(sizeof(char) * 4);
	//List is Empty
	if (lastEntry == NULL)
	{
		firstEntry = lastEntry = user;
		if (pthread_create(&(user->thread), NULL, (void*) &clientHandling, user)
				!= 0)
		{
			printf("Error at pthread_create.\n");
			exit(1);
		}

		svMsg[0] = SV_CON_REP;
		//We have to accept the first USer
		svMsg[1] = 0x00;
		if (sendto(serverfd, svMsg, 2, 0, (struct sockaddr *) &address,
				sizeof(address)) == -1)
			printf("Error at SV_CON_REP (addUSer).\n");

		printf("<%s> connected.\n", newUsersName);
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
			printf("<%s> declined. Name already in use.\n", newUsersName);
			svMsg[0] = SV_CON_REP;
			svMsg[1] = 0x01;
			if (sendto(serverfd, svMsg, 2, 0, (struct sockaddr *) &address,
					sizeof(address)) == -1)
				printf("Error at SV_CON_REP (addUser2).\n");
			free(newUsersNameCopy);
		}
		//User doesnt exist so we can save him
		else
		{
			lastEntry->next = user;
			user->previous = lastEntry;
			lastEntry = user;
			if (pthread_create(&(user->thread), NULL, (void*) &clientHandling,
					user) != 0)
			{
				printf("Error at pthread_create.\n");
				exit(1);
			}

			svMsg[0] = SV_CON_REP;
			svMsg[1] = 0x00;

			memcpy(svMsg + 2, &(newClientSocket.sin_port), sizeof(uint16_t));
			if (sendto(serverfd, svMsg, sizeof(svMsg), 0,
					(struct sockaddr *) &address, sizeof(address)) == -1)
				printf("Error at SV_CON_REP (addUser3).\n");

			printf("<%s> joined the server.\n", newUsersName);
			uint16_t nameLengthToNetwork = htons(userNameLength);
			char* svMsgToAll = malloc(
					sizeof(uint8_t) + sizeof(uint16_t)
							+ userNameLength * sizeof(char));
			svMsgToAll[0] = SV_CON_AMSG;
			memcpy(svMsgToAll + sizeof(uint8_t), &nameLengthToNetwork,
					sizeof(uint16_t));
			memcpy(svMsgToAll + sizeof(uint8_t) + sizeof(uint16_t),
					newUsersName, userNameLength * sizeof(char));

			//Introduce the newly joined User to everybody else
			uList* i;
			for (i = firstEntry; i != NULL; i = i->next)
			{
				if (sendto(i->clientFD, svMsgToAll, 3 + userNameLength, 0,
						(struct sockaddr *) &(i->clientAdress),
						sizeof(i->clientAdress)) == -1)
					printf("Error at SV_AMSG.\n");
			}
			free(svMsgToAll);
		}
	}
	free(newUsersName);
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

	svMsg[0] = SV_DISC_AMSG;

	memcpy(svMsg + sizeof(uint8_t), &userNameLength, sizeof(uint16_t));
	memcpy(svMsg + sizeof(uint8_t) + sizeof(uint16_t), user->userName, ntohs(userNameLength));

	uList* i;
	for (i = firstEntry; i != NULL; i = i->next)
	{
		if (sendto(i->clientFD, svMsg, 1 + 2 + ntohs(userNameLength), 0,
				(struct sockaddr *) &(i->clientAdress), sizeof(i->clientAdress))
				< 0)
			printf("Error at SV_DISC_AMSG.\n",
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
		if (recvfrom(fd, clientMessage, sizeof(char) * 256, 0,
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
