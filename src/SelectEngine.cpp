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
    printf("Server socket %d created, listening port %d ...\n", 
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
    int connfd;

    vector<ClientConnection*>::iterator it = clients.begin();
    while ( it != clients.end() )
    {
        ClientConnection *client = *it;
        connfd = client->getFd();

        /* If the descriptor is ready, echo a text line from it */
        if ((connfd > 0) && (FD_ISSET(connfd, &ready_set))) 
        {
            nready--;

            readHandler(client);
            processHandler(client);
            // if ((nread = read(connfd, buf, BUF_SIZE)) > 0) 
            // {
            //     byte_cnt += nread;
            //     printf("Server received %d bytes on fd %d\n", nread, connfd);
                
            //     nwrite = write(connfd, buf, nread);
            //     if (nwrite < 0) {
            //         fprintf(stderr, "Error: write client socket error\n");
            //     }
            // }
            // else /* EOF detected, remove descriptor from pool */
            // {
            //     printf("remove client socket %d\n", connfd);
            //     Socket::Close_Socket(connfd);
            //     FD_CLR(connfd, &read_set);
            //     delete (*it);
            //     clients.erase(it);
            //     continue;
            // }
        }
        it++;
    }
}


/** @brief read handler
 *  @return void
 */
void SelPool::readHandler(ClientConnection *client)
{
    // if(isHTTPS(connPtr) && !hasAcceptedSSL(connPtr)) 
    // {
    //     SSL_accept(connPtr->connSSL);
    //     setAcceptedSSL(connPtr);
    //     return;
    // }
    if (client->isFull()) {
        return;
    }

    ssize_t size;
    ssize_t retSize = 0;
    char* buf = client->getReadBuffer_ForWrite(&size);
    int connFd = client->getFd();

    if(client->isHTTP()) {
        printf( "HTTP client ...\n");
        retSize = read(connFd, buf, size);
    }
    else if(client->isHTTPS()) {
        printf( "HTTPS client ...\n");
        //retSize = SSL_read(connPtr->connSSL, buf, size);
    }
    else {
        retSize = -1;
    }

    if (retSize > 0)
    {
        printf("Read %d bytes\n", retSize);
        client->addReadSize(retSize);
    }
    /** EOF incurred, connection closed by client */
    else if(retSize == 0)
    {
        printf("set Client [%d] closed\n", connFd);
        client->setClosed();
        return;
    }
    else /* Read error: retSize < 0 */ 
    {
        printf("Error reading from client.\n");
        if(client->isHTTPS())
        {
            // int err = SSL_get_error(connPtr->connSSL, retSize);
            // switch(err) 
            // {
            //     case SSL_ERROR_WANT_READ:
            //     case SSL_ERROR_WANT_WRITE:
            //         printf("SSL WANT MORE.\n");
            //         return;
            //     default:
            //         ERR_print_errors_fp(getLogger());
            //         break;
            // }
        }
        else 
        {
            if(errno == EINTR) {
                printf("RECV EINTR. Try later again.\n");
                return;
            }
        }
        client->setClosed();
        return;
    }

}


void SelPool::processHandler(ClientConnection *client)
{
    /** skip connection that has been closed */
    if(client->isClosed()) {
        printf("Connection to client has been closed\n");
        return;
    }

    /** read request from read buffer */
    ssize_t size;
    char* buf = client->getReadBuffer_ForRead(&size);
    int full = client->isFull();
    
    /** pass request */
    HTTPRequest *req = client->getRequest();
    switch( req->httpParse(buf, &size, full) ) 
    {
        case HTTPRequest::Parsing:
            printf("Parsing ...\n");
            client->removeReadSize(size);
            break;
        
        case HTTPRequest::ParseError:
            printf("ParseError\n");
            
        case HTTPRequest::Parsed:
            printf("Parsed\n");
            client->removeReadSize(size);
            //TODO:
            // if(client->res == NULL)
            // {
            //     printf( "Create new response\n");
            //     connPtr->res = createResponseObj();
            //     buildResponseObj(connPtr->res, connPtr->req);
            //     if(isCGIResponse(connPtr->res)) {
            //         connPtr->CGIout = connPtr->res->CGIout;
            //     }
            // }
            // if(!req->isCGIResponse())
            // {
            //     /* Dump HTTP response to buffer */
            //     printf( "Dump response to buffer\n");
            //     getConnObjWriteBufferForWrite(connPtr, &buf, &size);
            //     printf( "Write buffer has %d bytes free\n", size);
            //     done = writeResponse(connPtr->res, buf, size, &retSize);
            //     printf( "%d bytes dumped, done? %d\n", retSize, done);
            //     addConnObjWriteSize(connPtr, retSize);
            //     connPtr->wbStatus = writingRes;
            //     if(done) {
            //         printf( "All dumped\n");
            //         connPtr->wbStatus = lastRes;
            //     }
            // }
            break;
        
        default:
            break;
    }
    return;
}


// void SelPool::pipeConnectionHandler(connObj *connPtr)
// {
//     char *buf;
//     ssize_t size;
//     getConnObjWriteBufferForWrite(connPtr, &buf, &size);
//     if(size > 0) 
//     {
//         ssize_t retSize = read(connPtr->CGIout, buf, size);
//         if(retSize > 0) 
//         {
//             printf( "%d bytes read from CGI pipe\n", retSize);
//             addConnObjWriteSize(connPtr, retSize);
//         }
//         else if(retSize == 0) 
//         {
//             printf( "CGI pipe broken\n", retSize);
//             cleanConnObjCGI(connPtr);
//         }
//         else 
//         {
//             if(errno != EINTR) {
//                 cleanConnObjCGI(connPtr);
//                 setConnObjClose(connPtr);
//             }
//         }
//     }
// }


// void SelPool::writeConnectionHandler(connObj *connPtr)
// {
//     char *buf;
//     ssize_t size, retSize;
//     int connFd = getConnObjSocket(connPtr);
//     getConnObjWriteBufferForRead(connPtr, &buf, &size);
//     printf( "Ready to write %d bytes...", size);
//     if(size <= 0) {
//         prepareNewConn(connPtr);
//         return;
//     }
//     if(isHTTP(connPtr)) {
//         retSize = send(connFd, buf, size, 0);
//     } else if(isHTTPS(connPtr)) {
//         retSize = SSL_write(connPtr->connSSL, buf, size);
//     } else {
//         retSize = -1;
//     }
//     if(retSize == -1 && errno == EINTR) {
//         return ;
//     }
//     if(retSize == -1 && isHTTPS(connPtr) ) {
//         int err = SSL_get_error(connPtr->connSSL, retSize);
//         switch(err) {
//         case SSL_ERROR_WANT_READ:
//         case SSL_ERROR_WANT_WRITE:
//             printf("SSL WANT MORE.\n");
//             return;
//         default:
//             ERR_print_errors_fp(getLogger());
//             break;
//         }
//     }
//     if (retSize != size) {
//         printf("Error sending to client.\n");
//         setConnObjClose(connPtr);
//     } else {
//         prepareNewConn(connPtr);
//         printf( "Done\n");
//         removeConnObjWriteSize(connPtr, size);
//     }

// }

// void SelPool::prepareNewConn(connObj *connPtr)
// {
//     if(connPtr->wbStatus == lastRes) {
//         connPtr->wbStatus = doneRes;
//         if(1 == toClose(connPtr->res)) {
//             printf( "[%d] set to close.\n", connPtr->connFd);
//             setConnObjClose(connPtr);
//         } else {
//             /* Prepare for next request */
//             freeResponseObj(connPtr->res);
//             connPtr->res = NULL;
//             freeRequestObj(connPtr->req);
//             connPtr->req = createRequestObj(
//                                connPtr->serverPort,
//                                connPtr->clientAddr,
//                                (connPtr->connType == T_HTTPS) ? 1 : 0);
//         }
//     }
// }

// int SelPool::closeConnectionHandler(connObj *connPtr)
// {
//     connPtr = connPtr;
//     return EXIT_SUCCESS;
// }

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


