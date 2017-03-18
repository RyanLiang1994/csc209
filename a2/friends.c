#include "friends.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define MAX_LINELENGTH 90 // Used in fgets function in order to read
			  // the profile pic
#define SEPRATE_LINE "------------------------------------------" 

/*
 * Create a new user with the given name.  Insert it at the tail of the list
 * of users whose head is pointed to by *user_ptr_add.
 *
 * Return:
 *   - 0 if successful
 *   - 1 if a user by this name already exists in this list
 *   - 2 if the given name cannot fit in the 'name' array
 *       (don't forget about the null terminator)
 */
int create_user(const char *name, User **user_ptr_add) {
    User *new_user = malloc(sizeof(User));
    
    // Check if malloc is successful
    if(new_user == NULL) {
        perror("malloc failed\n");
	free(new_user);
	exit(1);
    }

    if (strlen(name) > (MAX_NAME-1)) {
    	// Return 2 if username is not fit
        free(new_user);	
	return 2;
    } 
	    	
    User *current = *user_ptr_add;
    
    // Initialize all element in the user struct        
    strcpy(new_user->name, name);
    new_user->next = NULL;
    new_user->first_post = NULL;
    strcpy(new_user->profile_pic, "");
    for (int i = 0; i < MAX_FRIENDS; i++) {
    	(new_user->friends)[i] = NULL;
    }

    // Iterate to the tail of the list
    if (current != NULL) {
        while (current->next != NULL){
            if (strcmp((current->name), name) == 0) {
	        // Return 1 if current Username is same as user 
	        free(new_user);
                return 1;
	    } else {
    	        current = current -> next;
	    }
        }
		
	if (strcmp((current->name), name) == 0) {
            // Return 1 if current Username is same as user 
            free(new_user);
            return 1;
   	} 
    	(current->next) = new_user;
    } else {
	*user_ptr_add = new_user;
    }	
    return 0;
}


/*
 * Return a pointer to the user with this name in
 * the list starting with head. Return NULL if no such user exists.
 *
 * NOTE: You'll likely need to cast a (const User *) to a (User *)
 * to satisfy the prototype without warnings.
 */
User *find_user(const char *name, const User *head) {
    User *current = (User *)head;
    while (current != NULL) {
        // if current User has same name, then return the pointer
	if (strcmp((current->name), name) == 0) {
	    return current;
	}
	else {
	    // Otherwise go to next User
	    current = (User *)(current->next);
	}
    }
    //If after the iteration, nothing returns, then name is not found
    return NULL;
}


/*
 * Print the usernames of all users in the list starting at curr.
 * Names should be printed to standard output, one per line.
 */
void list_users(const User *curr) {
    User *current = (User *)curr;
    printf("User List\n");
    while (current !=  NULL) {
	printf("\t%s\n", current->name);
	current = current->next;
    }
}


/*
 * *********************** HELPER FUNCTION ***********************
 * Helper function to check the file of the given filename is valid 
 * Require: filename's length is less than 31 characters
 * Return: 
 *     - 0 if file is valid
 *     - 1 if file is not valid
 */
int check_valid_file(const char *filename) {
    FILE *file;
    file = fopen(filename, "r");

    // if file doesn't exist then return 1
    if (file == NULL) {
        return 1;
    }
    // if nothing return then closed the file
    int closed_result = fclose(file);
    if (closed_result != 0) {
        perror("closed failed\n");
	exit(1);
    }
    return 0;
}
/*
 * Change the filename for the profile pic of the given user.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if the file does not exist.
 *   - 2 if the filename is too long.
 */
int update_pic(User *user, const char *filename) {
    // if filename is too long then return 2
    if (strlen(filename) > (MAX_NAME-1)) {
        return 2;
    }

    // Check if the file is valid exist 
    int fileCheck = check_valid_file(filename);
    if (fileCheck == 1) {
	return 1;
    }
    // store the file name if nothing return
    strcpy((user->profile_pic), filename);

    // return 0 if success
    return 0;
}


/*
 * ************************* HELPER FUNCTION *************************
 * Helper function to check whether name1 and name2 are already friends.
 * Return:
 *       - 1 if they're already friends
 *       - 0 otherwise
 */
int friends_check(User *name1, User *name2) {
    int index = 0;
    int result = 0;
    while ((result == 0) && (index < MAX_FRIENDS)) {
        if (((name1->friends)[index] == name2) || 
	    ((name2->friends)[index] == name1)) {
            result = 1;
        }       
            index++;
        }       
    return result;
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
    int result = 0;
    
    // first to find the pointer of these two users with their name
    // for a purpose of convenience 
    User *userName1 = find_user(name1, head);
    User *userName2 = find_user(name2, head);
    
    // If either of userName1 or userName2 is NULL, then it means we found 
    // only one of both name or we even don't
    // find both of them, then result should be set to 4 no matter 
    // what value it is before, thus we can directly return
    if ((userName1 == NULL) || (userName2 == NULL)) {
        result = 4;
        return result;
    }
    
    // check whether both name is the same. we don't need to check
    // the result is larger than 3 since if it's the function will 
    // return in last steps
    if (strcmp(name1, name2) == 0) {
        result = 3;
        return result;
    }
    	
    // check if one of them's friend list is full. 
    if (((userName1->friends)[9] != NULL) || 
        ((userName2->friends)[9] != NULL)) {
        result = 2;
	return result;
    }
    	
    // Check the friend list, if name1 or name2 exist in the other one's 
    // friend list, friends_check should return 1 and then return 1
    int existFlag = friends_check(userName1, userName2);
    if (existFlag == 1) {
        result = 1;
        return result;
    }
     	
    // result will be 0 iff above steps don't return.
    // then we need to add them to their respectively friend list
    int index = 0;
    // loop to the NULL position and set the position to the pointer of
    // user Name2
    while ((userName1->friends)[index] != NULL) {
        index++;
    }	
    (userName1->friends)[index] = userName2;
	
    // add user Name1 to the name2's friend list
    index = 0;
    while ((userName2->friends)[index] != NULL) {
        index++;
    }
    (userName2->friends)[index] = userName1;
    	
    // we dont need to concern about the problem about index is beyond 
    // MAX_FRIENDS since if we still can't find a empty spot to add friends
    // then means this user's friends are full and should be return in 
    // the previous steps.
    return result;
}


/*
 * Print a user profile.
 * For an example of the required output format, see the example output
 * linked from the handout.
 * Return:
 *   - 0 on success.
 *   - 1 if the user is NULL.
 */
int print_user(const User *user) {
    if (user == NULL) {
        // return if user is NULL 
	return 1;
    } else { 
        if (strcmp(user->profile_pic, "") != 0) {
            int fileCheck = check_valid_file(user->profile_pic);
            if (fileCheck == 0) {
	        // print the profile_pic, notice that if profile_pic 
	        // is empty string then this part should be skipped
	        char temp_line[MAX_LINELENGTH + 1];
	        FILE *pic_file = fopen((user->profile_pic), "r");
                while (fscanf(pic_file, "%[^\n]%*c", temp_line) == 1) {
         	    printf("%s\n", temp_line);
		}
	        fclose(pic_file);
	        printf("\n\n");
            }		
	}

	// print the name of user
	printf("Name: %s\n\n%s\n", user->name, SEPRATE_LINE);
		
	// print the friend list of user
	printf("Friends:\n");
	int frd_index = 0;
	while (((user->friends)[frd_index] != NULL) && frd_index < 10) {
	    printf("%s\n",((user->friends)[frd_index])->name);
	    frd_index++;	
	}
	
	printf("%s\n", SEPRATE_LINE);
		
	// print the Post of this user
	printf("Posts:\n");
	Post *curr_post = user->first_post;;
	while (curr_post != NULL) {
	    printf("From: %s\nDate: %s\n%s\n", curr_post->author, 
	    ctime(curr_post->date), curr_post->contents);
	    curr_post = curr_post->next;
	    if (curr_post != NULL) {
	        printf("\n===\n\n");
	    } 
	}
	printf("%s\n", SEPRATE_LINE);	
	return 0;
    }
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
    // if either User pointer is Null
    if ((author == NULL) || (target == NULL)) {
        free(contents);
	return 2;
    }
	
    // This is for searching author and target's friend list to see if they 
    // are already friends or not. friend_flag will be set to 1 if they're
    // already friends, and 0 otherwise 
    int friend_flag = friends_check((User *)author, target);
	
    // return 1 if they are not friend yet
    if (friend_flag == 0) {
        free(contents);
	return 1;
    }

    // create a new post 
    // and check if malloc is successful
    Post *new_post = malloc(sizeof(Post));
    if (new_post == NULL) {
        free(contents);
	free(new_post);
	perror("malloc failed\n");
        exit(1);
    }
    time_t *timer;
    timer = malloc(sizeof(time_t));
    if (timer == NULL) {
 	free(contents);
	free(timer);
	free(new_post);
        perror("malloc failed\n");
        exit(1);
    }
    strcpy(new_post->author, author->name);
    new_post->contents = contents;
    new_post->next = target->first_post;
    *timer = time(NULL); 
    new_post->date = timer;
    target->first_post = new_post;
    return 0;
}


/*
 * ************************** HELPER FUNCTION *******************************
 * Helper Function to recursively free the linked list of post from the memory
 * if post is NULL then do nothing
 */
void recursive_free_post(Post *post) {
    if (post != NULL) {
        if (post->next == NULL) {
            free(post->contents);
            free(post->date);
            free(post);
        } else {
            recursive_free_post(post->next);
            free(post->contents);
            free(post->date);
            free(post);
        }
    }
}


/*
 * ******************** HELPER FUNCTION **********************
 * Helper Function to delete the targetUser from the user list, 
 * given the address of pointer of list of user.
 * if userPtr is NULL then do nothing 
 *
 * Require: targetUser must exist in userPtr
 */
void delete_next(const char *targetUser, User ** userPtr) {
    if ((*userPtr) != NULL) {
        if (strcmp((*userPtr)->name, targetUser) == 0) {
            *userPtr = (*userPtr)->next;
	} else {
	    delete_next(targetUser, &((*userPtr)->next));
	}
    }
}


/*
 * From the list pointed to by *user_ptr_del, delete the user
 * with the given name.
 * Remove the deleted user from any lists of friends.
 *
 * Return:
 *   - 0 on success.
 *   - 1 if a user with this name does not exist.
 */
int delete_user(const char *name, User **user_ptr_del) {
    // Find the user with the name first
    User *target = find_user(name, *user_ptr_del);
    if (target == NULL) {
        // Return 1 if we cannot find the user
	return 1;
    } else {
	// Delete target user from his frind's friend list.
	// tempUser:  is to temporary store the pointer of User in 
	// 	      target user's user's friend list 
	// index:     used to iterate target's friend list
	// tempIndex: used to iterate tempUser's friend list
	// curr:      used to temporary store the pointer of User
	//            in tempUser's friend list
	User *tempUser;
	int moveFlag;
	int tempIndex;
	int index = 0;
	User *curr;
	tempUser = (target->friends)[index];
	while ((index < MAX_FRIENDS) && (tempUser != NULL)) {
            tempIndex = 0;
	    curr = (tempUser->friends)[tempIndex];
	    moveFlag = 0;
				
	    // check every User in target's friend list, 
	    // delete target from these Users's friend list 
	    // In a specific user, once we friend the target, 
	    // then we set the moveFlag to be 1, then let the 
	    // current friend list spot be the
	    // pointer of user of next spot 
	    while ((curr != NULL) && (tempIndex < MAX_FRIENDS)) {
	        if (curr == target) {	
		    moveFlag = 1;	
		} 
		if (moveFlag == 1) {
		    if (tempIndex < (MAX_FRIENDS - 1)) {
		        // situation that we found 
			// target but tempIndex hasn't 
			// meet the MAX_FRIEND-2 yet
			(tempUser->friends)[tempIndex] = (tempUser->friends)
			                                 [tempIndex + 1];
		    } else {
			// situation that we found 
			// target but the friend list 
			// list is full before we 
			// delete, then we should set 
			// the last spot to be NULL 
			(tempUser->friends)[tempIndex] = NULL;
		    }
		}
		tempIndex++; 
		if (tempIndex < MAX_FRIENDS) {
			curr = (tempUser->friends)[tempIndex];
		}
            }
	    index++;	  
            if (index < MAX_FRIENDS) {
                tempUser = (target->friends)[index];
	    } 
        }
	delete_next(name, user_ptr_del);
	// free what we've just deleted 						
	recursive_free_post(target->first_post);
	free(target);
	return 0;
    }
}
