CFLAGS = -Wall -g -std=c99 -Werror

master: master.o mapworker.o reduceworker.o
	gcc $(CFLAGS) -o master master.o mapworker.o reduceworker.o

master.o: master.c mapreduce.h mapworker.o reduceworker.o
	gcc $(CFLAGS) -c master.c

mapworker.o: mapworker.c  mapreduce.h word_freq.c
	gcc $(CFLAGS) -c mapworker.c word_freq.c

reduceworker.o: reduceworker.c  mapreduce.h linkedlist.c word_freq.c
	gcc $(CFLAGS) -c reduceworker.c linkedlist.c word_freq.c 


clean:
	rm master *.o
