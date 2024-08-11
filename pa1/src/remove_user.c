#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./personal.h"

int remove_user(struct user **users, size_t *user_count, int fd)
{
    for (size_t i = 0; i < MAX_USERS; ++i)
    {
        if (users[i] != NULL)
	    {
            if(users[i]->fd == fd)
            {
                // Freeing backlog messages list...
                for (size_t j = 0; j < users[i]->backlog_count; j++)
                {
                    if (users[i]->backlog_list[j] != NULL)
                    {
                        free(users[i]->backlog_list[j]);
                    }
                }
                users[i]->backlog_list = NULL;

                // Freeing blocked users list...
                for (size_t j = 0; j < MAX_LIVE_USERS; j++)
                {
                    free(users[i]->blocked_list[j]);
                }

                // Freeing the user struct...
                free(users[i]); 
                users[i] = NULL;
                (*user_count)--;

                return 1; // User with the specified IP removed successfully
            }
        }
    }
    // User with the specified IP not found
    return 0;
}

int logout_user(struct user** users, const int fd)
{
    size_t location = MAX_USERS;
    for (size_t i = 0; i < MAX_USERS; i++)
    {
        if(users[i] != NULL && users[i]->fd == fd)
        {
            location = i;
        }
    }

    if (location == MAX_USERS)
    {
        return 0;
    }

    // users[location]->fd = 0;
    users[location]->status = OFF;

    for (size_t i = 0; i < users[location]->backlog_count; i++)
    {
        if (users[location]->backlog_list[i] != NULL)
        {
            free(users[location]->backlog_list[i]);
        }
    }
    users[location]->backlog_list = NULL;
    users[location]->backlog_count = 0;

    return 1;
}