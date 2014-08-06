/* 
 * @file sample_server.c - A concurrent echo server based on select.
 *
 * @author Hang Yuan (Andrew ID: hangyuan)
 * @bug no bugs found yet
 */
#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include "Utility.h"
#include "selectEngine.h"

#define ECHO_PORT 9090
#define BUF_SIZE 4096


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


/** init Select Pool */
void init_pool(int listenfd, pool_t *p);
/** add a client socket into select pool */
void add_client(int connfd, pool_t *p);
/** check client */
void check_clients(pool_t *p);

/** counts total bytes received by server */
int byte_cnt = 0; 

int main(int argc, char **argv)
{
    int listenfd, connfd, port; 
    socklen_t clientlen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    pool_t pool; 

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    port = atoi(argv[1]);

    listenfd = Open_ListenSocket(port);
    init_pool(listenfd, &pool);

    while (1) 
    {
        /* Wait for listening/connected descriptor(s) to become ready */
        pool.ready_set = pool.read_set;
        pool.nready = select(pool.maxfd+1, &pool.ready_set, NULL, NULL, NULL);

        /* If listening descriptor ready, add new client to pool */
        if (FD_ISSET(listenfd, &pool.ready_set)) 
        {
            connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
            add_client(connfd, &pool);
        }
        
        /* Echo a text line from each ready connected descriptor */ 
        check_clients(&pool);
    }
}


/** @brief init a select pool.
 *
 *  @param listenfd server socket
 *  @param pool select pool
 *  @return void
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


/** @brief add a client socket into select pool
 *
 *  @param connfd client socket
 *  @param pool select pool
 *  @return void
 */
void add_client(int connfd, pool_t *pool) 
{
    int i;
    pool->nready--;
    for (i = 0; i < FD_SETSIZE; i++)  /* Find an available slot */
    {
        if (pool->clientfd[i] < 0) 
        {
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
    if (i == FD_SETSIZE) {
        Liso_error("add_client error: Too many clients");
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
                printf("Server received %d (%d total) bytes on fd %d\n", 
                       nread, byte_cnt, connfd);
                
                nwrite = write(connfd, buf, nread);
                if (nwrite < 0) {
                    Liso_error("Error: write client socket failed");
                }
            }
            else /* EOF detected, remove descriptor from pool */
            { 
                close(connfd);
                FD_CLR(connfd, &pool->read_set);
                pool->clientfd[i] = -1;
            }
        }
    }
}

