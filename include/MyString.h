#ifndef __MYSTRING_H__
#define __MYSTRING_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <string>
#include <vector>

using namespace std;


class MyString
{
/*----------------------------------------------------------------------------*/
public:
    static char* strnstr(char* str, const char* match, int n);
        
};


#endif  /* __MYSTRING_H__ */
