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
/*----------------------------------------------------------------------------*/
private:
    /** listen fd */
    int listenfd;
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
    /** counts total bytes received by server */
    int byte_cnt;
    
    /** add a client socket into pool */
    void add_client(int connfd);

};

#endif /* __SELECTENGINE_H */
