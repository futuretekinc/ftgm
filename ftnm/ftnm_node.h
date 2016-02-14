#ifndef	__FTNM_NODE_H__
#define	__FTNM_NODE_H__

#include "ftm_types.h"
#include "ftm_object.h"
#include "ftm_list.h"
#include "ftnm_object.h"
#include <pthread.h>

typedef	FTM_ULONG		FTNM_NODE_STATE;

#define	FTNM_NODE_STATE_CREATED					0x00000001
#define	FTNM_NODE_STATE_INITIALIZED				0x00000002
#define	FTNM_NODE_STATE_SYNCHRONIZED			0x00000003
#define	FTNM_NODE_STATE_PROCESS_INIT			0x00000004
#define	FTNM_NODE_STATE_RUN						0x00000005
#define	FTNM_NODE_STATE_RUNNING					0x00000006
#define	FTNM_NODE_STATE_PROCESS_FINISHED		0x00000007
#define	FTNM_NODE_STATE_FINISHED				0x00000008
#define	FTNM_NODE_STATE_ABORT					0x00000009

typedef	struct _FTNM_NODE
{
	FTM_NODE_TYPE		xType;
	FTM_NODE_INFO		xInfo;
	FTM_LIST			xEPList;

	pthread_t			xPThread;
	pthread_mutex_t		xMutexLock;
	FTNM_NODE_STATE		xState;
	FTM_ULONG			ulRetry;
	int64_t				xTimeout;
	FTM_LIST			xTaskList;
}	FTNM_NODE, _PTR_ FTNM_NODE_PTR;

FTM_RET FTNM_NODE_init(FTNM_CONTEXT_PTR pCTX);
FTM_RET FTNM_NODE_final(FTNM_CONTEXT_PTR pCTX);

FTM_RET	FTNM_NODE_create(FTNM_CONTEXT_PTR pCTX, FTM_NODE_INFO_PTR pInfo, FTNM_NODE_PTR _PTR_ ppNode);
FTM_RET	FTNM_NODE_destroy(FTNM_CONTEXT_PTR pCTX, FTNM_NODE_PTR	pNode);

FTM_RET FTNM_NODE_get(FTNM_CONTEXT_PTR pCTX, FTM_CHAR_PTR pDID, FTNM_NODE_PTR _PTR_ ppNode);
FTM_RET FTNM_NODE_getAt(FTNM_CONTEXT_PTR pCTX, FTM_ULONG ulIndex, FTNM_NODE_PTR _PTR_ ppNode);
FTM_RET	FTNM_NODE_count(FTNM_CONTEXT_PTR pCTX, FTM_ULONG_PTR pulCount);

FTM_RET	FTNM_NODE_linkEP(FTNM_CONTEXT_PTR pCTX, FTNM_NODE_PTR pNode, FTNM_EP_PTR pEP);
FTM_RET	FTNM_NODE_unlinkEP(FTNM_CONTEXT_PTR pCTX, FTNM_NODE_PTR pNode, FTNM_EP_PTR pEP);

FTM_RET	FTNM_NODE_EP_count(FTNM_CONTEXT_PTR pCTX, FTNM_NODE_PTR pNode, FTM_ULONG_PTR pulCount);
FTM_RET	FTNM_NODE_EP_get(FTNM_CONTEXT_PTR pCTX, FTNM_NODE_PTR pNode, FTM_EPID xEPID, FTNM_EP_PTR _PTR_ pEP);
FTM_RET	FTNM_NODE_EP_getAt(FTNM_CONTEXT_PTR pCTX, FTNM_NODE_PTR pNode, FTM_ULONG ulIndex, FTNM_EP_PTR _PTR_ pEP);

FTM_RET	FTNM_NODE_run(FTNM_CONTEXT_PTR pCTX, FTNM_NODE_PTR pNode);
FTM_RET	FTNM_NODE_start(FTNM_CONTEXT_PTR pCTX, FTNM_NODE_PTR pNode);
FTM_RET	FTNM_NODE_stop(FTNM_CONTEXT_PTR pCTX, FTNM_NODE_PTR pNode);
FTM_RET	FTNM_NODE_restart(FTNM_CONTEXT_PTR pCTX, FTNM_NODE_PTR pNode);

FTM_CHAR_PTR	FTNM_NODE_stateString(FTNM_CONTEXT_PTR pCTX, FTNM_NODE_STATE xState);
#endif

