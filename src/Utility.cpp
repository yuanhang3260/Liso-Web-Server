#include <stdio.h>
#include <stdlib.h>
#include "Utility.h"


/** @brief print error message and exit -1 
 *  @param errmsg error message
 *
 *  @return void
 */
void Liso_error(string errmsg)
{
	fprintf(stderr, "ERROR: %s\n", errmsg.c_str());
	exit(ERROR);
}

/** debug */
void debug(int n)
{
	fprintf(stderr, "debug%d\n", n);
}

