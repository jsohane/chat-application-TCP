/**
 * @section REFERENCE
 *
 * Code was borrowed from Swetank Kumar Saha <swetank@buffalo.edu>'s beginner template.
 * Modifications have been made to suit the reuqirements of the assignment.
 *
 * @section DESCRIPTION
 *
 * This file contains the server init and main while loop for tha application.
 * Uses the select() API to multiplex between network I/O and STDIN.
 */
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

#define BACKLOG 5
#define STDIN 0
#define TRUE 1

/**
 * @section
 *
 * SERVER function:
 * This is the main function that drives the server application.
 * Initializes the list of all the users connected to server since it started.
 * Set up the listening socket.
 *
 * Roles:
 * Accepts connections
 * Process user (usually the server adminstrator) commands
 * Serves client requests
 *
 * @param  port Server's listening socket port number
 * @return 0 EXIT_SUCCESS
 */
int server(const char *port)
{
	// Creating an array of pointers to struct user and initializing them with NULLs
	struct user *users[MAX_USERS] = {NULL};
	size_t user_count = 0;

	// Write code for setting up the listening socket here
	int server_socket, head_socket, selret, sock_index, fdaccept = 0;
	socklen_t caddr_len;
	struct sockaddr_in client_addr;
	struct addrinfo hints, *res;
	fd_set master_list, watch_list;

	/* Set up hints structure */
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	/* Fill up address structures */
	if (getaddrinfo(NULL, port, &hints, &res) != 0)
		perror("getaddrinfo failed");

	/* Socket */
	server_socket = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if (server_socket < 0)
		perror("Cannot create socket");

	int reusePort = 1;
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &reusePort, sizeof(reusePort));

	/* Bind */
	if (bind(server_socket, res->ai_addr, res->ai_addrlen) < 0)
		perror("Bind failed");

	freeaddrinfo(res);

	/* Listen */
	if (listen(server_socket, BACKLOG) < 0)
		perror("Unable to listen on port");

	/* ---------------------------------------------------------------------------- */

	/* Zero select FD sets */
	FD_ZERO(&master_list);
	FD_ZERO(&watch_list);

	/* Register the listening socket */
	FD_SET(server_socket, &master_list);
	/* Register STDIN */
	FD_SET(STDIN, &master_list);

	head_socket = server_socket;

	// Setting a lock to deal with prompt appearing correctly
	// int lock = 1;

	// Keeeps track of new connections made to the server
	size_t incoming_count = 0;

	while (TRUE)
	{
		memcpy(&watch_list, &master_list, sizeof(master_list));

		// Printing the prompt only when lock is set
		// if (lock == 1)
		// {
		// 	printf("\n[Server@PA1]$ ");
		// 	fflush(stdout);
		// }

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
						{ // Mind the newline character that will be written to cmd
							exit(-1);
						}

						/* replacing newline character with null terminator */
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

						// Stage 1 - Implementing AUTHOR here...
						if (strncmp(cmd, "AUTHOR", strlen("AUTHOR")) == 0)
						{
							author();
						}

						// Stage 1 - Implementing IP here...
						if (strncmp(cmd, "IP", strlen("IP")) == 0)
						{
							get_ip();
						}

						// Stage 1 - Implementing LIST here...
						if (strncmp(cmd, "LIST", strlen("LIST")) == 0)
						{
							// printf("user_count before listing... %d\n", user_count);
							// for (size_t i = 0; i < MAX_USERS; i++){
							// 	if (users[i] != NULL){
							// 		printf("user found!\n");
							// 	}
							// 	else{
							// 		break;
							// 	}
							// }
							list_users(users, user_count);
							// printf("list in server finished\n");
						}

						// Stage 1 - Implementing PORT here...
						if (strncmp(cmd, "PORT", strlen("PORT")) == 0)
						{
							print_port(server_socket);
						}

						// Stage 1 & 2 - Implementing EXIT here...
						if (strncmp(cmd, "EXIT", strlen("EXIT")) == 0)
						{
							// printf("Shutting down the server...\n");
							if (clear_users(users, user_count, MAX_USERS) == 0)
							{
								// perror("Unable to clear users list\n");
								cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
								cse4589_print_and_log("[%s:END]\n", cmd);
								close(server_socket);
								return EXIT_SUCCESS;
							}
						}

						// Stage 2 - Implementing BLOCKED here...
						if (cmd[0] == 'B')
						{
						}

						free(cmd);
					}
					else if (sock_index == server_socket)
					{
						printf("New request incoming\n");
						caddr_len = sizeof(struct sockaddr);
						fdaccept = accept(server_socket, (struct sockaddr *)&client_addr, &caddr_len);
						if (fdaccept < 0)
							perror("Accept failed");

						// Extracting the IP address and host name of the client from client_addr
						char client_ip[INET_ADDRSTRLEN];
						if (inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN) == 0)
						{
							perror("Could not extract client IP");
						}

						// Extracting the client's port from client_addr
						unsigned short client_port;
						if ((client_port = ntohs(client_addr.sin_port)) == 0)
						{
							perror("Could not extract client port");
						}

						// Getting the host name for the client's IP address
						char hostname[NI_MAXHOST];
						if (getnameinfo((const struct sockaddr *)&client_addr, sizeof(client_addr), hostname, NI_MAXHOST, NULL, 0, NI_NAMEREQD) != 0)
						{
							perror("Could not resolve the host name from client's IP address");
						}
						int location;
						if ((location = old_user(users, client_ip, client_port, hostname)) != -1)
						{
							perror("New connection already exists in the users list.");
							break;
						}
						else
						{
							/* Filling in the details of the new connection to the users array */
							for (int i = 0; i < MAX_USERS; i++)
							{
								if (users[i] == NULL)
								{
									users[i] = malloc(sizeof(struct user));
									if (users[i] == NULL)
									{
										perror("allocating memory for user object failed");
										break;
									}
									users[i]->fd = fdaccept;
									strncpy(users[i]->ip, client_ip, INET_ADDRSTRLEN);
									users[i]->port = client_port;
									users[i]->status = ON;
									strncpy(users[i]->hostname, hostname, NI_MAXHOST);
									users[i]->msg_sent = 0;
									users[i]->msg_recv = 0;
									printf("here\n");
									users[i]->backlog_count = 0;
									// users[i]->backlog_list = malloc(sizeof(struct backlog_msg *) * BACKLOG_BUFFER_SIZE);
									// if (users[i]->backlog_list == NULL)
									// {
									//	perror("Allocating memeory for backlog_list failed");
									// }
									printf("Before looping backlog list\n");
									for (size_t index = 0; index < BACKLOG_BUFFER_SIZE; index++)
									{
										users[i]->backlog_list[index] = NULL;
									}
									printf("After looping backlog list\n");
									for (int j = 0; j < MAX_LIVE_USERS; j++)
									{
										users[i]->blocked_list[j] = NULL;
									}
									user_count++;
									break;
								}
							}
						}

						/* creating a response to send to the new user*/
						// printf("Sending the list of currently logged in users to the user...\n");
						char *buffer = (char *)malloc(BUFFER_SIZE * (sizeof(char)));
						memset(buffer, '\0', BUFFER_SIZE);

						buffer = generate_data_string("LOGIN", user_count, users);

						// printf("created string before sending is\n%s\n",buffer);
						// memcpy(buffer, response, sizeof(struct server_msg));

						/* send() the response to the new client */
						if (send(fdaccept, (buffer), BUFFER_SIZE, 0) == BUFFER_SIZE)
						{
							// printf("size of response sent is: %d\n", sizeof(response));
							// printf("Done!\n");
						}

						// If it is a returning user, send backlog messages
						// if (location != -1)
						// {
						// 	char *backlog_buffer =  (char*)malloc(BACKLOG_BUFFER_SIZE * (sizeof(char)));
						// 	memset(backlog_buffer,'\0',BACKLOG_BUFFER_SIZE);

						// 	backlog_buffer = generate_backlog_string("BACKLOG", user_count, users[location]);

						// 	/* send() the backlog messages to the returning client */
						// 	if(send(fdaccept, (backlog_buffer), BACKLOG_BUFFER_SIZE,0) == BACKLOG_BUFFER_SIZE)
						// 	{
						// 		printf("Backlog messages sent to the returning user\n");
						// 		size_t count = 0;
						// 		if ((count = clear_blist(users[location])) != users[location]->backlog_count)
						// 		{
						// 			perror("Unable to clear backlog messages after sending them to newly logged in client");
						// 		} else
						// 		{
						// 			users[location]->backlog_count = 0;
						// 		}
						// 	} else
						// 	{
						// 		perror("Unable to send the backlog messages to newly logged in client");
						// 	}
						// 	free(backlog_buffer);
						// }

						/* Add to watched socket list */
						FD_SET(fdaccept, &master_list);
						if (fdaccept > head_socket)
							head_socket = fdaccept;

						// lock = 0;

						// NEED TO FIGURE OUT WHY THIS BUFFER GIVING INVALID PONTER ERROR.
						free(buffer);
					}
					else
					{
						// Extract the information of the client socket which raised the select call...

						/* Initialize buffer to receieve response from one of the existing clients */

						char *incoming_cmd_buffer = (char *)malloc(sizeof(char) * BUFFER_SIZE);
						memset(incoming_cmd_buffer, '\0', BUFFER_SIZE);

						if (recv(sock_index, incoming_cmd_buffer, BUFFER_SIZE, 0) <= 0)
						{
							// printf("Remote Host terminated connection!\n");
							close(sock_index);

							/* Remove from watched list */
							FD_CLR(sock_index, &master_list);
							// lock = 0;
						}
						else
						{
							/* Process incoming buffers from existing clients here ... */

							// Stage 1 - "REFRESH" command
							if (strncmp(incoming_cmd_buffer, "REFRESH", sizeof("REFRESH")) == 0)
							{
								// printf("Received refresh command\n");
								// printf("One of the user is requesting the current user list.\n");

								/* creating a response to send to the new user*/
								// printf("Sending the list of currently logged in users to the user...\n");

								char *data_string = generate_data_string("REFRESH", user_count, users);

								// strncpy(buffer, data_string, MAX_USERS);

								// printf("created string before sending REFRESH LIST  is\n%s\n",data_string);
								// memcpy(buffer, response, sizeof(struct server_msg));

								/* send() the response to the new client */
								if (send(sock_index, data_string, BUFFER_SIZE, 0) == BUFFER_SIZE)
								{
									// printf("size of response sent is: %d\n", sizeof(response));
									// printf("Refresh list sent!\n");
								}

								// free(buffer);
								// Do we need to do this????
								free(data_string);
							}

							// Stage 2 - "LOGOUT" command
							else if (strncmp(incoming_cmd_buffer, "LOGOUT", sizeof("LOGOUT")) == 0)
							{
								// Call function to remove the user from the user_list
								if (logout_user(users, sock_index) == 0)
								{
									perror("Unable to logout user from the array");
								}

								/* Update count of active users and free the socket*/
								// user_count--;
								//  close(sock_index);

								/* Remove from watched list */
								// FD_CLR(sock_index, &master_list);
							}

							// Stage 1 & 2 - "EXIT" command
							else if (strncmp(incoming_cmd_buffer, "EXIT", sizeof("EXIT")) == 0)
							{
								// printf("Received command: %s\n",incoming_cmd_buffer);
								// Extracting the IP address and host name of the client from client_addr
								// char* client_ip = malloc(sizeof(char) * INET_ADDRSTRLEN);
								// if (client_ip == NULL)
								// {
								// 	perror("Could not allocate memory for client's IP address");
								// }

								// memset(client_ip, '\0', (sizeof(char) * INET_ADDRSTRLEN));

								// if (inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN) == 0) {
								// perror("Could not extract client IP");								}

								// // Extracting the client's port from client_addr
								// // printf("Extracted Ip of client requesting exit is: %s\n",client_ip);
								// unsigned short client_port;
								// if (ntohs(client_addr.sin_port) == 0) {
								// 	perror("Could not extract client port");
								// }

								// // Getting the host name for the client's IP address
								// char* hostname = malloc(sizeof(char) * NI_MAXHOST);
								// if (hostname == NULL)
								// {
								// 	perror("Could not allocate memory for client's Hostname");
								// }

								// memset(hostname, '\0', (sizeof(char) * NI_MAXHOST));
								// if (getnameinfo((const struct sockaddr *)&client_addr, sizeof(client_addr), hostname, NI_MAXHOST, NULL, 0, NI_NAMEREQD) != 0) {
								// 	perror("Could not resolve the host name from client's IP address");
								// }

								// Call function to remove the user from the user_list
								if (remove_user(users, &user_count, sock_index) == 0)
								{
									perror("Unable to remove user from the array");
								}

								/* Update count of active users and free the socket*/
								// user_count--;
								close(sock_index);
								// free(client_ip);
								// free(hostname);
								// printf("Remote Host terminated connection!\n");

								/* Remove from watched list */
								FD_CLR(sock_index, &master_list);
							}

							// Stage 2 - "BLOCK" command
							else if (incoming_cmd_buffer[0] == 'B')
							{
								size_t location = -1;
								// Getting the location of user in users
								for (size_t i = 0; i < MAX_USERS; i++)
								{
									if (users[i] != NULL && users[i]->fd == sock_index)
									{
										location = i;
										break;
									}
								}

								if (location == -1)
								{
									perror("Unable to locate the client requesting BLOCK in users list");
								}
								else
								{
									int token_c = 0;
									char delimiter[] = {30, '\0'};
									char *token_p = strtok(incoming_cmd_buffer, delimiter);

									char *command = malloc(sizeof("BLOCK"));
									if (command == NULL)
									{
										perror("Couldn't allocate memeory for string storing literal BLOCK");
									}

									char *ip = malloc(sizeof(char) * INET_ADDRSTRLEN);
									if (ip == NULL)
									{
										perror("Couldn't allocate memeory for string storing the ip to be BLOCKed");
									}

									memset(command, '\0', sizeof("BLOCK"));
									memset(ip, '\0', sizeof(char) * INET_ADDRSTRLEN);

									while (token_p != NULL)
									{
										token_c++;
										if (token_c == 1)
										{
											strncpy(command, token_p, strlen("BLOCK"));
										}

										if (token_c == 2)
										{
											strcpy(ip, token_p);
										}
										token_p = strtok(NULL, delimiter);
									}

									if (token_c != 2)
									{
										perror("Incorrect number of tokens sent for BLOCK");
									}

									if (block(ip, users[location]) != 0)
									{
										perror("Unable to block IP: User's blocked list is full");
									}

									free(command);
									free(ip);
								}
							}

							// Stage 2 - "UNBLOCK" command
							else if (incoming_cmd_buffer[0] == 'U')
							{
								size_t location = MAX_USERS;
								// Getting the location of user in users
								for (size_t i = 0; i < MAX_USERS; i++)
								{
									if (users[i]->fd == sock_index)
									{
										location = i;
										break;
									}
								}

								if (location == MAX_USERS)
								{
									perror("Unable to locate the client requesting UNBLOCK in users list");
								}

								int token_c = 0;
								char delimiter[] = {30, '\0'};
								char *token_p = strtok(incoming_cmd_buffer, delimiter);

								char *command = malloc(sizeof("BLOCK"));
								if (command == NULL)
								{
									perror("Couldn't allocate memeory for string storing literal BLOCK");
								}

								char *ip = malloc(sizeof(char) * INET_ADDRSTRLEN);
								if (command == NULL)
								{
									perror("Couldn't allocate memeory for string storing the ip to be BLOCKed");
								}

								memset(command, '\0', sizeof("BLOCK"));
								memset(ip, '\0', sizeof(char) * INET_ADDRSTRLEN);

								while (token_p != NULL)
								{
									token_c++;
									if (token_c == 1)
									{
										strncpy(command, token_p, sizeof("BLOCK"));
									}

									if (token_c == 2)
									{
										strcpy(ip, token_p);
									}
									token_p = strtok(NULL, delimiter);
								}

								if (token_c != 2)
								{
									perror("Incorrect number of tokens sent for BLOCK\n");
								}

								if (unblock(ip, users[location]) != 0)
								{
									perror("Unable to unblock IP: Unknown reason");
								}

								free(command);
								free(ip);
							}

							// Stage 2 - "SEND" command
							else if (incoming_cmd_buffer[0] == 'S')
							{
								printf("The client sent us the following buffer:\n%s\n", incoming_cmd_buffer);
								size_t send_loc = -1;

								char *send_ip = malloc(sizeof(char) * INET_ADDRSTRLEN);
								if (send_ip == NULL)
								{
									perror("Couldn't allocate memeory for string storing the sender's IP address in SEND command");
								}
								memset(send_ip, '\0', sizeof(char) * INET_ADDRSTRLEN);

								// Getting the index and IP address of sender in users
								for (size_t i = 0; i < MAX_USERS; i++)
								{
									if (users[i] != NULL && users[i]->fd == sock_index && users[i]->status == ON)
									{
										send_loc = i;
										strncpy(send_ip, users[i]->ip, INET_ADDRSTRLEN);
										break;
									}
								}

								if (send_loc == -1)
								{
									perror("Unable to locate the client SENDing message in users list");
								}

								printf("Got the location of sender in the USERs list %d\n", send_loc);

								int token_c = 0;
								char delimiter[] = {30, '\0'};
								char *token_p = strtok(incoming_cmd_buffer, delimiter);

								char *command = malloc(sizeof("SEND"));
								if (command == NULL)
								{
									perror("Couldn't allocate memeory for string storing literal SEND");
								}

								char *rec_ip = malloc(sizeof(char) * INET_ADDRSTRLEN);
								if (rec_ip == NULL)
								{
									perror("Couldn't allocate memeory for string receiver's IP address in SEND command");
								}

								char *message = malloc(sizeof(char) * MSG_SIZE);
								if (message == NULL)
								{
									perror("Couldn't allocate memeory for message string in SEND command");
								}

								memset(command, '\0', sizeof("SEND"));
								memset(rec_ip, '\0', sizeof(char) * INET_ADDRSTRLEN);
								memset(message, '\0', sizeof(char) * MSG_SIZE);

								while (token_p != NULL)
								{
									token_c++;
									if (token_c == 1)
									{
										strncpy(command, "SEND", strlen("SEND"));
									}

									if (token_c == 2)
									{
										strncpy(rec_ip, token_p, INET_ADDRSTRLEN);
									}

									if (token_c == 3)
									{
										strncpy(message, token_p, MSG_SIZE);
									}
									token_p = strtok(NULL, delimiter);
								}

								if (token_c != 3)
								{
									perror("Incorrect number of tokens sent for SEND");
								}

								// printf("The receiver's IP address is %s.\nIt will be sent the following message:\n\"%s\".\n", rec_ip, message);

								size_t rec_loc = -1;
								// Getting the receiver's location of user in users
								for (size_t i = 0; i < MAX_USERS; i++)
								{
									if (users[i] != NULL && strncmp(users[i]->ip, rec_ip, INET_ADDRSTRLEN) == 0)
									{
										rec_loc = i;
										break;
									}
								}

								if (rec_loc == -1)
								{
									perror("Unable to find the receiver in USERs list.");
								}

								// printf("Hello after extracting receiver's location %d\n", rec_loc);

								if (rec_loc != -1 && rec_loc < MAX_USERS)
								{
									if (isblocked(send_ip, users[rec_loc]))
									{
										users[send_loc]->msg_sent++;
									}
									else
									{
										printf("rec_loc is %d\n", rec_loc);
										if (users[rec_loc]->status)
										{
											printf("receiver has not blocked the sender...\n");
											char *message_buffer = generate_msg_string(command, send_ip, message);
											/* send() the message to the receiver */
											if (send(users[rec_loc]->fd, message_buffer, BUFFER_SIZE, 0) != BUFFER_SIZE)
											{
												if (add_msg_blist(send_ip, message, users[rec_loc]) != 0)
												{
													perror("Unable to add message to offline receiver's backlog list in SEND");
												}
												users[send_loc]->msg_sent++;
												users[rec_loc]->msg_recv++;
											}
											else
											{
												users[send_loc]->msg_sent++;
												users[rec_loc]->msg_recv++;
											}
											free(message_buffer);
										}
										else
										{
											printf("before calling BList method\n");
											if (users[rec_loc] != NULL && add_msg_blist(send_ip, message, users[rec_loc]) != 0)
											{
												perror("Unable to add message to offline receiver's backlog list in SEND");
											}
											printf("After calling BList method\n");
											users[send_loc]->msg_sent++;
											users[rec_loc]->msg_recv++;
											printf("After increasing count\n");
										}
									}
								}
								free(send_ip);
								free(command);
								free(rec_ip);
								free(message);
							}

							// Stage 2 - "BROADCAST" command
							else if (incoming_cmd_buffer[0] == 'B')
							{
								size_t send_loc = -1;

								char *send_ip = malloc(sizeof(char) * INET_ADDRSTRLEN);
								if (send_ip == NULL)
								{
									perror("Couldn't allocate memeory for string storing the sender's IP address in BROADCAST command");
								}
								memset(send_ip, '\0', sizeof(char) * INET_ADDRSTRLEN);

								// Getting the index and IP address of sender in users
								for (size_t i = 0; i < MAX_USERS; i++)
								{
									if (users[i] != NULL && users[i]->fd == sock_index && users[i]->status == ON)
									{
										send_loc = i;
										strncpy(send_ip, users[i]->ip, INET_ADDRSTRLEN);
										break;
									}
								}

								if (send_loc == -1)
								{
									perror("Unable to locate the client BROADCASTing message in users list");
								}

								int token_c = 0;
								char delimiter[] = {30, '\0'};
								char *token_p = strtok(incoming_cmd_buffer, delimiter);

								char *command = malloc(sizeof("SEND"));
								if (command == NULL)
								{
									perror("Couldn't allocate memeory for string storing literal SEND");
								}

								char *message = malloc(sizeof(char) * MSG_SIZE);
								if (message == NULL)
								{
									perror("Couldn't allocate memeory for message string for BROADCAST command");
								}

								memset(command, '\0', sizeof("SEND"));
								memset(message, '\0', sizeof(char) * MSG_SIZE);

								while (token_p != NULL)
								{
									token_c++;
									if (token_c == 1)
									{
										strncpy(command, "SEND", strlen("SEND"));
									}

									if (token_c == 2)
									{
										strncpy(message, token_p, sizeof(char) * MSG_SIZE);
									}
									token_p = strtok(NULL, delimiter);
								}

								if (token_c != 2)
								{
									perror("Incorrect number of tokens sent for BROADCAST");
								}

								size_t rec_loc;

								for (size_t i = 0; i < MAX_USERS; i++)
								{
									if (users[i] != NULL && strncmp(users[i]->ip, send_ip, INET_ADDRSTRLEN) != 0)
									{
										rec_loc = i;
										if (isblocked(send_ip, users[rec_loc]))
										{
											users[send_loc]->msg_sent++;
										}
										else
										{
											if (users[rec_loc]->status)
											{
												char *message_buffer = generate_msg_string(command, send_ip, message);
												/* send() the message to the receiver */
												if (send(users[rec_loc]->fd, message_buffer, BUFFER_SIZE, 0) != BUFFER_SIZE)
												{
													if (add_msg_blist(send_ip, message, users[rec_loc]) != 0)
													{
														perror("Unable to add message to offline receiver's backlog list in BROADCAST");
													}
													users[send_loc]->msg_sent++;
													users[rec_loc]->msg_recv++;
												}
												else
												{
													users[send_loc]->msg_sent++;
													users[rec_loc]->msg_recv++;
												}

												free(message_buffer);
											}
											else
											{
												if (add_msg_blist(send_ip, message, users[rec_loc]) != 0)
												{
													perror("Unable to add message to offline receiver's backlog list in BROADCAST");
												}
												users[send_loc]->msg_sent++;
												users[rec_loc]->msg_recv++;
											}
										}
									}
								}

								free(send_ip);
								free(command);
								free(message);
							}

							// Stage 2 - "RELOGIN" command
							else if (strncmp(incoming_cmd_buffer, "RELOGIN", sizeof("RELOGIN")) == 0)
							{
								printf("Received a new RELOGIN request\n");
								caddr_len = sizeof(struct sockaddr);

								// Getting the client information out of the socket
								if (getpeername(sock_index, (struct sockaddr *)&client_addr, &caddr_len) != 0)
								{
									perror("Couldn't extract returning user's info from getpeername().");
									break;
								}

								// Extracting the IP address and host name of the client from client_addr
								char client_ip[INET_ADDRSTRLEN];
								if (inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN) == 0)
								{
									perror("Could not extract client IP");
								}

								// Extracting the client's port from client_addr
								unsigned short client_port;
								if ((client_port = ntohs(client_addr.sin_port)) == 0)
								{
									perror("Could not extract client port");
								}

								// Getting the host name for the client's IP address
								char hostname[NI_MAXHOST];
								if (getnameinfo((const struct sockaddr *)&client_addr, sizeof(client_addr), hostname, NI_MAXHOST, NULL, 0, NI_NAMEREQD) != 0)
								{
									perror("Could not resolve the host name from client's IP address");
								}

								// Extract the location of the user in users LIST.
								size_t location = -1;
								location = old_user(users, client_ip, client_port, hostname);

								// Send BACKLOG messages if any.
								if (location != -1)
								{
									if (users[location]->fd == sock_index)
									{
										printf("Yes, the returning user does indeed exist in the users list.\n");
									}
									// Turn the user's STATUS to ON.
									users[location]->status = ON;

									char *data_string = generate_data_string("LOGIN", user_count, users);
									if (send(sock_index, data_string, BUFFER_SIZE, 0) == BUFFER_SIZE)
									{
										printf("Refresh list sent to the returning user!\n");
									}

									free(data_string);

									// char *backlog_buffer = (char *)malloc(BUFFER_SIZE * (sizeof(char)));
									// memset(backlog_buffer, '\0', BUFFER_SIZE);

									char *backlog_buffer = generate_backlog_string("BACKLOG", users[location]);
									// printf("backlog_buffer generated after calling generate_backlog_string is %s\n",backlog_buffer);
									if (backlog_buffer != NULL)
									{
										/* send() the backlog messages to the returning client */
										if (send(sock_index, (backlog_buffer), BUFFER_SIZE, 0) == BUFFER_SIZE)
										{
											printf("Backlog messages sent to the returning user and the backlog_buffer is %s\n", backlog_buffer);
											size_t count = 0;
											if ((count = clear_blist(users[location])) != users[location]->backlog_count)
											{
												perror("Unable to clear backlog messages after sending them to newly logged in client");
											}
											else
											{
												users[location]->backlog_count = 0;
											}
										}
										else
										{
											perror("Unable to send the backlog messages to newly logged in client");
										}
										free(backlog_buffer);
									}
									else
									{
										// No backlog messages.
									}
								}
								else
								{
									perror("Unable to figure out returning user's location.");
									break;
								}
							}

							// lock = 0;
						}

						// printf("here in server line 362 before calling free:\n");
						free(incoming_cmd_buffer);
					}
				}
			}
		}
	}

	close(server_socket);
	return EXIT_SUCCESS;
}
