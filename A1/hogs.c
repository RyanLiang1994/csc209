#include <stdio.h>
#include <string.h>
#include <strings.h>

int get_the_output(int maxPid,float maxMem, float maxCpu,char * maxCommand,
		 int existFlag, char *mode);

int main(int argc, char **argv) {
	char targetUser[128];
	char mode[3];
	
	/*Check if the input of argv satisfied the requirements, if no program 
	  will stop, otherwise the program will continuous with specific
	   targetUser and mode. 
	*/
	if ((argc != 3)){
		if (argc == 2) {
			if ((strcmp(argv[1], "-c") == 0) ||
				 ((strcmp(argv[1], "-m") == 0))) {
		//case that only "-c" or "-m" is given but no username 
		//provided
				return -1;
			} else {
		//case that satisfied the requirements that only username 
		//is given
				strncpy(targetUser, argv[1], 
					sizeof(targetUser));
				strncpy(mode, "-c", sizeof(mode));
				mode[2] = '\0';
				targetUser[127] = '\0';
			}
		} else if (argc == 1) {
			//case that with too few arguments
			printf("Too few arguments.\n");
			return -1;
		} else {
			//case that withtargetUSER too many arguments
			printf("Too many arguments.\n");
			return -1;
		}
	} else {
	 	if ((strcmp(argv[1], "-c") == 0) ||
			 ((strcmp(argv[1], "-m") == 0))) {
			//case that all requirements are satisfied
			strncpy(targetUser,argv[2], sizeof(targetUser));
			strncpy(mode, argv[1], sizeof(mode));
			mode[2] = '\0';
			targetUser[127] = '\0';
		} else {
			//case that argv[1] is not -c or -m
			//note that even if the position of username and 
			//-c or -m argument has been switch, the program
			//just terminate but won't crash.
			return -1;
		}
	}

	/* "command","user","cpu","mem","vsz","rss","pid","time","start", and
	 * "sta" are variables that TEMPORY store the infomation of current
	 * line."maxMem", "maxUser", "maxCommand", "maxCpu" are variable to
	 * record the data of our output. "existFlag" is used to mark that
	 * the user is found or not. If yes, it's 1, and 0 otherwise.	
	 */

	char command[128];
	char user[128];
	float maxCpu, maxMem, cpu, mem;
	int vsz, rss, pid, value, maxPid; 
	char tty[10];
	char sta[5];
	char start[10];
	char time[10];
	char line[99];
	char maxUser[128];
	char maxCommand[128];
	int existFlag = 0;
	
	/* first sctargetUSERanf below is to skip the first line and second
	 * is to in the maxPid, maxCpu, maxMem and maxCommand with the values
	 * in the second line. Third scanf is to update the current line info
	 * and the return value of scanf.  
	 */
	scanf(" %[^\n]", line);
	scanf("%s %d %f %f %d %d %s %s %s %s %127[^\n]\n", maxUser, &maxPid,
	    &maxCpu,&maxMem, &vsz, &rss, tty, sta, start, time, maxCommand);
	if (strcmp(maxUser, targetUser) != 0){
		//case that the first line isn't target user
		maxCpu = -1.0;
		maxMem = -1.0;
	}
	value = scanf("%s %d %f %f %d %d %s %s %s %s %127[^\n]\n", user, &pid,
	    &cpu,&mem, &vsz, &rss, tty, sta, start, time, command);
	while (value != EOF) {	
		if ((strcmp(targetUser, user) == 0)) {
		//change the existFlag if we found at least 1 line which is
		//corresponded with target user.	
			existFlag = 1;
			if ((strcmp(mode, "-c")) == 0) {
				//case of first argument is "-c"
				//notice that the tie situation has been
				//included in the if statement.
				if ((cpu > maxCpu) || ((mem == maxMem)    
				   && (strcasecmp(command,               
                                   maxCommand) > 0 ))){
					maxCpu = cpu;
					maxMem = mem;
					maxPid = pid;
					strncpy(maxCommand,command,
						 sizeof(maxCommand));
					maxCommand[127] = '\0';
					strncpy(maxUser,user, sizeof(maxUser));
					maxUser[127] = '\0';				}
			} else if ((strcmp(mode, "-m")==0)) {
				//case of first argument is "-m"
				//notice that the tie case has been include in
				//the if statement.
				if ((mem > maxMem) || ((mem == maxMem)
				   && (strcasecmp(command,
				   maxCommand) > 0 ))) {
					maxCpu = cpu;
					maxMem = mem;
					maxPid = pid;
					strncpy(maxCommand, command, 
						sizeof(maxCommand));
					maxCommand[127] = '\0';
					strncpy(maxUser,user, sizeof(maxUser));
					maxUser[127] = '\0';
				} 
			}
		}
		//scanf nextline and update the value	
		value = scanf("%s %d %f %f %d %d %s %s %s %s %127[^\n]\n",
				user, &pid, &cpu, &mem, &vsz, &rss, tty,
				sta, start, time, command);
		
	}
        return get_the_output(maxPid, maxMem, maxCpu, maxCommand, existFlag,
				mode);	
	
}

int get_the_output(int maxPid,float maxMem, float maxCpu,char * maxCommand, int existFlag, char *mode) {
	if (existFlag == 1) {
                //case that we find the target user
                if ((strcmp(mode, "-c") == 0)) {
                        printf("%d\t%.1f\t%s\n",maxPid, maxCpu, maxCommand);
                } else {
                        printf("%d\t%.1f\t%s\n", maxPid, maxMem, maxCommand);
                }
		return 1;
        } else {
                //case that we cannot find the target user
                return -1;
        }
}

