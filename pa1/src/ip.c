#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "../include/logger.h"

void get_ip(void) {
	const char servip[INET_ADDRSTRLEN] = "8.8.8.8";
	const unsigned short servport = 53;
	const char cmd[] = "IP";

	// creating an IPv4 UDP socket
	int sockfd;
	if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) == -1) {
		// perror("Failed to create UDP socket");
	}

	// creating struct sockaddr for Destination UDP socket using its IP and Port number
	struct sockaddr_in servinfo;
	servinfo.sin_family = AF_INET;
	servinfo.sin_addr.s_addr = inet_addr(servip);
	servinfo.sin_port = htons(servport);

	// Firing off a IPv$ UDP packet
	if(connect(sockfd, (struct sockaddr *)&servinfo, sizeof(servinfo)) == -1) {
		// perror("connection failed");
	}

	// retreiving the outward facing struct addrinfo for sockfd
	struct sockaddr sourceinfo;
	socklen_t sourceinfo_len = sizeof(sourceinfo);
	if(getsockname(sockfd, &sourceinfo, &sourceinfo_len) == -1) {
		// perror("Retreiving address of socket failed");
	}

	// Extracting the IP address from socket's address structure
	char ip_addr[INET_ADDRSTRLEN];
	struct sockaddr_in *sourceinfo_ip = (struct sockaddr_in *)&sourceinfo;
	
	if (inet_ntop(AF_INET, &(sourceinfo_ip->sin_addr), ip_addr, sizeof(ip_addr)) == NULL) {
		cse4589_print_and_log("[%s:ERROR]\n", cmd);
	}

	cse4589_print_and_log("[%s:SUCCESS]\n", cmd);
	cse4589_print_and_log("IP:%s\n", ip_addr);
	cse4589_print_and_log("[%s:END]\n", cmd);
}
