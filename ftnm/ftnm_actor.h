#ifndef	_FTNM_ACTOR_H_
#define	_FTNM_ACTOR_H_

#include <pthread.h>
#include "ftm_types.h"
#include "ftm_act.h"
#include "ftm_time.h"
#include "ftm_timer.h"
#include "ftm_msg_queue.h"

typedef	FTM_ACT_ID	FTNM_ACTOR_ID;

typedef	struct
{
	FTM_ACT		xAct;
	FTM_TIME	xTime;
	FTM_TIMER	xTimer;
}	FTNM_ACTOR, _PTR_ FTNM_ACTOR_PTR;

typedef	struct
{
}	FTNM_ACTOR_MANAGER_CONFIG, _PTR_ FTNM_ACTOR_MANAGER_CONFIG_PTR;

typedef	struct
{
	FTNM_ACTOR_MANAGER_CONFIG	xConfig;
	FTM_MSG_QUEUE_PTR			pMsgQ;
	FTM_BOOL					bStop;
	pthread_t					xPThread;
}	FTNM_ACTOR_MANAGER, _PTR_ FTNM_ACTOR_MANAGER_PTR;
	
FTM_RET	FTNM_ACTOR_init(FTM_VOID);
FTM_RET	FTNM_ACTOR_final(FTM_VOID);

FTM_RET	FTNM_ACTOR_loadConfig(FTM_CHAR_PTR pFileName);

FTM_RET	FTNM_ACTOR_start(FTM_VOID);
FTM_RET	FTNM_ACTOR_stop(FTM_VOID);

FTM_RET	FTNM_ACTOR_create(FTM_EVENT_PTR pEvent);
FTM_RET	FTNM_ACTOR_del(FTNM_ACTOR_ID  xActorID);
FTM_RET	FTNM_ACTOR_count(FTM_ULONG_PTR pulCount);
FTM_RET	FTNM_ACTOR_get(FTNM_ACTOR_ID xActorID, FTNM_ACTOR_PTR _PTR_ ppActor);
FTM_RET	FTNM_ACTOR_getAt(FTM_ULONG ulIndex, FTNM_ACTOR_PTR _PTR_ ppActor);

FTM_RET	FTNM_ACTOR_run(FTM_ACT_ID xActID);
#endif
