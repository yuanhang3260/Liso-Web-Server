#ifndef __SELECTENGINE_H_
#define __SELECTENGINE_H_

#include <netinet/in.h>
#include <netinet/ip.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

//#include "connHandler.h"
//#include "sslLib.h"

typedef  struct sockaddr SA;

typedef struct selectEngine {
    // int portHTTP;
    // int portHTTPS;
    // SSL_CTX *ctx;
    // int  (*newConnHandler)(connObj *, char **);
    // void (*readConnHandler)(connObj *);
    // void (*pipeConnHandler)(connObj *);
    // void (*processConnHandler)(connObj *);
    // void (*writeConnHandler)(connObj *);
    // int (*closeConnHandler)(connObj *);
} selectEngine_t;


/** open and return a listening socket on port */
int Open_ListenSocket(int port);
int close_socket(int sock);



#endif /* __SELECTENGINE_H */
