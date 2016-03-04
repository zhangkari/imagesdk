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
    if (NULL == json || NULL == eftcmd) {
        return -1;
    }
    eftcmd->cmd = ec_NORMAL;
    eftcmd->count = 0;
    eftcmd->params = NULL;
    eftcmd->valid = false;
    return 0;
}

static int parseRotateEffect (cJSON *json, eftcmd_t *eftcmd) {
    if (NULL == json || NULL == eftcmd){
        return -1;
    }
	cJSON *jparam = cJSON_GetObjectItem (json, "degree");
    if (NULL == jparam) {
        return -1;
    }

	if (jparam->type != cJSON_Number) {
		LogE ("parseRotateEffect error:Invalid param type\n");
		return -1;
	}
    
    eftcmd->cmd = ec_ROTATE;
    eftcmd->count = 1;
    *(eftcmd->params) = jparam->valueint;
    eftcmd->valid = true;

    return 0;
}

static int parseScaleEffect (cJSON *json, eftcmd_t *eftcmd) {
    if (NULL == json || NULL == eftcmd){
        return -1;
    }
	cJSON *jparam = cJSON_GetObjectItem (json, "percent");
    if (NULL == jparam) {
        return -1;
    }

	if (jparam->type != cJSON_Number) {
		LogE ("parseScaleEffect error:Invalid param type\n");
		return -1;
	}
    
    eftcmd->cmd = ec_SCALE;
    eftcmd->count = 1;
    *(eftcmd->params) = jparam->valueint;
    eftcmd->valid = true;

    return 0;
}

static int parseClipEffect (cJSON *json, eftcmd_t *eftcmd) {
    if (NULL == json || NULL == eftcmd){
        return -1;
    }
    cJSON *jparam = cJSON_GetObjectItem (json, "param");
    if (NULL == jparam) {
        return -1;
    }

    cJSON *jx = cJSON_GetObjectItem (jparam, "x");
    if (jx->type != cJSON_Number) {
        LogE ("parseClipEffect error:Invalid param type\n");
        return -1;
    }

    cJSON *jy = cJSON_GetObjectItem (jparam, "y");
    if (jy->type != cJSON_Number) {
        LogE ("parseClipEffect error:Invalid param type\n");
        return -1;
    }

    cJSON *jw = cJSON_GetObjectItem (jparam, "w");
    if (jw->type != cJSON_Number) {
        LogE ("parseClipEffect error:Invalid param type\n");
        return -1;
    }

    cJSON *jh = cJSON_GetObjectItem (jparam, "h");
    if (jy->type != cJSON_Number) {
        LogE ("parseClipEffect error:Invalid param type\n");
        return -1;
    }

    eftcmd->cmd = ec_CLIP;
    eftcmd->count = 4;
    eftcmd->params[0] = jx->valueint;
    eftcmd->params[1] = jy->valueint;
    eftcmd->params[2] = jw->valueint;
    eftcmd->params[3] = jh->valueint;
    eftcmd->valid = true;

    return -1;
}

static int parseSkinEffect (const cJSON *json, eftcmd_t *eftcmd) {
    LogE ("Not implemented yet!\n");
	return -1;
}

static int parseEyeEffect (const cJSON *json, eftcmd_t *eftcmd) {
    LogE ("Not implemented yet!\n");
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
int parseEffectCmd(const char *usercmd, eftcmd_t *eftcmd) 
{
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

	int retCode = -1;
	const char *eft = jeft->valuestring;
	if (strcmp (eft, "Normal") == 0) {
        if (parseNormalEffect (json, eftcmd) < 0) {
            LogE ("Failed parseNormalEffect\n");
        }
		else {
			retCode = 0;
		}
	}
    else if (strcmp(eft, "Rotate") == 0) {
        if (parseRotateEffect (json, eftcmd) < 0) {
            LogE ("Failed parseRotateEffect\n");
        }
		else {
			retCode = 0;
		}
	}
    else if (strcmp(eft, "Scale") == 0) {
        if (parseScaleEffect (json, eftcmd) < 0) {
            LogE ("Failed parseScaleEffect\n");
        }
		else {
			retCode = 0;
		}
	}
	else if (strcmp (eft, "Clip") == 0) {
        if (parseClipEffect (json, eftcmd) < 0) {
            LogE ("Failed parseClipEffect\n");
        }
		else {
			retCode = 0;
		}
	}
    else if (strcmp(eft, "Skin") == 0) {
        if (parseSkinEffect (json, eftcmd) < 0) {
            LogE ("Failed parseSkinEffect\n");
        }
		else {
			retCode = 0;
		}
	}
    else if (strcmp(eft, "Eye") == 0) {
        if (parseEyeEffect (json, eftcmd) < 0) {
            LogE ("Failed parseEyeEffect\n");
        }
		else {
			retCode = 0;
		}
	}
    else {
        LogE ("Invalid effect command\n");
    }

	cJSON_Delete (json);
	
	return retCode;
}
