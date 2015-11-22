/************************************
 * file name:   utility.h
 * description: define utility
 * author:      kari.zhang
 * date:        2015-11-22
 *
 ***********************************/

#ifndef __UTILITY__H__
#define __UTILITY__H__

#include "comm.h"

#define LOG_ENTRY Log("++++ %s ++++\n", __func__);
#define LOG_EXIT Log("---- %s ----\n", __func__);

/*
 * Get file postfix name 
 * Parameters:
 *		path: the file's full name 	
 *
 * Return:
 *      postfilx name if OK
 *		NULL          if ERROR		
 */
 const char const * getFilePostfix (const char *path);

#endif
