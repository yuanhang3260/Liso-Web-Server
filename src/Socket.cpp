#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include "Utility.h"
#include "Logger.h"
#include "Socket.h"


/**
 * @brief wrapper for open_listenfd
 * @param port the server port number
 * @return int socket discriptor
 */
int Socket::Open_ListenSocket(int port) 
{
    int re;

    if ((re = open_ListenSocket(port)) < 0) {
        Liso_error("Open_listenfd failed");
    }
    return re;
}


/** 
 * @brief close a socket
 * @param port the server port number
 * @return int
 */
int Socket::Close_Socket(int sock)
{
    if (close(sock)) {
        fprintf(stderr, "Failed closing socket.\n");
        return ERROR;
    }
    return 0;
}


/* 
 * @brief open and return a listening socket on port
 *  
 * @param port the server port number
 * @return int socket discriptor
 */
int Socket::open_ListenSocket(int port) 
{
    int listenfd, optval=1;
    struct sockaddr_in serveraddr;
  
    /* Create a socket descriptor */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Error: Create server socket failed\n");
        return -1;
    }

    /* Eliminates "Address already in use" error from bind. */
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, 
        (const void *)&optval , sizeof(int)) < 0) 
    {
        fprintf(stderr, "Error: set server socket failed\n");
        return -1;
    }

    /* Listenfd will be an endpoint for all requests to port
       on any IP address for this host */
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serveraddr.sin_port = htons((unsigned short)port); 
    if (bind(listenfd, (SA *)&serveraddr, sizeof(serveraddr)) < 0) 
    {
        fprintf(stderr, "Error: bind server socket to port %d failed\n", port);
        Close_Socket(listenfd);
        return -1;
    }

    /* Make it a listening socket ready to accept connection requests */
    if (listen(listenfd, LISTENQ) < 0)
    {
        fprintf(stderr, "Error: listen to port %d failed\n", port);
        Close_Socket(listenfd);
        return -1;
    }

    return listenfd;
}