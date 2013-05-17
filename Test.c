/*
 * Test.c
 *
 *  Created on: 16.05.2013
 *      Author: christoph
 */


#include"Server.c"


int main()
{
	initializeList();
	addNewPlayer("Christoph", 2222);
	addNewPlayer("Herbert", 23456);
	addNewPlayer("Felix", 4444);
	deletePlayer("Felix");

	uList* tmp = firstEntry;
	int i;

	printf("User->next: %s\n", tmp->userName);
	for(i = 0; i<userCount-1; i++)
	{
		tmp = tmp->next;
		printf("User->next: %s\n", tmp->userName);
	}
//	for(i = 0; i<userCount; i++)
//	{
//		printf("User->next: %s\n", tmp->userName);
//		tmp = tmp->previous;
//	}
	deletePlayer("Christoph");

	printf("First User: %s, User Count %i\n",firstEntry->userName, userCount);
}
