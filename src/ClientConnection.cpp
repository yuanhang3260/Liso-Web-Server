/** @brief file ClientConnection class routines
 *  @author Hang Yuan (hangyuan)
 *  @bug No bugs found yet
 */
#include "ClientConnection.h"


/** @brief Class ClientConnection Constructor 
 *
 *  @param connFd client fd
 *  @param bufferSize buffer size
 *  @port server port
 *  @addr client addr
 *  @HTTPType HTTP type
 */
ClientConnection::ClientConnection( int connFd,
                                    ssize_t bufferSize,
                                    int port,
                                    string addr,
                                    enum HTTPType connType)
{
    fd = connFd;
    serverPort = port;
    clientAddr = addr;
    
    connType = connType;
    //acceptedSSL = 0;
    curReadSize = 0;
    maxReadSize = bufferSize;
    readBuffer = (bufferSize > 0) ? (new char[bufferSize]) : NULL;
    
    curWriteSize = 0;
    maxWriteSize = bufferSize;
    writeBuffer = (bufferSize > 0) ? (new char[bufferSize]) : NULL;
    wbStatus = initRes;

    isOpen = 1;
    
    req = createRequestObj(serverPort, clientAddr, 
                           (connType == T_HTTPS) ? 1 : 0 );
    //res = NULL;

    CGIout = -1;
}


/** @brief Class ClientConnection Destructor */
ClientConnection::~ClientConnection()
{
    // if(isHTTPS()) {
    //     SSL_shutdown(connSSL);
    //     SSL_free(connSSL);
    // }
    close(connFd);
    if (clientAddr != NULL) {
        delete[] clientAddr;
    }
    if (readBuffer != NULL) {
        delete[] readBuffer;
    }
    if (writeBuffer != NULL) {
        delete[] writeBuffer);
    }
    
    req->~Request();

    //res = NULL;
    
    cleanCGI();
}


/** get readBuffer for read */
char* getReadBuffer_ForRead(ssize_t *size)
{
    *size = curReadSize;
    return readBuffer;
}

/** get readBuffer for write */
char* getReadBuffer_ForWrite(ssize_t *size)
{
    *size = maxReadSize - curReadSize;;
    return readBuffer + curReadSize;
}

/** get writeBuffer for read */
char* getConnObjWriteBufferForRead(ssize_t *size)
{
    *size = curWriteSize;
    return writeBuffer;
}

/** get writeBuffer for write */
char* getConnObjWriteBufferForWrite(ssize_t *size)
{
    *size = maxWriteSize - curWriteSize;
    return writeBuffer + curWriteSize;
}


/** remove read size */
void removeReadSize(ssize_t readSize)
{
    ssize_t curReadSize = curReadSize;
    if(readSize <= curReadSize) {
        char *buf = readBuffer;
        memmove(buf, buf + readSize, curReadSize - readSize);
        curReadSize -= readSize;
    }
}


/** remove write size */
void removeWriteSize(ssize_t writeSize)
{
    ssize_t curWriteSize = curWriteSize;
    if(writeSize <= curWriteSize) {
        char *buf = writeBuffer;
        memmove(buf, buf + writeSize, curWriteSize - writeSize);
        curWriteSize -= writeSize;
    }
}


/** if it's an empty */
int isEmpty()
{
    switch(wbStatus)
    {
        case initRes:
            return curWriteSize == 0;
        case writingRes:
        case lastRes:
            return 0;
        case doneRes:
            return 1;
        default:
            return -1;
    }
}


// void setConnObjHTTPS(SSL_CTX *ctx)
// {
//     /* Set non-blocking */
//     int flag = fcntl(connFd, F_GETFL, 0);
//     flag = flag | O_NONBLOCK;
//     fcntl(connFd, F_SETFL, flag);
//     /* Init SSL connection */
//     SSL *sslPtr = SSL_new(ctx);
//     SSL_set_fd(sslPtr, connFd);
//     connSSL = sslPtr;
// }


/** clean CGI object */
void cleanCGI()
{
    if (CGIout > 0) {
        close(CGIout);
    }
    CGIout = -1;
    wbStatus = lastRes;
};
