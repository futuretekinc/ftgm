#ifndef	_FTNM_TRIGGER_H_
#define	_FTNM_TRIGGER_H_

#include <pthread.h>
#include "ftm.h"

typedef	FTM_TRIGGER_ID	FTNM_TRIGGER_ID;

typedef	enum
{
	FTNM_TRIGGER_STATE_RESET,
	FTNM_TRIGGER_STATE_PRESET,
	FTNM_TRIGGER_STATE_SET,
	FTNM_TRIGGER_STATE_PRERESET
}	FTNM_TRIGGER_STATE, _PTR_ FTM_TRIGGER_STATE_PTR;

typedef	struct
{
	FTM_TRIGGER			xInfo;
	FTM_TIMER			xDetectionTimer;
	FTM_TIMER			xHoldingTimer;
	FTM_EP_DATA			xData;
	FTNM_TRIGGER_STATE	xState;
	FTM_TIME			xOccurrenceTime;
	FTM_TIME			xReleaseTime;
	FTM_LOCK			xLock;
}	FTNM_TRIGGER, _PTR_ FTNM_TRIGGER_PTR;

typedef	struct
{
}	FTNM_TRIGGERM_CONFIG, _PTR_ FTNM_TRIGGERM_CONFIG_PTR;

typedef	struct
{
	FTNM_TRIGGERM_CONFIG	xConfig;
	FTM_MSG_QUEUE_PTR	pMsgQ;
	FTM_BOOL			bStop;
	FTM_LIST			xEventList;
	pthread_t			xEventThread;
}	FTNM_TRIGGERM, _PTR_ FTNM_TRIGGERM_PTR;
	
FTM_RET	FTNM_TRIGGERM_init(FTNM_TRIGGERM_PTR pCTX);
FTM_RET	FTNM_TRIGGERM_final(FTNM_TRIGGERM_PTR pCTX);

FTM_RET	FTNM_TRIGGERM_loadConfig(FTNM_TRIGGERM_PTR pCTX, FTM_CHAR_PTR pFileName);

FTM_RET	FTNM_TRIGGERM_start(FTNM_TRIGGERM_PTR pCTX);
FTM_RET	FTNM_TRIGGERM_stop(FTNM_TRIGGERM_PTR pCTX);

FTM_RET	FTNM_TRIGGERM_create(FTNM_TRIGGERM_PTR pCTX, FTM_TRIGGER_PTR pEvent);
FTM_RET	FTNM_TRIGGERM_del(FTNM_TRIGGERM_PTR pCTX, FTNM_TRIGGER_ID  xEventID);
FTM_RET	FTNM_TRIGGERM_count(FTNM_TRIGGERM_PTR pCTX, FTM_ULONG_PTR pulCount);
FTM_RET	FTNM_TRIGGERM_get(FTNM_TRIGGERM_PTR pCTX, FTNM_TRIGGER_ID xEventID, FTNM_TRIGGER_PTR _PTR_ ppEvent);
FTM_RET	FTNM_TRIGGERM_getAt(FTNM_TRIGGERM_PTR pCTX, FTM_ULONG ulIndex, FTNM_TRIGGER_PTR _PTR_ ppEvent);

FTM_RET	FTNM_TRIGGERM_updateEP(FTNM_TRIGGERM_PTR pCTX, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData);
#endif
