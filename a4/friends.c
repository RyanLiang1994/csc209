#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "friends.h"

/*
 * Create a new user with the given name.  Insert it at the tail of the list 
 * of users whose head is pointed to by *user_ptr_add.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if a user by this name already exists in this list.
 *   - 2 if the given name cannot fit in the 'name' array
 *       (don't forget about the null terminator).
 */
int create_user(const char *name, User **user_ptr_add) {
    if (strlen(name) >= MAX_NAME) {
        return 2;
    }

    User *new_user = malloc(sizeof(User));
    if (new_user == NULL) {
        perror("malloc");
        exit(1);
    }
    strncpy(new_user->name, name, MAX_NAME); // name has max length MAX_NAME - 1

    for (int i = 0; i < MAX_NAME; i++) {
        new_user->profile_pic[i] = '\0';
    }

    new_user->first_post = NULL;
    new_user->next = NULL;
    for (int i = 0; i < MAX_FRIENDS; i++) {
        new_user->friends[i] = NULL;
    }

    // Add user to list
    User *prev = NULL;
    User *curr = *user_ptr_add;
    while (curr != NULL && strcmp(curr->name, name) != 0) {
        prev = curr;
        curr = curr->next;
    }

    if (prev == NULL) {
        *user_ptr_add = new_user;
        return 0;
    } else if (curr != NULL) {
        free(new_user);
        return 1;
    } else {
        prev->next = new_user;
        return 0;
    }
}


/* 
 * Return a pointer to the user with this name in
 * the list starting with head. Return NULL if no such user exists.
 *
 * NOTE: You'll likely need to cast a (const User *) to a (User *)
 * to satisfy the prototype without warnings.
 */
User *find_user(const char *name, const User *head) {
/*    const User *curr = head;
    while (curr != NULL && strcmp(name, curr->name) != 0) {
        curr = curr->next;
    }

    return (User *)curr;
*/
    while (head != NULL && strcmp(name, head->name) != 0) {
        head = head->next;
    }

    return (User *)head;
}


/*
 * Print the usernames of all users in the list starting at curr.
 * Names should be printed to standard output, one per line.
 */
char *list_users(const User *curr) {
	char *temp_string1 = NULL;
	char *temp_string2 = NULL;
	int count = 0;
	asprintf(&temp_string1, "User List\r\n");
	while (curr != NULL) {
		if (count %2 == 0) {
			asprintf(&temp_string2, "%s\t%s\r\n", temp_string1, 
				curr->name);
			free(temp_string1);
		} else {
			asprintf(&temp_string1, "%s\t%s\r\n", temp_string2, 
				curr->name);
			free(temp_string2);
		}
		curr = curr->next;
		count++;
	}
	if (count %2 == 0) {
		return temp_string1;
	} else {
		return temp_string2;
	}
}



/* 
 * Make two users friends with each other.  This is symmetric - a pointer to 
 * each user must be stored in the 'friends' array of the other.
 *
 * New friends must be added in the first empty spot in the 'friends' array.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if the two users are already friends.
 *   - 2 if the users are not already friends, but at least one already has
 *     MAX_FRIENDS friends.
 *   - 3 if the same user is passed in twice.
 *   - 4 if at least one user does not exist.
 *
 * Do not modify either user if the result is a failure.
 * NOTE: If multiple errors apply, return the *largest* error code that applies.
 */
int make_friends(const char *name1, const char *name2, User *head) {
    User *user1 = find_user(name1, head);
    User *user2 = find_user(name2, head);

    if (user1 == NULL || user2 == NULL) {
        return 4;
    } else if (user1 == user2) { // Same user
        return 3;
    }

    int i, j;
    for (i = 0; i < MAX_FRIENDS; i++) {
        if (user1->friends[i] == NULL) { // Empty spot
            break;
        } else if (user1->friends[i] == user2) { // Already friends.
            return 1;
        }
    }

    for (j = 0; j < MAX_FRIENDS; j++) {
        if (user2->friends[j] == NULL) { // Empty spot
            break;
        } 
    }

    if (i == MAX_FRIENDS || j == MAX_FRIENDS) { // Too many friends.
        return 2;
    }

    user1->friends[i] = user2;
    user2->friends[j] = user1;
    return 0;
}




/*
 *  Print a post.
 *  Use localtime to print the time and date.
 */
char *print_post(const Post *post) { 
	char *str_post = NULL;
	asprintf(&str_post, "From: %s\r\nDate: %s\r\n%s\r\n",
		post->author, asctime(localtime(post->date)), post->contents);
	return str_post;
}


/* 
 * Print a user profile.
 * For an example of the required output format, see the example output
 * linked from the handout.
 * Return:
 *   - 0 on success.
 *   - 1 if the user is NULL.
 */
char *print_user(const User *user) {
    char *fd_lst1 = NULL;
    char *fd_lst2 = NULL;
    asprintf(&fd_lst1, "Name: %s\r\n\r\nFriends:\r\n", user->name);
    int flag = 0;
    for (int i = 0; i < MAX_FRIENDS && user->friends[i] != NULL; i++) {
	if (i % 2 == 0) {
	    asprintf(&fd_lst2, "%s%s\r\n", fd_lst1, user->friends[i]->name);
	    free(fd_lst1);
	    flag = 1;
	} else {
	    asprintf(&fd_lst1, "%s%s\r\n", fd_lst2, user->friends[i]->name);
	    free(fd_lst2);
	    flag = 0;
	}
    }

    char seprate[] = "------------------------------------------\r\n";
    char *friends = NULL;
    if (flag == 0) {
	asprintf(&friends, "%s%s", fd_lst1, seprate);
	free(fd_lst1);
    } else {
	asprintf(&friends, "%s%s", fd_lst2, seprate);
	free(fd_lst2);
    }
		
    Post *curr = user->first_post;
    char *post_lst1 = NULL;
    char *post_lst2 = NULL;
    int count = 0;
    char post_seperate[] = "\r\n===\r\n\r\n"; 
    asprintf(&post_lst1, "Post:\n");
    char *single_post = NULL;
    while (curr != NULL) {
	if (curr->next == NULL) {
	    post_seperate[0] = '\0';
	}
	single_post = print_post(curr);
	if (count % 2 == 0) {
	    asprintf(&post_lst2, "%s%s%s", post_lst1, single_post, 
			post_seperate);
	    free(post_lst1);
	    free(single_post);
	} else {
	    asprintf(&post_lst1, "%s%s%s", post_lst2, single_post, 
			post_seperate);
	    free(post_lst2);
	    free(single_post);
	}
	count++;
	curr = curr->next;
    }
    char *post = NULL;
    if (count % 2 == 0) {
	asprintf(&post, "%s%s", post_lst1, seprate);
	free(post_lst1);
    } else {
	asprintf(&post, "%s%s", post_lst2, seprate);
	free(post_lst2);
    }
    char *one_str = NULL;
    asprintf(&one_str, "%s%s", friends, post);
    free(friends);
    free(post);
    return one_str;
}


/*
 * Make a new post from 'author' to the 'target' user,
 * containing the given contents, IF the users are friends.
 *
 * Insert the new post at the *front* of the user's list of posts.
 *
 * Use the 'time' function to store the current time.
 *
 * 'contents' is a pointer to heap-allocated memory - you do not need
 * to allocate more memory to store the contents of the post.
 *
 * Return:
 *   - 0 on success
 *   - 1 if users exist but are not friends
 *   - 2 if either User pointer is NULL
 */
int make_post(const User *author, User *target, char *contents) {
    if (target == NULL || author == NULL) {
        free(contents);
        return 2;
    }

    int friends = 0;
    for (int i = 0; i < MAX_FRIENDS && target->friends[i] != NULL; i++) {
        if (strcmp(target->friends[i]->name, author->name) == 0) {
            friends = 1;
            break;
        }
    }

    if (friends == 0) {
	free(contents);
        return 1;
    }

    // Create post
    Post *new_post = malloc(sizeof(Post));
    if (new_post == NULL) {
        perror("malloc");
        exit(1);
    }
    strncpy(new_post->author, author->name, MAX_NAME);
    new_post->contents = contents;
    new_post->date = malloc(sizeof(time_t));
    if (new_post->date == NULL) {
        perror("malloc");
        exit(1);
    }
    time(new_post->date);
    new_post->next = target->first_post;
    target->first_post = new_post;
    
    return 0;
}

