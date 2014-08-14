#ifndef __SOCKET_H_
#define __SOCKET_H_

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

#define LISTENQ 10
typedef struct sockaddr SA;

class Socket
{
public:
    /** open and return a listening socket on port */
    static int Open_ListenSocket(int port);

    /** close a socket */
    static int Close_Socket(int sock);

private:
    /** open server socket */
    static int open_ListenSocket(int port);
};


#endif /* __SOCKET_H_ */