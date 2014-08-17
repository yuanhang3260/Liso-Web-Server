#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "FileIO.h"
#include "Logger.h"

string FileIO::lockFile;
string FileIO::wwwFolder;
string FileIO::CGIFolder;

/** @brief init FileIO */
void FileIO::initFileIO(string _lockFile, string _wwwFolder, string _CGIFolder)
{
    FileIO::lockFile = _lockFile;
    FileIO::wwwFolder = _wwwFolder;
    FileIO::CGIFolder = _CGIFolder;
}


/** @brief Class FileIO Constructor */
FileIO::FileIO(string uri)
{
    if(uri.at(uri.length() - 1) == '/') {
        path = createPath(FileIO::wwwFolder, uri, "index.html");
    }
    else {
        path = createPath(FileIO::wwwFolder, uri, "");
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
    if(fd < 0) 
    {
        cout << "Error: Open file " << path << " failed\n";
        return NULL;
    }
    type = getFileTypeFromName();
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
enum FileIO::MIMEType FileIO::getFileTypeFromName()
{
    if(path.length() < 4) {
        return OTHER;
    }
    else
    {
        size_t pos = path.find_last_of(".");
        if (pos == string::npos) {
            return OTHER;
        }

        string type = path.substr(pos + 1);
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





