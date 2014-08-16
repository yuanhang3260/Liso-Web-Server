#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include "FileIO.h"
#include "Logger.h"


/** @brief init FileIO */
void FileIO::initFileIO(string _lockFile, string _wwwFolder, string _CGIFolder);
{
    lockFile = _lockFile;
    wwwFolder = _wwwFolder;
    CGIFolder = _CGIFolder;
}


/** @brief Class FileIO Constructor */
FileIO::FileIO(string uri)
{
    string path;
    struct stat fileStat;
    
    if(uri.at(uri.length() - 1) == '/') {
        path = createPath(wwwFolder, uri, "index.html");
    }
    else {
        path = createPath(wwwFolder, uri, "");
    }
    
    cout << "[File Path]: " << path << endl;
}


/** @brief Class FileIO Destructor */
FileIO::~FileIO()
{
}


/** @brief Load File into buffer (use mmap) */
char* FileIO::loadFile()
{
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) != 0) {
        return NULL;
    }

    fd = open(path.c_str(), O_RDONLY);
    if(fd == NULL) 
    {
        cout << "Error: Open file " << path << " failed\n";
        return NULL;
    }
    type = getFileTypeFromName(path);
    lastMod = fileStat.st_mtime;
    length = fileStat.st_size;
    
    /** map file to memory buffer */
    void* mem_addr = mmap(0, length, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mem_addr == (void*)-1) {
        close(fd);
        return NULL;
    }
    close(fd);

    return (char*)mem_addr;
}


/** @brief get file type */
string FileIO::getType()
{
    switch(type) 
    {
        case HTML:
            return "text/html";
        case CSS:
            return "text/css";
        case JPEG:
            return "image/jpeg";
        case PNG:
            return "image/png";
        case GIF:
            return "image/gif";
        default:
            return "application/octet-stream";
    }
}


/* Private methods */
enum MIMEType FileIO::getFileTypeFromName(string path)
{
    if(path.length() < 4) {
        return OTHER;
    }
    else
    {
        size_t pos = str.find_last_of(".");
        if (pos == string::npos) {
            return OTHER;
        }

        type = str.substr(found + 1);
        if (type.compare("html") == 0) {
            return HTML;
        }
        if (type.compare("htm") == 0) {
            return HTML;
        }
        if (type.compare("css") == 0) {
            return CSS;
        }
        if (type.compare("png") == 0) {
            return PNG;
        }
        if (type.compare("jpeg") == 0) {
            return JPEG;
        }
        if (type.compare("gif") == 0) {
            return GIF;
        }
        return OTHER;
    }

}


string FileIO::createPath(string dir, string path, string fileName)
{
    return dir + path + fileName;
}





