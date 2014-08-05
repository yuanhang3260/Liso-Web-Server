/* 
 * echoservers.c - A concurrent echo server based on select
 */
/* $begin echoserversmain */
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

typedef struct pool
{   
    /* represents a pool of connected descriptors */ //line:conc:echoservers:beginpool
    int maxfd;        /* largest descriptor in read_set */   
    fd_set read_set;  /* set of all active descriptors */
    fd_set ready_set; /* subset of descriptors ready for reading  */
    int nready;       /* number of ready descriptors from select */   
    int maxi;         /* highwater index into client array */
    int clientfd[FD_SETSIZE];    /* set of active descriptors */
} pool_t; //line:conc:echoservers:endpool

/* $end echoserversmain */
void init_pool(int listenfd, pool_t *p);
void add_client(int connfd, pool_t *p);
void check_clients(pool_t *p);
/* $begin echoserversmain */

int byte_cnt = 0; /* counts total bytes received by server */

int main(int argc, char **argv)
{
    int listenfd, connfd, port; 
    socklen_t clientlen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    static pool_t pool; 

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(0);
    }
    port = atoi(argv[1]);

    listenfd = Open_ListenSocket(port);
    init_pool(listenfd, &pool); //line:conc:echoservers:initpool

    while (1) 
    {
        /* Wait for listening/connected descriptor(s) to become ready */
        pool.ready_set = pool.read_set;
        pool.nready = select(pool.maxfd+1, &pool.ready_set, NULL, NULL, NULL);

        /* If listening descriptor ready, add new client to pool */
        if (FD_ISSET(listenfd, &pool.ready_set)) { //line:conc:echoservers:listenfdready
            connfd = accept(listenfd, (SA *)&clientaddr, &clientlen); //line:conc:echoservers:accept
            add_client(connfd, &pool); //line:conc:echoservers:addclient
        }
        
        /* Echo a text line from each ready connected descriptor */ 
        check_clients(&pool); //line:conc:echoservers:checkclients
    }
}
/* $end echoserversmain */

/* $begin init_pool */
void init_pool(int listenfd, pool_t *pool) 
{
    /* Initially, there are no connected descriptors */
    int i;
    pool->maxi = -1;                   //line:conc:echoservers:beginempty
    for (i=0; i< FD_SETSIZE; i++)  {
        pool->clientfd[i] = -1;        //line:conc:echoservers:endempty
    }

    /* Initially, listenfd is only member of select read set */
    pool->maxfd = listenfd;            //line:conc:echoservers:begininit
    FD_ZERO(&pool->read_set);
    FD_SET(listenfd, &pool->read_set); //line:conc:echoservers:endinit
}
/* $end init_pool */

/* $begin add_client */
void add_client(int connfd, pool_t *pool) 
{
    int i;
    pool->nready--;
    for (i = 0; i < FD_SETSIZE; i++)  /* Find an available slot */
    {
        if (pool->clientfd[i] < 0) 
        { 
            /* Add connected descriptor to the pool */
            pool->clientfd[i] = connfd;                 //line:conc:echoservers:beginaddclient

            /* Add the descriptor to descriptor set */
            FD_SET(connfd, &pool->read_set); //line:conc:echoservers:addconnfd

            /* Update max descriptor and pool highwater mark */
            if (connfd > pool->maxfd) //line:conc:echoservers:beginmaxfd
                pool->maxfd = connfd; //line:conc:echoservers:endmaxfd
            
            if (i > pool->maxi)       //line:conc:echoservers:beginmaxi
                pool->maxi = i;       //line:conc:echoservers:endmaxi
            
            break;
        }
    }
    /* Couldn't find an empty slot */
    if (i == FD_SETSIZE) {
        Liso_error("add_client error: Too many clients");
    }
}
/* $end add_client */

/* $begin check_clients */
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
                byte_cnt += nread; //line:conc:echoservers:beginecho
                printf("Server received %d (%d total) bytes on fd %d\n", 
                       nread, byte_cnt, connfd);
                
                nwrite = write(connfd, buf, nread); //line:conc:echoservers:endecho
                if (nwrite < 0) {
                    Liso_error("Error: write client socket failed");
                }
            }

            /* EOF detected, remove descriptor from pool */
            else 
            { 
                close(connfd); //line:conc:echoservers:closeconnfd
                FD_CLR(connfd, &pool->read_set); //line:conc:echoservers:beginremove
                pool->clientfd[i] = -1;          //line:conc:echoservers:endremove
            }
        }
    }
}
/* $end check_clients */
