#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/wait.h>
#include <ctype.h>
#include "mapreduce.h"

//////////////////////Prototype/////////////////////////////////////////
void open_or_close_pipe(int fd[][2], int num, const char *mode, int f);
void pipe_to_reduce(int fdin, int fdset[][2], int num, int **sheet);
int **create_end_sheet(int numprocs);
char *check_flags(int *mnumprocs, int *rnumprocs, int argc, char **argv);


int main(int argc, char **argv) {
	
    // mnumprocs: the number of mapworkers
    // rnumprocs: the number of reduceworkers
    // dir: the path we store the txt files 
    int mnumprocs;
    int rnumprocs;
    char *dir = check_flags(&mnumprocs, &rnumprocs, argc, argv);
	
    ////////////////////////////////////////////////////////////////////
    // Following part is to do the ls process and get the names of file 
    // in the specific dir, then write them to the specific pipe
    // Note: - "fd" is the pipe connect ls and the master process
    //       - "mfdin" is an array of pipes that connect each map worker and 
    //               master, it is used to send the filenames to map workers
    //       - "mfdout" is also an array of pipes that connect each map worker
    //               and master, it is used to send the Pairs which is produced
    //               by mapworkers
    //////////////////////////////////////////////////////////////////// 
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
        exit(1);
    } else if (n == 0) {
        //connect the pipe to the stdout
    	if (close(fd[0]) == -1) {
            perror("close");
            exit(1);
        }
    	if ((dup2(fd[1], STDOUT_FILENO)) == -1) {
    	    perror("dup error\n");
            exit(1);
    	}
	// execute the ls process and the result will go to pipe
	if (execl("/bin/ls", "ls", dir, (char *) 0) == -1) {
	    perror("exec failed");
	    exit(1);
	} 
	if (close(fd[1]) == -1) {
            perror("close");
            exit(1);
        }
	exit(0);
    } else {
	// wait until the child finish
	int status_ls;
	if (wait(&status_ls) != -1) {
	    if (WEXITSTATUS(status_ls) == 1) {
                fprintf(stderr, "error\n");
                exit(1);
            } 
	}
	if (close(fd[1]) == -1) {
            perror("close");
            exit(1);
        }
	// parent process
	// connect the pipe read to the stdin
	if ((dup2(fd[0], STDIN_FILENO)) == -1) {
	    perror("dup error\n");
	    exit(1);
	}

	char filenames[MAX_FILENAME];
	int count = 0;
	int pipe_flag = 0;
	char filedir[2*MAX_FILENAME];
	strcpy(filedir, dir);

	// read the filename from pipe(stdin)
	// create the set of pipes for each process and put the file name 
	// into each pipe evenly
	while ((scanf("%s", filenames)) != EOF) {
	    // pipe_flag is a flag to identify whether we need to open the 
	    // pipe. 
	    if (pipe_flag < mnumprocs) {
	        if (pipe(mfdin[count]) == -1) {
                    perror("pipe");
                    exit(1);
                } 
		if (pipe(mfdout[count]) == -1) {
                    perror("pipe");
                    exit(1);
                }
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
	    if (write(mfdin[count][1], filedir, (2*MAX_FILENAME)) == -1) {
                perror("write");
                exit(1);
            }
	    count++;
	    strcpy(filedir, dir);
	}

	// close the stdin
	if (close(fd[0]) == -1) {
            perror("close");
            exit(1);
        }	

	// close the writing end of the mfdin pipe since we put all the file 
        // name to the pipes and we dont need the writing end anymore
	open_or_close_pipe(mfdin, mnumprocs, "close-out", 0);	
    }
    
    //////////////////////////////////////////////////////////////////////////
    // following is the process to create and activate the reduce workers and 
    // mapworkers and allow them to work. 
    //////////////////////////////////////////////////////////////////////////

    // -rfd is an array of pipes which connect master and all the reduceworkers
    //      and master will send Pair to the reducewokers through these pipes 
    int rfd[rnumprocs][2];
    int **sheet;
    
    // sheet is an table to record which range each worker should take charge
    // it is a 2D int
    sheet = create_end_sheet(rnumprocs);

    // open the pipes for reduce workers and ready for write data
    open_or_close_pipe(rfd, rnumprocs, "open", 0);

    // create process for each map worker and map the files
    // the idea is we first create the reduce worker processes and let them
    // to work. Meanwhile, after all the reduce workers have been created and 
    // standby, we start to create map workers and let them to start working
    // meanwhile we also called the "pipe to reduce" function so that the 
    // products produced by map workers can be passed to the rfd pipe	
    int s;
    for (int j = 0; j < rnumprocs; j++) {
        if ((s = fork()) < 0) {
	    perror("fork");
	    exit(1);
	}
	else if (s == 0) {
	    // open the worker reduce processes
            // we dont need the following pipe so close them
	    open_or_close_pipe(mfdout, mnumprocs, "close-in", 0);
	    open_or_close_pipe(mfdout, mnumprocs, "close-out", 0);
	    open_or_close_pipe(mfdin, mnumprocs, "close-in", 0);
	    open_or_close_pipe(rfd, rnumprocs, "close-out", 0);

	    reduce_worker(rfd[j][1], rfd[j][0]);
	    
            // close the pipes we just used
            open_or_close_pipe(rfd, rnumprocs, "close-in", 0);
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
			if (close(mfdin[i][0]) == -1) {
                            perror("close");
                        }
			if (close(mfdout[i][1]) == -1) {
                            perror("close");
                        }
			pipe_to_reduce(mfdout[i][0], rfd, rnumprocs, sheet);
			if (close(mfdout[i][0]) == -1) {
                            perror("close");
                        }

			// if we've assign all the pair to the reduce fd 
                        // then close it
			if (i == (mnumprocs -1)) {
			    open_or_close_pipe(rfd, rnumprocs, "close-out", 0);
			    open_or_close_pipe(rfd, rnumprocs, "close-in", 0);
			}
			int status;
			wait(&status);	
		    }  else {
			// for the child we dont use the pipe for reduce worker
			// so close them
			open_or_close_pipe(rfd, rnumprocs, "close-out", 0);
			open_or_close_pipe(rfd, rnumprocs, "close-in", 0);
			
			// we also dont need to read the pipe for map worker 
                        // for child process so close it
			open_or_close_pipe(mfdout, mnumprocs, "close-in", i);
			// map worker start work
			map_worker(mfdout[i][1], mfdin[i][0]);
			open_or_close_pipe(mfdin, mnumprocs, "close-in", i);
			open_or_close_pipe(mfdout, mnumprocs, "close-out", i);
			exit(0);
		    }
		}
                // To make sure every reduce workers have finished working 
                // before we quit
		int status_r;
		for (int k = 0; k < rnumprocs; k++) {
		    wait(&status_r);
		}
	    }
	}
    }

    free(dir);
    for (int z = 0; z < rnumprocs; z++) {
	free(sheet[z]);
    } 
    free(sheet);
    return 1;
}

/* This is a helper function to check if the flags we pass in is valid 
 * 
 * Return the dir that we passed in
 *
 */
char *check_flags(int *mnumprocs, int *rnumprocs, int argc, char **argv) {
    
    int output;
    // check if there's a valid d, if yes dflag would be set to 0                                                                                                                        
    int dflag = -1;                                              
    // check if there's a valid r, if yes rflag would be set to 0           
    int rflag = -1;                                              
    // check if there's a valid m, if yes mflag would be set to 0           
    int mflag = -1;                                              
    // check if there's a valid number of option           
    int option_count = 0;                                                   
    char *dir = malloc(sizeof(char) * MAX_FILENAME);                                                                                                                             
    int digit_flag = 1;
    int temp;                                                   
    opterr = 0;                                                             
    while ((output = getopt(argc, argv, "m::r::d:")) != -1) {               
	if ((output == 'm') || (output == 'r') || (output == 'd')) {         
	    switch(output) {                                                
		case 'm':
		                                                
	            if (optarg == NULL) {                           
		        *mnumprocs = 2;                          
		    } else {                                        
			*mnumprocs = strtol(optarg, NULL, 10);   
                    }                                               
		    option_count++;                                 
		    for (temp = 0; temp < strlen(optarg); temp++) {
			if (!(isdigit(optarg[temp]))) {
			    digit_flag = 0;
			}
		    } 
		    if (digit_flag != 0) {
			mflag = 0;
		    }  
		    digit_flag = 1;                                   
		    break;                                          
		case 'r':
		                                               
		    if (optarg == NULL) {                           
			*rnumprocs = 2;                          
		    } else {                                        
			*rnumprocs = strtol(optarg, NULL, 10);   
		    }                                               
		    option_count++;
		    for (temp = 0; temp < strlen(optarg); temp++) {
			if (!(isdigit(optarg[temp]))) {
			    digit_flag = 0;
			}
		    }
		    if (digit_flag != 0) {                                 
			rflag = 0;
		    } 
		    digit_flag = 1;                                     
		    break;                                          
		case 'd':                                               
		    strncpy(dir, optarg, MAX_FILENAME);             
		    option_count++;                                 
		    if (strlen(optarg) < MAX_FILENAME) {
			dflag = 0;             
		    } else {
			fprintf(stderr, "dir too long\n");
			exit(1);
		    }                         
		    break; 
	    } 
	} else {
	    fprintf(stderr, "invalid option\n");
	    exit(1);
		                                         
        }                                                               
    }                                                                       
                                                                                
    if ((dflag != 0) || (mflag != 0) || (rflag != 0)) {                     
        fprintf(stderr,                                                 
            "Usage: m: # of mapper, mapworker; r: # of reducer; d: dir\n");
        exit(1);                                                        
    } 
    
    if (option_count != 3) {
	fprintf(stderr, "Too Many/Few arguments\n");
	exit(1);
    } 
	
    return dir;

} 


/* This is a helper function to do the open or close job for an array of pipes
 * fd - an array of fd which are open before the function execuatve 
 * num - number of pipe in fd
 * mode - mode of the function, could be "open", "close-in", "close-out"
 * f - f is a number represent we start from which pipe to close
 */
void open_or_close_pipe(int fd[][2], int num, const char *mode, int f) {
    for (int i = f; i < num; i++) {
	if (strcmp(mode, "open") == 0) {
	    if (pipe(fd[i]) == -1) {
		perror("pipe");
	    }
	} else if (strcmp(mode, "close-in") == 0) {
	    if (close(fd[i][0]) == -1) {
		perror("close");
	    }
	} else if (strcmp(mode, "close-out") == 0) {
	    if (close(fd[i][1]) == -1) {
		perror("close");
	    }
	}
    }
}


/* This is a helper function which read the pairs from fdin pipe and 
 * write the pairs to the fdset for being reduced
 * 
 * fdin - the opened pipe we use to read data
 * fdset - the array of pipe that we put data to, they are all opened 
 * num - how many pipes are in fdset
 * sheet - the table of rule how we assign data
 */
void pipe_to_reduce(int fdin, int fdset[][2], int num, int **sheet) {
    Pair curr_pair;
    int r;
    int s;
    r = read(fdin, &curr_pair, sizeof(Pair));
    if (r < 0) {
        perror("read error");
    }
    while (r != 0) {
	for (int i = 0; i < num; i++) {
	    if (!((curr_pair.key)[0] > sheet[i][0])) {
		s = write(fdset[i][1], &curr_pair, sizeof(Pair)); 
	        if (s == -1) {
                    perror("write");
                }
            }
	}
	r = read(fdin, &curr_pair, sizeof(Pair));
        if (r < 0) {
            perror("read error");
        }
    }	
}

/* this is a helper function to create a sheet for the workers about which 
 * worker should handle what range of pairs
 * 
 */
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
