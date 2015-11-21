/***************************************
 * file name:   eftcmd.c
 * description:	implement effect command
 * author:      kari.zhang
 * date:        2015-11-21
 *
 ***************************************/

#include <assert.h>
#include "comm.h"
#include "eftcmd.h"
#include "cJSON.h"

static int parseNormalEffect (const cJSON *json, eftcmd_t *eftcmd) {
	return -1;
}

static int parseRotateEffect (const cJSON *json, eftcmd_t *eftcmd) {
	return -1;
}

static int parseScaleEffect (const cJSON *json, eftcmd_t *eftcmd) {
	return -1;
}

static int parseClipEffect (const cJSON *json, eftcmd_t *eftcmd) {
	return -1;
}

static int parseSkinEffect (const cJSON *json, eftcmd_t *eftcmd) {
	return -1;
}

static int parseEyeEffect (const cJSON *json, eftcmd_t *eftcmd) {
	return -1;
}

/**
 * Parse user cmd to eftcmd
 * Params:
 *		userCmd:	[IN]  cmd user input
 *		eftcmd:		[OUT] effect cmd
 * Return:
 *		 0 OK
 *		-1 error
 */
int parseEffectCmd(const char *usercmd, eftcmd_t *eftcmd) {
	VALIDATE_NOT_NULL2 (usercmd, eftcmd);

	cJSON *json = cJSON_Parse (usercmd);
	if (NULL == json) {
		LogE ("parseEffectCmd error:%s\n", cJSON_GetErrorPtr() );
		return -1;
	}

	cJSON *jeft = cJSON_GetObjectItem (json, "effect");
	if (NULL == jeft) {
		LogE ("parseEffectCmd error:Invalid effect command\n");
		cJSON_Delete (json);
		return -1;
	}

	if (jeft->type != cJSON_String) {
		LogE ("parseEffectCmd error:Invalid effect command\n");
		cJSON_Delete (json);
		return -1;
	}

	// TODO
	assert (0);

	const char *eft = jeft->valuestring;
	if (strcmp (eft, "Normal") == 0) {

	} else if (strcmp(eft, "Rotate") == 0) {

	}

	cJSON_Delete (json);
	return -1;
}
