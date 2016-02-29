#ifndef	__FTNM_EP_H__
#define	__FTNM_EP_H__

#include <pthread.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "ftm.h"
#include "ftnm.h"

typedef	struct FTNM_NODE_STRUCT _PTR_ FTNM_NODE_PTR;
typedef	struct FTNM_EP_STRUCT _PTR_ FTNM_EP_PTR;
typedef	struct FTNM_EP_STRUCT
{
	FTM_EP			xInfo;

	FTNM_NODE_PTR 		pNode;

	FTM_EP_STATE		xState;
	FTM_ULONG			ulRetryCount;

	pthread_t			xPThread;
	sem_t				xLock;
	FTM_MSG_QUEUE		xMsgQ;

	FTM_LIST			xDataList;
	FTM_LIST			xEventList;

	union
	{
		struct	
		{
			oid		pOID[MAX_OID_LEN];
			size_t	nOIDLen;
		}	xSNMP;
	}	xOption;
}	FTNM_EP;

FTM_RET	FTNM_EP_init(void);
FTM_RET	FTNM_EP_final(void);

FTM_RET	FTNM_EP_create(FTM_EP_PTR pInfo, FTNM_EP_PTR _PTR_ ppNode);
FTM_RET	FTNM_EP_destroy(FTNM_EP_PTR pEP);

FTM_RET	FTNM_EP_count(FTM_EP_TYPE xType, FTM_ULONG_PTR pulCount);

FTM_RET FTNM_EP_getIDList(FTM_EP_TYPE 	xType, FTM_EP_ID_PTR pEPID, FTM_ULONG ulMaxCount, FTM_ULONG_PTR pulCount);

FTM_RET FTNM_EP_get(FTM_EP_ID xEPID, FTNM_EP_PTR _PTR_ ppEP);
FTM_RET FTNM_EP_getAt(FTM_ULONG ulIndex, FTNM_EP_PTR _PTR_ ppEP);

FTM_RET	FTNM_EP_attach(FTNM_EP_PTR pEP, FTNM_NODE_PTR pNode);
FTM_RET	FTNM_EP_detach(FTNM_EP_PTR pEP);

FTM_RET FTNM_EP_start(FTNM_EP_PTR pEP);
FTM_RET FTNM_EP_stop(FTNM_EP_PTR pEP, FTM_BOOL bWaitForStop);

FTM_RET	FTNM_EP_setData(FTNM_EP_PTR pEP, FTM_EP_DATA_PTR pData);
FTM_RET	FTNM_EP_getData(FTNM_EP_PTR pEP, FTM_EP_DATA_PTR pData);
FTM_RET FTNM_EP_setFloat(FTNM_EP_PTR pEP, FTM_ULONG ulTime, FTM_DOUBLE fValue);

FTM_RET	FTNM_EP_getEventCount(FTNM_EP_PTR pEP, FTM_ULONG_PTR pulCount);
FTM_RET	FTNM_EP_getEventAt(FTNM_EP_PTR pEP, FTM_ULONG ulIndex, FTM_EVENT_ID_PTR pEventID);

FTM_RET FTNM_EP_trap(FTNM_EP_PTR pEP, FTM_EP_DATA_PTR pData);

#endif

