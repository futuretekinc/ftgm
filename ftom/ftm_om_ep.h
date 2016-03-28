#ifndef	__FTM_OM_EP_H__
#define	__FTM_OM_EP_H__

#include <pthread.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "ftm.h"
#include "ftm_om.h"

typedef	struct FTM_OM_STRUCT _PTR_	FTM_OM_PTR;
typedef	struct FTM_OM_NODE_STRUCT _PTR_ FTM_OM_NODE_PTR;
typedef	struct FTM_OM_EP_STRUCT _PTR_ FTM_OM_EP_PTR;

typedef	struct FTM_OM_EPM_STRUCT
{
	FTM_OM_PTR	pOM;
	FTM_LIST_PTR		pEPList;
}	FTM_OM_EPM, _PTR_ FTM_OM_EPM_PTR;

FTM_RET	FTM_OM_EPM_create
(
	FTM_OM_PTR 	pOM, 
	FTM_OM_EPM_PTR _PTR_ ppEPM
);

FTM_RET	FTM_OM_EPM_destroy
(
	FTM_OM_EPM_PTR _PTR_ ppEPM
);

FTM_RET	FTM_OM_EPM_init
(
	FTM_OM_PTR 		pOM, 
	FTM_OM_EPM_PTR 	pEPM
);

FTM_RET	FTM_OM_EPM_final
(
	FTM_OM_EPM_PTR 	pEPM
);

FTM_RET	FTM_OM_EPM_createEP
(
	FTM_OM_EPM_PTR 	pEPM, 
	FTM_EP_PTR 		pInfo, 
	FTM_OM_EP_PTR _PTR_ ppEP
);

FTM_RET	FTM_OM_EPM_destroyEP
(
	FTM_OM_EPM_PTR 	pEPM, 
	FTM_OM_EP_PTR 	pEP
);

FTM_RET	FTM_OM_EPM_attachEP
(
	FTM_OM_EPM_PTR 	pEPM, 
	FTM_OM_EP_PTR 	pEP
);

FTM_RET	FTM_OM_EPM_detachEP
(
	FTM_OM_EPM_PTR 	pEPM, 
	FTM_OM_EP_PTR 	pEP
);

FTM_RET	FTM_OM_EPM_count
(
	FTM_OM_EPM_PTR 	pEPM, 
	FTM_EP_TYPE 	xType, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET FTM_OM_EPM_getIDList
(
	FTM_OM_EPM_PTR 	pEPM, 
	FTM_EP_TYPE 	xType, 
	FTM_EP_ID_PTR 	pEPID, 
	FTM_ULONG 		ulMaxCount, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET FTM_OM_EPM_get
(
	FTM_OM_EPM_PTR 	pEPM, 
	FTM_EP_ID 		xEPID, 
	FTM_OM_EP_PTR _PTR_ ppEP
);

FTM_RET FTM_OM_EPM_getAt
(
	FTM_OM_EPM_PTR 	pEPM, 
	FTM_ULONG 		ulIndex, 
	FTM_OM_EP_PTR _PTR_ ppEP
);

FTM_RET	FTM_OM_EPM_notifyEPUpdated
(
	FTM_OM_EPM_PTR 	pEPM, 
	FTM_EP_ID 		xEPID, 
	FTM_EP_DATA_PTR pData
);


typedef	struct FTM_OM_EP_STRUCT
{
	FTM_EP			xInfo;

	FTM_OM_EPM_PTR		pEPM;
	FTM_OM_NODE_PTR 		pNode;

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
}	FTM_OM_EP;

FTM_RET	FTM_OM_EP_create
(
	FTM_EP_PTR 			pInfo,
	FTM_OM_EP_PTR _PTR_ ppEP
);

FTM_RET	FTM_OM_EP_destroy
(
	FTM_OM_EP_PTR _PTR_ ppEP
);

FTM_RET	FTM_OM_EP_attach
(
	FTM_OM_EP_PTR 		pEP, 
	FTM_OM_NODE_PTR 	pNode
);

FTM_RET	FTM_OM_EP_detach
(
	FTM_OM_EP_PTR 		pEP
);

FTM_RET FTM_OM_EP_start
(
	FTM_OM_EP_PTR 		pEP
);

FTM_RET FTM_OM_EP_stop
(
	FTM_OM_EP_PTR 		pEP, 
	FTM_BOOL 			bWaitForStop
);

FTM_RET	FTM_OM_EP_getDataType
(
	FTM_OM_EP_PTR 		pEP, 
	FTM_EP_DATA_TYPE_PTR pType
);

FTM_RET	FTM_OM_EP_setData
(
	FTM_OM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
);

FTM_RET	FTM_OM_EP_getData
(
	FTM_OM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
);

FTM_RET	FTM_OM_EP_updateData
(
	FTM_OM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
);

FTM_RET FTM_OM_EP_setFloat
(
	FTM_OM_EP_PTR 	pEP, 
	FTM_ULONG 		ulTime, 
	FTM_DOUBLE 		fValue
);

FTM_RET	FTM_OM_EP_getTriggerCount
(
	FTM_OM_EP_PTR 	pEP, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTM_OM_EP_getTriggerAt
(
	FTM_OM_EP_PTR 	pEP, 
	FTM_ULONG 		ulIndex, 
	FTM_TRIGGER_ID_PTR pTriggerID
);

FTM_RET FTM_OM_EP_trap
(
	FTM_OM_EP_PTR 	pEP, 
	FTM_EP_DATA_PTR pData)
;

#endif

