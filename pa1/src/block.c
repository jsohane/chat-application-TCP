#define _POSIX_C_SOURCE 200809L

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

int block (const char *ip_address, struct user* client)
{
    
    for (size_t i = 0; i < MAX_LIVE_USERS; i++)
    {
        if (client->blocked_list[i] != NULL && strncmp(client->blocked_list[i], ip_address, INET_ADDRSTRLEN) == 0)
        {
            printf("%s is already blocked\n", client->blocked_list[i]);
            return 0; // Already blocked
        }
    }

    for (size_t i = 0; i < MAX_LIVE_USERS; i++)
    {
        if (client->blocked_list[i] == NULL)
        {
            client->blocked_list[i] = (char *) malloc (sizeof(char) * INET_ADDRSTRLEN);
            if (client->blocked_list[i] == NULL)
            {
                perror ("allocation for blocked list string failed.");
            }
            memset ((void *) client->blocked_list[i], '\0', sizeof(char) * INET_ADDRSTRLEN);

            strncpy(client->blocked_list[i], ip_address, INET_ADDRSTRLEN);
            printf("The following IP is now blocked: %s\n", client->blocked_list[i]);
            
            return 0;   // This is a new block and block is successful
        }
    }

    return 1;   // Block is unsuccessful because array is already full
}

int unblock (const char *ip_address, struct user* client)
{
    for (size_t i = 0; i < MAX_LIVE_USERS; i++)
    {
        if (client->blocked_list[i] != NULL && strncmp(client->blocked_list[i], ip_address, INET_ADDRSTRLEN) == 0)
        {
            free(client->blocked_list[i]);
            client->blocked_list[i] = NULL;
            return 0;
        }
    }
    return 0; // Already success because blocked_list is empty or the blocked_list is full and the IP is not in it.
}

int isblocked (const char *ip_address, const struct user* client)
{
    for (size_t i = 0; i < MAX_LIVE_USERS; i++)
    {
        if (client->blocked_list[i] != NULL && strncmp(client->blocked_list[i], ip_address, INET_ADDRSTRLEN) == 0)
        {
            return 1; // the user has blocked the IP
        }
    }

    printf("isblocked() finished successfully and returning 0...\n");
    return 0;
}
