#ifndef	_FTOM_TRIGGER_H_
#define	_FTOM_TRIGGER_H_

#include <pthread.h>
#include "ftm.h"
#include "ftom.h"

typedef	enum FTOM_TRIGGER_STATE_ENUM
{
       FTOM_TRIGGER_STATE_RESET,
       FTOM_TRIGGER_STATE_PRESET,
       FTOM_TRIGGER_STATE_SET,
       FTOM_TRIGGER_STATE_PRERESET
}      FTOM_TRIGGER_STATE, _PTR_ FTM_TRIGGER_STATE_PTR;

typedef struct FTOM_TRIGGER_STRUCT
{
	FTM_TRIGGER			xInfo;
	FTM_TIMER           xDetectionTimer;
	FTM_TIMER           xHoldingTimer;
	FTM_EP_DATA         xData;
	FTOM_TRIGGER_STATE  xState;
	FTM_TIME            xOccurrenceTime;
	FTM_TIME            xReleaseTime;
	FTM_LOCK            xLock;
}      FTOM_TRIGGER, _PTR_ FTOM_TRIGGER_PTR;

FTM_RET	FTOM_TRIGGER_init
(
	FTM_VOID
);

FTM_RET	FTOM_TRIGGER_final
(
	FTM_VOID
);

FTM_RET	FTOM_TRIGGER_start
(
	FTOM_TRIGGER_PTR	pTrigger
);

FTM_RET	FTOM_TRIGGER_stop
(	
	FTOM_TRIGGER_PTR	pTrigger
);

FTM_RET	FTOM_TRIGGER_create
(
	FTM_TRIGGER_PTR pInfo,
	FTM_BOOL		bNew,
	FTOM_TRIGGER_PTR _PTR_	ppTrigger
);

FTM_RET	FTOM_TRIGGER_destroy
(
	FTOM_TRIGGER_PTR _PTR_	ppTrigger	
);

FTM_RET	FTOM_TRIGGER_count
(
	FTM_ULONG_PTR pulCount
);

FTM_RET	FTOM_TRIGGER_get
(
	FTM_CHAR_PTR		pTriggerID,
	FTOM_TRIGGER_PTR _PTR_ ppTrigger
);

FTM_RET	FTOM_TRIGGER_getAt
(
	FTM_ULONG ulIndex, 
	FTOM_TRIGGER_PTR _PTR_ ppTrigger
);

FTM_RET	FTOM_TRIGGER_setInfo
(
	FTOM_TRIGGER_PTR	pTrigger,
	FTM_TRIGGER_FIELD	xFields,
	FTM_TRIGGER_PTR		pInfo
);

FTM_RET	FTOM_TRIGGER_updateEP
(
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR pData
);

FTM_RET	FTOM_TRIGGER_print
(
	FTOM_TRIGGER_PTR	pTrigger
);

FTM_RET	FTOM_TRIGGER_printList
(
	FTM_VOID
);

#endif
