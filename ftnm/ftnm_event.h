#ifndef	_FTNM_EVENT_H_
#define	_FTNM_EVENT_H_

#include <pthread.h>
#include "ftm_types.h"
#include "ftm_event.h"
#include "ftm_msg_queue.h"

typedef	struct
{

}	FTNM_EVENT_MANAGER_CONFIG, _PTR_ FTNM_EVENT_MANAGER_CONFIG_PTR;

typedef	struct
{
	FTM_MSG_QUEUE_PTR	pMsgQ;
	FTM_BOOL			bRun;
	pthread_t			xPThread;
}	FTNM_EVENT_MANAGER, _PTR_ FTNM_EVENT_MANAGER_PTR;
	
FTM_RET	FTNM_EVENTM_init(FTM_VOID);
FTM_RET	FTNM_EVENTM_final(FTM_VOID);

FTM_RET	FTNM_EVENTM_create(FTNM_EVENT_MANAGER_PTR _PTR_ ppManager);
FTM_RET	FTNM_EVENTM_destroy(FTNM_EVENT_MANAGER_PTR pManager);

FTM_RET	FTNM_EVENTM_start(FTNM_EVENT_MANAGER_PTR pManager);
FTM_RET	FTNM_EVENTM_stop(FTNM_EVENT_MANAGER_PTR pManager);
#endif
