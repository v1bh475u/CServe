#include "network.h"

int open_clientfd(char *hostname, char *port)
{
    struct addrinfo hints, *listp, *p;
    int clientfd;
    memset(&hints, 0, sizeof(SA));
    // TCP
    hints.ai_socktype = SOCK_STREAM;
    // Port number should be used
    hints.ai_flags = AI_NUMERICSERV;
    // return socket addresses that have a particular config(IPv4 only if localhost configured for IPv4)
    hints.ai_flags |= AI_ADDRCONFIG;
    getaddrinfo(hostname, port, &hints, &listp);
    for (p = listp; p; p = p->ai_next)
    {
        // Open a socket
        if ((clientfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;
        // make a request for connection to the address
        if (connect(clientfd, p->ai_addr, p->ai_addrlen) != -1)
            break;
        close(clientfd); // If connect fails
    }
    freeaddrinfo(listp);
    if (!p)
        return -1;
    else
        return clientfd;
}

int open_listenfd(char *port)
{
    struct addrinfo hints, *listp, *p;
    int listenfd, optval = 1;

    memset(&hints, 0, sizeof(SA));
    // TCP
    hints.ai_socktype = SOCK_STREAM;
    // return socket addresses are that available for listening
    // and have a particular config(IPv4 only if localhost configured for IPv4)
    hints.ai_flags = AI_PASSIVE | AI_ADDRCONFIG;
    // Forces port to be port number
    hints.ai_flags |= AI_NUMERICSERV;
    // get list of available socket addresses
    getaddrinfo(NULL, port, &hints, &listp);
    for (p = listp; p; p = p->ai_next)
    {
        // open a socket
        if ((listenfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) < 0)
            continue;
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
        // Bind the descriptor to the address
        if (bind(listenfd, p->ai_addr, p->ai_addrlen) == 0)
            break;
        close(listenfd); // If bind fails
    }
    freeaddrinfo(listp);
    if (!p)
        return -1;
    // make it ready to accept connection requests
    if (listen(listenfd, LISTENQ) < 0)
    {
        close(listenfd);
        return -1;
    }
    return listenfd;
}