#ifndef	_FTNM_TRIGGER_H_
#define	_FTNM_TRIGGER_H_

#include <pthread.h>
#include "ftm_types.h"
#include "ftm_event.h"
#include "ftm_time.h"
#include "ftm_timer.h"
#include "ftm_msg_queue.h"

typedef	FTM_EVENT_ID	FTNM_TRIG_ID;

typedef	struct
{
	FTM_EVENT	xEvent;
	FTM_TIME	xTime;
	FTM_TIMER	xTimer;
}	FTNM_TRIG, _PTR_ FTNM_TRIG_PTR;

typedef	struct
{
}	FTNM_TRIG_MANAGER_CONFIG, _PTR_ FTNM_TRIG_MANAGER_CONFIG_PTR;

typedef	struct
{
	FTNM_TRIG_MANAGER_CONFIG	xConfig;
	FTM_MSG_QUEUE_PTR			pMsgQ;
	FTM_BOOL					bStop;
	pthread_t					xPThread;
}	FTNM_TRIG_MANAGER, _PTR_ FTNM_TRIG_MANAGER_PTR;
	
FTM_RET	FTNM_TRIG_init(FTM_VOID);
FTM_RET	FTNM_TRIG_final(FTM_VOID);

FTM_RET	FTNM_TRIG_loadConfig(FTM_CHAR_PTR pFileName);

FTM_RET	FTNM_TRIG_start(FTM_VOID);
FTM_RET	FTNM_TRIG_stop(FTM_VOID);

FTM_RET	FTNM_TRIG_create(FTM_EVENT_PTR pEvent);
FTM_RET	FTNM_TRIG_del(FTNM_TRIG_ID  xTriggerID);
FTM_RET	FTNM_TRIG_count(FTM_ULONG_PTR pulCount);
FTM_RET	FTNM_TRIG_get(FTNM_TRIG_ID xTriggerID, FTNM_TRIG_PTR _PTR_ ppTrigger);
FTM_RET	FTNM_TRIG_getAt(FTM_ULONG ulIndex, FTNM_TRIG_PTR _PTR_ ppTrigger);

FTM_RET	FTNM_TRIG_updateEP(FTM_EPID xEPID, FTM_EP_DATA_PTR pLastData, FTM_EP_DATA_PTR pNewData);
#endif
