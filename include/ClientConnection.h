#ifndef __CLIENTCONNECTION_H__
#define __CLIENTCONNECTION_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "HTTPRequest.h"
#include "HTTPResponse.h"

using namespace std;

#define BUF_SIZE 16384

class ClientConnection
{
/*----------------------------------------------------------------------------*/
public:
    enum State {
        Ready_ForRead,
        Request_Parsed,
        Writing_Response,
        Done_Response,
        Closed
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
    void setRequest(HTTPRequest* _request) { req = _request; }
    HTTPRequest* getRequest() { return req; }
    HTTPResponse* getResponse() { return res; }
    void deleteResponse() { delete res; res = NULL; }

    enum State getState() { return state; }
    void setState(enum State _state) { state = _state; }
    
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
    int isEmpty() { return curReadSize == 0; }
    int isReadable() { return state == Ready_ForRead; }
    int isWritable() { return state == Writing_Response; }

    int isNew() { return curReadSize > 0; }

    /** create a new response */
    HTTPResponse* createResponse() { return (res = new HTTPResponse()); }
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
    enum State state;
    char *readBuffer;
    char *writeBuffer;
    HTTPRequest *req;
    HTTPResponse *res;
    int CGIout;
    //SSL *connSSL;
    //int acceptedSSL;
};


#endif /* __CLIENTCONNECTION_H__ */
