#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mapreduce.h"
#include "linkedlist.c"
void reduce_and_write(LLKeyValues **head);

void reduce_worker(int outfd, int infd){
	LLKeyValues *init_node = NULL;
	Pair temp_pair;
	int r;
	r = read(infd, &temp_pair, sizeof(Pair));
	if (r < 0) {
		perror("read error");
		exit(1);
	}
	// printf("key: %s\t", temp_pair.key);
	// create a list 

	while (r != 0) {
		if (init_node == NULL) {
			init_node = create_node(temp_pair);
		}
		else {
			insert_into_keys(&init_node, temp_pair);
		}
		//printf("reading\n");
		r = read(infd, &temp_pair, sizeof(Pair));
		//printf("finish read\n");	
		if (r < 0) {
			perror("read error");
			exit(1);
		}
		 //printf("key: %s\n", temp_pair.key);
	}
	printf("finish read now start reduce\n");
	reduce_and_write(&init_node);
	printf("finish reduce worker\n");
}


void reduce_and_write(LLKeyValues **head) {
	int pid = getpid();
	char name[10];
	sprintf(name, "%d.out", pid);
	FILE *ftw = fopen(name, "w+");
	LLKeyValues *curr = *head;
	Pair temp_pair;
	while (curr != NULL) {
		temp_pair = reduce(curr->key, curr->head_value);
		printf("%s: %s\n", temp_pair.key, temp_pair.value);
		fwrite(&temp_pair, sizeof(Pair), 1, ftw);;
		curr = curr->next;
	}
	fclose(ftw);
}
