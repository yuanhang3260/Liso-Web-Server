#include "MyString.h"


/** @brief similar as strstr, but limited the length starting from str to n 
 *  @return matched char* or NULL if no match found
 */
char* MyString::strnstr(char* str, const char* match, int n)
{
	int len = strlen(match);
	for (int i = 0; i <= (n - len); i++)
	{
		if (strncmp(str + i, match, len) == 0)
		{
			return str + i;
		}
	}
	return NULL;
}