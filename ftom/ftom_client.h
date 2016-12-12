#ifndef	__FTOM_CLIENT_H__
#define	__FTOM_CLIENT_H__

#include <pthread.h>
#include <semaphore.h>
#include "ftm.h"
#include "ftom_params.h"
#include "ftom_message_queue.h"

#define	FTOM_CLIENT_TYPE_NET	(0x01000000UL)
#define	FTOM_CLIENT_TYPE_TP		(FTOM_CLIENT_TYPE_NET | 0x01)
#define	FTOM_CLIENT_TYPE_AZURE	(FTOM_CLIENT_TYPE_NET | 0x02)

struct FTOM_CLIENT_CONFIG_STRUCT;

struct FTOM_CLIENT_STRUCT;

typedef	FTM_RET (*FTOM_CLIENT_NOTIFY_CB)
(
	FTOM_MSG_PTR	pMsg,
	FTM_VOID_PTR	pData
);

typedef	FTM_RET	(*FTOM_CLIENT_DESTROY)
(
	struct FTOM_CLIENT_STRUCT _PTR_ _PTR_ ppClient
);

typedef	FTM_RET	(*FTOM_CLIENT_INIT)
(
	struct FTOM_CLIENT_STRUCT _PTR_ pClient
);

typedef	FTM_RET	(*FTOM_CLIENT_FINAL)
(
	struct FTOM_CLIENT_STRUCT _PTR_ pClient
);

typedef	FTM_RET	(*FTOM_CLIENT_START)
(
	struct FTOM_CLIENT_STRUCT _PTR_	pClient
);

typedef	FTM_RET	(*FTOM_CLIENT_STOP)
(
	struct FTOM_CLIENT_STRUCT _PTR_	pClient
);

typedef FTM_RET	(*FTOM_CLIENT_WAITING_FOR_FINISHED)
(
	struct FTOM_CLIENT_STRUCT _PTR_	pClient
);

typedef	FTM_RET	(*FTOM_CLIENT_MESSAGE)
(
	struct FTOM_CLIENT_STRUCT _PTR_ pClient,
	FTOM_MSG_PTR					pMsg
);

typedef FTM_RET	(*FTOM_CLIENT_LOAD_CONFIG)
(
	struct FTOM_CLIENT_STRUCT _PTR_	pClient,
	FTM_CONFIG_PTR					pConfig
);

typedef FTM_RET	(*FTOM_CLIENT_SAVE_CONFIG)
(
	struct FTOM_CLIENT_STRUCT _PTR_	pClient,
	FTM_CONFIG_PTR					pConfig
);

typedef FTM_RET	(*FTOM_CLIENT_SHOW_CONFIG)
(
	struct FTOM_CLIENT_STRUCT _PTR_	pClient
);

typedef	FTM_RET	(*FTOM_CLIENT_SET_NOTIFY_CB)
(
	struct FTOM_CLIENT_STRUCT _PTR_	pClient,
	FTOM_CLIENT_NOTIFY_CB			fNotifyCB,
	FTM_VOID_PTR					pData
);

typedef	FTM_RET	(*FTOM_CLIENT_REQUEST)
(
	struct FTOM_CLIENT_STRUCT _PTR_	pClient, 
	FTOM_REQ_PARAMS_PTR				pReq,
	FTM_ULONG						ulReqLen,
	FTOM_RESP_PARAMS_PTR			pRespBuff,
	FTM_ULONG						ulRespBuffLen,
	FTM_ULONG_PTR					pulRespLen
);

typedef	struct	FTOM_CLIENT_FUCNTION_SET_STRUCT
{
	FTOM_CLIENT_DESTROY			fDestroy;

	FTOM_CLIENT_INIT			fInit;
	FTOM_CLIENT_FINAL			fFinal;

	FTOM_CLIENT_START			fStart;
	FTOM_CLIENT_STOP			fStop;
	FTOM_CLIENT_MESSAGE			fMessageProcess;

	FTOM_CLIENT_WAITING_FOR_FINISHED	fWaitingForFinished;
	FTOM_CLIENT_LOAD_CONFIG		fLoadConfig;
	FTOM_CLIENT_SAVE_CONFIG		fSaveConfig;
	FTOM_CLIENT_SHOW_CONFIG		fShowConfig;
	FTOM_CLIENT_SET_NOTIFY_CB	fSetNotifyCB;
	FTOM_CLIENT_REQUEST			fRequest;	
}	FTOM_CLIENT_FUNCTION_SET, _PTR_ FTOM_CLIENT_FUNCTION_SET_PTR;

typedef	struct	FTOM_CLIENT_STRUCT 
{
//	Object Type
	FTM_ULONG				xType;

//	Data
	FTM_VOID_PTR			pNotifyData;

//  Operator
	FTOM_CLIENT_DESTROY			fDestroy;

	FTOM_CLIENT_INIT			fInit;
	FTOM_CLIENT_FINAL			fFinal;

	FTOM_CLIENT_START			fStart;
	FTOM_CLIENT_STOP			fStop;
	FTOM_CLIENT_MESSAGE			fMessageProcess;

	FTOM_CLIENT_WAITING_FOR_FINISHED	fWaitingForFinished;
	FTOM_CLIENT_LOAD_CONFIG		fLoadConfig;
	FTOM_CLIENT_SAVE_CONFIG		fSaveConfig;
	FTOM_CLIENT_SHOW_CONFIG		fShowConfig;
	FTOM_CLIENT_SET_NOTIFY_CB	fSetNotifyCB;
	FTOM_CLIENT_REQUEST			fRequest;	
	FTOM_CLIENT_NOTIFY_CB 		fNotifyCB;


	struct
	{
		FTM_BOOL	bStop;
		pthread_t	xThread;
	}	xMain;

	FTOM_MSG_QUEUE_PTR			pMsgQ;
}	FTOM_CLIENT, _PTR_ FTOM_CLIENT_PTR;

#define	FTOM_CLIENT_TYPE_isCorrect(obj, type) ((((FTOM_CLIENT_PTR)(obj))->xType & (type)) == (type))

FTM_RET	FTOM_CLIENT_create
(
	FTM_ULONG	xType,
	FTOM_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTOM_CLIENT_destroy
(
	FTOM_CLIENT_PTR	_PTR_ ppClient
);

FTM_RET	FTOM_CLIENT_init
(
	FTOM_CLIENT_PTR	pClient
);

FTM_RET	FTOM_CLIENT_internalInit
(
	FTOM_CLIENT_PTR	pClient
);

FTM_RET	FTOM_CLIENT_final
(
	FTOM_CLIENT_PTR	pClient
);

FTM_RET	FTOM_CLIENT_internalFinal
(
	FTOM_CLIENT_PTR	pClient
);

FTM_RET	FTOM_CLIENT_start
(
	FTOM_CLIENT_PTR	pClient
);

FTM_RET	FTOM_CLIENT_stop
(
	FTOM_CLIENT_PTR	pClient
);

FTM_RET	FTOM_CLIENT_waitingForFinished
(
	FTOM_CLIENT_PTR	pClient
);

FTM_RET FTOM_CLIENT_sendMessage
(
	FTOM_CLIENT_PTR pClient,
	FTOM_MSG_PTR	pMsg
);

FTM_RET	FTOM_CLIENT_messageProcess
(
	FTOM_CLIENT_PTR pClient,
	FTOM_MSG_PTR	pMsg
);

FTM_RET	FTOM_CLIENT_loadConfigFromFile
(
	FTOM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR		pFileName
);

FTM_RET	FTOM_CLIENT_saveConfigToFile
(
	FTOM_CLIENT_PTR		pClient,
	FTM_CHAR_PTR		pFileName
);

FTM_RET	FTOM_CLIENT_loadConfig
(
	FTOM_CLIENT_PTR		pClient,
	FTM_CONFIG_PTR		pConfig
);

FTM_RET	FTOM_CLIENT_saveConfig
(
	FTOM_CLIENT_PTR		pClient,
	FTM_CONFIG_PTR		pConfig
);

FTM_RET	FTOM_CLIENT_showConfig
(
	FTOM_CLIENT_PTR		pClient
);

FTM_RET	FTOM_CLIENT_setNotifyCB
(
	FTOM_CLIENT_PTR			pClient,
	FTOM_CLIENT_NOTIFY_CB	pCB,
	FTM_VOID_PTR			pData
);

FTM_RET	FTOM_CLIENT_setNotifyInternalCB
(
	FTOM_CLIENT_PTR			pClient,
	FTOM_CLIENT_NOTIFY_CB	pCB,
	FTM_VOID_PTR			pData
);

FTM_RET	FTOM_CLIENT_setFunctionSet
(
	FTOM_CLIENT_PTR			pClient,
	FTOM_CLIENT_FUNCTION_SET_PTR	pFunctionSet
);

FTM_RET FTOM_CLIENT_NODE_create
(
	FTOM_CLIENT_PTR	pClient,
	FTM_NODE_PTR	pInfo,
	FTM_CHAR_PTR	pDID,
	FTM_ULONG		ulDIDLen
);

FTM_RET FTOM_CLIENT_NODE_destroy
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pDID
);

FTM_RET FTOM_CLIENT_NODE_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG_PTR	pulCount
);

FTM_RET FTOM_CLIENT_NODE_getAt
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG		ulIndex,
	FTM_NODE_PTR	pInfo
);

FTM_RET FTOM_CLIENT_NODE_get
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pDID,
	FTM_NODE_PTR	pInfo
);

FTM_RET FTOM_CLIENT_NODE_set
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pDID,
	FTM_NODE_FIELD	xFields,
	FTM_NODE_PTR	pInfo
);

FTM_RET FTOM_CLIENT_NODE_run
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pDID
);

FTM_RET FTOM_CLIENT_NODE_stop
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pDID
);

FTM_RET	FTOM_CLIENT_NODE_isRun
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pDID,
	FTM_BOOL_PTR	pbRun
);

FTM_RET	FTOM_CLIENT_NODE_registerAtServer
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pDID
);

FTM_RET	FTOM_CLIENT_NODE_setServerRegistered
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pDID,
	FTM_BOOL		bRegistered
);

FTM_RET	FTOM_CLIENT_NODE_getServerRegistered
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pDID,
	FTM_BOOL_PTR	pbRegistered
);

FTM_RET	FTOM_CLIENT_NODE_setReportInterval
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pDID,
	FTM_ULONG		ulReportInterval
);
/*****************************************************
 *
 *****************************************************/

FTM_RET FTOM_CLIENT_EP_create
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_PTR			pInfo,
	FTM_EP_PTR			pNewInfo
);

FTM_RET FTOM_CLIENT_EP_destroy
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID
);

FTM_RET	FTOM_CLIENT_EP_isRun
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_BOOL_PTR	pbRun
);

FTM_RET FTOM_CLIENT_EP_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_TYPE		xType,
	FTM_CHAR_PTR	pDID,
	FTM_ULONG_PTR	pnCount
);

FTM_RET	FTOM_CLIENT_EP_getList
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_TYPE		xType,
	FTM_CHAR_PTR	pDID,
	FTM_ULONG		ulIndex,
	FTM_CHAR		pEPIDList[][FTM_EPID_LEN+1],
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pnCount
);

FTM_RET FTOM_CLIENT_EP_get
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_PTR		pEPInfo
);

FTM_RET FTOM_CLIENT_EP_getAt
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG		nIndex,
	FTM_EP_PTR		pEPInfo
);

FTM_RET FTOM_CLIENT_EP_set
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_FIELD	xFields,
	FTM_EP_PTR		pInfo
);

FTM_RET	FTOM_CLIENT_EP_setReportInterval
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulReportInterval
);

FTM_RET	FTOM_CLIENT_EP_remoteSet
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR	pData
);

FTM_RET	FTOM_CLIENT_EP_setServerRegistered
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_BOOL		bRegistered
);

FTM_RET	FTOM_CLIENT_EP_getServerRegistered
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_BOOL_PTR	pbRegistered
);

/*****************************************************
 *
 *****************************************************/

FTM_RET	FTOM_CLIENT_EP_DATA_info
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR	pulBeginTime,
	FTM_ULONG_PTR	pulEndTime,
	FTM_ULONG_PTR	pCount
);

FTM_RET FTOM_CLIENT_EP_DATA_add
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR	pEPData
);

FTM_RET	FTOM_CLIENT_EP_DATA_getLast
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR	pEPData
);

FTM_RET FTOM_CLIENT_EP_DATA_getList
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		nStartIndex,
	FTM_EP_DATA_PTR	pData,
	FTM_ULONG		nMaxCount,
	FTM_ULONG_PTR	pnCount
);

FTM_RET FTOM_CLIENT_EP_DATA_getListWithTime
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		nBeginTime,
	FTM_ULONG		nEndTime,
	FTM_EP_DATA_PTR	pData,
	FTM_ULONG		nMaxCount,
	FTM_ULONG_PTR	pnCount
);

FTM_RET FTOM_CLIENT_EP_DATA_del
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		nIndex,
	FTM_ULONG		nCount,
	FTM_ULONG_PTR	pulDeletedCount
);

FTM_RET FTOM_CLIENT_EP_DATA_delWithTime
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		nBeginTime,
	FTM_ULONG		nEndTime,
	FTM_ULONG_PTR	pulDeletedCount
);

FTM_RET	FTOM_CLIENT_EP_DATA_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR	pCount
);

FTM_RET	FTOM_CLIENT_EP_DATA_countWithTime
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulStart,
	FTM_ULONG		ulEnd,
	FTM_ULONG_PTR	pCount
);

FTM_RET	FTOM_CLIENT_EP_DATA_type
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_VALUE_TYPE_PTR	pType
);

FTM_RET	FTOM_CLIENT_EP_DATA_setServerTime
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulTime
);

/*****************************************************************
 *
 *****************************************************************/

FTM_RET	FTOM_CLIENT_TRIGGER_add
(
	FTOM_CLIENT_PTR	pClient,
	FTM_TRIGGER_PTR	pTrigger
);

FTM_RET	FTOM_CLIENT_TRIGGER_del
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pTriggerID
);

FTM_RET	FTOM_CLIENT_TRIGGER_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTOM_CLIENT_TRIGGER_get
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pTriggerID,
	FTM_TRIGGER_PTR	pTriggerInfo
);

FTM_RET	FTOM_CLIENT_TRIGGER_getAt
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG		ulIndex,
	FTM_TRIGGER_PTR	pTriggerInfo
);

FTM_RET	FTOM_CLIENT_TRIGGER_set
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pTriggerID,
	FTM_TRIGGER_FIELD xFields,
	FTM_TRIGGER_PTR	pTriggerInfo
);

/*****************************************************************
 *
 *****************************************************************/

FTM_RET	FTOM_CLIENT_ACTION_add
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ACTION_PTR	pAction,
	FTM_CHAR_PTR	pActionID,
	FTM_ULONG		ulIDLen
);

FTM_RET	FTOM_CLIENT_ACTION_del
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pActionID
);

FTM_RET	FTOM_CLIENT_ACTION_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTOM_CLIENT_ACTION_get
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pActionID,
	FTM_ACTION_PTR	pAction
);

FTM_RET	FTOM_CLIENT_ACTION_getAt
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG		ulIndex,
	FTM_ACTION_PTR	pAction
);

FTM_RET	FTOM_CLIENT_ACTION_set
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pActionID,
	FTM_ACTION_FIELD	xFields,
	FTM_ACTION_PTR	pActionInfo
);

/*****************************************************************
 *
 *****************************************************************/

FTM_RET	FTOM_CLIENT_RULE_add
(
	FTOM_CLIENT_PTR	pClient,
	FTM_RULE_PTR	pRule,
	FTM_CHAR_PTR	pRuleID,
	FTM_ULONG		ulIDLen
);

FTM_RET	FTOM_CLIENT_RULE_del
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pRuleID
);

FTM_RET	FTOM_CLIENT_RULE_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTOM_CLIENT_RULE_get
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pRuleID,
	FTM_RULE_PTR	pRuleInfo
);

FTM_RET	FTOM_CLIENT_RULE_getAt
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG		ulIndex,
	FTM_RULE_PTR	pRuleInfo
);

FTM_RET	FTOM_CLIENT_RULE_set
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pRuleID,
	FTM_RULE_FIELD	xFields,
	FTM_RULE_PTR	pRuleInfo
);


/****************************************************************
 *
 ***************************************************************/
FTM_RET	FTOM_CLIENT_LOG_getList
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_LOG_PTR		pLogs,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTOM_CLIENT_LOG_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTOM_CLIENT_LOG_del
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulDeletedCount
);

/****************************************************************
 *
 ***************************************************************/
FTM_RET	FTOM_CLIENT_DISCOVERY_start
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pIP,
	FTM_USHORT		usPort,
	FTM_ULONG		ulRetryCount
);

FTM_RET	FTOM_CLIENT_DISCOVERY_getInfo
(
	FTOM_CLIENT_PTR	pClient,
	FTM_BOOL_PTR	pbFinished,
	FTM_ULONG_PTR	pulNodeCount,
	FTM_ULONG_PTR	pulEPCount
);

FTM_RET	FTOM_CLIENT_DISCOVERY_getNodeList
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_NODE_PTR	pNodeList,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTOM_CLIENT_DISCOVERY_getEPList
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_EP_PTR		pEPList,
	FTM_ULONG_PTR	pulCount
);

#endif

