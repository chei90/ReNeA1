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

checkPort(char* serverPort)
{

}

int main(int argc, char** argv)
{
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

	checkPort(serverPort);

	return 1;
}
