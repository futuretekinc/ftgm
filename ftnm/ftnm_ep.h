#ifndef	__FTNM_EP_H__
#define	__FTNM_EP_H__

#include <pthread.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "ftm.h"
#include "ftnm.h"

typedef	struct FTNM_CONTEXT_STRUCT _PTR_	FTNM_CONTEXT_PTR;
typedef	struct FTNM_NODE_STRUCT _PTR_ FTNM_NODE_PTR;
typedef	struct FTNM_EP_STRUCT _PTR_ FTNM_EP_PTR;

typedef	struct FTNM_EPM_STRUCT
{
	FTNM_CONTEXT_PTR	pCTX;
	FTM_LIST_PTR		pEPList;
}	FTNM_EPM, _PTR_ FTNM_EPM_PTR;

FTM_RET	FTNM_EPM_create(FTNM_CONTEXT_PTR pCTX, FTNM_EPM_PTR _PTR_ ppEPM);
FTM_RET	FTNM_EPM_destroy(FTNM_EPM_PTR _PTR_ ppEPM);

FTM_RET	FTNM_EPM_init(FTNM_CONTEXT_PTR pCTX, FTNM_EPM_PTR pEPM);
FTM_RET	FTNM_EPM_final(FTNM_EPM_PTR pEPM);

FTM_RET	FTNM_EPM_createEP(FTNM_EPM_PTR pEPM, FTM_EP_PTR pInfo, FTNM_EP_PTR _PTR_ ppEP);
FTM_RET	FTNM_EPM_destroyEP(FTNM_EPM_PTR pEPM, FTNM_EP_PTR pEP);

FTM_RET	FTNM_EPM_count(FTNM_EPM_PTR pEPM, FTM_EP_TYPE xType, FTM_ULONG_PTR pulCount);

FTM_RET FTNM_EPM_getIDList(FTNM_EPM_PTR pEPM, FTM_EP_TYPE 	xType, FTM_EP_ID_PTR pEPID, FTM_ULONG ulMaxCount, FTM_ULONG_PTR pulCount);

FTM_RET FTNM_EPM_get(FTNM_EPM_PTR pEPM, FTM_EP_ID xEPID, FTNM_EP_PTR _PTR_ ppEP);
FTM_RET FTNM_EPM_getAt(FTNM_EPM_PTR pEPM, FTM_ULONG ulIndex, FTNM_EP_PTR _PTR_ ppEP);

FTM_RET	FTNM_EPM_notifyEPUpdated(FTNM_EPM_PTR pEPM, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData);


typedef	struct FTNM_EP_STRUCT
{
	FTM_EP			xInfo;

	FTNM_EPM_PTR		pEPM;
	FTNM_NODE_PTR 		pNode;

	FTM_EP_STATE		xState;
	FTM_ULONG			ulRetryCount;

	pthread_t			xPThread;
	sem_t				xLock;
	FTM_MSG_QUEUE		xMsgQ;

	FTM_LIST			xDataList;
	FTM_LIST			xTriggerList;

	union
	{
		struct	
		{
			oid		pOID[MAX_OID_LEN];
			size_t	nOIDLen;
		}	xSNMP;
	}	xOption;
}	FTNM_EP;

FTM_RET	FTNM_EP_attach(FTNM_EP_PTR pEP, FTNM_NODE_PTR pNode);
FTM_RET	FTNM_EP_detach(FTNM_EP_PTR pEP);

FTM_RET FTNM_EP_start(FTNM_EP_PTR pEP);
FTM_RET FTNM_EP_stop(FTNM_EP_PTR pEP, FTM_BOOL bWaitForStop);

FTM_RET	FTNM_EP_setData(FTNM_EP_PTR pEP, FTM_EP_DATA_PTR pData);
FTM_RET	FTNM_EP_getData(FTNM_EP_PTR pEP, FTM_EP_DATA_PTR pData);
FTM_RET FTNM_EP_setFloat(FTNM_EP_PTR pEP, FTM_ULONG ulTime, FTM_DOUBLE fValue);

FTM_RET	FTNM_EP_getTriggerCount(FTNM_EP_PTR pEP, FTM_ULONG_PTR pulCount);
FTM_RET	FTNM_EP_getTriggerAt(FTNM_EP_PTR pEP, FTM_ULONG ulIndex, FTM_TRIGGER_ID_PTR pTriggerID);

FTM_RET FTNM_EP_trap(FTNM_EP_PTR pEP, FTM_EP_DATA_PTR pData);

#endif

