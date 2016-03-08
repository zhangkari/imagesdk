/************************************
 * file name:   utility.c
 * description: implement utility
 * author:      kari.zhang
 * date:        2015-11-22
 *
 ***********************************/

#include <string.h>
#include "utility.h"

/*
 * Get file postfix name 
 * Parameters:
 *		path: the file's full name 	
 *
 * Return:
 *      postfilx name if OK
 *		NULL          if ERROR		
 */
inline const char const * getFilePostfix (const char *path) 
{
	if (NULL == path) {
		return NULL;
	}
	int len = strlen (path);
	// min len is 1.jpg or 1.png
#define MIN_POSTFIX_LEN 5
	if (len < MIN_POSTFIX_LEN) {
		return NULL;
	}

	while (len-- > 0) {
		if ( *(path + len) == '.') {
			return path + len + 1;
		}
	}

	return NULL;
}
