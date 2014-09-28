/** @file HTTPRequest.cpp
 *  @brief http request class and parser routines
 *  @author Hang Yuan (hangyuan)
 *  @bug No bugs found yet
 */

#include "HTTPRequest.h"
#include "ClientConnection.h"
#include "MyString.h"


/** @brief Class HTTPRequest constructor */
HTTPRequest::HTTPRequest(ClientConnection *_client, int port, const char *addr, int https)
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
    isCGI = 0;

    (void)addr;

    client = _client;

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
    if (uri != NULL) {
        free(uri);
    }
    if (content != NULL) {
        free(content);
    }

    for (size_t i = 0; i < headers.size(); i++) {
        delete headers[i];
    }
    headers.clear();
    
    //freeList(envp);
}


/** @brief parse request */
void HTTPRequest::httpParse(char *bufPtr, ssize_t *size)
{
    ssize_t curSize = *size;
    char *thisLine, *nextLine;
    
    /* find the first \r\n\r\n marking the end of one request */
    char* request_end = MyString::strnstr(bufPtr, "\r\n\r\n", curSize);
    if (request_end == NULL)
    {
        /* no complete request in the read buffer. */
        if (client->isFull())
        {   
            // TODO: really should do that? 
            // No... should continue parsing. Parse to the last line in current buffer
            // and continue reading data from socket. 
            printf("Parsing result: Error - Abort request longer than buffer size\n");
            state = ParsedError;
            /* reset parse status */
            parseStatus = requestLine;
        }
        else {
            /* Do nothing. Wait for more reqeust reading. */
            *size = 0;
        }
    }
    else
    {
        int req_len = request_end - bufPtr + strlen("\r\n\r\n");
        char* request = new char[req_len + 1];
        
        /* copy the request */
        memcpy(request, bufPtr, req_len);
        request[req_len] = '\0';

        /* Begin Parsing one request */
        printf("Begin Parsing %d bytes reqeust: Start ...\n", curSize);
        printf("%s", request);

        thisLine = request;
        nextLine = NULL;
        while (req_len > 0)
        {
            /* get next line in the read buffer */
            nextLine = MyString::strnstr(thisLine, "\r\n", req_len) + strlen("\r\n");
            int line_length = nextLine - thisLine;

            /* parse one line in a state machine */
            httpParseLine(thisLine, line_length, &line_length);
            if (parseStatus == requestDone || parseStatus == requestError) {
                break;
            }

            /* go to next Line */
            thisLine = nextLine;
            /* decrease request size */
            req_len -= line_length;
        }

        /* return parsed request length. It will be removed from read buffer */
        *size = strlen(request);
    }
    
    /* Set request state */
    if(parseStatus == requestError)
    {
        printf("Parsing result: Error\n");
        state = ParsedError;
        /* reset parse status */
        parseStatus = requestLine;
    }
    else if(parseStatus == requestDone)
    {
        printf("Parsing result: Done\n");
        /* print parsed result */
        state = ParsedCorrect;
        /* reset parse status */
        parseStatus = requestLine;
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
                isCGI = checkIsCGIRequest(uri);
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
            if (isValidRequest()) 
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
        printf("[content Line] - \n");
        string lengthStr = getHeaderValueByKey("Content-Length");
        int length = atoi(lengthStr.c_str());
        int curLength = contentLength;
        if (curLength == length) {
            parseStatus = requestDone;
        }

        if (length - curLength <= lineSize)
        {
            printf("Stat content length = %d, curLength = %d, lineSize=%d \n", length, curLength, lineSize);
            ssize_t readSize = length - curLength;
            content = (char*) realloc(content, length + 1);
            content[length] = '\0';
            memcpy(content + curLength, line, readSize);
            contentLength = length;
            parseStatus = requestDone;
            *parsedSize = readSize;
            printf("Got content %d done\n", readSize);
        }
        else
        {
            content = (char*) realloc(content, curLength + lineSize + 1);
            content[curLength+lineSize] = '\0';
            memcpy(content + curLength, line, lineSize);
            contentLength = curLength + lineSize;
            parseStatus = contentLine;
            printf("Got content %d receiving\n", lineSize);
        }
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

int HTTPRequest::checkIsCGIRequest(char* uri)
{
    unsigned int len = 0;
    while (*(uri + len) != '\0') {
        len++;
    }
    if (len < strlen("/cgi-bin/")) {
        return 0;
    }
    if (strncmp(uri, "/cgi-bin/", 8) != 0) {
        return 0;
    }
    //printf("uri = %s\n", uri);
    return 1;
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






