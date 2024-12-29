#include "server.h"

void handle_HTTP_transcation(int fd)
{
    int is_static;
    struct stat sbuf;
    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];
    char filename[MAXLINE], cgiargs[MAXLINE];
    rio_t rio;

    // Reading the request headers
    rio_readinitb(&rio, fd);
    rio_readlineb(&rio, buf, MAXLINE);
    printf("Request Headers:\n");
    printf("%s", buf);
    sscanf(buf, "%s %s %s", method, uri, version);
    if (strcasecmp(method, "GET"))
    {
        client_error(fd, method, "501", "Not implemnted", "CServe does not implement this method");
        return;
    }
    read_requesthdrs(&rio);

    // Parse URI from GET request
    is_static = parse_uri(uri, filename, cgiargs);
    if (stat(filename, &sbuf) < 0)
    {
        client_error(fd, filename, "404", "Not Found", "CServe couldn\'t find this file");
        return;
    }
    if (is_static)
    {
        // Serving static content
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IRUSR & sbuf.st_mode))
        {
            client_error(fd, filename, "403", "Forbidden", "CServe unable to read this file");
            return;
        }
        serve_static(fd, filename, sbuf.st_size);
    }
    else
    {
        // Serving dynamic content
        if (!(S_ISREG(sbuf.st_mode)) || !(S_IXUSR & sbuf.st_mode))
        {
            client_error(fd, filename, "403", "Forbidden", "CServe unable to execute this file");
            return;
        }
        serve_dynamic(fd, filename, cgiargs);
    }
}

void read_requesthdrs(rio_t *rp)
{
    char buf[MAXLINE];
    rio_readlineb(rp, buf, MAXLINE);
    while (strcmp(buf, "\r\n"))
    {
        rio_readlineb(rp, buf, MAXLINE);
        // Simply print the request headers
        printf("%s", buf);
    }
}

int parse_uri(char *uri, char *filename, char *cgiargs)
{
    char *ptr;
    int is_static = 0;
    if (!strstr(uri, "cgi-bin"))
    { // Static content
        strcpy(cgiargs, "");
        is_static = 1;
    }
    else
    { // Dynamic content
        ptr = index(uri, '?');
        if (ptr)
        {
            strcpy(cgiargs, ptr + 1);
            *ptr = '\0';
        }
        else
            strcpy(cgiargs, "");
    }
    strcpy(filename, ".");
    strcat(filename, uri);
    if (uri[strlen(uri) - 1] == '/')
        strcat(filename, "static/home.html");
    return is_static;
}

void serve_static(int fd, char *filename, int filesize)
{
    int srcfd, len = 0;
    char *srcp, filetype[MAXLINE], buf[MAXLINE];

    get_filetype(filename, filetype);
    len += snprintf(buf + len, MAXLINE - len, "HTTP/1.0 200 OK\r\n");
    len += snprintf(buf + len, MAXLINE - len, "Server: CServe Web Server\r\n");
    len += snprintf(buf + len, MAXLINE - len, "Connection: close\r\n");
    len += snprintf(buf + len, MAXLINE - len, "Content-length: %d\r\n", filesize);
    len += snprintf(buf + len, MAXLINE - len, "Content-type: %s\r\n\r\n", filetype);
    rio_writen(fd, buf, len);
    printf("Response headers:\n");
    printf("%s", buf);
    srcfd = open(filename, O_RDONLY, 0);
    srcp = mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    close(srcfd);
    if (rio_writen(fd, srcp, filesize) < 0)
    {
        perror("rio_writen");
    }
    munmap(srcp, filesize);
}

void get_filetype(char *filename, char *filetype)
{
    // get the last occurrence of '.' in the filename
    char *extension = strrchr(filename, '.');
    if (!extension)
        strcpy(filetype, "text/plain");
    if (strcmp(extension, ".html") == 0) // handles both .htm and .html
        strcpy(filetype, "text/html");
    else if (strcmp(extension, ".jpg") == 0)
        strcpy(filetype, "image/jpeg");
    else if (strcmp(extension, ".gif") == 0)
        strcpy(filetype, "image/gif");
    else if (strcmp(extension, ".png") == 0)
        strcpy(filetype, "image/png");
    else if (strcmp(extension, ".mpg") == 0)
        strcpy(filetype, "video/mpeg");
    else
        strcpy(filetype, "text/plain");
}

void serve_dynamic(int fd, char *filename, char *cgiargs)
{
    char buf[MAXLINE], *emptylist[] = {NULL};
    int len = 0;

    // Return first part of HTTP response
    len = sprintf(buf, "HTTP/1.0 200 OK\r\n");
    rio_writen(fd, buf, len);
    len = sprintf(buf, "Server: CServe Web Server\r\n");
    rio_writen(fd, buf, len);

    if (fork() == 0)
    {
        // Child process
        setenv("QUERY_STRING", cgiargs, 1);
        dup2(fd, STDOUT_FILENO);              // Redirect stdout to client
        execve(filename, emptylist, environ); // Run CGI program
    }
}

void client_error(int fd, char *cause, char *errnum, char *shortmsg, char *longmsg)
{
    char buf[MAXLINE], body[MAXLINE];
    int body_len = 0, buf_len = 0;

    // Making the response body
    body_len += snprintf(body + body_len, MAXLINE - body_len, "<html><title>Error</title>");
    body_len += snprintf(body + body_len, MAXLINE - body_len, "<body bgcolor=\"ffff55\">\r\n");
    body_len += snprintf(body + body_len, MAXLINE - body_len, "%s: %s\r\n", errnum, shortmsg);
    body_len += snprintf(body + body_len, MAXLINE - body_len, "<p>%s: %s\r\n", longmsg, cause);
    body_len += snprintf(body + body_len, MAXLINE - body_len, "<hr><em>The CServe Server</em>\r\n</body>");

    // Print the response
    buf_len += snprintf(buf, MAXLINE, "HTTP/1.0 %s %s\r\n", errnum, shortmsg);
    rio_writen(fd, buf, buf_len);
    buf_len = 0;
    buf_len += snprintf(buf, MAXLINE, "Content-type: text/html\r\n");
    rio_writen(fd, buf, buf_len);
    buf_len = 0;
    buf_len += snprintf(buf, MAXLINE, "Content-length: %d\r\n\r\n", body_len);
    rio_writen(fd, buf, buf_len);
    rio_writen(fd, body, body_len);
}
static void sigchld_handler(int sig)
{
    while (waitpid(-1, 0, WNOHANG) > 0)
        ;
    return;
}

static void sigpipe_handler(int sig)
{
    perror("sigpipe_handler");
    return;
}
void init_signal_handlers()
{
    signal(SIGCHLD, sigchld_handler);
    signal(SIGPIPE, sigpipe_handler);
}