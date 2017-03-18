#include <stdio.h>
#include <stdlib.h>
#include "mapreduce.h"
#include "word_freq.c"

void map_worker(int outfd, int infd) {
	char temp_chunk[READSIZE + 1];
	char filedir[2*MAX_FILENAME];
	while (read(infd, filedir, (2*MAX_FILENAME)) != 0) {;
		FILE *fr = fopen(filedir, "r");
		if (fr == NULL) {
			perror("open failed");
			exit(1);
		}
	
		while (fgets(temp_chunk, READSIZE, fr) != NULL) {
			temp_chunk[READSIZE] = '\0';

			map(temp_chunk, outfd);
		}
		fclose(fr);
	}
}
