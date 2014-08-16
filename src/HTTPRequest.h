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


class HTTPRequest
{
/*----------------------------------------------------------------------------*/
public:
    /** @brief parse status */
    enum Status {
        Parsing,
        Parsed,
        ParseError,
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
    enum State {
        requestLine,
        headerLine,
        contentLine,
        requestError,
        requestDone,
    };


    /* Constructor and Destructor */
    HTTPRequest(int, const char *, int);
    ~HTTPRequest();

    /** parse http request */
    enum Status httpParse(char *bufPtr, ssize_t *size, int full );
    /** is a CGI request */
    //int isCGIRequest();
    /** is a new request */
    int isNewRequest() { return isNew; }

    /** print the http request */
    void print();

    enum Method getMethod() { return method; }
    char* getURI() { return uri; }
    int getVersion() { return version; }
    vector<HTTPHeader*> getHeaders() {  return headers; }
    enum State getState() { return curState; }
    int getStatusCode() { return statusCode; }
    string getHeaderValueByKey(string key);
    
/*----------------------------------------------------------------------------*/
private:
    enum Method method;
    char *uri;
    int version;
    vector<HTTPHeader*> headers;
    
    char *content;
    int contentLength;
    
    int statusCode;
    enum State curState;
    
    int isNew;
    int isCGI;
    
    // char *exePath;
    // DLL *envp;

    /* Private methods */
    char* nextToken(char *, char *);
    void httpParseLine(char *, ssize_t , ssize_t *);
    void setRequestError(enum StatusCode );

    int isValidRequest();
    
    //void buildENVP();
    //void insertENVP(char*, char*);
    string getMethodString(enum Method);
};


struct methodEntry {
    HTTPRequest::Method m;
    string s;
};


#endif  /* __HTTPREQUEST_H__ */
