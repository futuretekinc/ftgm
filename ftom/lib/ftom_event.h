#ifndef	_FTOM_EVENT_H_
#define	_FTOM_EVENT_H_

#include <pthread.h>
#include "ftm.h"
#include "ftom.h"

typedef	enum FTOM_EVENT_STATE_ENUM
{
       FTOM_EVENT_STATE_RESET,
       FTOM_EVENT_STATE_PRESET,
       FTOM_EVENT_STATE_SET,
       FTOM_EVENT_STATE_PRERESET
}      FTOM_EVENT_STATE, _PTR_ FTM_EVENT_STATE_PTR;

typedef struct FTOM_EVENT_STRUCT
{
	FTM_EVENT			xInfo;
	FTM_TIMER           xDetectionTimer;
	FTM_TIMER           xHoldingTimer;
	FTM_EP_DATA         xData;
	FTOM_EVENT_STATE  xState;
	FTM_TIME            xOccurrenceTime;
	FTM_TIME            xReleaseTime;
	FTM_LOCK            xLock;
}      FTOM_EVENT, _PTR_ FTOM_EVENT_PTR;

FTM_RET	FTOM_EVENT_init
(
	FTM_VOID
);

FTM_RET	FTOM_EVENT_final
(
	FTM_VOID
);

FTM_RET	FTOM_EVENT_start
(
	FTOM_EVENT_PTR	pEvent
);

FTM_RET	FTOM_EVENT_stop
(	
	FTOM_EVENT_PTR	pEvent
);

FTM_RET	FTOM_EVENT_create
(
	FTM_EVENT_PTR pInfo,
	FTM_BOOL		bNew,
	FTOM_EVENT_PTR _PTR_	ppEvent
);

FTM_RET	FTOM_EVENT_destroy
(
	FTOM_EVENT_PTR _PTR_	ppEvent,
	FTM_BOOL		bRemoveDB
);

FTM_RET	FTOM_EVENT_count
(
	FTM_ULONG_PTR pulCount
);

FTM_RET	FTOM_EVENT_get
(
	FTM_CHAR_PTR		pEventID,
	FTOM_EVENT_PTR _PTR_ ppEvent
);

FTM_RET	FTOM_EVENT_getAt
(
	FTM_ULONG ulIndex, 
	FTOM_EVENT_PTR _PTR_ ppEvent
);

FTM_RET	FTOM_EVENT_setInfo
(
	FTOM_EVENT_PTR	pEvent,
	FTM_EVENT_FIELD	xFields,
	FTM_EVENT_PTR		pInfo
);

FTM_RET	FTOM_EVENT_updateEP
(
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR pData
);

FTM_RET	FTOM_EVENT_print
(
	FTOM_EVENT_PTR	pEvent
);

FTM_RET	FTOM_EVENT_printList
(
	FTM_VOID
);

#endif
