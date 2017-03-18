#include "friends.c"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define filename "alien.ascii"
int main () {
	printf("1");
	User *front = malloc(sizeof(User));
	strcpy(front->name,  "front");
	strcpy(front->profile_pic,  filename);
	printf("2");
	create_user("alien", &front);
	printf("3");
	if (find_user("alien", front) != NULL) {
		printf("find succeed!\n");
		list_users(front);
		int result = update_pic(front, filename);
		if (result == 0) {
			printf("pic succed\n");
		} else {
			printf("pic failed\n");
		}

	} else { printf("find fail!\n");}
	return 0;
}
