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
		server.sin_port = ntohs(atoi(port));

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

void receive()
{
	char* serverMessage = (char*) malloc(128);
	uint8_t identifyer, size;

	while (1)
	{
		size = recvfrom(fd, serverMessage, 128, 0,
				(struct sockaddr*) &server, &ssLength);


		if (size == 0)
			printf("No characters were received...");
		else
		{
			memcpy(&identifyer, serverMessage, sizeof(uint8_t));
			serverMessage += sizeof(uint8_t);

			printf("Identifyer: %u \n", identifyer);
			switch (identifyer)
			{
			case SV_CON_REP:
			{
				//check if server accepted or not...
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

			case SV_AMSG:
			{
				uint16_t userNameLength;
				uint32_t messageLength;
				char* userName;
				char* message;

				memcpy(&userNameLength, serverMessage, sizeof(uint16_t));
				serverMessage += sizeof(uint16_t);
				userNameLength = ntohs(userNameLength);
				userName = (char*) malloc(userNameLength * sizeof(char));

				memcpy(userName, serverMessage, sizeof(userNameLength));
				serverMessage += sizeof(userNameLength);

				memcpy(&messageLength, serverMessage, sizeof(uint32_t));
				messageLength = ntohl(messageLength);
				serverMessage += sizeof(uint32_t);
				message = (char*) malloc(messageLength * sizeof(char));

				printf("<%s>: %s", userName, message);
				free(userName);
				free(message);
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

			case SV_MSG:
			{
				uint32_t messageLength;

				memcpy(&messageLength, serverMessage, sizeof(uint32_t));
				messageLength = ntohl(messageLength);

				char* message = (char*) malloc(messageLength * sizeof(char));
				memcpy(message, serverMessage, messageLength);

				printf("#server#: %s \n", message);
				free(message);
				break;
			}
			}
		}
	}
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
		printf("Socket initialization failed...");
		exit(1);
	}
	checkInput(serverIp, serverPort, userName);

	char* clientMessage = malloc(
			sizeof(uint8_t) + sizeof(uint16_t) + strlen(userName)*sizeof(char));

	clientMessage[0] = 0x01;
	//clientMessage += sizeof(uint8_t);

	uint16_t length = strlen(userName)*sizeof(char);
	length = htons(length);
	printf("Length: %d",  htons(length));
	memcpy(clientMessage+1, &length, sizeof(uint16_t));
	printf("länge msg: %d\n", strlen(clientMessage));


	//clientMessage += sizeof(uint16_t);
	//clientMessage -= sizeof(uint8_t) + sizeof(uint16_t);
	memcpy(clientMessage+3, userName, strlen(userName));


	printf("länge msg: %d\n", strlen(clientMessage));
	int i;
	for(i = 0; i < strlen(clientMessage); i++)
	{
		printf("%d: %u\n",i, clientMessage[i]);
	}

	int tries, characters;
	//pthread_create(&receiver, NULL, (void*) &receive, NULL);

	for (tries = 0; tries <= 3; tries++)
	{
		characters = sendto(fd, clientMessage, sizeof(clientMessage)*sizeof(char), 0,
				(struct sockaddr*) &server, sizeof(struct sockaddr_in));
		sleep(1);
		if (characters == -1)
		{
			printf("Something went wrong while connecting... retrying");
			continue;
		}
		else
		{
			printf("Signs sent: %d", characters);
		}
		sleep(5);
		//Todo. Receive clientMessages...
	}
	return 1;
}
