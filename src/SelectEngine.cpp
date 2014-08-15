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
#include <arpa/inet.h>
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
    port = server_port;
    listenfd = Socket::Open_ListenSocket(port);
    Logger::log("Server socket %d created, listening port %d ...\n", 
                listenfd, port);

    /* Initially, listenfd is only member of select read set */
    FD_ZERO(&read_set);
    FD_ZERO(&ready_set);
    FD_SET(listenfd, &read_set);
    maxfd = listenfd;
}


/** 
 * @brief pool select
 * @return void
 */
void SelPool::Select()
{
    printf("Selecting in server and %d clients...\n", clients.size());
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
        string addr(inet_ntoa(clientaddr.sin_addr));
        add_client(connfd, addr); /* adding client fd */
    }
}


/** @brief check select pool and process client reqeusts
 *
 *  @param pool select pool
 *  @return void
 */
void SelPool::check_clients() 
{
    int connfd, nread, nwrite;
    char buf[BUF_SIZE]; 

    vector<ClientConnection*>::iterator it = clients.begin();
    while ( it != clients.end() )
    {
        connfd = (*it)->getFd();

        /* If the descriptor is ready, echo a text line from it */
        if ((connfd > 0) && (FD_ISSET(connfd, &ready_set))) 
        {
            nready--;
            if ((nread = read(connfd, buf, BUF_SIZE)) > 0) 
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
                printf("remove client socket %d\n", connfd);
                Socket::Close_Socket(connfd);
                FD_CLR(connfd, &read_set);
                delete (*it);
                clients.erase(it);
                continue;
            }
        }
        it++;
    }
}


/** @brief print clients
 *  @return void
 */
void SelPool::print_clients()
{
    cout << "------- Cliens -------\n";
    if (clients.size() <= 0) {
        cout << "No clients in pool\n";
    }
    for (size_t i = 0; i < clients.size(); i++) {
        cout << "fd: " << clients[i]->getFd() << " addr: " << clients[i]->getAddr();
        cout << endl;
    }
    cout << "---------------------\n";
}


/*-------------------------- private functions -------------------------------*/
/** @brief add a client socket into select pool
 *
 *  @param connfd client socket
 *  @param pool select pool
 *  @return void
 */
void SelPool::add_client(int connfd, string addr)
{
    nready--;
    
    printf("adding client socket %d ...\n", connfd);

    /* Add the descriptor to descriptor set */
    if (connfd > maxfd) {
        maxfd = connfd;
    }
    FD_SET(connfd, &read_set);

    /* create a new ClientConnection Object and add it to client list */
    ClientConnection *newClient = 
                     new ClientConnection(connfd,
                                          BUF_SIZE,
                                          port,
                                          addr,
                                          ClientConnection::T_HTTP);
    clients.push_back(newClient);
    //cout << "added\n";
}
