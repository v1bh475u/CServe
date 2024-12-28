#include "server.h"

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    // To keep the code protocol-independent
    struct sockaddr_storage clientaddr;
    char client_hostname[MAXLINE], client_port[MAXLINE];

    if (argc != 2)
    {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    // open a listening socket
    listenfd = open_listenfd(argv[1]);
    while (1)
    {
        clientlen = sizeof(struct sockaddr_storage);
        // accept a connection request or hangs until 1 request arrives
        connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
        // resolve client_host and port from the client socket address
        getnameinfo((SA *)&clientaddr, clientlen, client_hostname, MAXLINE, client_port, MAXLINE, 0);
        printf("Connected to %s:%s\n", client_hostname, client_port);
        // Complete 1 HTTP transcation and close the connection
        handle_HTTP_transcation(listenfd);
        close(connfd);
    }
    exit(0);
}