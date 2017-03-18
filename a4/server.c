#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

/*
 * Function to setup a server
 */
int setup(void) {
    int on = 1;
    int status;
    struct sockaddr_in addr;
    int listenfd;
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
	perror("socket");
	exit(1);
    }

    status = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
		(const char *) &on, sizeof(on));
	
    if (status == -1) {
	perror("setsockopt -- REUSEADDR");
    } 	
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(PORT);
    memset(&addr.sin_zero, 0, sizeof(addr.sin_zero));
    if (bind(listenfd, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
	perror("bind");
    }
	
    if (listen(listenfd, 5) == -1) {
	perror("listen");
	exit(1);
    }
    return listenfd;
}


/*
 * From lab11, helper function to define a newline character
 */
int find_network_newline(const char *buf, int inbuf) {
  int position = 0;
  int found = 0;
  while ((position < inbuf) && (found < 1)) {
    if (buf[position] == '\r') {
      if (buf[position+1] == '\n') {
        found++;
        // we found \r\n string and avoid position jump
        // to the \n character when this loop is finished
        position--;
      }
    }
    position++;
  }
  if (found == 0) {
    return -1; // return the location of '\r' if found
  } else {
    return position;
  }
}

