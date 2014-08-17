#ifndef __SELECTENGINE_H_
#define __SELECTENGINE_H_

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include "ClientConnection.h"

#define LISTENQ 10

using namespace std;

typedef struct sockaddr SA;

class SelPool
{
/*----------------------------------------------------------------------------*/
public: 
    SelPool(int server_port);

    /** pool select */
    void Select();

    /** check server socket has received new client's connection */
    void check_server();

    /** check client */
    void check_clients();

    /** print clients */
    void print_clients();

    /***** Connection Handlers *****/
    /** read Handler */
    void readHandler(ClientConnection *client);
    /** pipe Handler */
    void pipeHandler(ClientConnection *client);
    /** process Handler */
    int processHandler(ClientConnection *client);
    /** write Handler */
    void writeHandler(ClientConnection *client);
    /** close Handler */
    int closeHandler(ClientConnection *client);

/*----------------------------------------------------------------------------*/
private:
    /** listen fd */
    int listenfd;
    /** server port */
    int port;
    /** largest descriptor in read_set */
    int maxfd;
    /** set of all active descriptors */      
    fd_set read_set;
    /** subset of descriptors ready for reading */
    fd_set write_set;
    /** number of ready descriptors from select */
    int nready;
    /** Client Connections */
    vector<ClientConnection*> clients;
    /** counts total bytes received by server */
    int byte_cnt;

    /** add a client socket into pool */
    void add_client(int connfd, string addr);

};

#endif /* __SELECTENGINE_H */
