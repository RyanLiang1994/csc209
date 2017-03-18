#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mapreduce.h"

///////////////////prototype//////////////////////////////
void reduce_and_write(LLKeyValues **head);
LLKeyValues *create_node(Pair pair); 
void insert_into_keys(LLKeyValues **head_ptr, Pair pair);
void free_key_values_list(LLKeyValues *head);


/* function of reduce worker 
 * outfd - the pipe we write the final data to
 * infd - the pipe we read data from
 */
void reduce_worker(int outfd, int infd){
    LLKeyValues *init_node = NULL;
    Pair temp_pair;
    int r;
    r = read(infd, &temp_pair, sizeof(Pair));
    if (r < 0) {
	perror("read error");
    }
    // create a list 
    while (r != 0) {
	if (init_node == NULL) {
	    init_node = create_node(temp_pair);
	}
	else {
	    insert_into_keys(&init_node, temp_pair);
	}
	r = read(infd, &temp_pair, sizeof(Pair));
	if (r < 0) {
	    perror("read error");
	}
    }
    // after read all pairs, start reduce
    reduce_and_write(&init_node);
    free_key_values_list(init_node);
}


/* This function is helper function to do the reduce job and
 * write the pairs to fhe file in binary
 *
 */
void reduce_and_write(LLKeyValues **head) {
    int pid = getpid();
    char name[10];
    sprintf(name, "%d.out", pid);
    FILE *ftw = fopen(name, "w+");
    LLKeyValues *curr = *head;
    Pair temp_pair;
    while (curr != NULL) {
	temp_pair = reduce(curr->key, curr->head_value);
	//printf("%s: %s\n", temp_pair.key, temp_pair.value);
	fwrite(&temp_pair, sizeof(Pair), 1, ftw);;
	curr = curr->next;
    }
    fclose(ftw);
}
