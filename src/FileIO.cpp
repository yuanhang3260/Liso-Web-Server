#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/stat.h>
#include "FileIO.h"
#include "Logger.h"


int initFileIO(char *lockFile, char *wwwFolder, char *CGIFolder)
{
    if(lockFile == NULL || wwwFolder == NULL || CGIFolder == NULL) {
        return -1;
    }
    _lockFile = lockFile;
    _wwwFolder = wwwFolder;
    _CGIFolder = CGIFolder;
    return 0;
}

void freeFileMeta(fileMetadata *fm)
{
    if(fm != NULL) {
        free(fm->path);
        free(fm);
    }
}

fileMetadata *prepareFile(char *uri, char *mode)
{
    char *path;
    struct stat fileStat;
    FILE *fd;
    fileMetadata *fm;
    if(uri[strlen(uri)-1] == '/') {
        path = createPath(_wwwFolder, uri, "index.html");
    } else {
        path = createPath(_wwwFolder, uri, NULL);
    }
    logger(Logger::LOG_DEBUG, "FilePath:[%s]\n", path);
    if(stat(path, &fileStat) != 0) {
        free(path);
        return NULL;
    }else{
        if(S_ISDIR(fileStat.st_mode)){
            free(path);
            path=createPath(_wwwFolder, uri, "/index.html");
        }
    }
    
    fd = fopen(path, mode);
    if(fd == NULL) {
        free(path);
        logger(Logger::LOG_DEBUG, "Failed open\n");
        return NULL;
    } else {
        logger(Logger::LOG_DEBUG, "Opened\n");
        fm = malloc(sizeof(fileMetadata));
        fm->fd = fd;
        fm->path = path;
        fm->type = getFileType(path);
        fm->lastMod = fileStat.st_mtime;
        fseek(fd, 0, SEEK_END);
        fm->length = ftell(fd);
        rewind(fd);
        return fm;
    }
}

char *loadFile(fileMetadata *fm)
{
    char *buffer = calloc(fm->length + 1, 1);
    size_t retSize;
    retSize=fread(buffer, fm->length, 1, fm->fd);
    if(retSize==0){
        buffer[0]='\0';
    }
    fclose(fm->fd);
    return buffer;
}

char *getCGIPath(){
    return _CGIFolder;
}

char *getFilePath(fileMetadata *fm)
{
    return fm->path;
}

char *getContentType(fileMetadata *fm)
{
    switch(fm->type) {
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

char *getContentLength(fileMetadata *fm)
{
    char *buffer = malloc(512);
    sprintf(buffer, "%d", fm->length);
    return buffer;
}

time_t getLastMod(fileMetadata *fm)
{
    return fm->lastMod;
}


/* Private methods */
enum MIMEType getFileType(char *path)
{
    if(strlen(path) < 4) {
        return OTHER;
    } else {
        char *ext = strrchr(path, '.');
        if(ext == NULL) {
            return OTHER;
        }
        if(strcmp(ext, ".html") == 0) {
            return HTML;
        }
        if(strcmp(ext, ".htm") == 0) {
            return HTML;
        }
        if(strcmp(ext, ".css") == 0) {
            return CSS;
        }
        if(strcmp(ext, ".png") == 0) {
            return PNG;
        }
        if(strcmp(ext, ".jpeg") == 0) {
            return JPEG;
        }
        if(strcmp(ext, ".gif") == 0) {
            return GIF;
        }
        return OTHER;
    }

}

char *createPath(char *dir, char *path, char *fileName)
{
    int dirLength = (dir == NULL) ? 0 : strlen(dir);
    int pathLength = (path == NULL) ? 0 : strlen(path);
    int fileLength = (fileName == NULL) ? 0 : strlen(fileName);
    char *fullPath = malloc(dirLength + pathLength + fileLength + 1);
    if(dir != NULL) {
        strcpy(fullPath, dir);
    }
    if(path != NULL) {
        strcat(fullPath, path);
    }
    if(fileName != NULL) {
        strcat(fullPath, fileName);
    }
    return fullPath;
}





