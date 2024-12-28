#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>

#define MAXLINE 256
#define LISTENQ 1024

// Simplified server socket creation
// returns descriptor to listen socket if OK, -1 on failure
int open_listenfd(char *port);

// Simplified client socket creation
// returns descriptor to connection socket if OK, -1 on failure
int open_clientfd(char *hostname, char *port);

typedef struct sockaddr SA;