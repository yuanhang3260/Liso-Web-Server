#include "HTTPHeader.h"

// headerEntry *newHeaderEntry(char *key, char *value)
// {
//     if(key == NULL) {
//         return NULL;
//     }
//     headerEntry *hd = malloc(sizeof(headerEntry));
//     char *thisKey = malloc(strlen(key) + 1);
//     char *thisValue = NULL;
//     strcpy(thisKey, key);
//     if(value != NULL) {
//         char *trimed = strTrim(value);
//         thisValue = malloc(strlen(trimed) + 1);
//         strcpy(thisValue, trimed);
//     }
//     strLower(thisKey);
//     hd->key = thisKey;
//     hd->value = thisValue;
//     //logger(LogDebug, "Node[%s;%s] inserted\n", hd->key, hd->value);
//     return hd;
// }

// headerEntry *newENVPEntry(char *key, char *value)
// {
//     if(key == NULL) {
//         return NULL;
//     }
//     headerEntry *hd = malloc(sizeof(headerEntry));
//     char *thisKey = malloc(strlen(key) + 1);
//     char *thisValue = malloc(strlen(value) + 1);
//     strcpy(thisKey, key);
//     strcpy(thisValue, value);
//     hd->key = thisKey;
//     hd->value = thisValue;
//     return hd;

// }


