#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "mapreduce.h"

void open_or_close_pipe(int fd[][2], int num, const char *mode);
void pipe_to_reduce(int fdin, int fdset[][2], int num, int **sheet);
int **create_end_sheet(int numprocs);



int main(int argc, char **argv) {
	int output;
	int mnumprocs;
	int rnumprocs;
	int dflag = -1;
	int rflag = -1;
	int mflag = -1;
	int option_count = 0;
	char dir[MAX_FILENAME];

	//strcpy(s[0], "");
	//strcpy(s[1], "");
	opterr = 0;
	printf("bigining read\n");
	while ((output = getopt(argc, argv, "m::r::d:")) != -1) {
		switch(output) {
			case 'm': 
				if (optarg == NULL) {
					mnumprocs = 2;
				} else {
					mnumprocs = strtol(optarg, NULL, 10);
				}
				option_count++;
				mflag = 0;
				break;
			case 'r':
				if (optarg == NULL) {
					rnumprocs = 2;
				} else {
					rnumprocs = strtol(optarg, NULL, 10);
				}
				option_count++;
				rflag = 0;
				break;
			case 'd':
				strncpy(dir, optarg, MAX_FILENAME);
				option_count++;
				dflag = 0;
				break;
		}
	}
	printf("%d, %d\n", mnumprocs, rnumprocs);

	if ((dflag != 0) || (mflag != 0) || (rflag != 0)) {
		fprintf(stderr, 
		"Flag Missing. Usage: m: number of");
		fprintf(stderr, "mapworker; r: number of reduceworker; d: dir\n");
		exit(1);
	}

	// ls process: get the names of file in the specific dir
	int fd[2];
	int n;
	int mfdin[mnumprocs][2];
	int mfdout[mnumprocs][2];
	if (pipe(fd) == -1) {
		perror("pipe");
		exit(1);
	}
	if ((n = fork()) < 0) {
		perror("fork");
	} else if (n == 0) {
		//connect the pipe to the stdout
		printf("child process runing\n");
		close(fd[0]);
		if ((dup2(fd[1], fileno(stdout))) == -1) {
			perror("dup error\n");
		}
		// execute the ls process and the result will go to pipe
		if (execl("/bin/ls", "ls", dir, (char *) 0) == -1) {
			perror("exec failed");
			exit(1);
		} 
		close(fd[1]);
		exit(0);
	} else {
		// wait until the child finish
		int status_ls;
		if (wait(&status_ls) != -1) {
			if (WEXITSTATUS(status_ls) == 0) {
				printf("11111 exit properately!\n");
				
			}
		}
		close(fd[1]);
		// parent process
		// connect the pipe read to the stdin
		if ((dup2(fd[0], fileno(stdin))) == -1) {
			perror("dup error\n");
			exit(1);
		}


		printf("2\n");


		char filenames[MAX_FILENAME];
		int count = 0;
		int pipe_flag = 0;
		char filedir[2*MAX_FILENAME];
		strcpy(filedir, dir);
		// readd the filename from pipe(stdin)
		// create the set of pipes for each process and put the file name 
		// into each pipe evenly
		while ((scanf("%s", filenames)) != EOF) {
			// pipe_flag is a flag to identify whether we need to open pipe the 
			// pipe. 
			if (pipe_flag < mnumprocs) {
				pipe(mfdin[count]);
				pipe(mfdout[count]);
				pipe_flag++;
			} 

			// count is to record how many workers have been assign filename 
			// for this round of distribution, how many round of distribution
			// should depend on how many filename and mapworkers.
			// For example: we have 5 files and 2 workers, it should take 
			// 3 rounds to finish distribute.
			if (count >= mnumprocs) {
				count = 0;
			}
			strcat(filedir, "/");
			strcat(filedir, filenames);
			printf("%s\n",filedir);
			write(mfdin[count][1], filedir, (2*MAX_FILENAME));
			count++;
			strcpy(filedir, dir);
		}
		// close the stdin
		close(fd[0]);
		
		printf("3\n");

		// close the writing end of the mfdin pipe since we put all the file name 
		// to the pipes and we dont need the writing end anymore
		open_or_close_pipe(mfdin, mnumprocs, "close-out");	
	}
	int rfd[rnumprocs][2];
	int **sheet;
	sheet = create_end_sheet(rnumprocs);
	// open the pipes for reduce workers and ready for write data
	open_or_close_pipe(rfd, rnumprocs, "open");

	// create process for each map worker and map the files
	

	printf("4\n");
	int s;
	for (int j = 0; j < rnumprocs; j++) {
		if ((s = fork()) < 0) {
			perror("fork");
			exit(1);
		}
		else if (s == 0) {
			printf("r process %d\n", j);
			// open the worker reduce processes
			open_or_close_pipe(mfdout, mnumprocs, "close-in");
			open_or_close_pipe(mfdout, mnumprocs, "close-out");
			open_or_close_pipe(mfdin, mnumprocs, "close-in");
			open_or_close_pipe(rfd, rnumprocs, "close-out");
			reduce_worker(rfd[j][1], rfd[j][0]);
			open_or_close_pipe(rfd, rnumprocs, "close-in");
			printf("now exit\n");
			exit(0);
		} else {
            if (j < (rnumprocs - 1)) {
				// wait until we open rnumprocs number of reduce
				// workers
				continue;
			} else {
				// start mapping 
				for (int i = 0; i < mnumprocs; i++) {
					if ((n = fork()) < 0) {
						perror("fork");	
					} else if (n != 0) {
						close(mfdin[i][0]);
						close(mfdout[i][1]);
						pipe_to_reduce(mfdout[i][0], rfd, rnumprocs, sheet);
						printf("finish reduce pipe\n");
						close(mfdout[i][0]);
						// if we've assign all the pair to the reduce fd then close it
						if (i == (mnumprocs -1)) {
							open_or_close_pipe(rfd, rnumprocs, "close-out");
							open_or_close_pipe(rfd, rnumprocs, "close-in");
						}
						int status;
						wait(&status);
						
						
					}  else {
						printf("b\n");
						// for the child we dont use the pipe for reduce worker
						// so close them
						open_or_close_pipe(rfd, rnumprocs, "close-out");
						open_or_close_pipe(rfd, rnumprocs, "close-in");
			
						// we also dont need to read the pipe for map worker for child process
						// so close it
						open_or_close_pipe(mfdout, mnumprocs, "close-in");
						printf("start map\n");
						// map worker start work
						map_worker(mfdout[i][1], mfdin[i][0]);
						printf("end map\n");
						open_or_close_pipe(mfdin, mnumprocs, "close-in");
						open_or_close_pipe(mfdout, mnumprocs, "close-out");
						printf("now exit\n");
						exit(0);
					}
				}
				int status_r;
				for (int k = 0; k < rnumprocs; k++) {
					wait(&status_r);
				}
			}

		}
	}
	printf("5\n");

	return 1;
}

// do the open or close job for an array of pipes
void open_or_close_pipe(int fd[][2], int num, const char *mode) {
	for (int i = 0; i < num; i++) {
		if (strcmp(mode, "open") == 0) {
			pipe(fd[i]);
		} else if (strcmp(mode, "close-in") == 0) {
			close(fd[i][0]);
		} else if (strcmp(mode, "close-out") == 0) {
			close(fd[i][1]);
		}
	}
}


// pipe the pairs to the fdset and wait for being reduced
void pipe_to_reduce(int fdin, int fdset[][2], int num, int **sheet) {
	Pair curr_pair;
	int r;
	r = read(fdin, &curr_pair, sizeof(Pair));
	if (r < 0) {
		perror("read error");
	}
	printf("****************start piping\n");
	// printf("r: %d\n", r);
	while (r != 0) {
		for (int i = 0; i < num; i++) {
			if (!((curr_pair.key)[0] > sheet[i][0])) {
				write(fdset[i][1], &curr_pair, sizeof(Pair)); 
			}
		}
		r = read(fdin, &curr_pair, sizeof(Pair));
	}	
	printf("pipe finish\n");
}

// create a sheet for the workers about which worker should handle 
// what range of pairs
int **create_end_sheet(int numprocs) {
	int **sheet = malloc(sizeof(int*) * numprocs);
	for (int i = 0; i < numprocs; i++) {
		sheet[i] = malloc(sizeof(int));
	}
	
	int quotient = (256 / numprocs);
	int start_num = 0;
	for (int i = 0; i < numprocs; i++) {
		sheet[i][0] = (start_num + quotient) -1;

	}
	sheet[numprocs-1][0] = 255;
	return sheet;
}
