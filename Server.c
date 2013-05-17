/*
 * Server.c
 *
 *  Created on: 16.05.2013
 *      Author: christoph
 */

#include"Server.h"

void initializeList()
{
	firstEntry = malloc(sizeof(uList));
}

void addNewPlayer(char* uName, int port)
{
	uList* tmp;
	tmp = firstEntry;
	if (!userCount)
	{
		printf("First User\n");
		firstEntry->userName = uName;
		firstEntry->port = port;
	}
	else
	{
		int i;
		for (i = 0; i < userCount - 1; ++i)
		{
			tmp = tmp->next;
		}

		uList* insert;
		insert = malloc(sizeof(uList));

		tmp->next = insert;
		insert->previous = tmp;
		insert->userName = uName;
		insert->port = port;
	}
	userCount++;
}

int checkForPlayer(char* uName)
{
	uList* tmp = firstEntry;
	int i;

	for (i = 0; i < userCount; i++)
	{
		if (!strcmp(tmp->userName, uName))
		{
			return 1;
		}
		else
			tmp = tmp->next;
	}
	return 0;
}
int checkForPort(int uPort)
{
	printf("Step1\n");
	uList* tmp = firstEntry;
	int i;

	printf("Step1\n");
	for (i = 0; i < userCount; i++)
	{
		if (uPort == tmp->port)
		{
			return 1;
		}
		else
			tmp = tmp->next;
	}
	return 0;
}
int returnPort()
{
	printf("Step2\n");
	int i = portMin;
	while (checkForPort(i))
	{
		printf("I: %d\n", i);
		i++;
	}
	printf("I: %d\n", i);
	return i;
}
int deletePlayer(char* uName)
{
	uList* tmp = firstEntry;
	int i;

	for (i = 0; i < userCount; i++)
	{
		if (!strcmp(tmp->userName, uName))
		{
			if (i == (userCount - 1))
			{
				printf("Last User deleted\n");
				uList* prev = malloc(sizeof(uList));
				prev = tmp->previous;
				prev->next = NULL;
				free(tmp);
				userCount--;
			}
			else if (i == 0)
			{
				printf("I=0\n");
				if (userCount == 1)
				{
					printf("Letzter User\n");
					firstEntry->port = 0;
					firstEntry->userName = NULL;
				}
				else
				{
					printf("Erster User wird gelöscht!\n");
					firstEntry = tmp->next;
					free(tmp);
				}
				userCount--;
			}
			else
			{
				uList* prev = malloc(sizeof(uList));
				uList* next = malloc(sizeof(uList));

				printf("In the Middle\n");
				userCount--;
				prev = tmp->previous;
				next = tmp->next;

				prev->next = next;
				next->previous = prev;

				free(tmp);
			}
		}
		else
			tmp = tmp->next;
	}
	return 0;
}

void printUsage()
{
	printf("\nUsage: \n\n"
			"./udp_server -p <port>\n"
			" -p: The Port you want to use\n\n");
}

int checkPort(char* serverPort)
{
	if (atoi(serverPort) <= 1024 || atoi(serverPort) > 65535)
	{
		return 0;
	}
	return 1;
}

int main(int argc, char** argv)
{
	initializeList();
	if (argc != 3)
		printUsage();

	int opt;
	char* serverPort;

	while ((opt = getopt(argc, argv, "p:")) != -1)
	{
		switch (opt)
		{
		case 'p':
			serverPort = optarg;
			break;

		default:
			printUsage();
			exit(1);
			break;
		}
	}

	if (checkPort(serverPort))
	{
		server.sin_port = htons(atoi(serverPort));
	}
	else
	{
		printf("Invalid Port! Has to be in between 1025 and 65535\n");
		exit(1);
	}

	server.sin_family = AF_INET;
	inet_aton("127.0.0.1", &server.sin_addr);
	fd = socket(AF_INET, SOCK_DGRAM, 0);

	if (fd < 0)
	{
		printf("Fehler bei Socketerstellung\n");
		exit(1);
	}

	if (bind(fd, (struct sockaddr*) &server, sizeof(struct sockaddr_in)) < 0)
	{
		printf("Fehler beim Binding");
		exit(1);
	}

	char* clMsg = malloc(64 * sizeof(char));
	char* ipAdress = inet_ntoa(server.sin_addr);
	printf("ServerAdress is: %s\n", ipAdress);

	while (1)
	{
		if (recvfrom(fd, clMsg, 64 * sizeof(char), 0,
				(struct sockaddr*) &server, &ssLength) == -1)
		{
			printf("Error occured while receiving new connector\n");
			exit(1);
		}
		else
		{
			if (clMsg[0] == CL_CON_REQ)
			{
				uint16_t userNameLength;
				memcpy(&userNameLength, clMsg + sizeof(uint8_t),
						sizeof(uint16_t));
				userNameLength = ntohs(userNameLength);
				char* userName = (char*) malloc(sizeof(char) * userNameLength);

				memcpy(userName, clMsg + sizeof(uint8_t) + sizeof(uint16_t),
						sizeof(char) * userNameLength);
				int userPort = returnPort();
				printf("%s connected\n", userName);
				if (checkForPlayer(userName))
				{
					char* svMsg = malloc(sizeof(char) * 2);
					svMsg[0] = 0x02;
					svMsg[1] = 0x01;

					if (sendto(fd, svMsg, 2 * sizeof(char), 0,
							(struct sockaddr*) &server,
							sizeof(struct sockaddr_in)) < 0)
					{
						printf("Error at sending");
					}
				}
				else
				{
					char* svMsg = malloc(sizeof(char)*4);
					svMsg[0] = 0x02;
					svMsg[1] = 0x00;
					uint16_t tmpUPort = htons(userPort);
					memcpy(svMsg+sizeof(uint8_t)+sizeof(uint8_t), &tmpUPort, sizeof(uint16_t));
					addNewPlayer(userName, userPort);
					if (sendto(fd, svMsg, 4 * sizeof(char), 0,
							(struct sockaddr*) &server,
							sizeof(struct sockaddr_in)) < 0)
					{
						printf("Error at sending");
					}
				}


			}
		}
	}

	return 1;
}
