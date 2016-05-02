#ifndef	_FTOM_ACTION_MANAGEMENT_H_
#define	_FTOM_ACTION_MANAGEMENT_H_

#include <pthread.h>
#include "ftm.h"
#include "ftom.h"
#include "ftom_action.h"


typedef	struct
{
}	FTOM_ACTIONM_CONFIG, _PTR_ FTOM_ACTIONM_CONFIG_PTR;

typedef	struct FTOM_ACTIONM_STRUCT
{
	FTOM_ACTIONM_CONFIG	xConfig;
	
	FTOM_PTR			pOM;
	pthread_t			xThread;
	FTM_BOOL			bStop;

	FTOM_MSG_QUEUE_PTR	pMsgQ;
	FTM_LIST_PTR		pActionList;
}	FTOM_ACTIONM, _PTR_ FTOM_ACTIONM_PTR;
	
FTM_RET	FTOM_ACTIONM_create
(
	FTOM_PTR	pOM,
	FTOM_ACTIONM_PTR _PTR_ ppActionM
);

FTM_RET	FTOM_ACTIONM_destroy
(
	FTOM_ACTIONM_PTR _PTR_ ppActionM
);

FTM_RET	FTOM_ACTIONM_init
(
	FTOM_ACTIONM_PTR pActionM,
	FTOM_PTR	pOM
);

FTM_RET	FTOM_ACTIONM_final
(
	FTOM_ACTIONM_PTR pActionM
);

FTM_RET	FTOM_ACTIONM_loadConfig
(
	FTOM_ACTIONM_PTR 			pActionM, 
	FTOM_ACTIONM_CONFIG_PTR 	pConfig
);

FTM_RET	FTOM_ACTIONM_loadConfigFromFile
(
	FTOM_ACTIONM_PTR 	pActionM, 
	FTM_CHAR_PTR 		pFileName
);

FTM_RET	FTOM_ACTIONM_start
(
	FTOM_ACTIONM_PTR	pActionM
);

FTM_RET	FTOM_ACTIONM_stop
(
	FTOM_ACTIONM_PTR	pActionM
);

FTM_RET	FTOM_ACTIONM_createAction
(
	FTOM_ACTIONM_PTR 	pActionM, 
	FTM_ACTION_PTR 		pInfo,
	FTOM_ACTION_PTR _PTR_ ppAction
);

FTM_RET	FTOM_ACTIONM_destroyAction
(
	FTOM_ACTIONM_PTR 	pActionM, 
	FTOM_ACTION_PTR _PTR_	pAction
);

FTM_RET	FTOM_ACTIONM_count
(
	FTOM_ACTIONM_PTR 	pActionM, 
	FTM_ULONG_PTR 		pulCount
);

FTM_RET	FTOM_ACTIONM_get
(
	FTOM_ACTIONM_PTR 	pActionM, 
	FTM_CHAR_PTR		pActionID,
	FTOM_ACTION_PTR _PTR_ ppActor
);

FTM_RET	FTOM_ACTIONM_getAt
(
	FTOM_ACTIONM_PTR 	pActionM, 
	FTM_ULONG 			ulIndex, 
	FTOM_ACTION_PTR _PTR_ ppActor
);

FTM_RET	FTOM_ACTIONM_active
(
	FTOM_ACTIONM_PTR 	pActionM, 
	FTM_CHAR_PTR		pActionID,
	FTM_BOOL			bActivate
);

#endif
