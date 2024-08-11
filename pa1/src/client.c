/**
 * @section REFERENCE
 *
 * Code was borrowed from Swetank Kumar Saha <swetank@buffalo.edu>'s beginner template.
 * Modifications have been made to suit the reuqirements of the assignment.
 *
 * @section DESCRIPTION
 *
 * This file contains the client init logic.
 * Uses the select() API to multiplex between network I/O and STDIN.
 */
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <strings.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include "./personal.h"

#include "../include/logger.h"
#include "../include/global.h"

#define BACKLOG 5
#define STDIN 0
#define TRUE 1

int connect_to_host(int fdsocket, char *server_ip, char *server_port);
bool isValidAscii(const char *str);

int client(const char *port)
{
    // Creating an array of pointers to struct user and initializing them with NULLs
    struct user *users[MAX_USERS];
    for (int i = 0; i < MAX_USERS; i++)
    {
        users[i] = NULL;
    }
    size_t user_count = 0;

    // Write code for setting up the listening socket here
    int client_socket, head_socket, selret, sock_index, fdaccept = 0;
    socklen_t caddr_len;
    struct sockaddr_in client_addr;
    struct addrinfo hints, *res;
    fd_set master_list, watch_list;

    struct sockaddr_in my_addrs;
    client_socket = socket(AF_INET, SOCK_STREAM, 0); // return socket file descriptor
    if (client_socket < 0)
    {
        perror("Failed to create socket");
        return 0;
    }

    // setting up client socket
    my_addrs.sin_family = AF_INET;
    my_addrs.sin_addr.s_addr = INADDR_ANY;
    my_addrs.sin_port = htons(atoi(port));
    int optval = 1;
    setsockopt(client_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)); // so OS will release the port immediately for reuse
    if (bind(client_socket, (struct sockaddr *)&my_addrs, sizeof(struct sockaddr_in)) != 0)
    {
        perror("Error in binding client port");
    }

    /* ---------------------------------------------------------------------------- */

    /* Zero select FD sets */
    FD_ZERO(&master_list);
    FD_ZERO(&watch_list);

    /* Register the listening socket */
    // FD_SET(client_socket, &master_list);
    /* Register STDIN */
    FD_SET(STDIN, &master_list);

    head_socket = STDIN;

    int logged_in = 0;
    int login_count = 0;
    char *server_actual_ip;
    int server_actual_port;

    while (TRUE)
    {
        FD_ZERO(&master_list);
        FD_ZERO(&watch_list);

        // /* Register the listening socket */
        FD_SET(client_socket, &master_list);
        // /* Register STDIN */
        FD_SET(STDIN, &master_list);

        head_socket = client_socket;

        memcpy(&watch_list, &master_list, sizeof(master_list));

        /* select() system call. This will BLOCK */
        selret = select(head_socket + 1, &watch_list, NULL, NULL, NULL);
        if (selret < 0)
        {
            perror("select failed.");
        }

        if (selret > 0)
        {
            /* Loop through socket descriptors to check which ones are ready */
            for (sock_index = 0; sock_index <= head_socket; sock_index += 1)
            {
                if (FD_ISSET(sock_index, &watch_list))
                {
                    if (sock_index == STDIN)
                    {
                        char *cmd = (char *)malloc(sizeof(char) * CMD_SIZE);
                        memset(cmd, '\0', CMD_SIZE);
                        if (fgets(cmd, CMD_SIZE - 1, stdin) == NULL)
                        {
                            exit(-1);
                        }

                        // Removing the newline character will null character
                        char *cmd_index = cmd;
                        while (*cmd_index != '\0')
                        {
                            if (*cmd_index == '\n')
                            {
                                *cmd_index = '\0';
                                break;
                            }
                            cmd_index++;
                        }

                        // STAGE 1 - Implement AUTHOR here...
                        if (strncmp(cmd, "AUTHOR", strlen("AUTHOR")) == 0)
                        {
                            author();
                        }

                        // STAGE 1 - Implement IP here...
                        if (strncmp(cmd, "IP", strlen("IP")) == 0)
                        {
                            get_ip();
                        }

                        // STAGE 1 - Implement LIST here...
                        if (strncmp(cmd, "LIST", strlen("LIST")) == 0 && logged_in == 1)
                        {
                            // printf("user count is %d\n", user_count);
                            list_users(users, user_count);
                        }

                        // STAGE 1 - Implement PORT here...
                        if (strncmp(cmd, "PORT", strlen("PORT")) == 0)
                        {
                            print_port(client_socket);
                        }

                        // STAGE 1 - Implement LOGIN here...
                        if (strncmp(cmd, "LOGIN", strlen("LOGIN")) == 0)
                        {

                            // First we need to extract the three strings embedded in our command
                            /* Making a copy...*/
                            char *cmd_copy = malloc(sizeof(char) * CMD_SIZE);
                            memset(cmd_copy, '\0', CMD_SIZE);
                            if (cmd_copy == NULL)
                            {
                                perror("Allocating memory for command copy failed");
                            }

                            if (strncpy(cmd_copy, cmd, sizeof(char) * CMD_SIZE) == NULL)
                            {
                                perror("Copying user command for LOGIN failed");
                            }
                            char *token;
                            char *s_ip;
                            int s_porti;

                            // Get the first token ("LOGIN")
                            token = strtok(cmd_copy, " ");
                            if (token != NULL)
                            {
                                // check to see if the length is exactly the same or not.
                                if (strlen(token) != strlen("LOGIN"))
                                {
                                    cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
                                    cse4589_print_and_log("[%s:END]\n", "LOGIN");
                                    free(cmd_copy);
                                    free(cmd);
                                    break;
                                }
                                // Get the IP address
                                s_ip = strtok(NULL, " ");
                                if (s_ip != NULL)
                                {
                                    // Get the port number as a string
                                    token = strtok(NULL, " ");
                                    if (token != NULL)
                                    {
                                        s_porti = atoi(token);
                                    }
                                    else
                                    {
                                        cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
                                        cse4589_print_and_log("[%s:END]\n", "LOGIN");
                                        free(cmd_copy);
                                        free(cmd);
                                        break;
                                    }
                                }
                                else
                                {
                                    cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
                                    cse4589_print_and_log("[%s:END]\n", "LOGIN");
                                    free(cmd_copy);
                                    free(cmd);
                                    break;
                                }
                            }

                            // Validate IP
                            unsigned char buf[sizeof(struct in_addr)];
                            if (inet_pton(AF_INET, s_ip, buf) == 0)
                            {
                                cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
                                cse4589_print_and_log("[%s:END]\n", "LOGIN");
                                free(cmd_copy);
                                free(cmd);
                                break;
                            }

                            if ((s_porti > 65535) || (s_porti < 1024))
                            {
                                cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
                                cse4589_print_and_log("[%s:END]\n", "LOGIN");
                                free(cmd_copy);
                                free(cmd);
                                break;
                            }
                            printf("login_count is %d, server_actual_ip is %s, server_actual_port is %d",login_count, server_actual_ip, server_actual_port);
                            // second time login. no need to connect to host again
                            if (login_count > 0 && strcmp(s_ip, server_actual_ip) == 0 && s_porti == server_actual_port)
                            {
                                printf("trying to login second time:");
                                

                                // sending the server message that this is a relogin
                                char *outgoing_msg = (char *)malloc(sizeof(char) * BUFFER_SIZE);
                                memset(outgoing_msg, '\0', BUFFER_SIZE);
                                if (outgoing_msg == NULL)
                                {
                                    perror("Creating message for the server failed...");
                                }
                                // copying the Refresh command to msg buffer
                                strcpy(outgoing_msg, "RELOGIN");
                                if (send(client_socket, outgoing_msg, sizeof(outgoing_msg), 0) != sizeof(outgoing_msg))
                                {
                                    cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
                                }
                                logged_in = 1;
                                login_count += 1;
                                printf("client side logged in successfully");
                                // Freeing the created memory for outgoing message
                                free(outgoing_msg);
                            }
                            else if (login_count == 0)
                            {
                                if (connect_to_host(client_socket, s_ip, token) <= 0)
                                {
                                    cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
                                }
                                printf("inside login, client successfully logins using socket number %d\n", client_socket);
                                FD_SET(client_socket, &master_list);
                                if (client_socket > head_socket)
                                {
                                    head_socket = client_socket;
                                }
                                login_count += 1;
                                logged_in = 1;
                                server_actual_ip = malloc(strlen(s_ip) + 1);
                                strcpy(server_actual_ip, s_ip);

                                server_actual_port = s_porti;
                            }

                            // // Connecting to the host for the first time....
                            // if (connect_to_host(client_socket, s_ip, token) <= 0)
                            // {
                            //     cse4589_print_and_log("[%s:ERROR]\n", "LOGIN");
                            // }
                            // printf("inside login, client successfully logins using socket number %d\n",client_socket);
                            // FD_SET(client_socket, &master_list);
                            // if (client_socket > head_socket)
                            // {
                            //     head_socket = client_socket;
                            // }
                            // login_count += 1;
                            // logged_in = 1;

                            free(cmd_copy);
                        }

                        // STAGE 1 - Implement REFRESH here...
                        if (strncmp(cmd, "REFRESH", strlen("REFRESH")) == 0 && logged_in == 1)
                        {
                            /*
                             * 1. Create a char array to store refresh command
                             * 2. send() the above message
                             */
                            char *outgoing_msg = (char *)malloc(sizeof(char) * BUFFER_SIZE);
                            memset(outgoing_msg, '\0', BUFFER_SIZE);
                            if (outgoing_msg == NULL)
                            {
                                perror("Creating message for the server failed...");
                            }
                            // copying the Refresh command to msg buffer
                            strcpy(outgoing_msg, cmd);
                            if (send(client_socket, outgoing_msg, sizeof(outgoing_msg), 0) != sizeof(outgoing_msg))
                            {
                                cse4589_print_and_log("[%s:ERROR]\n", cmd);
                            }

                            // Freeing the created memory for outgoing message
                            free(outgoing_msg);
                        }

                        // STAGE 2 - Implement SEND here...
                        if ((strncmp(cmd, "SEND", strlen("SEND")) == 0) && logged_in == 1)
                        {
                            printf("Inside user command SEND\n");
                            char *cmd_copy = malloc(sizeof(char) * BUFFER_SIZE);
                            memset(cmd_copy, '\0', BUFFER_SIZE);
                            if (cmd_copy == NULL)
                            {
                                perror("Allocating memory for command copy failed");
                            }

                            if (strncpy(cmd_copy, cmd, sizeof(char) * BUFFER_SIZE) == NULL)
                            {
                                perror("Copying user command for SEND failed");
                            }

                            char *token;
                            char *s_ip;
                            char *msg_to_send;

                            // Get the first token ("SEND")
                            token = strtok(cmd_copy, " ");
                            if (token != NULL)
                            {
                                // Check to see if the command is exactly "SEND"
                                if (strcmp(token, "SEND") != 0)
                                {
                                    cse4589_print_and_log("[%s:ERROR]\n", "SEND");
                                    cse4589_print_and_log("[%s:END]\n", "SEND");
                                    free(cmd_copy);
                                    free(cmd);
                                    break;
                                }
                                // Get the IP address
                                s_ip = strtok(NULL, " ");
                                if (s_ip != NULL)
                                {
                                    // Validate IP
                                    unsigned char buf[sizeof(struct in_addr)];
                                    if (inet_pton(AF_INET, s_ip, buf) == 0)
                                    {
                                        cse4589_print_and_log("[%s:ERROR]\n", "SEND");
                                        cse4589_print_and_log("[%s:END]\n", "SEND");
                                        free(cmd_copy);
                                        free(cmd);
                                        break;
                                    }
                                    size_t rec_loc = -1;
                                    // Getting the receiver's location of user in users
                                    for (size_t i = 0; i < MAX_USERS; i++)
                                    {
                                        if (users[i] != NULL && strncmp(users[i]->ip, s_ip, INET_ADDRSTRLEN) == 0)
                                        {
                                            rec_loc = i;
                                            break;
                                        }
                                    }

                                    if (rec_loc != -1)
                                    {

                                        msg_to_send = strtok(NULL, "");
                                        if (msg_to_send != NULL)
                                        {
                                            // Checking if message length is within 256 bytes
                                            if (strlen(msg_to_send) <= 256 && isValidAscii(msg_to_send))
                                            {

                                                char *formatted_msg;
                                                formatted_msg = malloc(sizeof(char) * BUFFER_SIZE);
                                                if (formatted_msg == NULL)
                                                {
                                                    perror("Allocating memory for buffer faiiled");
                                                }

                                                memset(formatted_msg, '\0', sizeof(char) * BUFFER_SIZE);

                                                strcpy(formatted_msg, "SEND");
                                                sprintf(formatted_msg + strlen(formatted_msg), "%c", 30);
                                                sprintf(formatted_msg + strlen(formatted_msg), "%s", s_ip);
                                                sprintf(formatted_msg + strlen(formatted_msg), "%c", 30);
                                                sprintf(formatted_msg + strlen(formatted_msg), "%s", msg_to_send);

                                                // Send the formatted message
                                                if (send(client_socket, formatted_msg, strlen(formatted_msg), 0) < 0)
                                                {
                                                    cse4589_print_and_log("[%s:ERROR]\n", "SEND");
                                                }
                                                else
                                                {
                                                    cse4589_print_and_log("[%s:SUCCESS]\n", "SEND");
                                                }

                                                // Free the formatted message memory
                                                free(formatted_msg);
                                            }
                                            else
                                            {
                                                // error if msg length > 256
                                                cse4589_print_and_log("[%s:ERROR]\n", "SEND");
                                                cse4589_print_and_log("[%s:END]\n", "SEND");
                                                free(cmd_copy);
                                                free(cmd);
                                                break;
                                            }
                                        }
                                        else
                                        {
                                            // error if msg_to_send is NULL
                                            cse4589_print_and_log("[%s:ERROR]\n", "SEND");
                                            cse4589_print_and_log("[%s:END]\n", "SEND");
                                            free(cmd_copy);
                                            free(cmd);
                                            break;
                                        }
                                    }
                                    else
                                    {
                                        cse4589_print_and_log("[%s:ERROR]\n", "SEND");
                                        cse4589_print_and_log("[%s:END]\n", "SEND");
                                    }
                                }
                                else
                                {
                                    // error if ip is NULL
                                    cse4589_print_and_log("[%s:ERROR]\n", "SEND");
                                    cse4589_print_and_log("[%s:END]\n", "SEND");
                                    free(cmd_copy);
                                    free(cmd);
                                    break;
                                }
                            }
                            free(cmd_copy);
                        }

                        // STAGE 2 - Implement BROADCAST here...
                        if ((strncmp(cmd, "BROADCAST", strlen("BROADCAST")) == 0) && logged_in == 1)
                        {
                            char *cmd_copy = malloc(sizeof(char) * CMD_SIZE);
                            memset(cmd_copy, '\0', CMD_SIZE);
                            if (cmd_copy == NULL)
                            {
                                perror("Allocating memory for command copy failed");
                            }

                            if (strncpy(cmd_copy, cmd, sizeof(char) * CMD_SIZE) == NULL)
                            {
                                perror("Copying user command failed");
                            }

                            char *token;
                            char *msg_to_broadcast;

                            // Get the first token ("BROADCAST")
                            token = strtok(cmd_copy, " ");
                            if (token != NULL)
                            {
                                // Check to see if the command is exactly "BROADCAST"
                                if (strcmp(token, "BROADCAST") == 0)
                                {
                                    msg_to_broadcast = strtok(NULL, "");
                                    if (msg_to_broadcast != NULL)
                                    {
                                        // Checking if message length is within 256 bytes
                                        if (strlen(msg_to_broadcast) <= 256 && isValidAscii(msg_to_broadcast))
                                        {
                                            // Changed: Include space for "BROADCAST " prefix and NULL terminator
                                            char *formatted_msg;
                                            formatted_msg = malloc(sizeof(char) * BUFFER_SIZE);
                                            if (formatted_msg == NULL)
                                            {
                                                perror("Allocating memory for buffer faiiled");
                                            }

                                            memset(formatted_msg, '\0', sizeof(char) * BUFFER_SIZE);

                                            strcpy(formatted_msg, "BROADCAST");
                                            sprintf(formatted_msg + strlen(formatted_msg), "%c", 30);
                                            sprintf(formatted_msg + strlen(formatted_msg), "%s", msg_to_broadcast);

                                            // Send the formatted message
                                            if (send(client_socket, formatted_msg, strlen(formatted_msg), 0) < 0)
                                            {
                                                cse4589_print_and_log("[%s:ERROR]\n", "BROADCAST");
                                            }
                                            else
                                            {
                                                // Log success
                                                cse4589_print_and_log("[%s:SUCCESS]\n", "BROADCAST");
                                            }

                                            // Free the formatted message memory
                                            free(formatted_msg);
                                        }
                                        else
                                        {
                                            // Error if msg length > 256
                                            cse4589_print_and_log("[%s:ERROR]\n", "BROADCAST");
                                            cse4589_print_and_log("[%s:END]\n", "BROADCAST");
                                        }
                                    }
                                    else
                                    {
                                        // Error if msg_to_broadcast is NULL
                                        cse4589_print_and_log("[%s:ERROR]\n", "BROADCAST");
                                        cse4589_print_and_log("[%s:END]\n", "BROADCAST");
                                    }
                                }
                                else
                                {
                                    // Error if command is not "BROADCAST"
                                    cse4589_print_and_log("[%s:ERROR]\n", "BROADCAST");
                                    cse4589_print_and_log("[%s:END]\n", "BROADCAST");
                                }
                            }
                            free(cmd_copy);
                        }

                        // STAGE 2 - Implement BLOCK here...
                        if ((strncmp(cmd, "BLOCK", strlen("BLOCK")) == 0) && logged_in == 1)
                        {
                            printf("Inside user command BLOCK\n");
                            // Copy the command to a new buffer to avoid modifying the original command string
                            char *cmd_copy = malloc(sizeof(char) * CMD_SIZE);
                            if (cmd_copy == NULL)
                            {
                                perror("Allocating memory for command copy failed");
                            }

                            strncpy(cmd_copy, cmd, CMD_SIZE);
                            char *token;
                            char *client_ip;

                            // Get the first token, which should be "BLOCK"
                            token = strtok(cmd_copy, " ");
                            // Proceed only if the token correctly matches "BLOCK"
                            if (token != NULL && strcmp(token, "BLOCK") == 0)
                            {
                                // Get the client IP address
                                client_ip = strtok(NULL, " ");
                                if (client_ip != NULL)
                                {
                                    // Validate the client IP address
                                    unsigned char buf[sizeof(struct in_addr)];
                                    if (inet_pton(AF_INET, client_ip, buf) != 1) // inet_pton returns 1 on success for AF_INET
                                    {
                                        cse4589_print_and_log("[%s:ERROR]\n", "BLOCK");
                                        cse4589_print_and_log("[%s:END]\n", "BLOCK");
                                    }
                                    else
                                    {
                                        char *formatted_msg;
                                        formatted_msg = malloc(sizeof(char) * BUFFER_SIZE);
                                        if (formatted_msg == NULL)
                                        {
                                            perror("Allocating memory for buffer faiiled");
                                        }

                                        memset(formatted_msg, '\0', sizeof(char) * BUFFER_SIZE);

                                        strcpy(formatted_msg, "BLOCK");
                                        sprintf(formatted_msg + strlen(formatted_msg), "%c", 30);
                                        sprintf(formatted_msg + strlen(formatted_msg), "%s", client_ip);

                                        // Send the formatted message
                                        if (send(client_socket, formatted_msg, strlen(formatted_msg), 0) < 0)
                                        {
                                            cse4589_print_and_log("[%s:ERROR]\n", "BLOCK");
                                        }
                                        else
                                        {
                                            cse4589_print_and_log("[%s:SUCCESS]\n", "BLOCK");
                                            // Log success if needed
                                        }

                                        // Free the formatted message memory
                                        free(formatted_msg);
                                        // IP address is valid, proceed with the block operation...
                                        // Implement your logic for blocking the client IP here
                                    }
                                }
                                else
                                {
                                    cse4589_print_and_log("[%s:ERROR]\n", "BLOCK");
                                    cse4589_print_and_log("[%s:END]\n", "BLOCK");
                                }
                            }
                            else
                            {
                                cse4589_print_and_log("[%s:ERROR]\n", "BLOCK");
                                cse4589_print_and_log("[%s:END]\n", "BLOCK");
                            }

                            // Free the allocated memory for the command copy
                            free(cmd_copy);
                        }

                        // STAGE 2 - Implement UNBLOCK here...
                        if ((strncmp(cmd, "UNBLOCK", strlen("UNBLOCK")) == 0) && logged_in == 1)
                        {
                            // Copy the command to a new buffer to avoid modifying the original command string
                            char *cmd_copy = malloc(sizeof(char) * CMD_SIZE);
                            if (cmd_copy == NULL)
                            {
                                perror("Allocating memory for command copy failed");
                            }

                            strncpy(cmd_copy, cmd, CMD_SIZE);
                            char *token;
                            char *client_ip;

                            // Get the first token, which should be "BLOCK"
                            token = strtok(cmd_copy, " ");
                            // Proceed only if the token correctly matches "BLOCK"
                            if (token != NULL && strcmp(token, "UNBLOCK") == 0)
                            {
                                // Get the client IP address
                                client_ip = strtok(NULL, " ");
                                if (client_ip != NULL)
                                {
                                    // Validate the client IP address
                                    unsigned char buf[sizeof(struct in_addr)];
                                    if (inet_pton(AF_INET, client_ip, buf) != 1) // inet_pton returns 1 on success for AF_INET
                                    {
                                        cse4589_print_and_log("[%s:ERROR]\n", "UNBLOCK");
                                        cse4589_print_and_log("[%s:END]\n", "UNBLOCK");
                                    }
                                    else
                                    {
                                        char *formatted_msg;
                                        formatted_msg = malloc(sizeof(char) * BUFFER_SIZE);
                                        if (formatted_msg == NULL)
                                        {
                                            perror("Allocating memory for buffer faiiled");
                                        }

                                        memset(formatted_msg, '\0', sizeof(char) * BUFFER_SIZE);

                                        strcpy(formatted_msg, "UNBLOCK");
                                        sprintf(formatted_msg + strlen(formatted_msg), "%c", 30);
                                        sprintf(formatted_msg + strlen(formatted_msg), "%s", client_ip);

                                        // Send the formatted message
                                        if (send(client_socket, formatted_msg, strlen(formatted_msg), 0) < 0)
                                        {
                                            cse4589_print_and_log("[%s:ERROR]\n", "UNBLOCK");
                                        }
                                        else
                                        {
                                            cse4589_print_and_log("[%s:SUCCESS]\n", "UNBLOCK");
                                            // Log success if needed
                                        }

                                        // Free the formatted message memory
                                        free(formatted_msg);
                                        // IP address is valid, proceed with the block operation...
                                        // Implement your logic for blocking the client IP here
                                    }
                                }
                                else
                                {
                                    cse4589_print_and_log("[%s:ERROR]\n", "UNBLOCK");
                                    cse4589_print_and_log("[%s:END]\n", "UNBLOCK");
                                }
                            }
                            else
                            {
                                cse4589_print_and_log("[%s:ERROR]\n", "UNBLOCK");
                                cse4589_print_and_log("[%s:END]\n", "UNBLOCK");
                            }

                            // Free the allocated memory for the command copy
                            free(cmd_copy);
                        }

                        // STAGE 2 - Implement LOGOUT here...
                        if ((strncmp(cmd, "LOGOUT", strlen("LOGOUT")) == 0) && logged_in == 1)
                        {

                            char *outgoing_msg = (char *)malloc(sizeof(char) * BUFFER_SIZE);
                            memset(outgoing_msg, '\0', BUFFER_SIZE);
                            if (outgoing_msg == NULL)
                            {
                                perror("Creating message for the server failed...");
                            }

                            strcpy(outgoing_msg, cmd);
                            if (send(client_socket, outgoing_msg, sizeof(outgoing_msg), 0) != sizeof(outgoing_msg))
                            {
                                cse4589_print_and_log("[%s:ERROR]\n", cmd);
                                free(cmd);
                                break;
                            }

                            free(outgoing_msg);

                            logged_in = 0;
                            // close(client_socket);
                            // FD_CLR(client_socket, &master_list);
                            // FD_CLR(STDIN, &master_list);
                            cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
                        }

                        // STAGE 1 & 2 - Process EXIT here...
                        if ((strncmp(cmd, "EXIT", strlen("EXIT")) == 0))
                        {

                            char *outgoing_msg = (char *)malloc(sizeof(char) * BUFFER_SIZE);
                            memset(outgoing_msg, '\0', BUFFER_SIZE);
                            if (outgoing_msg == NULL)
                            {
                                perror("Creating message for the server failed...");
                            }

                            strcpy(outgoing_msg, cmd);
                            if (send(client_socket, outgoing_msg, sizeof(outgoing_msg), 0) != sizeof(outgoing_msg))
                            {
                                perror("Unable to clear users list");
                                cse4589_print_and_log("[%s:ERROR]\n", cmd);
                                free(cmd);
                                break;
                            }

                            free(outgoing_msg);

                            close(client_socket);
                            // FD_CLR(client_socket, &master_list);
                            // FD_CLR(STDIN, &master_list);
                            cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
                            exit(0);
                        }

                        free(cmd);
                    }
                    else
                    {

                        char command_received[CMD_SIZE];

                        char *buffer = (char *)malloc(BUFFER_SIZE * (sizeof(char)));

                        if (recv(sock_index, buffer, (BUFFER_SIZE * sizeof(char)), 0) == (BUFFER_SIZE * sizeof(char)))
                        {
                            // printf("client sent this buffer: %s\n",buffer);
                            // const char *data, char *command, int *count, struct user *users[]

                            // STAGE 2 We first inspect the buffer to check if another client sent us message through server or to load the backlog message
                            
                            if (buffer[0] == 'S')
                            {
                                // Confirm it starts with "SEND"
                                if (strncmp(buffer, "SEND", 4) == 0)
                                {
                                    const char delimiter[2] = {30, 0}; // Record separator followed by NULL terminator

                                    // Skip past "SEND"
                                    char *startOfData = buffer + 4;

                                    // Use strtok to find the IP, then the message
                                    char *token = strtok(startOfData, delimiter);
                                    char *s_ip = NULL;
                                    char *msg_to_send = NULL;

                                    if (token != NULL)
                                    {
                                        s_ip = token;             // First token should be the IP
                                        token = strtok(NULL, ""); // Get the rest as the message
                                        if (token != NULL)
                                        {
                                            msg_to_send = token; // Remaining part is the message
                                        }
                                    }

                                    // Verification and action based on extracted data
                                    if (s_ip && msg_to_send)
                                    {
                                        printf("Extracted IP: %s\n", s_ip);
                                        printf("Extracted Message: %s\n", msg_to_send);
                                        cse4589_print_and_log(("msg from:%s\n[msg]:%s\n", s_ip, msg_to_send));
                                    }
                                    else
                                    {
                                        // Error handling: either IP or message couldn't be extracted
                                        printf("Failed to parse IP or message.\n");
                                    }
                                }
                                else
                                {
                                    // Error handling: buffer does not start with "SEND"
                                    printf("Buffer does not start with SEND.\n");
                                }
                            }
                            else if (buffer[0] == 'B')
                            {
                                printf("Ready to parse_backlog_string, input buffer is %s\n ",buffer);
                                parse_backlog_string(buffer);
                            }
                            else
                            {

                                user_count = parse_user_data(buffer, command_received, users);

                                // Process the server's response here...
                                if (strncmp(command_received, "LOGIN", sizeof("LOGIN")) == 0)
                                {
                                    cse4589_print_and_log("[%s:SUCCESS]\n", "LOGIN");
                                    cse4589_print_and_log("[%s:END]\n", "LOGIN");
                                }
                                else if (strncmp(command_received, "REFRESH", sizeof("REFRESH")) == 0)
                                {
                                    cse4589_print_and_log("[%s:SUCCESS]\n", "REFRESH");
                                    cse4589_print_and_log("[%s:END]\n", "REFRESH");
                                }
                            }
                        }
                        fflush(stdout);
                    }
                }
                fflush(stdout);
            }
        }
    }
    return EXIT_SUCCESS;
}

int connect_to_host(int fdsocket, char *server_ip, char *server_port)
{
    int len;
    struct sockaddr_in remote_server_addr;
    memset(&remote_server_addr, 0, sizeof(remote_server_addr));
    remote_server_addr.sin_family = AF_INET;
    inet_pton(AF_INET, server_ip, &remote_server_addr.sin_addr);
    remote_server_addr.sin_port = htons(atoi(server_port));

    if (connect(fdsocket, (struct sockaddr *)&remote_server_addr, sizeof(remote_server_addr)) < 0)
    {
        printf("tried to connect using fdsocket number %d but couldn't:\n", fdsocket);
        return -1;
    }

    return fdsocket;
}

// Function to check if a string consists of valid ASCII characters
bool isValidAscii(const char *str)
{
    while (*str)
    {
        if ((unsigned char)*str > 127)
        {
            return false; // Non-ASCII character found
        }
        str++;
    }
    return true; // All characters are valid ASCII
}
