#ifndef __FILEIO_H_
#define __FILEIO_H_ 

char *_lockFile;
char *_wwwFolder;
char *_CGIFolder;

enum MIMEType{
    HTML,
    CSS,
    JPEG,
    PNG,
    GIF,
    OTHER,
};

typedef struct fileMetadata {
   FILE *fd;
   char *path;
   enum MIMEType type;
   int length;
   time_t lastMod;
} fileMetadata;




/* Public methods */
int initFileIO(char *, char*, char *);

fileMetadata *prepareFile(char *, char*);
void freeFileMeta(fileMetadata *);
char *loadFile(fileMetadata *fm);

char* getContentType(fileMetadata *fm);
char* getFilePath(fileMetadata *fm);
char* getContentLength(fileMetadata *fm);
time_t getLastMod(fileMetadata *fm);

char *getCGIPath();


/* Private methods */
char *createPath(char *dir, char *path, char *fileName);
enum MIMEType getFileType(char *path);



#endif /* __FILEIO_H_ */