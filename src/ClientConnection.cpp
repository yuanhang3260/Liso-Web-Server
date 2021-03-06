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
                                    enum HTTPType _connType)
{
    fd = connFd;
    serverPort = port;
    clientAddr = addr;
    
    connType = _connType;
    //acceptedSSL = 0;
    curReadSize = 0;
    maxReadSize = bufferSize;
    readBuffer = (bufferSize > 0) ? (new char[bufferSize]) : NULL;
    if (bufferSize > 0) {
        bzero(readBuffer, bufferSize);
    }
    
    curWriteSize = 0;
    maxWriteSize = bufferSize;
    writeBuffer = (bufferSize > 0) ? (new char[bufferSize]) : NULL;
    if (bufferSize > 0) {
        bzero(writeBuffer, bufferSize);
    }

    state = Ready_ForRead;

    isOpen = 1;
    
    req = NULL;
    res = NULL;
    
    CGIout = -1;
}


/** @brief Class ClientConnection Destructor */
ClientConnection::~ClientConnection()
{
    printf("deleting Client %d\n", fd);
    // if(isHTTPS()) {
    //     SSL_shutdown(connSSL);
    //     SSL_free(connSSL);
    // }
    close(fd);
    if (readBuffer != NULL) {
        delete[] readBuffer;
        readBuffer = NULL;
    }
    if (writeBuffer != NULL) {
        delete[] writeBuffer;
        writeBuffer = NULL;
    }
    
    if (res != NULL) {
        delete res;
    }
    if (req != NULL) {
        delete res;
    }

    //res = NULL;
    
    cleanCGI();
}


/** get readBuffer for read */
char* ClientConnection::getReadBuffer_ForRead(ssize_t *size)
{
    *size = curReadSize;
    return readBuffer;
}

/** get readBuffer for write */
char* ClientConnection::getReadBuffer_ForWrite(ssize_t *size)
{
    *size = maxReadSize - curReadSize;;
    return readBuffer + curReadSize;
}

/** get writeBuffer for read */
char* ClientConnection::getWriteBuffer_ForRead(ssize_t *size)
{
    *size = curWriteSize;
    return writeBuffer;
}

/** get writeBuffer for write */
char* ClientConnection::getWriteBuffer_ForWrite(ssize_t *size)
{
    *size = maxWriteSize - curWriteSize;
    return writeBuffer + curWriteSize;
}


/** remove read size */
void ClientConnection::removeReadSize(ssize_t readSize)
{
    if(readSize <= curReadSize) {
        char *buf = readBuffer;
        memmove(buf, buf + readSize, curReadSize - readSize);
        curReadSize -= readSize;
    }
}


/** remove write size */
void ClientConnection::removeWriteSize(ssize_t writeSize)
{
    if(writeSize <= curWriteSize) {
        char *buf = writeBuffer;
        memmove(buf, buf + writeSize, curWriteSize - writeSize);
        curWriteSize -= writeSize;
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
void ClientConnection::cleanCGI()
{
    if (CGIout > 0) {
        close(CGIout);
    }
    CGIout = -1;
    //wbStatus = lastRes;
};
