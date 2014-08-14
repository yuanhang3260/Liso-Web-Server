#ifndef __CLIENTCONNECTION_H__
#define __CLIENTCONNECTION_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "HTTPRequest.h"

using namespace std;

class ClientConnection
{
/*----------------------------------------------------------------------------*/
public:
    enum writeStatus {
        initRes,
        writingRes,
        lastRes,
        doneRes,
    };

    enum HTTPType {
        T_HTTP,
        T_HTTPS,
    };

    /* Constructor */
    ClientConnection(int, ssize_t, int, char *, enum HTTPType);
    ~ClientConnection();

    /** compare and map functions */
    int compare(void *data1, void *data2) { return (fd - clientCon2->fd); }
    int map(void *data) { return isOpen; }

    /* Getters and Setters */
    int getSocket() { return fd; }
    HTTPRequest* getReqest() { return req; }
    void getReadBufferForRead(char **, ssize_t *);
    void getReadBufferForWrite(char **, ssize_t *);
    void getWriteBufferForRead(char **, ssize_t *);
    void getWriteBufferForWrite(char **, ssize_t *);

    void setClose() { isOpen = 0; }
    
    void addReadSize(ssize_t) { curReadSize += readSize; }
    void removeReadSize(ssize_t);
    
    void addWriteSize(ssize_t) { curWriteSize += writeSize; }
    void removeWriteSize(ssize_t);

    void setHTTP() { connSSL = NULL; }
    //void setHTTPS(SSL_CTX *);

    int isHTTP() { return connType == T_HTTP; }
    //int isHTTPS() { return connType == T_HTTPS; }

    //int hasAcceptedSSL() { return acceptedSSL == 1; }
    //void setAcceptedSSL() { acceptedSSL = 1; };
    int isFull() { return curReadSize == maxReadSize; }
    int isEmpty();
    int isNew() { return curReadSize > 0; }

    void cleanCGI();

/*----------------------------------------------------------------------------*/
private:
    int fd;
    int serverPort;
    string clientAddr;
    enum HTTPType connType;
    ssize_t curReadSize;
    ssize_t maxReadSize;
    ssize_t curWriteSize;
    ssize_t maxWriteSize;
    int isOpen;
    enum writeStatus wbStatus;
    char *readBuffer;
    char *writeBuffer;
    HTTPRequest *req;
    //responseObj *res;
    int CGIout;
    //SSL *connSSL;
    //int acceptedSSL;
};


#endif /* __CLIENTCONNECTION_H__ */
