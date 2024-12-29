#include "rio.h"
#include "network.h"
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>

#pragma once

extern char **environ;

// handles a single HTTP transcation
void handle_HTTP_transcation(int fd);

// parses the requests headers
void read_requesthdrs(rio_t *rp);

// parses uri for arguments of cgi
int parse_uri(char *uri, char *filename, char *cgiargs);

// serves static content
void serve_static(int fd, char *filename, int filesize);

// returns the type of file
void get_filetype(char *filename, char *filetype);

// serves dynamic content
void serve_dynamic(int fd, char *filename, char *cgiargs);

// handles client error
void client_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
