#ifndef	__FTOM_NODE_H__
#define	__FTOM_NODE_H__

#include <pthread.h>
#include "ftm.h"
#include "ftom_ep.h"

typedef	FTM_ULONG		FTOM_NODE_STATE;

struct FTM_NODE_CLASS_STRUCT ;

#define	FTOM_NODE_STATE_CREATED					0x00000001
#define	FTOM_NODE_STATE_INITIALIZED				0x00000002
#define	FTOM_NODE_STATE_RUN						0x00000003
#define	FTOM_NODE_STATE_STOP					0x00000004
#define	FTOM_NODE_STATE_FINALIZED				0x00000005


typedef	struct FTOM_NODE_STRUCT
{
	FTM_NODE		xInfo;
	FTM_LIST		xEPList;

	FTM_BOOL		bStop;
	pthread_t		xThread;
	FTM_LOCK		xLock;
	FTOM_NODE_STATE	xState;
	FTOM_MSG_QUEUE	xMsgQ;

	FTM_TIMER		xReportTimer;

	struct
	{
		FTM_BOOL	bRegistered;	
		FTM_ULONG	ulLastReportTime;	
	}	xServer;

	struct
	{
		FTM_ULONG	ulGetCount;
		FTM_ULONG	ulGetError;
		FTM_ULONG	ulSetCount;
		FTM_ULONG	ulSetError;
		FTM_ULONG	ulTxCount;
		FTM_ULONG	ulRxCount;
		FTM_ULONG	ulTxError;
		FTM_ULONG	ulRxError;
		FTM_ULONG	ulInvalidFrame;
	}	xStatistics;

	struct FTOM_NODE_CLASS_STRUCT _PTR_	pClass;
}	FTOM_NODE, _PTR_ FTOM_NODE_PTR;

FTM_RET	FTOM_NODE_init
(
	FTM_VOID
);

FTM_RET FTOM_NODE_final
(
	FTM_VOID
);

FTM_RET	FTOM_NODE_create
(
	FTM_NODE_PTR 		pInfo, 
	FTM_BOOL			bNew,
	FTOM_NODE_PTR _PTR_ ppNode
);

FTM_RET	FTOM_NODE_createFromDB
(
	FTM_CHAR_PTR	pDID,
	FTOM_NODE_PTR _PTR_ ppNode
);

FTM_RET	FTOM_NODE_destroy
(
	FTOM_NODE_PTR _PTR_ ppNode,
	FTM_BOOL			bRemoveDB
);

FTM_RET	FTOM_NODE_count
(
	FTM_ULONG_PTR		pulCount
);

FTM_RET FTOM_NODE_get
(
	FTM_CHAR_PTR pDID, 
	FTOM_NODE_PTR _PTR_ ppNode
);

FTM_RET FTOM_NODE_getAt
(
	FTM_ULONG ulIndex, 
	FTOM_NODE_PTR _PTR_ ppNode
);

FTM_RET FTOM_NODE_getAttr
(
	FTOM_NODE_PTR	pNode,
	FTM_NODE_PTR 	pInfo
);

FTM_RET FTOM_NODE_setAttr
(
	FTOM_NODE_PTR	pNode,
	FTM_NODE_FIELD	xFields,
	FTM_NODE_PTR 	pInfo
);

FTM_RET	FTOM_NODE_linkEP
(
	FTOM_NODE_PTR 	pNode, 
	FTOM_EP_PTR 	pEP
);

FTM_RET	FTOM_NODE_unlinkEP
(
	FTOM_NODE_PTR 	pNode, 
	FTOM_EP_PTR 	pEP
);

FTM_RET	FTOM_NODE_getEPCount
(
	FTOM_NODE_PTR	pNode, 
	FTM_EP_TYPE		xType,
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTOM_NODE_getEPID
(
	FTOM_NODE_PTR	pNode,
	FTM_EP_TYPE		xEPType,
	FTM_ULONG		ulIndex,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulMaxLen
);

FTM_RET	FTOM_NODE_getEPName
(
	FTOM_NODE_PTR	pNode,
	FTM_EP_TYPE		xEPType,
	FTM_ULONG		ulIndex,
	FTM_CHAR_PTR	pName,
	FTM_ULONG		ulMaxLen
);

FTM_RET	FTOM_NODE_getEPState
(
	FTOM_NODE_PTR	pNode,
	FTM_EP_TYPE		xEPType,
	FTM_ULONG		ulIndex,
	FTM_BOOL_PTR	pbEnable
);

FTM_RET	FTOM_NODE_getEPUpdateInterval
(
	FTOM_NODE_PTR	pNode,
	FTM_EP_TYPE		xEPType,
	FTM_ULONG		ulIndex,
	FTM_ULONG_PTR	pulUpdateInterval
);

FTM_RET	FTOM_NODE_getEP
(
	FTOM_NODE_PTR 	pNode, 
	FTM_CHAR_PTR	pEPID,
	FTOM_EP_PTR _PTR_ pEP
);

FTM_RET	FTOM_NODE_getEPAt
(
	FTOM_NODE_PTR 	pNode, 
	FTM_ULONG 		ulIndex, 
	FTOM_EP_PTR _PTR_ pEP
);

FTM_RET	FTOM_NODE_getEPData
(
	FTOM_NODE_PTR pNode, 
	FTOM_EP_PTR 	pEP, 
	FTM_EP_DATA_PTR pData
);

FTM_RET	FTOM_NODE_setEPData
(
	FTOM_NODE_PTR 	pNode, 
	FTOM_EP_PTR 	pEP, 
	FTM_EP_DATA_PTR pData
);

FTM_RET	FTOM_NODE_getEPDataAsync
(
	FTOM_NODE_PTR	pNode,
	FTOM_EP_PTR		pEP
);

FTM_RET	FTOM_NODE_setEPDataAsync
(
	FTOM_NODE_PTR 	pNode, 
	FTOM_EP_PTR 	pEP, 
	FTM_EP_DATA_PTR pData
);

FTM_RET	FTOM_NODE_setReportInterval
(
	FTOM_NODE_PTR	pNode,
	FTM_ULONG		ulInterval
);

FTM_RET	FTOM_NODE_getReportInterval
(
	FTOM_NODE_PTR	pNode,
	FTM_ULONG_PTR	pulInterval
);

FTM_RET	FTOM_NODE_start
(
	FTOM_NODE_PTR 	pNode
);

FTM_RET	FTOM_NODE_stop
(
	FTOM_NODE_PTR 	pNode
);

FTM_RET	FTOM_NODE_isRun
(
	FTOM_NODE_PTR	pNode,
	FTM_BOOL_PTR	pbRun
);

FTM_CHAR_PTR	FTOM_NODE_stateToStr
(
	FTOM_NODE_STATE xState
);

FTM_RET	FTOM_NODE_print
(
	FTOM_NODE_PTR	pNode
);

FTM_RET	FTOM_NODE_printList
(
	FTM_VOID
);

#endif

