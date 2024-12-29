#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#pragma once

#define MAXLINE 8192
#define LISTENQ 1024

// Simplified server socket creation
// returns descriptor to listen socket if OK, -1 on failure
int open_listenfd(char *port);

// Simplified client socket creation
// returns descriptor to connection socket if OK, -1 on failure
int open_clientfd(char *hostname, char *port);

typedef struct sockaddr SA;