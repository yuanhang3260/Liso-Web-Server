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

#define BUF_SIZE 4096

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
    ClientConnection(int, ssize_t, int, string, enum HTTPType);
    ~ClientConnection();

    /** compare and map functions */
    int compare(ClientConnection *clientCon2) { return (fd - clientCon2->fd); }
    int map(void *data) { return isOpen; }

    /* Getters and Setters */
    int getFd() { return fd; }
    string getAddr() { return clientAddr; }
    HTTPRequest* getRequest() { return req; }
    
    char* getReadBuffer_ForRead(ssize_t *size);
    char* getReadBuffer_ForWrite(ssize_t *);
    char* getWriteBuffer_ForRead(ssize_t *);
    char* getWriteBuffer_ForWrite(ssize_t *);

    int isClosed() { return !isOpen; }
    void setClosed() { isOpen = 0; }
    
    void addReadSize(ssize_t readSize) { curReadSize += readSize; }
    void removeReadSize(ssize_t);
    
    void addWriteSize(ssize_t writeSize) { curWriteSize += writeSize; }
    void removeWriteSize(ssize_t);

    //void setHTTP() { connSSL = NULL; }
    //void setHTTPS(SSL_CTX *);

    int isHTTP() { return connType == T_HTTP; }
    int isHTTPS() { return connType == T_HTTPS; }

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
    responseObj *res;
    int CGIout;
    //SSL *connSSL;
    //int acceptedSSL;
};


#endif /* __CLIENTCONNECTION_H__ */
