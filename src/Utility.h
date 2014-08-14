#ifndef __UTILITY_H_
#define __UTILITY_H_

#include <string>

using namespace std;

#define ERROR -1
#define SUCCESS 0

/** print error message and exit -1 */
void Liso_error(string errmsg);


/** debug */
void debug(int n);

#endif /* __UTILITY_H_ */