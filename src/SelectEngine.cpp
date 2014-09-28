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
#define CLIENT_CLOSED 1

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
    FD_ZERO(&write_set);
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

    FD_ZERO(&read_set);
    FD_ZERO(&write_set);
    FD_SET(listenfd, &read_set);
    vector<ClientConnection*>::iterator it = clients.begin();
    while ( it != clients.end() )
    {
        ClientConnection *client = *it;
        int connFd = client->getFd();

        if( client->isEmpty() && client->isReadable())
        {
            printf("put %d to read_set\n", connFd);
            FD_SET(connFd, &read_set);
        }
        if( client->isWritable() ) /* state == Writing_Response */
        {
            printf("put %d to write_set\n", connFd);
            FD_SET(connFd, &write_set);
            /* if it is CGI request, put its CGI_out pipe to read set */
            if (client->getRequest()->isCGIRequest()) {
                int CGIout = client->getResponse()->getCGIout();
                if (CGIout > maxfd) {
                    maxfd = CGIout;
                }
                FD_SET(CGIout, &read_set);
            }
        }
        it++;
    }

    nready = select(maxfd + 1, &read_set, &write_set, NULL, NULL);
    //printf("Select Finished\n");
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

    if (FD_ISSET(listenfd, &read_set))
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
        printf("Handling %d\n", connfd);

        /* Client Connection State Machine */
        switch ( client->getState() )
        {
            case ClientConnection::Ready_ForRead:
            {
                printf("Client State: Ready_ForRead\n");
                /* read ready client socket */
                if (FD_ISSET(connfd, &read_set))
                {
                    readHandler(client);
                    if (client->getState() == ClientConnection::Request_Parsed)
                    {
                        processHandler(client);
                        /* if the client is closed after processing */
                        if (client->getState() == ClientConnection::Closed)
                        {
                            Socket::Close_Socket(connfd);
                            FD_CLR(connfd, &read_set);
                            delete client;
                            clients.erase(it);
                            continue;
                        }
                        writeHandler(client);
                    }
                }
                break;
            }

            case ClientConnection::Request_Parsed: {
                printf("Client State: Request_Parsed\n");
                break;
            }

            case ClientConnection::Writing_Response: 
            {
                printf("Client State: Writing_Response\n");
                if (FD_ISSET(connfd, &write_set)) 
                {
                    if (!client->getRequest()->isCGIRequest())
                    {
                        processHandler(client);
                        writeHandler(client);
                    }
                    else if (FD_ISSET(client->getResponse()->getCGIout(), &read_set))
                    {
                        /* CGI request : if CGIout is also ready for reading */
                        pipeHandler(client);
                        writeHandler(client);
                    }
                }
                break;
            }

            case ClientConnection::Done_Response: {
                printf("Client State: Done_Response\n");
                break;
            }

            default: {
                break;
            }
        }
        it++;
    }
}


/** @brief read handler
 *  @return void
 */
void SelPool::readHandler(ClientConnection *client)
{
    if (client->isFull()) {
        return;
    }

    ssize_t size;
    int retSize = 0;
    char* buf = client->getReadBuffer_ForWrite(&size);
    int connFd = client->getFd();
    //printf("reading client %d\n", connFd);

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
        printf("Successfully Read %d Bytes from client %d\n", retSize, client->getFd());
        client->addReadSize(retSize);
        
        /* get read buffer */
        ssize_t size;
        char* parse_buf = client->getReadBuffer_ForRead(&size);
        
        /* try to parse the request from buffer */
        HTTPRequest *req = client->getRequest();
        req->httpParse(parse_buf, &size);
        client->removeReadSize(size);

        // TODO: to add a status "Request_Parsing"
        if (client->getRequest()->getState() == HTTPRequest::ParsedCorrect
         || client->getRequest()->getState() == HTTPRequest::ParsedError) {
            client->setState(ClientConnection::Request_Parsed);
        }
    }
    /** EOF incurred, connection closed by client */
    else if (retSize == 0)
    {
        printf("set Client [%d] closed\n", connFd);
        client->setClosed();
        client->setState(ClientConnection::Request_Parsed);
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
    printf("\nProcessing\n");
    /** skip connection that has been closed */
    if (client->isClosed()) {
        printf("Connection to client has been closed\n");
        client->setState(ClientConnection::Closed);
        return;
    }

    switch( client->getRequest()->getState() )
    {
        case HTTPRequest::Parsing:
            printf("Parsing ...\n");
            break;
        
        case HTTPRequest::ParsedError:
            printf("ParseError\n");
            
        case HTTPRequest::ParsedCorrect: {
            printf("Parsed\n");
            
            /* build response for this client */
            HTTPResponse *res = NULL;
            if ((res = client->getResponse()) == NULL)
            {
                printf( "Create new response\n");
                res = client->createResponse();
                res->buildResponse(client->getRequest());
            }
            
            /* if it's CGI request, wait until the next select loop to write */
            if (client->getRequest()->isCGIRequest()) {
                client->setState(ClientConnection::Writing_Response);
                break;
            }

            /* Dump HTTP response to send buffer */
            ssize_t size, retSize;
            char *write_buf = client->getWriteBuffer_ForWrite(&size);
            printf("Write buffer has %d bytes free\n", size);
            
            int re = res->writeResponse(write_buf, size, &retSize);
            client->addWriteSize(retSize);
            if (re) {
                printf("All dumped\n");
                client->setState(ClientConnection::Done_Response);
                //delete client->getResponse();
            }
            else {
                printf("Not all dumped\n");
                client->setState(ClientConnection::Writing_Response);
            }
            printf("------- Printing Response Buffer --------\n");
            printf("%s", write_buf);
            break;
        }
        default:
            break;
    }
    return;
}


void SelPool::writeHandler(ClientConnection *client)
{
    ssize_t size, retSize;
    int connFd = client->getFd();
    
    char *buf = client->getWriteBuffer_ForRead(&size);
    printf( "Ready to write %d bytes ...\n", size);

    if(client->isHTTP()) {
        retSize = write(connFd, buf, size);
    }
    // else if(isHTTPS(connPtr)) {
    //     retSize = SSL_write(connPtr->connSSL, buf, size);
    // }
    else {
        retSize = -1;
    }

    // if(retSize == -1 && isHTTPS(connPtr) )
    // {
    //     int err = SSL_get_error(connPtr->connSSL, retSize);
    //     switch(err) {
    //     case SSL_ERROR_WANT_READ:
    //     case SSL_ERROR_WANT_WRITE:
    //         printf("SSL WANT MORE.\n");
    //         return;
    //     default:
    //         ERR_print_errors_fp(getLogger());
    //         break;
    //     }
    // }
    if (retSize != size || (retSize == -1 && errno == EINTR)) 
    {
        printf("Error sending to client.\n");
        client->setClosed();
    }
    else
    {
        printf( "----------------------------- Done -----------------------------\n");
        client->removeWriteSize(size);
    }

    /* Set connection state to Ready_ForRead */
    if (client->getState() == ClientConnection::Done_Response) {
        client->setState(ClientConnection::Ready_ForRead);
        client->deleteResponse();
    }
}


void SelPool::pipeHandler(ClientConnection *client)
{
    ssize_t size;
    char *buf = client->getWriteBuffer_ForWrite(&size); 
    if (size <= 0) {
        return;
    }
    
    ssize_t retSize = read(client->getResponse()->getCGIout(), buf, size);
    printf("CGI output: %s\n", buf);
    if(retSize > 0)
    {
        printf( "%d bytes read from CGI pipe\n", retSize);
        client->addWriteSize(retSize);
    }
    else if (retSize == 0)
    {
        printf("CGI out pipe closed\n");
        client->setState(ClientConnection::Done_Response);
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
    
    /* add request for this ClientConnection */
    newClient->setRequest(new HTTPRequest(newClient, port, addr.c_str(), 0));

    printf("BUF_SIZE = %d\n", BUF_SIZE);
    clients.push_back(newClient);
    //cout << "added\n";
}


