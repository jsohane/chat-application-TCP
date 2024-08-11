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

#include "../include/global.h"
#include "../include/logger.h"

static int compare_users(const void *a, const void *b);

void list_users(struct user **users, size_t user_count)
{
    // printf("user_count inside list_user: %d\n", user_count);
    if (user_count < 1)
    {
        cse4589_print_and_log("[%s:FAILURE]\n", "LIST");
        cse4589_print_and_log("[%s:END]\n", "LIST");
        return;
    }

    // sorting based on port numbers
    qsort(users, user_count, sizeof(struct user *), compare_users);

    size_t index;
    int count = 0;
    cse4589_print_and_log("[%s:SUCCESS]\n", "LIST");
    // printf("user count is %d\n", user_count);
    for (index = 0; index < MAX_USERS; index++)
    {
        if (users[index] != NULL)
        {
            count++;
            cse4589_print_and_log("%-5d%-35s%-20s%-8d\n", count, users[index]->hostname, users[index]->ip, users[index]->port);
            user_count--;
            if (!user_count)
            {
                break;
            }
        }
    }
    cse4589_print_and_log("[%s:END]\n", "LIST");
}

int add_msg_blist(char *send_ip, char *message, struct user *receiver)
{
    printf("Inside add_msg_blist\n");

    for (int i = 0; i < BACKLOG_BUFFER_SIZE; i++)
    {
        if (receiver != NULL && receiver->backlog_list[i] == NULL)
        {
            printf("location %d of backlog list has NULL\n", i);
            receiver->backlog_list[i] = malloc(sizeof(struct backlog_msg));
            strncpy(receiver->backlog_list[i]->sender_ip, send_ip, INET_ADDRSTRLEN);
            strncpy(receiver->backlog_list[i]->message, message, MSG_SIZE);
            printf("updated backlog with this message: %s\n", receiver->backlog_list[i]->message);
            receiver->backlog_count++;
            printf("successfully returning 0 -> success from add_msg_blist\n");
            return 0;
        }
    }

    return -1;
}

int clear_blist(struct user *client)
{
    size_t count = 0;
    for (size_t i = 0; i < BACKLOG_BUFFER_SIZE; i++)
    {
        if (client->backlog_list[i] != NULL)
        {
            free(client->backlog_list[i]);
            count++;
        }
    }

    if (count == client->backlog_count)
    {
        return count;
    }
    else
    {
        return count;
    }
}

/*
 * @section
 *
 * COMPARE_USERS is the comparator function used to sort the users in the array before they can be displayed for the LIST command
 * Because new users can be added anywhere there is an empty spot in the array, it also means that when users are removed, there might
 * be empty spots left in the array that we have to deal with. For our case, since we have to display the list in an ascending order
 * of ports, we decided to consider NULL to be this hypothetical struct with the highest possible port number (infinity).
 */

static int compare_users(const void *a, const void *b)
{
    const struct user *user_a = *(const struct user **)a;
    const struct user *user_b = *(const struct user **)b;

    if (user_a == NULL && user_b == NULL)
    {
        return 0;
    }
    else if (user_a == NULL)
    {
        return 1; // NULL is considered greater than a valid user
    }
    else if (user_b == NULL)
    {
        return -1; // Valid user is considered less than NULL
    }

    // Comparing port numbers
    return user_a->port - user_b->port;
}

int old_user(struct user **users, const char *client_ip, unsigned short client_port, const char *hostname)
{
    for (int i = 0; i < MAX_USERS; i++)
    {
        if (users[i] != NULL)
        { // Ensure the user slot is in use
            if (strncmp(users[i]->ip, client_ip, INET_ADDRSTRLEN) == 0 && users[i]->port == client_port && strncmp(users[i]->hostname, hostname, NI_MAXHOST) == 0)
            {
                return i; // Match found, return index
            }
        }
    }
    return -1; // No match found
}
