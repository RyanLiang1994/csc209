#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "mapreduce.h"

/* this is the function of map_worker which do the job of mapping chunk
 * outfd - the target pipe we are going to write in
 * infd - the pipe we are going to read from
 */
void map_worker(int outfd, int infd) {
    char temp_chunk[READSIZE + 1];
    char filedir[2 * MAX_FILENAME];
    while (read(infd, filedir, (2*MAX_FILENAME)) != 0) {;
	FILE *fr = fopen(filedir, "r");
	if (fr == NULL) {
	    perror("open failed");
	}
	
	while (fgets(temp_chunk, READSIZE, fr) != NULL) {
	    temp_chunk[READSIZE] = '\0';
	    map(temp_chunk, outfd);
	}
	if (fclose(fr) == -1) {
	    perror("fclose");
	}
    }
}
