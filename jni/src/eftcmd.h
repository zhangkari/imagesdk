/************************************
 * file name:   eftcmd.h
 * description:	define effect command
 * author:      kari.zhang
 * date:        2015-11-21
 *
 ***********************************/

#ifndef __EFTCMD__H__
#define __EFTCMD__H__

#include <comm.h>

/*
 * Define effect command
 */
typedef enum {
	ec_NORMAL = 0,		// normal effect:no parameters
	ec_ROTATE,			// roate image:1 parameters (degree)
	ec_SCALE,			// scale image:1 parameters (zoom factor)
	ec_CLIP,			// clip sub image:4 params (x, y, width, height)
	ec_SKIN,			// skin effect 
	ec_EYE,				// eye effect
	ec_END				// == end == 
} ecEnum;

typedef struct {
	ecEnum	 cmd;		    // effect command
	bool     valid;	        // if the cmd is valid 
	int		 count;	        // parameter count
	int		 *params;	    // params
    int      capacity;      // capacity of paramSet
} eftcmd_t;

/**
 * Parse user cmd to eftcmd
 * Params:
 *		userCmd:	[IN]  cmd user input
 *		eftcmd:		[OUT] effect cmd
 * Return:
 *		 0 OK
 *		-1 error
 */
int parseEffectCmd(const char *userCmd, eftcmd_t *eftcmd);

#endif
