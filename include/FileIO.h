#ifndef __FILEIO_H_
#define __FILEIO_H_ 


#include <time.h>
#include <string>
#include <iostream>

using namespace std;


class FileIO 
{
/*----------------------------------------------------------------------------*/
public:
    enum MIMEType {
        HTML,
        CSS,
        JPEG,
        PNG,
        GIF,
        OTHER,
    };

    static string lockFile;
    static string wwwFolder;
    static string CGIFolder;
    
    static void initFileIO(string, string, string);

    /** Constructor and Destructor */
    FileIO(string uri);
    ~FileIO();

    char* loadFile();

    int getFile() { return fd; }
    string getPath() { return path; }
    string getType();
    int getSize() { return length; }
    time_t getLastMod() { return lastMod; }

    string getCGIPath() { return CGIFolder; }
    string getWWWFolder() { return wwwFolder; }
    string getLockFile() { return lockFile; }

    static string createPath(string dir, string uri, string fileName);

/*----------------------------------------------------------------------------*/
private:
    int fd;
    string path;
    enum MIMEType type;
    int length;
    time_t lastMod;

    enum MIMEType getFileTypeFromName();
};








#endif /* __FILEIO_H_ */