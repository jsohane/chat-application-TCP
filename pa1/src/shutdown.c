#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include "./personal.h"

#include "../include/logger.h"
#include "../include/global.h"

int clear_users(struct user** users, size_t user_count, size_t bufsize) {
    size_t index;

    if (user_count > bufsize) {
        return EXIT_FAILURE;
    }

    size_t count = 0;

    for (index = 0; index < bufsize; index++)
    {
        if (users[index] != NULL)
        {
            // Freeing backlog messages list...
            for (size_t j = 0; j < users[index]->backlog_count; j++)
            {
                if (users[index]->backlog_list[j] != NULL)
                {
                    free(users[index]->backlog_list[j]);
                }
            }
            users[index]->backlog_list = NULL;

            // Freeing blocked users list...
            for (size_t j = 0; j < MAX_LIVE_USERS; j++)
            {
                free(users[index]->blocked_list[j]);
            }
            close(users[index]->fd);
            free(users[index]);
            count++;
        }
    }
    if (count != user_count)
    {
        return EXIT_FAILURE;
    }
    // printf("Clearing users list successful...\n");
    
    return EXIT_SUCCESS;
}