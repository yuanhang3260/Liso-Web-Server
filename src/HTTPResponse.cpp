/** @brief file  HTTPResponse class routines
 *  @author Hang Yuan (hangyuan)
 *  @bug No bugs found yet
 */

#include <sstream>
#include <fcntl.h>

#include "HTTPResponse.h"


/** @brief Class HTTPResponse Constructor */
HTTPResponse::HTTPResponse()
{
    isCGI = 0;
    statusLine = "";
    file = NULL;
    headerBuffer = NULL;
    fileBuffer = NULL;
    headerPtr = 0;
    filePtr = 0;
    maxHeaderPtr = 0;
    maxFilePtr = 0;
    //close = 0;
    CGIout = -1;
    pid = 0;
}


/** @brief Class HTTPResponse Destructor */
HTTPResponse::~HTTPResponse()
{
    if (file != NULL) {
        free(file);
    }

    if (headerBuffer) {
        free(headerBuffer);
    }

    for (size_t i = 0; i < headers.size(); i++) {
        delete headers[i];
    }
    headers.clear();

    // if(pid != 0) 
    // {
    //     int status;
    //     if(-1 == waitpid(pid, &status, 0)) {
    //         printf("Error reaping child\n");
    //     } 
    //     else {
    //         printf("-PID");
    //     }
    // }
}


/** @brief Build Response */
void HTTPResponse::buildResponse(HTTPRequest *req)
{
    if(req->isCGIRequest()) {
        buildCGIResponse(req); 
    }
    else {
        buildHTTPResponse(req);
    }
    return;
}


int HTTPResponse::buildHTTPResponse(HTTPRequest *req)
{
    /* Add general headers */
    headers.push_back( new HTTPHeader("Date", getHTTPDate(time(0))) );
    headers.push_back( new HTTPHeader("Server", "Liso/v1.0") );

    int errorFlag = addStatusLine(req);
    if(errorFlag == 1)
    {
        /* Serve Error request */
        headers.push_back( new HTTPHeader("Connection", "close") );
        //close = 1;
    }
    else
    {
        switch(req->getMethod())
        {
            case HTTPRequest::POST:

            case HTTPRequest::GET:
                if (file != NULL) {
                    free(file);
                }
                printf("Load file for GET\n");
                file = new FileIO(req->getURI());
                fileBuffer = file->loadFile();
                if (fileBuffer == NULL) {
                    fprintf(stderr, "Load File %s failed\n", (file->getPath()).c_str());
                    statusLine = "HTTP/1.1 404 NOT FOUND\r\n";
                }
                maxFilePtr = file->getSize();
                
            case HTTPRequest::HEAD : {
                /* Connection */
                string val = req->getHeaderValueByKey("Connection");
                if(val.compare("close") == 0) {
                    headers.push_back( new HTTPHeader("Connection", "close") );
                    //close = 1;
                }
                else {
                    headers.push_back( new HTTPHeader("Connection", "keep-alive") );
                }
                /* Content size */
                ostringstream size;
                size << (file->getSize());
                headers.push_back( new HTTPHeader("Content-length", size.str()) );
                
                /* file size */
                headers.push_back( new HTTPHeader("Content-type",
                                                  file->getType()) );
                /* Last-modified */
                headers.push_back( new HTTPHeader("Last-modified",
                                                  getHTTPDate(file->getLastMod())) );
                break;
            }
            default:
                break;
        }
    }
    printf("Fill header...\n");
    fillHeader();
    return 0;
    //print();
}


/** @brief fill header 
 *  @return void
 */
void HTTPResponse::fillHeader()
{
    string responseOrder[] = {
        "Connection",
        "Date",
        "Server",
        "Content-length",
        "Content-type",
        "Last-modified",
    };

    string headerContent = statusLine;

    for(unsigned i = 0; i < sizeof(responseOrder) / sizeof(char *); i++) 
    {
        string key = responseOrder[i];
        string value = getHeaderValueByKey(key);
        if(value != "") {
            headerContent += (key + ": " + value + "\r\n");
        }
    }
    headerContent += "\r\n";
    maxHeaderPtr = headerContent.length();

    headerBuffer = new char[headerContent.length() + 1];
    strcpy(headerBuffer, headerContent.c_str());
}


int HTTPResponse::buildCGIResponse(HTTPRequest *req)
{
    pid_t pid;
    int stdin_pipe[2];
    int stdout_pipe[2];
    if (pipe(stdin_pipe) < 0 ) {
        printf("Error piping for stdin.\n");
        return -1;
    }
    if (pipe(stdout_pipe) < 0 ) {
        printf("Error piping for stdout.\n");
        return -1;
    }

    pid = fork();
    /* not good */
    if (pid < 0) {
        printf("Error forking.\n");
        return -1;
    }
    /* child, setup environment, execve */
    if (pid == 0) 
    {
        char *pathCGI = (char*) FileIO::createPath(".", req->getURI(), "").c_str();
        char *argvCGI[] = {pathCGI, NULL};
        //char **envpCGI = fillENVP(req);
        printf("To execve [%s]\n", pathCGI);
        close(stdout_pipe[0]);
        close(stdin_pipe[1]);
        dup2(stdout_pipe[1], fileno(stdout));
        dup2(stdin_pipe[0], fileno(stdin));

        if (execv(pathCGI, argvCGI)) {
            printf("Error execve [%s]\n", pathCGI);
            return -1;
        }
    }
    /* parent process */
    if (pid > 0) {
        printf("Parent: Heading to select() loop.\n");
        close(stdout_pipe[1]);
        close(stdin_pipe[0]);
        // if (req->contentLength > 0) 
        // {
        //     if (write(stdin_pipe[1],
        //               req->content,
        //               req->contentLength) < 0) {
        //         printf("Parent: Error writing to child.\n");
        //         close(stdin_pipe[1]);
        //         return -1;
        //     }
        // }
        close(stdin_pipe[1]);

        CGIout = stdout_pipe[0];
        return 0;
    }
    printf("Ooops! Unreachable code reached!\n");
    return -1;
}


// char** HTTPResponse::fillENVP(HTTPRequest *req)
// {
//     DLL *envpList = req->envp;
//     int size = envpList->size;
//     int i = 0;
//     char **ret = malloc((size + 1) * sizeof(char *));
//     char *line;
//     for(i = 0; i < size; i++) {
//         headerEntry *hd = (headerEntry *)getNodeDataAt(envpList, i);
//         line = malloc(strlen(hd->key) + strlen("=") + strlen(hd->value) + 1);
//         strcpy(line, hd->key);
//         strcat(line, "=");
//         strcat(line, hd->value);
//         ret[i] = line;
//         printf("%s\n", line);
//     }
//     ret[i] = NULL;
//     return ret;
// }


void HTTPResponse::print()
{
    printf("----Begin New Response----\n");
    printf("%s\n", headerBuffer);
    printf("----End New Response----\n");
}


/** @brief write response to write buffer */
int HTTPResponse::writeResponse(char *buf, ssize_t maxSize, ssize_t *retSize)
{
    size_t hdPart_size = 0;
    size_t fdPart_size = 0;
    
    if(maxSize <= 0) {
        *retSize = 0;
        return 0;
    }
    
    printf("Remain header = %d, file = %d\n", maxHeaderPtr - headerPtr, maxFilePtr - filePtr);
    if(headerPtr + maxSize <= maxHeaderPtr) {
        hdPart_size = maxSize;
    }
    else
    {
        hdPart_size = maxHeaderPtr - headerPtr;
        fdPart_size = maxSize - hdPart_size;
        if(filePtr + fdPart_size > maxFilePtr) {
            fdPart_size = maxFilePtr - filePtr;
        }
    }
    
    memcpy(buf, headerBuffer + headerPtr, hdPart_size);
    memcpy(buf + hdPart_size, fileBuffer + filePtr, fdPart_size);
    headerPtr += hdPart_size;
    filePtr += fdPart_size;
    *retSize = hdPart_size + fdPart_size;
    return (headerPtr == maxHeaderPtr && filePtr == maxFilePtr);
}


/** @brief create response building time */
string HTTPResponse::getHTTPDate(time_t tmraw)
{
    char dateStr[64];
    struct tm ctm = *gmtime(&tmraw);
    strftime(dateStr, 64, "%a, %d %b %Y %H:%M:%S %Z", &ctm);
    return (string)(dateStr);
}


/** @brief add Status Line */
int HTTPResponse::addStatusLine(HTTPRequest *req)
{
    int errorFlag = 0;
    
    statusLine = "HTTP/1.1 200 OK\r\n";
    if(req->getState() == HTTPRequest::ParsedError) 
    {
        errorFlag = 1;
        switch((enum HTTPRequest::StatusCode)req->getStatusCode()) 
        {
            case HTTPRequest::BAD_REQUEST:
                statusLine = "HTTP/1.1 400 BAD REQUEST\r\n";
                break;
            case HTTPRequest::NOT_FOUND:
                statusLine = "HTTP/1.1 404 NOT FOUND\r\n";
                break;
            case HTTPRequest::LENGTH_REQUIRED:
                statusLine = "HTTP/1.1 411 LENGTH REQUIRED\r\n";
                break;
            case HTTPRequest::INTERNAL_SERVER_ERROR:
                statusLine = "HTTP/1.1 500 INTERNAL SERVER ERROR\r\n";
                break;
            case HTTPRequest::NOT_IMPLEMENTED:
                statusLine = "HTTP/1.1 501 NOT IMPLEMENTED\r\n";
                break;
            case HTTPRequest::SERVICE_UNAVAILABLE:
                statusLine = "HTTP/1.1 503 SERVICE UNAVAILABLE\r\n";
                break;
            case HTTPRequest::HTTP_VERSION_NOT_SUPPORTED:
                statusLine = "HTTP/1.1 505 HTTP VERSION NOT SUPPORTED\r\n";
                break;
            default:
                statusLine = "HTTP/1.1 500 INTERNAL SERVER ERROR\r\n";
                break;
        }
    }
    
    return errorFlag;
}


/** if it is a CGI response */
int HTTPResponse::isCGIResponse()
{
    return isCGI == 1;
}

/** get a header value by its key from the header list */
string HTTPResponse::getHeaderValueByKey(string key)
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