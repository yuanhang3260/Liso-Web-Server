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
#include "selectEngine.h"

/** open server socket */
static int open_ListenSocket(int port);
/** add a client socket into pool */
static void add_client(int connfd, pool_t *pool);

/** counts total bytes received by server */
int byte_cnt = 0; 


/** @brief init a select pool.
 *
 *  @param listenfd server socket
 *  @param pool select pool
 */
void init_pool(int listenfd, pool_t *pool) 
{
    /* Initially, there are no connected descriptors */
    int i;
    pool->maxi = -1;
    for (i=0; i< FD_SETSIZE; i++) {
        pool->clientfd[i] = -1;
    }

    /* Initially, listenfd is only member of select read set */
    pool->maxfd = listenfd;
    FD_ZERO(&pool->read_set);
    FD_SET(listenfd, &pool->read_set);
}


/**
 * @brief wrapper for open_listenfd
 * @param port the server port number
 * @return int socket discriptor
 */
int Open_ListenSocket(int port) 
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
 * @return 
 */
int Close_Socket(int sock)
{
    if (close(sock)) {
        fprintf(stderr, "Failed closing socket.\n");
        return ERROR;
    }
    return 0;
}


/** @brief check server socket has received new client's connection.
 *
 *  @param listenfd server socket
 *  @pool select pool
 */
void check_server(int listenfd, pool_t *pool)
{
    socklen_t clientlen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    int connfd;

    if (FD_ISSET(listenfd, &pool->ready_set))
    {
        connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
        add_client(connfd, pool); /* adding client fd */
    }
}


/** @brief check select pool and process client reqeusts
 *
 *  @param pool select pool
 *  @return void
 */
void check_clients(pool_t *pool) 
{
    int i, connfd, nread, nwrite;
    char buf[BUF_SIZE]; 

    for (i = 0; (i <= pool->maxi) && (pool->nready > 0); i++)
    {
        connfd = pool->clientfd[i];

        /* If the descriptor is ready, echo a text line from it */
        if ((connfd > 0) && (FD_ISSET(connfd, &pool->ready_set))) 
        {
            pool->nready--;
            if ((nread = read(connfd, buf, BUF_SIZE)) != 0) 
            {
                byte_cnt += nread;
                printf("Server received %d bytes on fd %d\n", nread, connfd);
                
                nwrite = write(connfd, buf, nread);
                if (nwrite < 0) {
                    fprintf(stderr, "Error: write client socket error\n");
                }
            }
            else /* EOF detected, remove descriptor from pool */
            { 
                Close_Socket(connfd);
                FD_CLR(connfd, &pool->read_set);
                printf("remove client socket %d\n", connfd);
                pool->clientfd[i] = -1;
            }
        }
    }
}


/*-------------------------- static functions --------------------------------*/
/*  
 * @brief open and return a listening socket on port
 *  
 * @param port the server port number
 * @return int socket discriptor
 */
static int open_ListenSocket(int port) 
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


/** @brief add a client socket into select pool
 *
 *  @param connfd client socket
 *  @param pool select pool
 *  @return void
 */
static void add_client(int connfd, pool_t *pool) 
{
    int i;
    pool->nready--;
    for (i = 0; i < FD_SETSIZE; i++)  /* Find an available slot */
    {
        if (pool->clientfd[i] < 0) 
        {
            printf("adding client socket %d ...\n", connfd);
            /* Add connected descriptor to the pool */
            pool->clientfd[i] = connfd;

            /* Add the descriptor to descriptor set */
            FD_SET(connfd, &pool->read_set);

            /* Update max descriptor and pool highwater mark */
            if (connfd > pool->maxfd){
                pool->maxfd = connfd;
            }
            
            if (i > pool->maxi) {
                pool->maxi = i;
            }
            
            break;
        }
    }
    /* Can't find an empty slot */
    if (i == FD_SETSIZE) { /* TODO */
        fprintf(stderr, "Error: Too many clients\n");
    }
}
