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

char *generate_data_string(const char *command, int count, struct user *users[])
{
    char * data_string;
    data_string = malloc(sizeof(char) * BUFFER_SIZE);
    if (data_string == NULL)
    {
        perror("Allocating memory for REFRESH buffer failed");
    }
    memset(data_string, '\0', sizeof(char) * BUFFER_SIZE);

    /* Building the string into data_string in the format:
    *  COMMAND,COUNT,USER1->IP:USER1->HOSTNAME:USER1->PORT,USER2->IP:....
    */
    strcpy(data_string, command);
    strcat(data_string, ",");
    sprintf(data_string + strlen(data_string), "%d", count);
    strcat(data_string, ",");

    for (int i = 0; i < MAX_USERS; i++) {
        if (users[i] != NULL)
        {
            strcat(data_string, users[i]->ip);
            strcat(data_string, ":");
            strcat(data_string, users[i]->hostname);
            strcat(data_string, ":");
            sprintf(data_string + strlen(data_string), "%d", users[i]->port);
            strcat(data_string, ",");
        }
    }

    // Removing the last comma
    data_string[strlen(data_string) - 1] = '\0';

    // printf("The function generate_data_string executed successfully and is now returning!!\n");
    return data_string;
}

size_t parse_user_data(const char *msg, char *command, struct user *users[]) {
    // Temporary storage for parsing input msg
    int tempCount = 0;
    size_t commas = 0; // This one is specifically to make sure we didn't make any memory mistakes. No other significance.
    char *userData; // Temporary copy for the string token given by strtok
    char *msg_copy = malloc(sizeof(char) * BUFFER_SIZE); // Temporary copy of msg for strtok
    
    if (msg_copy == NULL) {
        perror("Memeory allocation failed in parse_user_data function");
    }
    memset(msg_copy, '\0', sizeof(char) * BUFFER_SIZE);
    

    /* Creating a duplicte of the server's message */
    strcpy(msg_copy, msg);

    // Extract command and count
    userData = strtok(msg_copy, ",");
    if (userData == NULL) return -1;
    strcpy(command, userData);

    userData = strtok(NULL, ",");
    if (userData == NULL) return -1;
    tempCount = atoi(userData);

    size_t user_index = 0;

    /* Reinitializing the list before filling it up with users details */
    for (; user_index < MAX_USERS; user_index++)
    {
        users[user_index] = NULL;
    }

    // Resetting our index to 0
    user_index = 0;

    userData = strtok(NULL, ","); // Now start extracting each user msg
    while (userData != NULL && user_index < MAX_USERS)
    {
        // updating the number of active users
        commas++;
        users[user_index] = malloc(sizeof(struct user));
        if (users[user_index] == NULL)
        {
            perror("memory allocation for user failed");
            return -1;
        }

        // Use sscanf to extract ip, hostname, and port from the segment
        if (sscanf(userData, "%[^:]:%[^:]:%hu", users[user_index]->ip, users[user_index]->hostname, &users[user_index]->port) != 3)
        {
            perror("unable to parse user information from the received buffer");
            return -1;
        }
        users[user_index]->status = ON;

        userData = strtok(NULL, ",");
        user_index++;

    }
    if (commas == tempCount)
    {
        return commas;
    }
    free(msg_copy);

    return -1;
}

char *generate_backlog_string(const char *command, struct user *client)
{
    if (client->backlog_count == 0)
    {
        printf("backlog count 0, returning NULL");
        return NULL;
    }

    char * data_string; // This is the final string to be returned
    data_string = malloc(sizeof(char) * BUFFER_SIZE);
    if (data_string == NULL)
    {
        perror("Allocating memory for BACKLOG buffer failed");
    }
    memset(data_string, '\0', sizeof(char) * BUFFER_SIZE);

    /* Building the string into data_string in the format:
    *  COMMAND(RS)COUNT(RS)USER1->IP(US)USER1->MSG(RS)USER2->IP(US)USER2->MSG(RS)....
    */
    strcpy(data_string, command);
    sprintf(data_string + strlen(data_string), "%c", 30);
    sprintf(data_string + strlen(data_string), "%d", client->backlog_count);
    sprintf(data_string + strlen(data_string), "%c", 30);

    for (int i = 0; i < BACKLOG_BUFFER_SIZE; i++) {
        if (client->backlog_list[i] != NULL)
        {
            strcat(data_string, client->backlog_list[i]->sender_ip);
            sprintf(data_string + strlen(data_string), "%c", 31);
            strcat(data_string, client->backlog_list[i]->message);
            sprintf(data_string + strlen(data_string), "%c", 30);
        }
    }

    // Replacing the last delimiter with null terminator
    data_string[strlen(data_string) - 1] = '\0';

    printf("The function generate_backlog_string %s, executed successfully and is now returning!!\n",data_string);
    return data_string;
}

// Write a function to parse the backlog_list (inverse of the function above)
void parse_backlog_string(char *data_string) {
    printf("Inside parse_backlog_string method, data_string is %s\n",data_string);
    const char RS = 31; // Different messages are separated by 31
    const char US = 30; // Fields within messages are separated by 30

    char *rest = data_string;

    // Skip the command part
    strtok_r(rest, &US, &rest);

    // Extract the count of messages
    char *countStr = strtok_r(rest, &US, &rest);
    int count = atoi(countStr); // Convert count from string to integer

    for (int i = 0; i < count; ++i) {
        char *ip = strtok_r(rest, &RS, &rest);
        if (ip == NULL) {
            printf("ip is NULL ,returning");
            break; // No more data
        }

        char *msg = strtok_r(rest, &US, &rest);
        if (msg == NULL) {
            // If we reach here, it means there's an inconsistency in data
            printf("Data inconsistency encountered. Expected message not found.\n");
            break;
        }

        cse4589_print_and_log("msg from:%s\n[msg]:%s\n",ip,msg);
    }
}

// Below function generates a message string
char *generate_msg_string(char *command, char *send_ip, char *message)
{
    char * data_string; // This is the final string to be returned
    data_string = malloc(sizeof(char) * BUFFER_SIZE);
    if (data_string == NULL)
    {
        perror("Allocating memory for BACKLOG buffer failed");
    }
    memset(data_string, '\0', sizeof(char) * BUFFER_SIZE);

    /* Building the string into data_string in the format:
    *  COMMAND(RS)SENDER_IP(RS)MESSAGE(RS)....
    */
    strcpy(data_string, command);
    sprintf(data_string + strlen(data_string), "%c", (char)30);
    strcat(data_string, send_ip);
    sprintf(data_string + strlen(data_string), "%c", (char)30);
    strcat(data_string, message);
    sprintf(data_string + strlen(data_string), "%c", (char)30);


    // Replacing the last delimiter with null terminator
    // data_string[strlen(data_string) - 1] = '\0';

    printf("The function generate_msg_string is going to return the following string:\n%s\n", data_string);
    printf("The function generate_message_string executed successfully and is now returning!!\n");
    return data_string;
}
