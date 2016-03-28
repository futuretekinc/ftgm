#ifndef	__FTM_OM_NODE_H__
#define	__FTM_OM_NODE_H__

#include <pthread.h>
#include "ftm.h"
#include "ftm_om_ep.h"

typedef	FTM_ULONG		FTM_OM_NODE_STATE;

#define	FTM_OM_NODE_STATE_CREATED					0x00000001
#define	FTM_OM_NODE_STATE_INITIALIZED				0x00000002
#define	FTM_OM_NODE_STATE_SYNCHRONIZED			0x00000003
#define	FTM_OM_NODE_STATE_PROCESS_INIT			0x00000004
#define	FTM_OM_NODE_STATE_RUN						0x00000005
#define	FTM_OM_NODE_STATE_STOP					0x00000006
#define	FTM_OM_NODE_STATE_RUNNING					0x00000007
#define	FTM_OM_NODE_STATE_PROCESS_FINISHED		0x00000008
#define	FTM_OM_NODE_STATE_FINISHED				0x00000009
#define	FTM_OM_NODE_STATE_ABORT					0x0000000A

#define	FTM_OM_NODE_STATE_TIMEOUT					0x0000000B
#define	FTM_OM_NODE_STATE_ERROR					0x0000000C
#define	FTM_OM_NODE_STATE_COMPLETED				0x0000000D

#define	FTM_OM_NODE_STATE_CANT_OPEN_SESSION		0x0000000E

typedef	struct FTM_OM_NODE_STRUCT _PTR_ FTM_OM_NODE_PTR;
typedef	struct FTM_OM_EP_STRUCT   _PTR_ FTM_OM_EP_PTR;

typedef	FTM_RET		(*FTM_OM_NODE_START)(FTM_OM_NODE_PTR pNode);
typedef	FTM_RET		(*FTM_OM_NODE_STOP)(FTM_OM_NODE_PTR pNode);
typedef	FTM_RET		(*FTM_OM_NODE_GET_EP_DATA)(FTM_OM_NODE_PTR pNode, FTM_OM_EP_PTR pEP, FTM_EP_DATA_PTR pData);
typedef	FTM_RET		(*FTM_OM_NODE_SET_EP_DATA)(FTM_OM_NODE_PTR pNode, FTM_OM_EP_PTR pEP, FTM_EP_DATA_PTR pData);

typedef	struct FTM_OM_NODE_STRUCT
{
	FTM_NODE			xInfo;
	FTM_LIST				xEPList;

	pthread_t				xPThread;
	pthread_mutex_t			xMutexLock;
	FTM_OM_NODE_STATE			xState;
	FTM_ULONG				ulRetry;
	int64_t					xTimeout;

	FTM_OM_NODE_START			fStart;
	FTM_OM_NODE_STOP			fStop;
	FTM_OM_NODE_GET_EP_DATA	fGetEPData;
	FTM_OM_NODE_SET_EP_DATA	fSetEPData;
}	FTM_OM_NODE;

FTM_RET FTM_OM_NODE_init
(
	FTM_VOID
);

FTM_RET FTM_OM_NODE_final
(
	FTM_VOID
);

FTM_RET	FTM_OM_NODE_create
(
	FTM_NODE_PTR 			pInfo, 
	FTM_OM_NODE_PTR _PTR_ 	ppNode
);

FTM_RET	FTM_OM_NODE_destroy
(
	FTM_OM_NODE_PTR	pNode
);

FTM_RET FTM_OM_NODE_get
(
	FTM_CHAR_PTR 	pDID, 
	FTM_OM_NODE_PTR _PTR_ ppNode
);

FTM_RET FTM_OM_NODE_getAt
(
	FTM_ULONG 		ulIndex, 
	FTM_OM_NODE_PTR _PTR_ ppNode
);

FTM_RET	FTM_OM_NODE_count
(
	FTM_ULONG_PTR pulCount
);

FTM_RET	FTM_OM_NODE_linkEP
(
	FTM_OM_NODE_PTR pNode, 
	FTM_OM_EP_PTR 	pEP
);

FTM_RET	FTM_OM_NODE_unlinkEP
(
	FTM_OM_NODE_PTR pNode, 
	FTM_OM_EP_PTR 	pEP
);

FTM_RET	FTM_OM_NODE_getEPCount
(
	FTM_OM_NODE_PTR pNode, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTM_OM_NODE_getEP
(
	FTM_OM_NODE_PTR pNode, 
	FTM_EP_ID 		xEPID, 
	FTM_OM_EP_PTR _PTR_ pEP
);

FTM_RET	FTM_OM_NODE_getEPAt
(
	FTM_OM_NODE_PTR pNode, 
	FTM_ULONG 		ulIndex, 
	FTM_OM_EP_PTR _PTR_ pEP
);

FTM_RET	FTM_OM_NODE_getEPData
(
	FTM_OM_NODE_PTR pNode, 
	FTM_OM_EP_PTR 	pEP, 
	FTM_EP_DATA_PTR pData
);

FTM_RET	FTM_OM_NODE_setEPData
(
	FTM_OM_NODE_PTR pNode, 
	FTM_OM_EP_PTR 	pEP, 
	FTM_EP_DATA_PTR pData
);


FTM_RET	FTM_OM_NODE_start
(
	FTM_OM_NODE_PTR pNode
);

FTM_RET	FTM_OM_NODE_stop
(
	FTM_OM_NODE_PTR pNode
);

FTM_CHAR_PTR	FTM_OM_NODE_stateToStr
(
	FTM_OM_NODE_STATE xState
);

#include "ftm_om_node_snmpc.h"

#endif

