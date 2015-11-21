/************************************
 * file name:   chrbuf.c
 * description: implement char buffer	
 * author:      kari.zhang
 * date:        2015-11-21
 *
 ***********************************/

#include <assert.h>
#include <malloc.h>
#include <string.h>
#include "chrbuf.h"
#include "comm.h"

/*
 * Create a char buffer with the specified capability
 * Parameters:
 *		cap:	the capability of char buffer	
 *
 * Return:
 *		NULL if ERROR		
 */
chrbuf_t* newChrbuf (int cap) {
	chrbuf_t *buf = NULL;
	if (cap > 0) {
		buf = (chrbuf_t *)calloc (sizeof(chrbuf_t), 1);
		if (NULL != buf) {
			buf->base = (char *)calloc (cap, 1);
			if (NULL != buf->base) {
				buf->used = 0;
				buf->capability = cap;
			} else {
				free (buf);
				buf = NULL;
			}
		}
	}
	return buf;
}

/*
 * Release the char buffer
 */
void freeChrbuf (chrbuf_t *buf) {
	if (NULL != buf) {
		if (NULL != buf->base) {
			free (buf->base);
			buf->base = NULL;
		}
		free (buf);
	}
}

/*
 * Ensure the char buff has the specified capability.
 *		if not, try to calloc memory.
 * Return:
 *		 0 OK
 *		-1 ERROR
 */
int ensureChrbufCap (chrbuf_t *buf, int cap) {
	VALIDATE_NOT_NULL2 (buf, buf->base);
	if (cap <= 0) {
		return -1;
	}

	if (buf->capability - buf->used < cap) {
		buf->base = realloc (buf->base, buf->used + cap);
		assert (NULL != buf->base);
	}

	return 0;
}

/*
 * Append chars to the char buff
 * Parameters:
 *		buf:		char buffer instance
 *		str:		string will be appended
 * Return:		
 *		0  OK
 *     -1  ERROR
 */
int appendChrbuf (chrbuf_t *buf, const char *str) {
	VALIDATE_NOT_NULL3 (buf, buf->base, str);
	int free = buf->capability - buf->used;
	int len = strlen (str);
	ensureChrbufCap (buf, buf->capability + len - free);
	strcat (buf->base + buf->used, str);
	buf->used += len;
	return 0;
}

/*
 * Clear the char buff
 * Parameters:
 *		buf:		char buffer instance
 * Return:		
 *		0  OK
 *     -1  ERROR
 */
int clearChrbuf (chrbuf_t *buf) {
	VALIDATE_NOT_NULL (buf);
	buf->used = 0;
	return 0;
}
