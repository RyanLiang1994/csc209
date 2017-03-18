CFLAGS = -Wall -g -std=c99 -Werror

master: master.o mapworker.o reduceworker.o word_freq.o linkedlist.o
	gcc $(CFLAGS) -o mapreduce master.o mapworker.o reduceworker.o word_freq.o linkedlist.o

master.o: master.c mapreduce.h mapworker.o reduceworker.o
	gcc $(CFLAGS) -c master.c

mapworker.o: mapworker.c  mapreduce.h word_freq.c 
	gcc $(CFLAGS) -c mapworker.c 

reduceworker.o: reduceworker.c  mapreduce.h linkedlist.c word_freq.c
	gcc $(CFLAGS) -c reduceworker.c 

linkedlist.o: linkedlist.c linkedlist.h
	gcc $(CFLAGS) -c linkedlist.c 

word_freq.o: word_freq.c
	gcc $(CFLAGS) -c word_freq.c

clean:
	rm master *.o
