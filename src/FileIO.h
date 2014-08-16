#ifndef __FILEIO_H_
#define __FILEIO_H_ 

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

    char *loadFile(fileMetadata *fm);

    int getFile() { return fd; }
    string getPath() { return path; }
    string getType();
    int getSize() { return length; }
    time_t getLastMod() { return lastMod; }

    string getCGIPath() { return CGIFolder; }
    string getWWWFolder() { return wwwFolder; }
    string getLockFile() { return lockFile; }

/*----------------------------------------------------------------------------*/
private:
    int fd;
    string path;
    enum MIMEType type;
    int length;
    time_t lastMod;

    string createPath(string dir, string uri, string fileName);
    enum MIMEType getFileTypeFromName(string path);
};








#endif /* __FILEIO_H_ */