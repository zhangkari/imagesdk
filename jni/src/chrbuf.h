/************************************
 * file name:   chrbuf.h
 * description: define char buffer	
 * author:      kari.zhang
 * date:        2015-11-21
 *
 ***********************************/

#ifndef __CHRBUF__H__
#define __CHRBUF__H__

typedef struct {
	int		capability;
	int		used;
	char	*base;
} chrbuf_t;

/*
 * Create a char buffer with the specified capability
 * Parameters:
 *		cap:	the capability of char buffer	
 *
 * Return:
 *		NULL if ERROR		
 */
chrbuf_t* newChrbuf (int cap);

/*
 * Release the char buffer
 */
void freeChrbuf (chrbuf_t *buf);

/*
 * Ensure the char buff has the specified capability.
 *		if not, try to calloc memory.
 * Return:
 *		 0 OK
 *		-1 ERROR
 */
int ensureChrbufCap (chrbuf_t *buf, int cap);

/*
 * Append chars to the char buff
 * Parameters:
 *		buf:		char buffer instance
 *		str:		string will be appended
 * Return:		
 *		0  OK
 *     -1  ERROR
 */
int appendChrbuf (chrbuf_t *buf, const char *str);

/*
 * Clear the char buff
 * Parameters:
 *		buf:		char buffer instance
 * Return:		
 *		0  OK
 *     -1  ERROR
 */
int clearChrbuf (chrbuf_t *buf);

#endif
