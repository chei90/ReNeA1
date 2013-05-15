all:
	clear
	gcc Client.c -o client -Wall -pthread
	
clean: 
	rm client
