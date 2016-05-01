#ifndef	__FTOM_CLIENT_H__
#define	__FTOM_CLIENT_H__

#include <pthread.h>
#include <semaphore.h>
#include "ftm.h"
#include "ftom_params.h"
#include "ftom_client_config.h"

typedef	struct	FTOM_CLIENT_STRUCT _PTR_ FTOM_CLIENT_PTR;
typedef	union	FTOM_CLIENT_CONFIG_UNION _PTR_ FTOM_CLIENT_CONFIG_PTR;

typedef	FTM_RET	(*FTOM_CLIENT_START)
(
	FTOM_CLIENT_PTR	pClient
);

typedef	FTM_RET	(*FTOM_CLIENT_STOP)
(
	FTOM_CLIENT_PTR	pClient
);

typedef FTM_RET	(*FTOM_CLIENT_LOAD_CONFIG)
(
	FTOM_CLIENT_PTR			pClient,
	FTOM_CLIENT_CONFIG_PTR	pConfig
);

typedef	FTM_RET (*FTOM_CLIENT_NOTIFY_CALLBACK)
(
	FTM_VOID_PTR	pData
);

typedef	FTM_RET	(*FTOM_CLIENT_LOAD_CONFIG_FROM_FILE)
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pFileName
);

typedef	FTM_RET	(*FTOM_CLIENT_SET_NOTIFY_CALLBACK)
(
	FTOM_CLIENT_PTR	pClient,
	FTOM_CLIENT_NOTIFY_CALLBACK	fCB
);

typedef	FTM_RET	(*FTOM_CLIENT_REQUEST)
(
	FTOM_CLIENT_PTR			pClient, 
	FTOM_REQ_PARAMS_PTR		pReq,
	FTM_ULONG				ulReqLen,
	FTOM_RESP_PARAMS_PTR	pRespBuff,
	FTM_ULONG				ulRespBuffLen,
	FTM_ULONG_PTR			pulRespLen
);

typedef	struct	FTOM_CLIENT_STRUCT 
{
	FTOM_CLIENT_START					fStart;
	FTOM_CLIENT_STOP					fStop;
	FTOM_CLIENT_LOAD_CONFIG				fLoadConfig;
	FTOM_CLIENT_LOAD_CONFIG_FROM_FILE	fLoadConfigFromFile;
	FTOM_CLIENT_SET_NOTIFY_CALLBACK		fSetNotifyCallback;
	FTOM_CLIENT_REQUEST					fRequest;	

}	FTOM_CLIENT, _PTR_ FTOM_CLIENT_PTR;

FTM_RET	FTOM_CLIENT_start
(
	FTOM_CLIENT_PTR	pClient
);

FTM_RET	FTOM_CLIENT_stop
(
	FTOM_CLIENT_PTR	pClient
);

#if 0
FTM_RET	FTOM_CLIENT_ReadConfig
(
	FTOM_CLIENT_CONFIG_PTR 	pConfig, 
	FTM_CHAR_PTR 			pFileName
);
#endif

FTM_RET	FTOM_CLIENT_loadConfig
(
	FTOM_CLIENT_PTR			pClient,
	FTM_VOID_PTR			pConfig
);

FTM_RET	FTOM_CLIENT_loadConfigFromFile
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pFileName
);

FTM_RET	FTOM_CLIENT_setNotifyCallback
(
	FTOM_CLIENT_PTR	pClient,
	FTOM_CLIENT_NOTIFY_CALLBACK	pCB
);

FTM_RET FTOM_CLIENT_NODE_create
(
	FTOM_CLIENT_PTR	pClient,
	FTM_NODE_PTR		pInfo,
	FTM_CHAR_PTR		pDID,
	FTM_ULONG			ulDIDLen
);

FTM_RET FTOM_CLIENT_NODE_destroy
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pDID
);

FTM_RET FTOM_CLIENT_NODE_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG_PTR		pulCount
);

FTM_RET FTOM_CLIENT_NODE_getAt
(
	FTOM_CLIENT_PTR	pClient,
	FTM_ULONG			ulIndex,
	FTM_NODE_PTR		pInfo
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

FTM_RET FTOM_CLIENT_EP_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_TYPE		xType,
	FTM_ULONG_PTR	pnCount
);

FTM_RET	FTOM_CLIENT_EP_getList
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_TYPE		xType,
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

FTM_RET FTOM_CLIENT_EP_DATA_getWithTime
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
	FTM_ULONG		nCount
);

FTM_RET FTOM_CLIENT_EP_DATA_delWithTime
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		nBeginTime,
	FTM_ULONG		nEndTime
);

FTM_RET	FTOM_CLIENT_EP_DATA_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR	pCount
);

FTM_RET	FTOM_CLIENT_EP_DATA_type
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_TYPE_PTR pType
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

#endif

