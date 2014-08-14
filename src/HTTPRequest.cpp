/** @file HTTPRequest.cpp
 *  @brief http request class and parser routines
 *  @author Hang Yuan (hangyuan)
 *  @bug No bugs found yet
 */

#include "HTTPRequest.h"


/** @brief Class HTTPRequest constructor */
HTTPRequest::HTTPRequest(int port, char *addr, int https)
{
    //char buf[16];
    method = UNIMPLEMENTED;
    uri = NULL;
    version = 0;
    
    content = NULL;
    contentLength = 0;
    
    statusCode = 0;
    curState = requestLine;
    
    isNew = 1;
    isCGI = -1;

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
    //Logger::log("Staring free Req: ");
    free(uri);
    free(content);

    for (int i = 0; i < headers.size(); i++) {
        delete &(headers[i]);
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
        Logger::log("%c", *buf);
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
enum Status HTTPRequest::httpParse( char *bufPtr, 
                                    ssize_t *size, 
                                    int full )
{
    if(curState == requestDone) 
    {
        *size = 0;
        Logger::log("Existing ReqObj. No Parsing Needed\n");
        return Parsed;
    }
    if(curState == requestError)
    {
        *size = 0;
        Logger::log("RequestError. No Parsing Needed\n");
        return ParseError;
    }

    ssize_t curSize = *size;
    char *bufEnd = bufPtr + curSize;
    char *thisPtr = bufPtr;
    char *nextPtr;
    ssize_t parsedSize;
    
    /* Begin Parsing in a state machine */
    Logger::log("Start parsing %d bytes\n", *size);
    while(1)
    {
        if(curState == content) {
            nextPtr = bufEnd;
        } 
        else 
        {
            /* get next line in the read buffer */
            nextPtr = nextToken(thisPtr, bufEnd);
            if(nextPtr == NULL && full)
            {
                /* Reject header longer than buffer size */
                setRequestError(BAD_REQUEST);
                break;
            } 
            else {
                full = 0;
            }
        }
        if(nextPtr != NULL) 
        {
            parsedSize = (size_t)(nextPtr - thisPtr);
            
            /* httpParseLine is a state machine switching in states: 
             * [requestLine], [headerLine], [content], [requestDone], [requestError]
             */
            httpParseLine(thisPtr, parsedSize, &parsedSize);
            Logger::log("One line parsed\n");
        }
        else {
            break;
        }
        if(curState == requestError ) {
            break;
        }
        /* Prepare for next line */
        thisPtr = thisPtr + parsedSize;
        if(thisPtr >= bufEnd) {
            break;
        }
        if(curState == requestDone) {
            break;
        }
    }
    /* clean up parsed buffer */
    if(thisPtr < bufEnd) {
        *size = thisPtr - (bufEnd - curSize);
    }
    if(isNew && *size > 0) {
        isNew = 0;
    }

    /* Set return status */
    if(curState == requestError) 
    {
        Logger::log("Parsing result: Error\n");
        return ParseError;
    } 
    else if(curState == requestDone) 
    {
        Logger::log("Parsing result: Done\n");
        print();
        return Parsed;
    } 
    else
    {
        Logger::log("Parsing result: Parsing .. \n");
        return Parsing;
    }


}


/** parse a request line */
void HTTPRequest::httpParseLine( char *line, 
                                 ssize_t lineSize, 
                                 ssize_t *parsedSize )
{
    static struct methodEntry methodTable[] = 
    {
        {GET, "GET"},
        {HEAD, "HEAD"},
        {POST, "POST"},
    };

    switch(curState) {
    case requestLine: 
    {
        char *method = malloc(lineSize + 1);
        char *uri = malloc(lineSize + 1);
        char *lineStr = calloc(lineSize + 1, 1);
        memcpy(lineStr, line, lineSize);
        int version1, version2;
        
        if(sscanf(lineStr, "%s %s HTTP/%d.%d\r\n", 
                   method, uri, &version1, &version2) != 4) 
        {
            free(lineStr);
            setRequestError(BAD_REQUEST);
        }
        else
        {
            free(lineStr);
            Logger::log("Parsed Status Line: Method = %s, URI = %s, Version = %d\n",
                         method, uri, version2);
            int numMethods = sizeof(methodTable) / sizeof(methodEntry);
            int i;
            for(i = 0; i < numMethods; i++) 
            {
                if(strcmp(methodTable[i].s, method) == 0) {
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
            else {
                uri = realloc(uri, strlen(uri) + 1);
                uri = uri;
                version = version2;
                curState = headerLine;
            }
        }
        /* Clean up */
        free(method);
        if(curState == requestError) {
            free(uri);
        }
    }
    break;
    case headerLine:
    {
        Logger::log("curState: headerLine\n");
        if(lineSize == 2 && line[0] == '\r' && line[1] == '\n') 
        {
            Logger::log("Header Close line\n");
            if(isValidRequest(req)) 
            {
                Logger::log("isValid\n");
                if(method == GET || method == HEAD) {
                    curState = requestDone;
                } 
                else if(method == POST) {
                    curState = content;
                } 
                else {
                    curState = requestError;
                }
            } 
            else {
                setRequestError(LENGTH_REQUIRED);
            }
        } 
        else 
        {
            Logger::log("KeyValue header\n");
            char *key = calloc(lineSize);
            char *value = calloc(lineSize);
            char *strLine = calloc(lineSize + 1, 1);
            memcpy(strLine, line, lineSize);
            if(sscanf(strLine, "%[a-zA-Z0-9-]:%[^\r\n]", key, value) != 2) {
                setRequestError(BAD_REQUEST);
            }
            else {
                headers.push_back(HTTPHeader(key, value));
            }
            free(strLine);
            free(key);
            free(value);
        }
    }
    break;
    case content: {
        // char *lengthStr = getValueByKey(header, "content-length");
        // int length = atoi(lengthStr);
        // int curLength = contentLength;
        // if(curLength == length) {
        //     curState = requestDone;
        // }
        // if(length - curLength <= lineSize) 
        // {
        //     Logger::log("Stat content length=%d, curLength=%d, lineSize=%d \n", length, curLength, lineSize);
        //     size_t readSize = length - curLength;
        //     content = realloc(content, length + 1);
        //     content[length] = '\0';
        //     memcpy(content + curLength, line, readSize);
        //     contentLength = length;
        //     curState = requestDone;
        //     *parsedSize = readSize;
        //     Logger::log("Got content %d done\n", readSize);
        // } 
        // else 
        // {
        //     content = realloc(content, curLength + lineSize + 1);
        //     content[curLength+lineSize] = '\0';
        //     memcpy(content + curLength, line, lineSize);
        //     contentLength = curLength + lineSize;
        //     curState = content;
        //     Logger::log("Got content %d in middle\n", lineSize);
        // }
        // break;
    }
    case requestError:
    case requestDone:
    default:
        break;
    }
}


/** convert method to string */
string getMethodString(enum Method method)
{
    switch(method) 
    {
        case GET:
            return "GET";
        case HEAD:
            return "HEAD";
        case POST:
            return "POST";
        default:
            return "";
    }
    return NULL;
}


/** get a header value by its key from the header list */
string HTTPRequest::getHeaderValueByKey(string key)
{
    for (int i = 0; i < headers.size(); i++) 
    {
        if (headers[i].getKey().compare(key) == 0)
        {
            return headers[i].getValue();
        }
    }
    return NULL;
}


/** check this request is a valid request */
int HTTPRequest::isValidRequest()
{
    if(curState == requestError) {
        return 0;
    }
    switch(method) 
    {
        case POST: {
            string length = getHeaderValueByKey("content-length");
            if(length != NULL) {
                return 1;
            }
        }
        case HEAD:
        case GET: {
            string host = getHeaderValueByKey("host");
            if(host != NULL) {
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
    curState = requestError;
    statusCode = (int)code;
    Logger::log("Parse Error with code = %d\n", (int)code);
}


/** print the request */
void HTTPRequest::print()
{
    Logger::log("--- Start Printing Request ---\n");

    Logger::log("-Method: ");
    switch(method) 
    {
        case GET:
            Logger::log("GET\n");
            break;
        case HEAD:
            Logger::log("HEAD\n");
            break;
        case POST:
            Logger::log("POST\n");
            break;
        default:
            Logger::log("OTHER\n");
            break;
    }
    Logger::log("-URI: %s\n", uri);
    Logger::log("-Version: HTTP/1.%d\n", version);
    for (int i = 0; i < headers.size(); i++) {
        headers[i].print();
    }        

    if(contentLength > 0) {
        Logger::log("-Content : %s\n", content);
    }
    Logger::log("--- End Printing Request ---\n");
}






