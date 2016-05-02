#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include "ftm.h"
#include "ftom_action.h"
#include "ftom_utils.h"
#include "ftom_msg.h"
#include "libconfig.h"

FTM_RET	FTOM_ACTION_create
(
	FTM_ACTION_PTR 		pInfo,
	FTOM_ACTION_PTR _PTR_ ppAction
)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppAction != NULL);

	FTM_RET			xRet;
	FTOM_ACTION_PTR	pAction;

	pAction = (FTOM_ACTION_PTR)FTM_MEM_malloc(sizeof(FTOM_ACTION));
	if (pAction == NULL)
	{
		ERROR("Not enough memory\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_ACTION_init(pAction, pInfo);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pAction);
		return	xRet;
	}

	*ppAction = pAction;

	return	xRet;
}

FTM_RET	FTOM_ACTION_init
(
	FTOM_ACTION_PTR	pAction,
	FTM_ACTION_PTR	pInfo
)
{
	ASSERT(pAction != NULL);
	ASSERT(pInfo != NULL);

	memset(pAction, 0, sizeof(FTOM_ACTION));
	memcpy(&pAction->xInfo, pInfo, sizeof(FTM_ACTION));

	if (strlen(pAction->xInfo.pID) == 0)
	{
		FTOM_genNewID(pAction->xInfo.pID, 16);
	}
	return	FTM_RET_OK;
}

FTM_RET	FTOM_ACTION_destroy
(
	FTOM_ACTION_PTR _PTR_ ppAction
)
{
	FTOM_ACTION_final(*ppAction);
	FTM_MEM_free(*ppAction);
	
	*ppAction = NULL;

	return	FTM_RET_OK;	
}

FTM_RET	FTOM_ACTION_final
(
	FTOM_ACTION_PTR	pAction
)
{
	return	FTM_RET_OK;	
}


