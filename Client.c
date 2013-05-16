/*
 * Client.c
 *
 *  Created on: 09.05.2013
 *      Author: Christoph
 */

#include"Client.h"

/*
 * @brief prints usage information at wrong input
 */
void printUsage()
{
	printf(
			"Usage: \n \
			./udp_chat_client -s <serv_addr> -p <serv_port> -u <user> \n \
			<serv_addr>: Ip of the Chat-Server you wish to connect to \n \
			<serv_port>: Port of the Chat-Server \n \
			<user>: Your User-Name \n");
	exit(1);
}

/**
 * @brief checks userinput whether is valid oder not and if so, stores the input
 * @param ip char* containing the ip of the server - given by the user
 * @param port char* containing the Port of the server - given by userinput
 * @param name char* containing the username - given by the user
 */
void checkInput(char* ip, char* port, char* name)
{
	int check = 1, i = 0;

	//checks if adress is valid and if so, stores the adress in specified struct
	if (inet_aton(ip, &server.sin_addr) == 0)
	{
		printf("Invalid IP! \n");
		check = 0;
	}

	//checks if port is in between the valid values
	if (1024 > atoi(port) && atoi(port) < 65535)
	{
		printf("Invalid Port! Has to be in between 1024 and 65535 \n");
		check = 0;
	}
	//store port if valid
	else
		server.sin_port = htons(atoi(port));

	//checks if username contains alphanum signs only
	for (i = 0; i < strlen(name); i++)
	{
		if (!isalnum(name[i]))
		{
			check = 0;
			printf("Invalid name! Only alphanum. signs allowed. \n");
			break;
		}
	}
	if (!check)
		exit(1);

	server.sin_family = AF_INET;
}
/*
 void receive()
 {
 char* serverMessage = (char*) malloc(128);
 uint8_t identifyer;
 int size;

 size = recvfrom(fd, serverMessage, 100, 0, (struct sockaddr*) &server, &ssLength);
 printf("\nPort normal:%d htons %d ntohs %d\n", server.sin_port, htons(server.sin_port), ntohs(server.sin_port));

 printf("\nSize: %d\n", size);
 memcpy(&identifyer, serverMessage, sizeof(uint8_t));
 fflush(stdout);
 while (1)
 {

 size = recvfrom(fd, serverMessage, 128, 0, (struct sockaddr*) &server, &ssLength);
 if (size == -1 || size == 0)
 //printf("No characters were received...");
 break;
 else
 {
 printf("Characters received! %d ", size);
 break;
 fflush(stdin);
 memcpy(&identifyer, serverMessage, sizeof(uint8_t));
 if(identifyer)printf("Identifyer is: %u\n",identifyer);
 //serverMessage += sizeof(uint8_t);

 switch (identifyer)
 {
 /*case SV_CON_REP:
 {
 //check if server accepted or not...
 printf("Juhu!");
 uint8_t tmp;
 memcpy(&tmp, serverMessage, sizeof(uint8_t));
 if (tmp == 0)
 {
 uint16_t tmpPort;
 serverMessage += sizeof(uint8_t);
 memcpy(&tmpPort, serverMessage, sizeof(uint16_t));
 server.sin_port = ntohs(tmpPort);
 printf("Connection established! New Port is %d", tmpPort);
 }
 if (tmp == 1)
 {
 printf("Connection rejected...");
 exit(1);
 }
 break;
 }

 case SV_CON_AMSG:
 {
 uint16_t userNameLength;

 //print name of newly connected user
 memcpy(&userNameLength, serverMessage, sizeof(uint16_t));
 userNameLength = ntohs(userNameLength);
 serverMessage += sizeof(uint16_t);

 char* newUser = (char*) malloc(userNameLength * sizeof(char));

 memcpy(newUser, serverMessage, userNameLength);
 printf("%s has entered the Chat.", newUser);
 free(newUser);
 break;
 }


 case SV_DISC_REP:
 {
 printf("Verbindung erfolgreich beendet\n");
 exit(1);
 break;
 }

 case SV_DISC_AMSG:
 {
 uint16_t userNameLength;

 //print name of newly connected user
 memcpy(&userNameLength, serverMessage, sizeof(uint16_t));
 userNameLength = ntohs(userNameLength);
 serverMessage += sizeof(uint16_t);

 char* newUser = (char*) malloc(userNameLength * sizeof(char));

 memcpy(newUser, serverMessage, userNameLength);
 printf("%s has disconnected from Chat.", newUser);
 free(newUser);
 break;
 }

 case SV_PING_REQ:
 {
 break;
 }

 }
 }
 }
 }*/
void inputHandler()
{
	while (1)
	{
		char* inputBuffer = malloc(1019 * sizeof(char));

		int size = readline(0, inputBuffer, 1019);
		if (size > 0)
		{
			printf("CharPointer hat die Länge: %d und %s \n", size, inputBuffer);
			sendInformation(inputBuffer);

		}
	}
}

void sendInformation(char* input)
{
	int characters = sendto(fd, input,strlen(input) * sizeof(char)*10+100, 0,
					 &server, sizeof(struct sockaddr_in));

	printf("Sent [%d] signs.\n", characters);
	printf( "Error in transmitting: %s\n", strerror( errno ) );
}

int main(int argc, char** argv)
{
	if (argc != 7)
		printUsage();

	int opt;
	char* serverIp;
	char* serverPort;
	char* userName;

	while ((opt = getopt(argc, argv, "s:p:u:")) != -1)
	{
		switch (opt)
		{
		case 's':
			serverIp = optarg;
			break;
		case 'p':
			serverPort = optarg;
			break;
		case 'u':
			userName = optarg;
			break;
		}
	}

	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd == -1)
	{
		printf("Socket initialization failed...\n");
		exit(1);
	}
	checkInput(serverIp, serverPort, userName);

	char* clientMessage = malloc(
			sizeof(uint8_t) + sizeof(uint16_t)
					+ strlen(userName) * sizeof(char));

	uint8_t conreq = 1;
	memcpy(clientMessage, &conreq, sizeof(uint8_t));

	uint16_t length = strlen(userName) * sizeof(char);
	length = htons(length);
	memcpy(clientMessage + sizeof(uint8_t), &length, sizeof(uint16_t));
	memcpy(clientMessage + sizeof(uint8_t) + sizeof(uint16_t), userName,
			strlen(userName));

	int tries, characters;

	for (tries = 0; tries <= 3; tries++)
	{
		characters = sendto(fd, clientMessage,
				sizeof(uint8_t) + sizeof(uint16_t)
						+ strlen(userName) * sizeof(char), 0,
				(struct sockaddr*) &server, sizeof(struct sockaddr_in));
		if (characters == -1)
		{
			printf("Something went wrong while connecting... retrying\n");
			sleep(5);
			continue;
		}
		else
		{
			char* buffer = (char*) malloc(128);
			characters = recvfrom(fd, buffer, 128, 0,
					(struct sockaddr*) &server, &ssLength);

			if (characters <= 0)
			{
				printf("Server didn't answer... retrying.\n");
				continue;
			}
			else if (buffer[0] == SV_CON_REP)
			{
				if (!buffer[1])
				{
					uint16_t port;
					memcpy(&port, buffer + 2, sizeof(uint16_t));
					server.sin_port = port;
					break;
				}
				else
				{
					printf(
							"For some reason the server rejected you... retry with an other username\n");
					exit(0);
				}
			}
		}
	}
	printf("Connected Successfully...\n");
	pthread_create(&listener, NULL, (void*) &inputHandler, NULL);
	while (1)
	{

		char* serverMessage = malloc(128);
		int size = recvfrom(fd, serverMessage, 128, 0,
				(struct sockaddr*) &server, &ssLength);

		if (size == -1)
		{
			printf("Error at receiving!\n");
			exit(1);
		}
		if (size == 0)
		{
			printf("No signs received!\n");
		}

		uint8_t identifyer;
		memcpy(&identifyer, serverMessage, sizeof(uint8_t));

		serverMessage += sizeof(uint8_t);

		switch (identifyer)
		{
		case SV_MSG:
		{
			uint32_t messageLength;

			memcpy(&messageLength, serverMessage, sizeof(uint32_t));
			messageLength = ntohl(messageLength);
			serverMessage += sizeof(uint32_t);

			char* message = (char*) malloc(messageLength * sizeof(char));
			memcpy(message, serverMessage, messageLength);

			printf("#server#: %s \n", message);
			free(message);
			break;
		}
		case SV_AMSG:
		{
			uint16_t userNameLength;
			uint32_t messageLength;
			char* userName;
			char* message;

			memcpy(&userNameLength, serverMessage, sizeof(uint16_t));
			serverMessage += sizeof(uint16_t);
			userNameLength = ntohs(userNameLength);
			printf("userNameLength: %u\n", userNameLength);
			userName = (char*) malloc(userNameLength * sizeof(char));

			memcpy(userName, serverMessage, userNameLength * sizeof(char));
			serverMessage += userNameLength * sizeof(char);

			memcpy(&messageLength, serverMessage, sizeof(uint32_t));
			messageLength = ntohl(messageLength);
			serverMessage += sizeof(uint32_t);
			message = (char*) malloc(messageLength * sizeof(char));

			memcpy(message, serverMessage, messageLength * sizeof(char));

			printf("<%s>: %s", userName, message);
			free(userName);
			free(message);
			break;
		}
		case SV_PING_REQ:
		{
			char* pingBuffer = malloc(sizeof(char));
			uint8_t pingRep = 10;
			memcpy(pingBuffer, &pingRep, sizeof(uint8_t));
			int size = sendto(fd, pingBuffer, 1, 0, (struct sockaddr*) &server,
					sizeof(struct sockaddr_in));
			if (size == -1)
				printf("Error occured at Pingreply!\n");
			if (size == 0)
				printf("No Sings were send to Pingreply!\n");
			break;
		}
		}
	}
	return 1;
}
