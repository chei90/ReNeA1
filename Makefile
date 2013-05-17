all:	
	clear
	gcc Client.c -o client -g -w -Wall
	gcc Server.c -o server -g -w -Wall
			
clean: 
	rm client
	rm server

