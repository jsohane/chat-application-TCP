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

const char cmd[] = "PORT";

void print_port(int fd) {
	struct sockaddr_in portvalue;
    socklen_t len = sizeof(portvalue);
    if (getsockname(fd, (struct sockaddr *)&portvalue, &len) == -1)
    {
        cse4589_print_and_log("[PORT:ERROR]\n");
    }
    else
    {
        cse4589_print_and_log("[PORT:SUCCESS]\n");
        cse4589_print_and_log("PORT:%d\n", ntohs(portvalue.sin_port));
        cse4589_print_and_log("[PORT:END]\n");
    }
}