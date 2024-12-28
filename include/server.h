#include "network.h"
#include <stdlib.h>
#include "rio.h"
#include <sys/stat.h>
#include <strings.h>

// handles a single HTTP transcation
void handle_HTTP_transcation(int fd);

// parses the requests headers
void read_requesthdrs(char *bp);

// parses uri for arguments of cgi
int parse_uri(char *uri, char *filename, char *cgiargs);

// serves static content
void serve_static(int fd, char *filename, int filesize);

// returns the type of file
void get_filetype(char *filename, char *filetype);

// serves dynamic content
void serve_dynamic(int fd, char *filename, char *cgiargs);

// handles client error
void clienterror(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg);
