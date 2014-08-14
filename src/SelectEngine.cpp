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
#include "SelectEngine.h"


#define DFT_PORT 9090

/** @Class SelPool Constructor.
 *
 *  @param listenfd server socket
 *  @param pool select pool
 */
SelPool::SelPool(int server_port) 
{
    /** create listen socket */
    int port = server_port;
    listenfd = Socket::Open_ListenSocket(port);
    Logger::log("Server socket %d created, listening port %d ...\n", 
                listenfd, port);

    /* Initially, there are no connected descriptors */
    int i;
    maxi = -1;
    for (i=0; i< FD_SETSIZE; i++) {
        clientfd[i] = -1;
    }

    /* Initially, listenfd is only member of select read set */
    maxfd = listenfd;
    FD_ZERO(&read_set);
    FD_SET(listenfd, &read_set);
}


/** 
 * @brief pool select
 * @return void
 */
void SelPool::Select()
{
    ready_set = read_set;
    nready = select(maxfd + 1, &ready_set, NULL, NULL, NULL);
}


/** @brief check server socket has received new client's connection.
 *
 *  @param listenfd server socket
 *  @pool select pool
 */
void SelPool::check_server()
{
    socklen_t clientlen = sizeof(struct sockaddr_in);
    struct sockaddr_in clientaddr;
    int connfd;

    if (FD_ISSET(listenfd, &ready_set))
    {
        connfd = accept(listenfd, (SA *)&clientaddr, &clientlen);
        add_client(connfd); /* adding client fd */
    }
}


/** @brief check select pool and process client reqeusts
 *
 *  @param pool select pool
 *  @return void
 */
void SelPool::check_clients() 
{
    int i, connfd, nread, nwrite;
    char buf[BUF_SIZE]; 

    for (i = 0; (i <= maxi) && (nready > 0); i++)
    {
        connfd = clientfd[i];

        /* If the descriptor is ready, echo a text line from it */
        if ((connfd > 0) && (FD_ISSET(connfd, &ready_set))) 
        {
            nready--;
            if ((nread = read(connfd, buf, BUF_SIZE)) >= 0) 
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
                Socket::Close_Socket(connfd);
                FD_CLR(connfd, &read_set);
                printf("remove client socket %d\n", connfd);
                clientfd[i] = -1;
            }
        }
    }
}


/*-------------------------- private functions -------------------------------*/
/** @brief add a client socket into select pool
 *
 *  @param connfd client socket
 *  @param pool select pool
 *  @return void
 */
void SelPool::add_client(int connfd) 
{
    int i;
    nready--;
    for (i = 0; i < FD_SETSIZE; i++)  /* Find an available slot */
    {
        if (clientfd[i] < 0) 
        {
            printf("adding client socket %d ...\n", connfd);
            /* Add connected descriptor to the pool */
            clientfd[i] = connfd;

            /* Add the descriptor to descriptor set */
            FD_SET(connfd, &read_set);

            /* Update max descriptor and pool highwater mark */
            if (connfd > maxfd){
                maxfd = connfd;
            }
            
            if (i > maxi) {
                maxi = i;
            }
            
            break;
        }
    }
    /* Can't find an empty slot */
    if (i == FD_SETSIZE) { /* TODO */
        fprintf(stderr, "Error: Too many clients\n");
    }
}
