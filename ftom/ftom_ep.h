#ifndef	__FTOM_EP_H__
#define	__FTOM_EP_H__

#include <pthread.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include "ftm.h"
#include "ftom_msg.h"
#include "ftom_message_queue.h"
#include "ftm_snmp.h"

typedef	struct FTOM_EP_STATISTICS_STRUCT
{
	FTM_ULONG	ulReqCount;
	FTM_ULONG	ulRespCount;
}	FTOM_EP_STATISTICS, _PTR_ FTOM_EP_STATISTICS_PTR;
	
typedef	struct FTOM_EP_STRUCT
{
	FTM_EP					xInfo;

	struct FTOM_EPM_STRUCT _PTR_	pEPM;
	struct FTOM_NODE_STRUCT _PTR_	pNode;

	FTM_BOOL			bStop;
	FTM_ULONG			ulRetryCount;

	pthread_t			xPThread;
	FTM_LOCK_PTR		pLock;
	FTOM_MSG_QUEUE		xMsgQ;

	FTM_LIST			xDataList;
	FTM_LIST			xTriggerList;

	pthread_t			xUpdateThread;
	FTM_TIMER			xUpdateTimer;
	FTM_TIMER			xReportTimer;
	FTM_TIME			xServerTime;
	struct
	{
		FTM_BOOL		bRegistered;
		FTM_ULONG		ulLastReportTime;	
	}	xServer;

	FTM_VOID_PTR		pOpts;
}	FTOM_EP, _PTR_ FTOM_EP_PTR;

typedef	struct FTOM_EP_CLASS_STRUCT
{
	FTM_EP_CLASS	xInfo;
}	FTOM_EP_CLASS, _PTR_ FTOM_EP_CLASS_PTR;

FTM_RET	FTOM_EP_init
(
	FTM_VOID
);

FTM_RET	FTOM_EP_final
(
	FTM_VOID
);

FTM_RET	FTOM_EP_create
(
	FTM_EP_PTR 			pInfo,
	FTM_BOOL			bNew,
	FTOM_EP_PTR _PTR_ 	ppEP
);

FTM_RET	FTOM_EP_destroy
(
	FTOM_EP_PTR _PTR_ 	ppEP,
	FTM_BOOL			bRemoveDB
);

FTM_RET	FTOM_EP_count
(
	FTM_EP_TYPE			xType,
	FTM_CHAR_PTR		pDID,
	FTM_ULONG_PTR		pulCount
);

FTM_RET FTOM_EP_getIDList
(
	FTM_EP_TYPE		xType,
	FTM_CHAR_PTR	pDID,
	FTM_CHAR		pEPID[][FTM_EPID_LEN+1],
	FTM_ULONG 		ulMaxCount, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTOM_EP_get
(
	FTM_CHAR_PTR		pEPID,
	FTOM_EP_PTR _PTR_ 	ppEP
);

FTM_RET FTOM_EP_getAt
(
	FTM_ULONG 			ulIndex, 
	FTOM_EP_PTR _PTR_ 	ppEP
);

FTM_RET	FTOM_EP_setInfo
(
	FTOM_EP_PTR			pEP,
	FTM_EP_FIELD		xFields,
	FTM_EP_PTR			pInfo
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

FTM_RET FTOM_EP_isRun
(
	FTOM_EP_PTR 		pEP, 
	FTM_BOOL_PTR		pbRun
);

/**************************************************
 * Data management operations
 **************************************************/
FTM_RET	FTOM_EP_getDataType
(
	FTOM_EP_PTR 		pEP, 
	FTM_VALUE_TYPE_PTR	pType
);

FTM_RET	FTOM_EP_setData
(
	FTOM_EP_PTR			pEP,
	FTM_EP_DATA_PTR		pData
);

FTM_RET	FTOM_EP_getData
(
	FTOM_EP_PTR			pEP,
	FTM_EP_DATA_PTR		pData
);

FTM_RET	FTOM_EP_getDataInfo
(
	FTOM_EP_PTR		pEP,
	FTM_ULONG_PTR	pulBegin,
	FTM_ULONG_PTR	pulEnd,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTOM_EP_setDataLimit
(
	FTOM_EP_PTR			pEP,
	FTM_EP_LIMIT_PTR	pLimit
);

FTM_RET	FTOM_EP_getDataCount
(
	FTOM_EP_PTR			pEP,
	FTM_ULONG_PTR		pulCount
);

FTM_RET	FTOM_EP_getDataList
(
	FTOM_EP_PTR		pEP,
	FTM_ULONG		ulIndex,
	FTM_EP_DATA_PTR	pDatas,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount,
	FTM_BOOL_PTR	pbRemain
);

FTM_RET	FTOM_EP_getDataListWithTime
(
	FTOM_EP_PTR		pEP,
	FTM_ULONG		ulBegin,
	FTM_ULONG		ulEnd,
	FTM_BOOL		bAscending,
	FTM_EP_DATA_PTR	pDatas,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount,
	FTM_BOOL_PTR	pbRemain
);

FTM_RET	FTOM_EP_removeDataList
(
	FTOM_EP_PTR 		pEP, 
	FTM_INT				nIndex,
	FTM_ULONG			ulCount,
	FTM_ULONG_PTR		pulDeletedCount
);

FTM_RET	FTOM_EP_removeDataListWithTime
(
	FTOM_EP_PTR 		pEP, 
	FTM_ULONG			ulBegin,
	FTM_ULONG			ulEnd,
	FTM_ULONG_PTR		pulDeletedCount
);

FTM_RET FTOM_EP_setFloat
(
	FTOM_EP_PTR 	pEP, 
	FTM_ULONG 		ulTime, 
	FTM_DOUBLE 		fValue
);

FTM_RET	FTOM_EP_setServerTime
(
	FTOM_EP_PTR		pEP,
	FTM_ULONG		ulTime
);

/********************************************
 * Trigger management operations
 ********************************************/
FTM_RET	FTOM_EP_getTriggerCount
(
	FTOM_EP_PTR 	pEP, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTOM_EP_getTriggerAt
(
	FTOM_EP_PTR 	pEP, 
	FTM_ULONG 		ulIndex, 
	FTM_CHAR_PTR	pTriggerID
);

FTM_RET	FTOM_EP_setReportInterval
(
	FTOM_EP_PTR		pEP,
	FTM_ULONG		ulInterval
);

FTM_RET	FTOM_EP_getReportInterval
(
	FTOM_EP_PTR		pEP,
	FTM_ULONG_PTR	pulInterval
);

FTM_RET	FTOM_EP_print
(
	FTOM_EP_PTR		pEP
);

FTM_RET	FTOM_EP_printList
(
	FTM_VOID
);

FTM_RET	FTOM_EP_sendMessage
(
	FTOM_EP_PTR		pEP,
	FTOM_MSG_PTR	pMsg
);

FTM_RET	FTOM_EP_isAsyncMode
(
	FTOM_EP_PTR		pEP
);

FTM_RET	FTOM_EP_CLASS_create
(
	FTM_EP_CLASS_PTR 	pInfo,
	FTOM_EP_CLASS_PTR _PTR_	ppEPClass
);

FTM_RET	FTOM_EP_CLASS_destroy
(
	FTOM_EP_CLASS_PTR _PTR_	ppEPClass
);

FTM_RET FTOM_EP_CLASS_get
(
	FTM_EP_TYPE 	xType, 
	FTOM_EP_CLASS_PTR _PTR_ ppEPClass
);

FTM_RET FTOM_EP_CLASS_getAt
(
	FTM_ULONG 		ulIndex, 
	FTOM_EP_CLASS_PTR _PTR_ ppEPClass
);

FTM_RET	FTOM_EP_CLASS_print
(
	FTOM_EP_CLASS_PTR	pClass
);

/******************************************
 * Remote device control
 ******************************************/
FTM_RET	FTOM_EP_remoteSetData
(
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
);

FTM_RET	FTOM_EP_remoteGetData
(
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
);

FTM_RET	FTOM_EP_remoteGetDataAsync
(
	FTOM_EP_PTR 		pEP 
);

FTM_RET	FTOM_EP_remoteSetDataAsync
(
	FTOM_EP_PTR 		pEP,
	FTM_EP_DATA_PTR 	pData
);
#endif

