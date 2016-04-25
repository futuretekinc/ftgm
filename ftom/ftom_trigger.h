#ifndef	_FTOM_TRIGGER_H_
#define	_FTOM_TRIGGER_H_

#include <pthread.h>
#include "ftm.h"
#include "ftom.h"

typedef	enum
{
	FTOM_TRIGGER_STATE_RESET,
	FTOM_TRIGGER_STATE_PRESET,
	FTOM_TRIGGER_STATE_SET,
	FTOM_TRIGGER_STATE_PRERESET
}	FTOM_TRIGGER_STATE, _PTR_ FTM_TRIGGER_STATE_PTR;

typedef	struct
{
	FTM_TRIGGER			xInfo;
	FTM_TIMER			xDetectionTimer;
	FTM_TIMER			xHoldingTimer;
	FTM_EP_DATA			xData;
	FTOM_TRIGGER_STATE	xState;
	FTM_TIME			xOccurrenceTime;
	FTM_TIME			xReleaseTime;
	FTM_LOCK			xLock;
}	FTOM_TRIGGER, _PTR_ FTOM_TRIGGER_PTR;

typedef	struct
{
}	FTOM_TRIGGERM_CONFIG, _PTR_ FTOM_TRIGGERM_CONFIG_PTR;

typedef	struct FTOM_TRIGGERM_STRUCT
{
	FTOM_TRIGGERM_CONFIG	xConfig;
	FTOM_PTR				pOM;
	FTM_MSG_QUEUE_PTR		pMsgQ;
	FTM_BOOL				bStop;
	FTM_LIST				xTriggerList;
	pthread_t				xTriggerThread;
}	FTOM_TRIGGERM, _PTR_ FTOM_TRIGGERM_PTR;
	
FTM_RET	FTOM_TRIGGERM_create
(
	FTOM_PTR	pOM,
	FTOM_TRIGGERM_PTR _PTR_ pTriggerM
);

FTM_RET	FTOM_TRIGGERM_destroy
(
	FTOM_TRIGGERM_PTR _PTR_ pTriggerM
);

FTM_RET	FTOM_TRIGGERM_init
(
	FTOM_TRIGGERM_PTR pTriggerM
);

FTM_RET	FTOM_TRIGGERM_final
(
	FTOM_TRIGGERM_PTR pTriggerM
);

FTM_RET	FTOM_TRIGGERM_loadConfig
(
	FTOM_TRIGGERM_PTR pTriggerM, 
	FTM_CHAR_PTR pFileName
);

FTM_RET	FTOM_TRIGGERM_start
(
	FTOM_TRIGGERM_PTR pTriggerM
);

FTM_RET	FTOM_TRIGGERM_stop
(	
	FTOM_TRIGGERM_PTR pTriggerM
);

FTM_RET	FTOM_TRIGGERM_add
(
	FTOM_TRIGGERM_PTR pTriggerM, 
	FTM_TRIGGER_PTR pTriggerInfo,
	FTOM_TRIGGER_PTR _PTR_	ppTrigger	
);

FTM_RET	FTOM_TRIGGERM_del
(
	FTOM_TRIGGERM_PTR pTriggerM, 
	FTM_CHAR_PTR	pTriggerID
);

FTM_RET	FTOM_TRIGGERM_count
(
	FTOM_TRIGGERM_PTR pTriggerM, 
	FTM_ULONG_PTR pulCount
);

FTM_RET	FTOM_TRIGGERM_get
(
	FTOM_TRIGGERM_PTR pTriggerM, 
	FTM_CHAR_PTR		pTriggerID,
	FTOM_TRIGGER_PTR _PTR_ ppTrigger
);

FTM_RET	FTOM_TRIGGERM_getAt
(
	FTOM_TRIGGERM_PTR pTriggerM, 
	FTM_ULONG ulIndex, 
	FTOM_TRIGGER_PTR _PTR_ ppTrigger
);

FTM_RET	FTOM_TRIGGERM_updateEP
(
	FTOM_TRIGGERM_PTR pTriggerM, 
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR pData
);

#endif
