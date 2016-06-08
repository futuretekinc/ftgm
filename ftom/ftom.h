#ifndef	__FTOM_H__
#define	__FTOM_H__

#include "ftm.h"
#include "ftom_types.h"
#include "ftom_config.h"
#include "ftom_dmc.h"
#include "ftom_server.h"
#include "ftom_snmpc.h"
#include "ftom_snmptrapd.h"
#include "ftom_node.h"
#include "ftom_msg.h"

typedef	FTM_ULONG	FTOM_STATE;

#define	FTOM_STATE_INITIALIZED			0x00000001
#define	FTOM_STATE_SYNCHRONIZED			0x00000002
#define	FTOM_STATE_CONNECTED			0x00000003
#define	FTOM_STATE_PROCESSING			0x00000004
#define	FTOM_STATE_STOPED				0x00000005

typedef	struct
{
	FTM_CHAR		pDID[FTM_DID_LEN + 1];
}	FTOM_CONFIG, _PTR_ FTOM_CONFIG_PTR;

typedef	struct FTOM_EPM_STRUCT 			_PTR_ FTOM_EPM_PTR;
typedef struct FTOM_TRIGGERM_STRUCT 	_PTR_ FTOM_TRIGGERM_PTR;
typedef struct FTOM_ACTIONM_STRUCT 		_PTR_ FTOM_ACTIONM_PTR;
typedef struct FTOM_RULEM_STRUCT 		_PTR_ FTOM_RULEM_PTR;

typedef	FTM_RET	(*FTOM_ON_MESSAGE_CALLBACK)(FTOM_MSG_PTR pMsg, FTM_VOID_PTR pData);

FTM_CHAR_PTR	FTOM_getProgramName
(
	FTM_VOID
);

pid_t	FTOM_getPID
(
	FTM_VOID
);

FTM_RET	FTOM_init
(
	FTM_VOID
);

FTM_RET	FTOM_final
(
	FTM_VOID
);

FTM_RET	FTOM_loadFromFile
(
	FTM_CHAR_PTR 	pConfigFileName
);

FTM_RET	FTOM_showConfig
(
	FTM_VOID
);

FTM_RET FTOM_start
(
	FTM_VOID
);

FTM_RET FTOM_stop
(
	FTM_VOID
);

FTM_RET FTOM_waitingForFinished
(
	FTM_VOID
);

FTM_RET	FTOM_getDID
(
	FTM_CHAR_PTR 	pBuff, 
	FTM_ULONG 		ulBuffLen
);

FTM_RET	FTOM_DB_NODE_add
(
	FTM_NODE_PTR	pInfo
);

FTM_RET	FTOM_DB_NODE_remove
(
	FTM_CHAR_PTR	pDID
);

FTM_RET	FTOM_DB_NODE_count
(
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTOM_DB_NODE_getInfo
(
	FTM_CHAR_PTR	pDID,
	FTM_NODE_PTR	pInfo
);

FTM_RET	FTOM_DB_NODE_getAt
(
	FTM_ULONG		ulIndex,
	FTM_NODE_PTR	pInfo
);

FTM_RET	FTOM_DB_NODE_set
(
	FTM_CHAR_PTR	pDID,
	FTM_NODE_FIELD	xFields,
	FTM_NODE_PTR 	pInfo
);

FTM_RET	FTOM_DB_EP_add
(
	FTM_EP_PTR pInfo
);

FTM_RET	FTOM_DB_EP_remove
(
	FTM_CHAR_PTR	pEPID
);

FTM_RET	FTOM_DB_EP_getInfo
(
	FTM_CHAR_PTR	pEPID,
	FTM_EP_PTR		pInfo
);

FTM_RET	FTOM_DB_EP_getInfo
(
	FTM_CHAR_PTR	pEPID,
	FTM_EP_PTR		pInfo
);

FTM_RET	FTOM_DB_EP_setInfo
(
	FTM_CHAR_PTR	pEPID,
	FTM_EP_FIELD	xFields,
	FTM_EP_PTR		pInfo
);

FTM_RET	FTOM_DB_EP_getDataList
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG 		ulStart, 
	FTM_EP_DATA_PTR pDataList, 
	FTM_ULONG 		ulMaxCount, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTOM_DB_EP_getDataInfo
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR 	pulBeginTime, 
	FTM_ULONG_PTR 	pulEndTime, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTOM_DB_EP_getDataCount
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR 	ulCount
);

FTM_RET	FTOM_DB_EP_getDataInfo
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR	pulBegin,
	FTM_ULONG_PTR	pulEnd,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTOM_DB_EP_addData
(
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR 	pData
);

FTM_RET	FTOM_DB_EP_removeData
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulDeletedCount
);

FTM_RET	FTOM_DB_EP_removeDataWithTime
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulBegin,
	FTM_ULONG		ulEnd,
	FTM_ULONG_PTR	pulDeletedCount
);

FTM_RET	FTOM_DB_TRIGGER_add
(
	FTM_TRIGGER_PTR	pInfo
);

FTM_RET	FTOM_DB_TRIGGER_remove
(
	FTM_CHAR_PTR	pTriggerID
);

FTM_RET	FTOM_DB_TRIGGER_getInfo
(
	FTM_CHAR_PTR	pTriggerID,
	FTM_TRIGGER_PTR	pInfo
);

FTM_RET	FTOM_DB_TRIGGER_getInfoAt
(
	FTM_ULONG		ulIndex,
	FTM_TRIGGER_PTR	pInfo
);

FTM_RET	FTOM_DB_TRIGGER_setInfo
(
	FTM_CHAR_PTR		pTriggerID,
	FTM_TRIGGER_FIELD	xFields,
	FTM_TRIGGER_PTR		pInfo
);

FTM_RET	FTOM_DB_ACTION_add
(
	FTM_ACTION_PTR	pInfo
);

FTM_RET	FTOM_DB_ACTION_remove
(
	FTM_CHAR_PTR	pActionID
);

FTM_RET	FTOM_DB_ACTION_getInfo
(
	FTM_CHAR_PTR	pActionID,
	FTM_ACTION_PTR	pInfo
);

FTM_RET	FTOM_DB_ACTION_getInfoAt
(
	FTM_ULONG		ulIndex,
	FTM_ACTION_PTR	pInfo
);

FTM_RET	FTOM_DB_ACTION_setInfo
(
	FTM_CHAR_PTR	pActionID,
	FTM_ACTION_FIELD	xField,
	FTM_ACTION_PTR		pInfo
);

FTM_RET	FTOM_DB_RULE_add
(
	FTM_RULE_PTR	pInfo
);

FTM_RET	FTOM_DB_RULE_remove
(
	FTM_CHAR_PTR	pRuleID
);

FTM_RET	FTOM_DB_RULE_getInfo
(
	FTM_CHAR_PTR	pRuleID,
	FTM_RULE_PTR	pInfo
);

FTM_RET	FTOM_DB_RULE_getInfoAt
(
	FTM_ULONG		ulIndex,
	FTM_RULE_PTR	pInfo
);

FTM_RET	FTOM_DB_RULE_setInfo
(
	FTM_CHAR_PTR	pRuleID,
	FTM_RULE_FIELD	xFields,
	FTM_RULE_PTR	pInfo
);

FTM_RET	FTOM_nodeDiscovery
(
	FTM_CHAR_PTR	pNetwork,
	FTM_USHORT		usPort
);

FTM_RET	FTOM_SYS_EP_publishStatus
(
	FTM_CHAR_PTR	pEPID,
	FTM_BOOL		bStatus,
	FTM_ULONG		ulTimeout
);

FTM_RET	FTOM_SYS_EP_publishData
(
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR	pData,
	FTM_ULONG		ulCount
);

FTM_RET	FTOM_sendAlert
(
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR	pData
);

FTM_RET	FTOM_discoveryStart
(
	FTM_CHAR_PTR	pNetwork,
	FTM_USHORT		usPort,
	FTM_ULONG		ulRetryCount
);

FTM_RET	FTOM_discoveryIsFinished
(
	FTM_BOOL_PTR	pbFinished
);

FTM_RET	FTOM_receivedDiscovery
(
	FTM_CHAR_PTR	pName,
	FTM_CHAR_PTR	pDID,
	FTM_CHAR_PTR	pIP,
	FTM_EP_TYPE_PTR	pTypes,
	FTM_ULONG		ulCount
);

FTM_RET	FTOM_discoveryNodeCount
(
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTOM_discoveryEPCount
(
	FTOM_NODE_PTR	pNode,
	FTM_EP_TYPE		xType,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTOM_discoveryEP
(
	FTOM_NODE_PTR	pNode,
	FTM_EP_TYPE		xType,
	FTM_ULONG		ulIndex,
	FTM_EP_PTR		pEPInfo
);

FTM_RET	FTOM_sendMessage
(
	FTOM_MSG_PTR	pMsg
);

FTM_RET	FTOM_setMessageCallback
(
	FTOM_MSG_TYPE 	xMsg, 
	FTOM_ON_MESSAGE_CALLBACK	fMessageCB,
	FTM_VOID_PTR				pData,
	FTOM_ON_MESSAGE_CALLBACK _PTR_	pOldCB,
	FTM_VOID_PTR _PTR_			pOldData
);

FTM_RET	FTOM_NOTIFY_rule
(
	FTM_CHAR_PTR	pRuleID,
	FTM_RULE_STATE	xRuleState
);

FTM_RET	FTOM_NOTIFY_SNMPTrap
(
	FTM_CHAR_PTR 		pTrapMsg
);

#endif

