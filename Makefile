all:	
	clear
	gcc Client.c -o client -g -w -Wall -pthread
	
clean: 
	rm client
