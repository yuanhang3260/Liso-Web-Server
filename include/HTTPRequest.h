#ifndef __HTTPREQUEST_H__
#define __HTTPREQUEST_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <string>
#include <vector>
#include "Utility.h"
#include "HTTPHeader.h"

using namespace std;

class ClientConnection;

class HTTPRequest
{
/*----------------------------------------------------------------------------*/
public:
    /** @brief State */
    enum State {
        Init,
        Parsing,
        ParsedCorrect,
        ParsedError,
    };

    /** @brief request status code */
    enum StatusCode {
        OK = 200,
        BAD_REQUEST = 400,
        NOT_FOUND = 404,
        LENGTH_REQUIRED = 411,
        INTERNAL_SERVER_ERROR = 500,
        NOT_IMPLEMENTED = 501,
        SERVICE_UNAVAILABLE = 503,
        HTTP_VERSION_NOT_SUPPORTED = 505,
    };

    /** request method */
    enum Method {
        GET,
        HEAD,
        POST,
        UNIMPLEMENTED,
    };

    /** parser state machine */
    enum ParseState {
        requestLine,
        headerLine,
        contentLine,
        requestError,
        requestDone,
    };


    /* Constructor and Destructor */
    HTTPRequest(ClientConnection*, int, const char *, int);
    ~HTTPRequest();

    /** parse http request */
    void httpParse(char *bufPtr, ssize_t *size);
    /** is a CGI request */
    int isCGIRequest() { return isCGI; }
    /** is a new request */
    int isNewRequest() { return isNew; }

    /** print the http request */
    void print();

    enum Method getMethod() { return method; }
    char* getURI() { return uri; }
    int getVersion() { return version; }
    vector<HTTPHeader*> getHeaders() {  return headers; }
    enum State getState() { return state; }
    int getStatusCode() { return statusCode; }
    string getHeaderValueByKey(string key);

    void setParseStatus(enum ParseState _parseStatus) { parseStatus = _parseStatus; }
    enum ParseState getParseStatus() { return parseStatus; }
    
/*----------------------------------------------------------------------------*/
private:
    enum Method method;
    char *uri;
    int version;
    vector<HTTPHeader*> headers;
    
    char *content;
    int contentLength;
    
    int statusCode;
    enum State state;
    enum ParseState parseStatus;
    
    int isNew;
    int isCGI;

    ClientConnection *client;
    
    // char *exePath;
    // DLL *envp;

    /* Private methods */
    void httpParseLine(char *, ssize_t , ssize_t *);
    void setRequestError(enum StatusCode );

    int isValidRequest();
    int checkIsCGIRequest(char* uri);

    
    //void buildENVP();
    //void insertENVP(char*, char*);
    string getMethodString(enum Method);
};


struct methodEntry {
    HTTPRequest::Method m;
    string s;
};


#endif  /* __HTTPREQUEST_H__ */
