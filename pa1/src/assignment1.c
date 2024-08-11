/**
 * @assignment1
 * @author  Team Members <ubitname@buffalo.edu>
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details at
 * http://www.gnu.org/copyleft/gpl.html
 *
 * @section DESCRIPTION
 *
 * This contains the main function. Add further description here....
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include "../include/global.h"
#include "../include/logger.h"

/**
 * main function
 *
 * @param  argc Number of arguments
 * @param  argv The argument list
 * @return 0 EXIT_SUCCESS
 */

int server(const char *port);
int client(const char *port);

int main(int argc, char *argv[])
{
	/*Init. Logger*/
	cse4589_init_log(argv[2]);

	/*Clear LOGFILE*/
	fclose(fopen(LOGFILE, "w"));

	/*Start Here*/
	if (argc != 3) {
		perror("Usage ./assignment1 <mode> <port_number>");
		exit(EXIT_FAILURE);	
	}

	/* Executing the program to run in the mode we want (client or server)
 	
 	* First we need to check if the correct mode is passed by the user:
 	* 	1. Check the length of the string. If not 1, exit with error.
 	* 	2. If 1 character string, exit with error for any other character than 's' or 'c'.

 	* Check for the port number:
 	* 	1. convert the string to integer using atoi()
 	* 	2. check to see if the port provided is valid (i.e, within acceptable range and not system reserved)

 	* Call the respective program using uncondition jump (switch statement)

 	*/

 	const char server_token[2] = "s";

	const char client_token[2] = "c";


	if ((strncmp(argv[1], "c", strlen(server_token)) != 0) && (strncmp(argv[1], server_token, strlen(server_token)) != 0)) {
		perror("Only acceptable modes are:\n'c' for client mode\n's' for server mode");
		exit(EXIT_FAILURE);
	}
	
	int  port = atoi(argv[2]);
	
	if ((port > 65353) || (port < 1024)) {
		perror("Port should be between 1024 and 65353");
		exit(EXIT_FAILURE);
	}
	

	switch (argv[1][0]) {
		case 's':
			if (server(argv[2]) != 0)
			{
				perror("Server could not exit properly\n");
			}
			break;	
		case 'c':
			if (client(argv[2])) {
				perror("Client could not exit properly!\n");
			}
			break;
	}
	
	return EXIT_SUCCESS;
}
