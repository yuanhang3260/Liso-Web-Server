#ifndef __SELECTENGINE_H_
#define __SELECTENGINE_H_

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define LISTENQ 10
#define BUF_SIZE 4096

typedef struct sockaddr SA;


/*----------------------------------------------------------------------------*/
/** @brief A pool of connected descriptors */
typedef struct SelectPool
{
    /** largest descriptor in read_set */
    int maxfd;
    /** set of all active descriptors */      
    fd_set read_set;
    /** subset of descriptors ready for reading */
    fd_set ready_set;
    /** number of ready descriptors from select */
    int nready;
    /** highwater index into client array */
    int maxi;
    /** set of active descriptors */
    int clientfd[FD_SETSIZE];
} pool_t;


/*----------------------------------------------------------------------------*/
/** init Select Pool */
void init_pool(int listenfd, pool_t *pool);

/** check server socket has received new client's connection */
void check_server(int listenfd, pool_t *pool);

/** check client */
void check_clients(pool_t *pool);


/*------------------------- wrapper functions --------------------------------*/
/** open and return a listening socket on port */
int Open_ListenSocket(int port);

/** close a socket */
int Close_Socket(int sock);



#endif /* __SELECTENGINE_H */
