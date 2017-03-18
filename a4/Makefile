PORT=54367
CFLAGS = -DPORT=\$(PORT) -Wall -g -std=c99 -Werror -D_GNU_SOURCE


friends_server: server.o friendme.o friends.o 
	gcc ${CFLAGS} -o friends_server server.o friendme.o friends.o

friendme.o: friendme.c friends.h 
	gcc ${CFLAGS} -c friendme.c 
	
server.o: server.c 
	gcc ${CFLAGS} -c server.c


friends.o: friends.c friends.h 
	gcc ${CFLAGS} -c friends.c


clean:  
	rm *.o 
