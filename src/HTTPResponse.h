#ifndef __HTTPRESPONSE_H__
#define __HTTPRESPONSE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <wait.h>
#include <string>
#include <iostream>
#include <vector>
#include "HTTPRequest.h"

using namespace std;

class HTTPResponse 
{
/*----------------------------------------------------------------------------*/
public:
    /** Constructor and Destructor */
    HTTPResponse();
    ~HTTPResponse();

    int writeResponse(char *, ssize_t , ssize_t *);
    void buildResponse(HTTPRequest *);
    void buildHTTPResponse(HTTPRequest *);
    //int buildCGIResponse(HTTPRequest *);

    /** Get close status */
    int toClose() { return close; }
    int isCGIResponse();
    string getHeaderValueByKey(string key);

/*----------------------------------------------------------------------------*/
private:
    int isCGI;
    /* HTTP Response */
    
    string statusLine;
    vector<HTTPHeaders*> headers;
    // fileMetadata *fileMeta;

    //Write Buffer related
    char *headerBuffer;
    char *fileBuffer;
    size_t headerPtr;
    size_t filePtr;
    size_t maxHeaderPtr;
    size_t maxFilePtr;
    
    /* CGI Response */
    int CGIout;
    pid_t pid;

    int close;

    /* Private methods */
    void fillHeader();
    //char **fillENVP(HTTPRequest *);
    string getHTTPDate(time_t);
    int addStatusLine(HTTPRequest*);
    void printResponse(HTTPRequest*);

};

#endif  /* __HTTPRESPONSE_H__ */
