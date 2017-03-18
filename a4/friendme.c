#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "friends.h"

 
#define INPUT_BUFFER_SIZE 256
#define INPUT_ARG_MAX_NUM 12
#define DELIM " \n"
#define MAX_ERROR_SIZE 90

#ifndef PORT 
    #define PORT 54366
#endif
 
  
/*
 *  prototype of function from server.c
 */
int setup(void);
int find_network_newline(const char *buf, int inbuf);


/*
 * Helper function to check the write error
 */
void check_write_error(int result) {
    if (result == -1) {
	perror("write");
    }
}


/*
 * Helper function to check the read error
 */
void check_read_error(int result) {
    if (result == -1) {
	perror("read");
    }
}


/*
 * Helper function to check the close error
 */
void check_close_error(int result) {
    if (result == -1) {
	perror("close");
    }
}

  
/* 
 * Print a formatted error message to stderr.
 */
void error(char *msg, int fd) {
    char error_msg[MAX_ERROR_SIZE];
    snprintf(error_msg, MAX_ERROR_SIZE, "Error: %s\n", msg);
    check_write_error(write(fd, error_msg, strlen(error_msg)));
}
 

/*
 * //From the sample code//. Client is a linked list structure to store the 
 * existing clients.
 */
static struct client {
    int fd;
    int client_exist;
    char name[32];
    int name_set_flag; 
    struct in_addr ipaddr;
    struct client *next;
} *top = NULL;



/*
 * From the sample code
 * helper function to remove the client from the client list
 * fd - the file descriptor of the remove target 
 */
void removeclient (int fd) {
    struct client **p;
    for (p = &top; *p && (*p)->fd != fd; p = &(*p)->next) {
    }	
    if (*p) {
	// check_close_error(close(fd));
	struct client *t = (*p)->next;
	free(*p);
	*p = t;
    } else {
	fprintf(stderr, "cannot find fd\n");
    }
    
}


/*
 * Helper function to find the file discriptor by the given name
 * Return: -1 if we dont find the client
 *         otherwise the file despritor of the specific client
 */
int find_fd(const char *name) {
    struct client *p;
    for (p = top; p && (strcmp((p->name),name) != 0); p = p->next) {		
	    
    }
    if (p) {
	return p->fd;
    } else {
	return -1;
    }
}


/* 
 * Helper function that nofificate the user with message 
 *	msg - the message we want to notify
 *	fd - if it's -1 then the program will find 
 *	name - the target name want to find
 */ 
void notification(int fd, const char *name , char *msg) {
    int size = strlen(msg);
    int error;
    if (fd == -1) { 
	int ffd = find_fd(name);
	error = write(ffd, msg, size);
    } else {
	error = write(fd, msg, size);
    }
    check_write_error(error);

} 


/* 
 * Read and process commands
 * Return:  -1 for quit command
 *          0 otherwise
 */
int process_args(int cmd_argc, char **cmd_argv, User **user_list_ptr, int fd,
		    fd_set *lst) {
    User *user_list = *user_list_ptr;
    char *buf;
    if (cmd_argc <= 0) {
        return 0;
    } else if (strcmp(cmd_argv[0], "quit") == 0 && cmd_argc == 1) {
        
        removeclient(fd);
        check_close_error(close(fd));
        return -1;
    } else if (strcmp(cmd_argv[0], "list_users") == 0 && cmd_argc == 1) {
        buf = list_users(user_list);
        int error = write(fd, buf, strlen(buf));
	check_write_error(error);
	free(buf);
    } else if (strcmp(cmd_argv[0], "make_friends") == 0 && cmd_argc == 3) {
        int flag = 0;
	switch (make_friends(cmd_argv[1], cmd_argv[2], user_list)) {
            case 1:
                error("users are already friends", fd);
                flag = 1;
		break;
            case 2:
                error("at least one userentered has the max number of friends", 
			fd);
                flag = 1;
		break;
            case 3:
		flag = 1;
                error("you must enter two different users",fd);
                break;
            case 4:
		flag = 1;
                error("at least one user you entered does not exist", fd);
                break;
        }
	if (flag == 0) {
	    int size = 31+strlen(cmd_argv[1]);
	    char temp_name[size];
	    sprintf(temp_name, "you have been friended by %s\n ", cmd_argv[1]);
	    notification(-1, cmd_argv[2] , temp_name); 
	    temp_name[0] = '\0';
	    sprintf(temp_name, "You are now friends with %s\n ", cmd_argv[2]);
	    notification(fd, NULL, temp_name); 
	}
                    
    } else if (strcmp(cmd_argv[0], "post") == 0 && cmd_argc >= 3) {
        // first determine how long a string we need
        int space_needed = 0;
        for (int i = 2; i < cmd_argc; i++) {
            space_needed += strlen(cmd_argv[i]) + 1;
        }

        // allocate the space
        char *contents = malloc(space_needed);
        if (contents == NULL) {
            perror("malloc");
            exit(1);
        }

        // copy in the bits to make a single string
        strcpy(contents, cmd_argv[2]);
        for (int i = 3; i < cmd_argc; i++) {
            strcat(contents, " ");
            strcat(contents, cmd_argv[i]);
        }
        char name[32]; 
	for (struct client *p = top; p; p = p->next) {
	    if ((p->fd) == fd) {
		strcpy(name, p->name);
	    }	
	}
        User *author = find_user(name, user_list);
        User *target = find_user(cmd_argv[1], user_list);
	// If theres error for the post then post_flag is 1
        // otherwise 0 
	int post_flag = 0;
        switch (make_post(author, target, contents)) {
            case 1:
                error("the users are not friends", fd);
		post_flag = 1;
                break;
            case 2:
                error("at least one user you entered does not exist", fd);
                post_flag = 1;
		break;
        }
	if (post_flag == 0) {
	    char temp_msg[8 + strlen(cmd_argv[1]) + strlen(contents)];
	    sprintf(temp_msg, "From %s: %s\n", name, contents);
	    notification(-1, cmd_argv[1], temp_msg);    
	} 
    } else if (strcmp(cmd_argv[0], "profile") == 0 && cmd_argc == 2) {
        User *user = find_user(cmd_argv[1], user_list);
	buf = print_user(user);
	int error = write(fd, buf, strlen(buf));
	check_write_error(error);
	free(buf);
    } else {
        error("Incorrect syntax", fd);
    }
    return 0;
}


/*
 * Tokenize the string stored in cmd.
 * Return the number of tokens, and store the tokens in cmd_argv.
 */
int tokenize(char *cmd, char **cmd_argv, int fd) {
    int cmd_argc = 0;
    char *next_token = strtok(cmd, DELIM);    
    
    while (next_token != NULL) {
        if (cmd_argc >= INPUT_ARG_MAX_NUM - 1) {
            error("Too many arguments!", fd);
            cmd_argc = 0;
            break;
        }
        cmd_argv[cmd_argc] = next_token;
        cmd_argc++;
        next_token = strtok(NULL, DELIM);
    }

    return cmd_argc;
}

/* From sample code
 * to add client to the linked list structure
 */ 
void addclient(int fd, struct in_addr addr, fd_set *lst) {
    struct client *p = malloc(sizeof(struct client));
    if (!p) {
        error("out of memory!\n", fd);
    }
    p->fd = fd;
    p->ipaddr = addr;
    p->next = top;
    p->client_exist = 0;
    p->name_set_flag = 0;
    top = p;
}


/*
 * Run the main process to read buffer from the file discripter. 
 *	fd - the target client's file descriptor 
 *	maxfd - the max fd in the fdlst before the function run
 *	client - the current client the system dealing with
 *	lst - the current fd set
 *	user_list - the linkedlist of users
 */
int run_process(int fd, int maxfd, int listenfd, struct client *client, 
		fd_set *lst, User **user_list) {
    int byte;
    int inbuf;
    char input[INPUT_BUFFER_SIZE];
    char *curr;
    int where;
    char buffer[INPUT_BUFFER_SIZE];
    char *cmd_argv[INPUT_ARG_MAX_NUM];
    int cmd_argc;
    int room = sizeof(input);
    curr = input;
    int name_set_flag = (client)->name_set_flag;
    input[0] = '\0';
    inbuf = 0;
    fd_set new_set;
    FD_ZERO(&new_set);
    int flag = 0;
    struct client *p;
    byte = read(fd, curr, room);
    int write_error;
    check_read_error(byte);
    //to check if the client is disconnected
    if (byte == 0) {
	// client is disconnected
	removeclient(fd);
    }

    while (byte > 0) {
	inbuf = byte;
	where = find_network_newline(curr, inbuf);
	if (where >= 0) {
	    curr[where] = '\0';
	    curr[where+1] = '\0';
	    if ((name_set_flag == 0) && ((client->client_exist) == 0)) {
		cmd_argc = tokenize(input, cmd_argv, fd);
		// if the client doesnt exist in our client list
		if (find_user(cmd_argv[0], *user_list) == NULL) {
		    //case we cannot find the username in our database
		    snprintf(buffer, sizeof(buffer), "%s %s", "add_user", 
				input);
		    cmd_argc = tokenize(buffer, cmd_argv, fd);
		    // add user to the data list	
		    if (strcmp(cmd_argv[0], "add_user") == 0){
			if (name_set_flag == 0) {
			    // case that username is too long
			    if (strlen(cmd_argv[1]) > 31) {
				error("Username too long, truncated to 31 chars",
				       fd);
				char temp_name[32];
				strncpy(temp_name, cmd_argv[1], 31);
				cmd_argv[1] = temp_name;
			    }
			    switch (create_user(cmd_argv[1], user_list)) {
				case 1:
				    error("user by this name already exists", 
					   fd);
				    break;
				case 2:
				    error("username is too long", fd);
				    break;
			    }
			    flag = 1;
			    name_set_flag = 1;
			}
		    }
		    write_error = write(fd, 
			"Welcome.\nGo ahead and enter user commands\n", 42);
		    check_write_error(write_error);
		    strncpy((client->name), cmd_argv[1], 31);
	    
		} else {
		    write_error = write(fd, 
			"Welcome back.\nGo ahead enter user commands\n", 43);
		    check_write_error(write_error);	
		    flag = 1;
		    name_set_flag = 1;
		    strncpy((client->name), cmd_argv[0], 31);
		}
	
                (client->client_exist) = 1;
            } else {
		// if the client is already exist in our client list
		strcpy(buffer, input);
	    }
	    memmove(input, curr + where + 2, (inbuf - where -2));
	    if (flag == 0) {
		// if we didn't tokenize before
		cmd_argc = tokenize(buffer, cmd_argv, fd);
	    }
            if (strcmp(cmd_argv[0], "make_friends") == 0) {
		char *name;
		name = client->name;
		cmd_argv[2] = cmd_argv[1];
		cmd_argv[1] = name;
		cmd_argc++;
	    }
	    if (flag != 1) {
		if (cmd_argc > 0 && process_args(cmd_argc, cmd_argv, user_list,
		     fd, lst) == -1) {
		    break; // can only reach if quit command was entered
		}
	    }         
            flag = 0;        
	}
	// manage the room and new curr
	if (where != -1) {
	    room = sizeof(input) - (inbuf - where - 2);
	    curr = input + (inbuf - where - 2);
	} else {
	    room -= inbuf;
	    curr = curr + inbuf;
	}
	
        FD_ZERO(lst);
        FD_SET(listenfd, lst);
        // Update the fd we are observing, to see if there's other fd ready
        for (p = top; p; p = p->next) {
            FD_SET(p->fd, lst);
            if (p->fd > maxfd) {
                maxfd = p->fd;
            }
        }
	
	if (select(maxfd+1, lst, NULL, NULL, NULL) < 0) {
            return 0;
        } else {
            // if current fd is not ready but other's fd is ready, then 
            // go out
	    if (!FD_ISSET(fd, lst)) {
 		return 0;
	    }
	}
	byte = read(fd, curr, room);
	check_read_error(byte);
    }
    // only being reach if fd is closed
    return 0;
}

int main(int argc, char* argv[]) {
    //begin setup ////////
    int listenfd = setup();
    int fd;
    struct sockaddr_in addr;
    socklen_t socklen;
    User *user_list = NULL;
    struct client *p;
    char greeting[] = "Welcome to FriendMe!\nWhat's your name:\n> ";
    while (1) {
        socklen = sizeof(addr);
        fd_set fd_lst;
        int maxfd = listenfd;
        FD_ZERO(&fd_lst);
        FD_SET(listenfd, &fd_lst);
        // loop to add all fd to the fd set
        for (p = top; p; p = p->next) {
            FD_SET(p->fd, &fd_lst);
            if (p->fd > maxfd) {
                maxfd = p->fd;
            }
        }
        if (select(maxfd + 1, &fd_lst, NULL, NULL,NULL) < 0) {
            
        } else {
            // check old fd is avaliable
            for (p = top; p; p = p->next) {
                if(FD_ISSET(p->fd, &fd_lst)) {
		    // if any previous fd is ready then process it
                    break;
                }
            }
            if (p) {
		run_process(p->fd, maxfd, listenfd, p, &fd_lst, &user_list);
	    }
	    if (FD_ISSET(listenfd, &fd_lst)) {
		// if there's new fd come in then create new connection
		if ((fd = accept(listenfd, (struct sockaddr *)&addr, &socklen))
		     < 0) {
		    perror("accept");
		
		} else {
		    addclient(fd, addr.sin_addr, &fd_lst);
		    int error = write(fd, greeting, sizeof(greeting));
		    check_write_error(error);
		}
	    }	
	}
    }
    return 0;
 }
