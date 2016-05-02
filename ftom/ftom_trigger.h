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

FTM_RET	FTOM_TRIGGER_create
(
	FTM_TRIGGER_PTR	pInfo,
	FTOM_TRIGGER_PTR _PTR_ ppTrigger
);

FTM_RET	FTOM_TRIGGER_destroy
(
	FTOM_TRIGGER_PTR _PTR_ ppTrigger
);

FTM_RET	FTOM_TRIGGER_init
(
	FTOM_TRIGGER_PTR pTrigger,
	FTM_TRIGGER_PTR	pInfo
);

FTM_RET	FTOM_TRIGGER_final
(
	FTOM_TRIGGER_PTR pTrigger
);

#endif
