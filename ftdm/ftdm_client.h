#ifndef	__FTDM_CLIENT_H__
#define	__FTDM_CLIENT_H__

#include "ftdm_types.h"
#include "ftdm_cmd.h"
#include "ftdm_params.h"
#include "ftdm_client_config.h"

typedef struct 
{
	FTM_INT		hSock;
	FTM_INT		nTimeout;
	FTM_LOCK	xLock;
}	FTDMC_SESSION, _PTR_ FTDMC_SESSION_PTR;

FTM_RET	FTDMC_init
(
	FTDMC_CFG_PTR pConfig
);

FTM_RET FTDMC_final
(
	FTM_VOID
);

FTM_RET FTDMC_connect
(
	FTDMC_SESSION_PTR 		pSession,
	FTM_IP_ADDR				xIP,
	FTM_USHORT 				usPort 
);

FTM_RET FTDMC_disconnect
(
	FTDMC_SESSION_PTR		pSession
);

FTM_RET FTDMC_isConnected
(
	FTDMC_SESSION_PTR		pSession,
	FTM_BOOL_PTR			pbConnected
);

FTM_RET FTDMC_NODE_append
(
	FTDMC_SESSION_PTR		pSession,
	FTM_NODE_PTR			pInfo
);

FTM_RET FTDMC_NODE_remove
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pDID
);

FTM_RET FTDMC_NODE_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pnCount
);

FTM_RET FTDMC_NODE_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				ulIndex,
	FTM_NODE_PTR			pNode
);

FTM_RET FTDMC_NODE_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pDID,
	FTM_NODE_PTR			pNode
);

FTM_RET FTDMC_NODE_set
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pDID,
	FTM_NODE_FIELD			xFields,
	FTM_NODE_PTR			pNode
);

FTM_RET	FTDMC_NODE_getDIDList
(	
	FTDMC_SESSION_PTR		pSession,
	FTM_DID_PTR				pDIDs,
	FTM_ULONG				ulIndex,
	FTM_ULONG				ulMaxCount,
	FTM_ULONG_PTR			pulCount
);

FTM_RET FTDMC_EP_append
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_PTR				pEP
);

FTM_RET FTDMC_EP_remove
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pEPID
);

FTM_RET FTDMC_EP_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_TYPE				xType,
	FTM_ULONG_PTR			pnCount
);

FTM_RET FTDMC_EP_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pEPID,
	FTM_EP_PTR				pEP
);

FTM_RET FTDMC_EP_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				ulIndex,
	FTM_EP_PTR				pEP
);

FTM_RET	FTDMC_EP_set
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pEPID,
	FTM_NODE_FIELD			xFields,
	FTM_EP_PTR				pEP
);

FTM_RET	FTDMC_EP_getEPIDList
(	
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID_PTR			pEPIDs,
	FTM_ULONG				ulIndex,
	FTM_ULONG				ulMaxCount,
	FTM_ULONG_PTR			pulCount
);

FTM_RET	FTDMC_EP_CLASS_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pnCount
);

FTM_RET	FTDMC_EP_CLASS_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_TYPE				xType,
	FTM_EP_CLASS_PTR		pEPClass
);

FTM_RET	FTDMC_EP_CLASS_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				ulIndex,
	FTM_EP_CLASS_PTR		pEPClass
);

FTM_RET FTDMC_EP_DATA_append
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pEPID,
	FTM_EP_DATA_PTR			pData
);

FTM_RET FTDMC_EP_DATA_remove
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pEPID,
	FTM_ULONG				ulIndex,
	FTM_ULONG				nCount,
	FTM_ULONG_PTR			pulDeletedCount
);

FTM_RET FTDMC_EP_DATA_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pEPID,
	FTM_ULONG				nStartIndex,
	FTM_EP_DATA_PTR			pData,
	FTM_ULONG				ulMaxCount,
	FTM_ULONG_PTR			pnCount
);

FTM_RET	FTDMC_EP_DATA_removeWithTime
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pEPID,
	FTM_ULONG				nBeginTime,
	FTM_ULONG				nEndTime,
	FTM_ULONG_PTR			pulCount
);

FTM_RET FTDMC_EP_DATA_info
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pEPID,
	FTM_ULONG_PTR			pulBeginTime,
	FTM_ULONG_PTR			pulEndTime,
	FTM_ULONG_PTR			pulCount
);

FTM_RET FTDMC_EP_DATA_setLimit
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pEPID,
	FTM_EP_LIMIT_PTR		pLimit
);

FTM_RET FTDMC_EP_DATA_getWithTime
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pEPID,
	FTM_ULONG				ulBeginTime,
	FTM_ULONG				ulEndTime,
	FTM_EP_DATA_PTR			pData,
	FTM_ULONG				ulMaxCount,
	FTM_ULONG_PTR			pnCount
);

FTM_RET	FTDMC_EP_DATA_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pEPID,
	FTM_ULONG_PTR			pCount
);

FTM_RET	FTDMC_EP_DATA_countWithTime
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pEPID,
	FTM_ULONG				ulBeginTime,
	FTM_ULONG				ulEndTime,
	FTM_ULONG_PTR			pCount
);

FTM_RET	FTDMC_TRIGGER_add
(
	FTDMC_SESSION_PTR		pSession,
	FTM_TRIGGER_PTR     		pTrigger
);

FTM_RET	FTDMC_TRIGGER_del
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pTriggerID
);

FTM_RET	FTDMC_TRIGGER_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pulCount
);

FTM_RET	FTDMC_TRIGGER_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pTriggerID,
	FTM_TRIGGER_PTR			pTrigger
);

FTM_RET	FTDMC_TRIGGER_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				ulIndex,
	FTM_TRIGGER_PTR			pTrigger
);

FTM_RET	FTDMC_TRIGGER_set
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pTriggerID,
	FTM_TRIGGER_FIELD		xFields,
	FTM_TRIGGER_PTR			pTrigger
);

FTM_RET	FTDMC_TRIGGER_getIDList
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ID_PTR				pIDs,
	FTM_ULONG				ulIndex,
	FTM_ULONG				ulMaxCount,
	FTM_ULONG_PTR			pulCount
);

FTM_RET	FTDMC_ACTION_add
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ACTION_PTR     		pAction
);

FTM_RET	FTDMC_ACTION_del
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pActionID
);

FTM_RET	FTDMC_ACTION_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pulCount
);

FTM_RET	FTDMC_ACTION_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pActionID,
	FTM_ACTION_PTR			pAction
);

FTM_RET	FTDMC_ACTION_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				ulIndex,
	FTM_ACTION_PTR				pAction
);

FTM_RET	FTDMC_ACTION_set
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pActionID,
	FTM_ACTION_FIELD		xFields,
	FTM_ACTION_PTR			pInfo
);

FTM_RET	FTDMC_ACTION_getIDList
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ID_PTR				pIDs,
	FTM_ULONG				ulIndex,
	FTM_ULONG				ulMaxCount,
	FTM_ULONG_PTR			pulCount
);

FTM_RET	FTDMC_RULE_add
(
	FTDMC_SESSION_PTR		pSession,
	FTM_RULE_PTR     		pRule
);

FTM_RET	FTDMC_RULE_del
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pRuleID
);

FTM_RET	FTDMC_RULE_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pulCount
);

FTM_RET	FTDMC_RULE_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pRuleID,
	FTM_RULE_PTR			pRule
);

FTM_RET	FTDMC_RULE_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				ulIndex,
	FTM_RULE_PTR				pRule
);

FTM_RET	FTDMC_RULE_set
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pRuleID,
	FTM_RULE_FIELD			xFields,
	FTM_RULE_PTR			pInfo
);

FTM_RET	FTDMC_RULE_getIDList
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ID_PTR				pIDs,
	FTM_ULONG				ulIndex,
	FTM_ULONG				ulMaxCount,
	FTM_ULONG_PTR			pulCount
);


FTM_RET	FTDMC_LOG_add
(
	FTDMC_SESSION_PTR		pSession,
	FTM_LOG_PTR     		pLog
);

FTM_RET	FTDMC_LOG_del
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				ulIndex,
	FTM_ULONG				ulCount,
	FTM_ULONG_PTR			pulDeletedCount
);

FTM_RET	FTDMC_LOG_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pulCount
);

FTM_RET	FTDMC_LOG_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				ulIndex,
	FTM_ULONG				ulCount,
	FTM_LOG_PTR				pLogs,
	FTM_ULONG_PTR			pulCount
);

FTM_RET	FTDMC_LOG_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				ulIndex,
	FTM_LOG_PTR				pLog
);
#endif

