#ifndef	__FTOM_EP_H__
#define	__FTOM_EP_H__

#include <pthread.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "ftm.h"
#include "ftom.h"

typedef	struct FTOM_STRUCT _PTR_	FTOM_PTR;
typedef	struct FTOM_NODE_STRUCT _PTR_ FTOM_NODE_PTR;
typedef	struct FTOM_EPM_STRUCT _PTR_ FTOM_EPM_PTR;

typedef	struct FTOM_EP_STRUCT
{
	FTM_EP				xInfo;

	FTOM_EPM_PTR		pEPM;
	FTOM_NODE_PTR		pNode;

	FTM_BOOL			bStop;
	FTM_ULONG			ulRetryCount;

	pthread_t			xPThread;
	sem_t				xLock;
	FTOM_MSG_QUEUE		xMsgQ;

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
}	FTOM_EP, _PTR_ FTOM_EP_PTR;

FTM_RET	FTOM_EP_create
(
	FTM_EP_PTR 			pInfo,
	FTOM_EP_PTR _PTR_ ppEP
);

FTM_RET	FTOM_EP_destroy
(
	FTOM_EP_PTR _PTR_ ppEP
);

FTM_RET	FTOM_EP_init
(
	FTOM_EP_PTR 		pEP, 
	FTM_EP_PTR 			pInfo
);

FTM_RET	FTOM_EP_final
(
	FTOM_EP_PTR 		pEP 
);

FTM_RET	FTOM_EP_attach
(
	FTOM_EP_PTR 		pEP, 
	FTOM_NODE_PTR 	pNode
);

FTM_RET	FTOM_EP_detach
(
	FTOM_EP_PTR 		pEP
);

FTM_RET FTOM_EP_start
(
	FTOM_EP_PTR 		pEP
);

FTM_RET FTOM_EP_stop
(
	FTOM_EP_PTR 		pEP, 
	FTM_BOOL 			bWaitForStop
);

FTM_RET	FTOM_EP_getDataType
(
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_TYPE_PTR pType
);

FTM_RET	FTOM_EP_setData
(
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
);

FTM_RET	FTOM_EP_getData
(
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
);

FTM_RET	FTOM_EP_pushData
(
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
);

FTM_RET	FTOM_EP_pullData
(
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
);

FTM_RET FTOM_EP_setFloat
(
	FTOM_EP_PTR 	pEP, 
	FTM_ULONG 		ulTime, 
	FTM_DOUBLE 		fValue
);

FTM_RET	FTOM_EP_getTriggerCount
(
	FTOM_EP_PTR 	pEP, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTOM_EP_getTriggerAt
(
	FTOM_EP_PTR 	pEP, 
	FTM_ULONG 		ulIndex, 
	FTM_TRIGGER_ID_PTR pTriggerID
);

FTM_RET FTOM_EP_trap
(
	FTOM_EP_PTR 	pEP, 
	FTM_EP_DATA_PTR pData)
;

#endif

