#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <string.h>

/**
 * @section
 *
 * DECLARATIONS FOR CUSTOM DATA STRUCTURES
 * > enum login_status indicates whether a user is online or offline
 * > struct user stores the information about a connected client.  *   Contains additional fields to make it compatible with linux in-built data structures' APIs
 */
#ifndef MAX_LIVE_USERS
#define MAX_LIVE_USERS 5
#endif

#ifndef MAX_USERS
#define MAX_USERS 100
#endif

#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif

#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1000
#endif

#ifndef CMD_SIZE
#define CMD_SIZE 100
#endif

#ifndef MSG_SIZE
#define MSG_SIZE 256
#endif

#ifndef BACKLOG_BUFFER_SIZE
#define BACKLOG_BUFFER_SIZE 100
#endif

typedef enum
{
	OFF = 0,
	ON = 1
} login_status;

struct backlog_msg
{
	char sender_ip[INET_ADDRSTRLEN];
	char message[MSG_SIZE];
};

struct user
{
	int fd;
	char ip[INET_ADDRSTRLEN];
	unsigned short port;
	login_status status;
	char hostname[NI_MAXHOST];
	unsigned int msg_sent;
	unsigned int msg_recv;
	char *blocked_list[MAX_LIVE_USERS];
	struct backlog_msg *backlog_list[BACKLOG_BUFFER_SIZE];
	size_t backlog_count;
};

/* Utilitarian Functions for their respective commands */
void author(void);
void get_ip(void);
void print_port(int fd);

/*-----------------------------------------------------------LIST_USERS.C----------------------------------------------------------------------------*/
/* Used for LIST command */
void list_users(struct user **users, size_t count);

// int compare_users(const void *a, const void *b);

/* Adds a struct backlog_msg to receiver's backlog messages list */
int add_msg_blist(char *send_ip, char *message, struct user *receiver);

/* Clears the newly LOGGED in client's backlog messages list.
NOTE: Should only be called after successfully sending them to the client */
int clear_blist(struct user *client);

/* Checkes if the given CLIENT_IP, CLIENT_PORT and HOSTNAME correspond to a
previous user (not currently logged in) and returns the location of the client
in the USERS array. Returns -1 if the corresponding USER doesn't exist. */
int old_user(struct user **users, const char *client_ip, unsigned short client_port, const char *hostname);

/*-----------------------------------------------------------SHUTDOWN.C----------------------------------------------------------------------------*/
/* Used for Shutting down the app safely */
int clear_users(struct user **users, size_t user_count, size_t bufsize);

/*-----------------------------------------------------------SERIALIZE.C----------------------------------------------------------------------------*/
/* Used for serializing currently logged in users' details */
char *generate_data_string(const char *command, int count, struct user *users[]);

/* Used by client application to un-serialize the users' list */
size_t parse_user_data(const char *data, char *command, struct user *users[]);

/* Used to serialize backlog messages of a newly LOGIN client */
char *generate_backlog_string(const char *command, struct user *client);

/* Used to serialize SEND and BROADCAST messages */
char *generate_msg_string(char *command, char *send_ip, char *message);

/* Un-serialize buffer for BACKLOG messages */
void parse_backlog_string(char *data_string);

/*-----------------------------------------------------------REMOVE_USER.C----------------------------------------------------------------------------*/
/* Used for removing EXITed client */
int remove_user(struct user **users, size_t *user_count, int fd);

/* Used for client that has LOGOUT*/
int logout_user(struct user **users, const int fd);

/*-------------------------------------------------------------BLOCK.C--------------------------------------------------------------------------------*/
/* BLOCKs ip_address for client */
int block(const char *ip_address, struct user *client);

/* UNBLOCKs the ip_address for the client */
int unblock(const char *ip_address, struct user *client);

/* checks if the ip_address is blocked by the client or not. */
int isblocked(const char *ip_address, const struct user *client);
