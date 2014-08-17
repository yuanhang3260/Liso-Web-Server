/** @file HTTPRequest.cpp
 *  @brief http request class and parser routines
 *  @author Hang Yuan (hangyuan)
 *  @bug No bugs found yet
 */

#include "HTTPRequest.h"


/** @brief Class HTTPRequest constructor */
HTTPRequest::HTTPRequest(int port, const char *addr, int https)
{
    //char buf[16];
    method = UNIMPLEMENTED;
    uri = NULL;
    version = 0;
    
    content = NULL;
    contentLength = 0;
    
    statusCode = 0;
    state = Init;
    parseStatus = requestLine;
    
    isNew = 1;
    isCGI = -1;

    (void)addr;

    // envp = malloc(sizeof(DLL));
    // initList(envp, compareHeaderEntry, freeHeaderEntry, NULL);
    // insertENVP(newObj, "REMOTE_ADDR", addr);
    // snprintf(buf, 16, "%d", port);
    // insertENVP(newObj, "SERVER_PORT", buf);
    // if(https) {
    //     insertENVP(newObj, "HTTPS", "on");
    // } else {
    //     insertENVP(newObj, "HTTPS", "off");
    // }
}


/** @brief Class HTTPRequest destructor */
HTTPRequest::~HTTPRequest()
{
    //printf("Staring free Req: ");
    free(uri);
    free(content);

    for (size_t i = 0; i < headers.size(); i++) {
        delete headers[i];
    }
    headers.clear();
    
    //freeList(envp);
}


/** get the next line */
char* HTTPRequest::nextToken(char *buf, char *bufEnd)
{
    char *next = NULL;
    for(; buf < bufEnd; buf++) 
    {
        //printf("%c", *buf);
        if(buf[0] == '\r' && buf[1] == '\n') 
        {
            next = buf;
            break;
        }
    }

    if(next == NULL) {
        return NULL;
    } 
    else {
        return next + 2;
    }
}


/** @brief parse request */
void HTTPRequest::httpParse(char *bufPtr, ssize_t *size, int full)
{
    // if(state == ParsedCorrect) 
    // {
    //     *size = 0;
    //     printf("Existing Passed Request. No Parsing Needed\n");
    // }
    // if(state == ParsedError)
    // {
    //     *size = 0;
    //     printf("Existing Parsed Error. No Parsing Needed\n");
    // }
    /* reset parse status */
    parseStatus = requestLine;

    ssize_t curSize = *size;
    char *bufEnd = bufPtr + curSize;
    char *thisPtr = bufPtr;
    char *nextPtr;
    ssize_t parsedSize;
    
    /* Begin Parsing in a state machine */
    printf("Parsing %d bytes: Start ...\n", *size);
    //printf("%s\n", bufPtr);
    while(1)
    {
        if(parseStatus == contentLine) {
            nextPtr = bufEnd;
        }
        else 
        {
            /* get next line in the read buffer */
            nextPtr = nextToken(thisPtr, bufEnd);
            if(nextPtr == NULL && full)
            {
                // Reject header longer than buffer size */
                setRequestError(BAD_REQUEST);
                break;
            } 
            else {
                full = 0;
            }
        }
        if(nextPtr != NULL) 
        {
            parsedSize = (ssize_t)(nextPtr - thisPtr);
            
            /* httpParseLine is a state machine switching in states: 
             * [requestLine], [headerLine], [content], [requestDone], [requestError]
             */
            httpParseLine(thisPtr, parsedSize, &parsedSize);
        }
        else {
            break;
        }
        if(parseStatus == requestError ) {
            break;
        }
        /* Prepare for next line */
        thisPtr = thisPtr + parsedSize;
        if(thisPtr >= bufEnd) {
            break;
        }
        if(parseStatus == requestDone) {
            break;
        }
    }

    /* refresh parsed buffer */
    if(thisPtr < bufEnd) {
        *size = thisPtr - (bufEnd - curSize);
    }
    if(isNew && *size > 0) {
        isNew = 0;
    }
    
    /* Set return status */
    if(parseStatus == requestError)
    {
        printf("Parsing result: Error\n");
        state = ParsedError;
    } 
    else if(parseStatus == requestDone) 
    {
        printf("Parsing result: Done\n");
        /* print parsed result */
        //print();
        state = ParsedCorrect;
    }
    else
    {
        printf("Parsing result: Parsing .. \n");
        state = Parsing;
    }

}


/** parse a request line */
void HTTPRequest::httpParseLine( char *_line, 
                                 ssize_t lineSize, 
                                 ssize_t *parsedSize )
{
    static struct methodEntry methodTable[] = 
    {
        {GET, "GET"},
        {HEAD, "HEAD"},
        {POST, "POST"},
    };

    char *line = new char[(ssize_t)lineSize];
    strncpy(line, _line, (ssize_t)lineSize);
    printf("[Parse line]: %s", line);

    switch (parseStatus) {
    case requestLine: 
    {
        //printf("[requst Line] - ");
        char* _method = new char[lineSize + 1];
        char* _uri = new char[lineSize + 1];
        int version1, version2;
        
        if(sscanf(line, "%s %s HTTP/%d.%d\r\n", 
                   _method, _uri, &version1, &version2) != 4) 
        {
            delete[] _method;
            delete[] _uri;
            setRequestError(BAD_REQUEST);
        }
        else
        {
            // printf("Method = %s, URI = %s, Version = %d\n",
            //        _method, _uri, version2);
            int numMethods = sizeof(methodTable) / sizeof(methodEntry);
            for(int i = 0; i < numMethods; i++) 
            {
                if(strcmp(methodTable[i].s.c_str(), _method) == 0) {
                    method = methodTable[i].m;
                    break;
                }
            }
            if(method == UNIMPLEMENTED) {
                setRequestError(NOT_IMPLEMENTED);
            } 
            else if(version1 != 1 || version2 != 1  ) {
                setRequestError(HTTP_VERSION_NOT_SUPPORTED);
            }
            else 
            {
                uri = new char[strlen(_uri) + 1];
                strncpy(uri, _uri, strlen(_uri) + 1);
                version = version2;
                parseStatus = headerLine;
            }
        }
        /* Clean up */
        delete[] _method;
        if(parseStatus == requestError) {
            delete[] uri;
        }
    }
    break;
    case headerLine:
    {
        //printf("[header Line] - ");
        if(lineSize == 2 && line[0] == '\r' && line[1] == '\n') 
        {
            printf("Header Close line\n");
            if(isValidRequest()) 
            {
                if(method == GET || method == HEAD) {
                    parseStatus = requestDone;
                } 
                else if(method == POST) {
                    parseStatus = contentLine;
                } 
                else {
                    parseStatus = requestError;
                }
            }
            else {
                setRequestError(LENGTH_REQUIRED);
            }
        } 
        else 
        {
            char *key = new char[lineSize];
            char *value = new char[lineSize];
            if(sscanf(line, "%[a-zA-Z0-9-]: %[^\r\n]", key, value) != 2) {
                setRequestError(BAD_REQUEST);
            }
            else {
                HTTPHeader *header = new HTTPHeader(key, value);
                headers.push_back(header);
            }
            //printf("%s: %s\n", key, value);
            free(key);
            free(value);
        }
    }
    break;
    case contentLine: {
        //printf("[content Line] - \n");
        // char *lengthStr = getValueByKey(header, "content-length");
        // int length = atoi(lengthStr);
        // int curLength = contentLength;
        // if(curLength == length) {
        //     parseStatus = requestDone;
        // }
        // if(length - curLength <= lineSize) 
        // {
        //     printf("Stat content length=%d, curLength=%d, lineSize=%d \n", length, curLength, lineSize);
        //     ssize_t readSize = length - curLength;
        //     content = realloc(content, length + 1);
        //     content[length] = '\0';
        //     memcpy(content + curLength, line, readSize);
        //     contentLength = length;
        //     state = requestDone;
        //     *parsedSize = readSize;
        //     printf("Got content %d done\n", readSize);
        // } 
        // else 
        // {
        //     content = realloc(content, curLength + lineSize + 1);
        //     content[curLength+lineSize] = '\0';
        //     memcpy(content + curLength, line, lineSize);
        //     contentLength = curLength + lineSize;
        //     state = content;
        //     printf("Got content %d in middle\n", lineSize);
        // }
        break;
    }
    case requestError:
        //printf("[request Error] - \n");
        break;
    case requestDone:
        //printf("[request done] - \n");
        break;
    default:
        //printf("[Unknown] - \n");
        break;
    }
    delete[] line;
}


/** convert method to string */
string HTTPRequest::getMethodString(HTTPRequest::Method method)
{
    switch(method) 
    {
        case HTTPRequest::GET:
            return "GET";
        case HTTPRequest::HEAD:
            return "HEAD";
        case HTTPRequest::POST:
            return "POST";
        default:
            return "";
    }
    return NULL;
}


/** get a header value by its key from the header list */
string HTTPRequest::getHeaderValueByKey(string key)
{
    //printf("%d headers \n", headers.size());
    for (size_t i = 0; i < headers.size(); i++) 
    {
        //cout<<"key = "<<headers[i]->getKey() <<endl;
        if (headers[i]->getKey().compare(key) == 0)
        {
            return headers[i]->getValue();
        }
    }
    return "";
}


/** check this request is a valid request */
// TODO: return ErrorCode or 0 if correct
int HTTPRequest::isValidRequest()
{
    if(parseStatus == requestError) {
        return 0;
    }
    switch(method) 
    {
        case POST: {
            string length = getHeaderValueByKey("Content-length");
            if(length != "") {
                return 1;
            }
        }
        case HEAD:
        case GET: {
            string host = getHeaderValueByKey("Host");
            if(host != "") {
                return 1;
            }
            break;
        }
        case UNIMPLEMENTED:
        default:
            return 0;
    }
    return 0;
}


/** set this request status as error */
void HTTPRequest::setRequestError(enum StatusCode code)
{
    parseStatus = requestError;
    statusCode = (int)code;
    printf("Parse Error with code = %d\n", (int)code);
}


/** print the request */
void HTTPRequest::print()
{
    printf("------- Printing Request -------\n");

    printf("[Method]  ");
    switch(method) 
    {
        case GET:
            printf("GET\n");
            break;
        case HEAD:
            printf("HEAD\n");
            break;
        case POST:
            printf("POST\n");
            break;
        default:
            printf("OTHER\n");
            break;
    }
    printf("[URI]     %s\n", uri);
    printf("[Versin]  HTTP/1.%d\n", version);
    for (size_t i = 0; i < headers.size(); i++) {
        headers[i]->print();
    }        

    if(contentLength > 0) {
        printf("-Content : %s\n", content);
    }
    printf("-------------------------------\n");
}






