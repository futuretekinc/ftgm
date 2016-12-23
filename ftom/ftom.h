#ifndef	__FTOM_H__
#define	__FTOM_H__

#include "ftm.h"
#include "ftom_types.h"
#include "ftom_config.h"
#include "ftom_modules.h"
#include "ftom_dmc.h"
#include "ftom_server.h"
#include "ftom_snmpc.h"
#include "ftom_node.h"
#include "ftom_event.h"
#include "ftom_action.h"
#include "ftom_rule.h"
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
typedef struct FTOM_EVENTM_STRUCT 	_PTR_ FTOM_EVENTM_PTR;
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

FTM_RET	FTOM_loadConfigFromFile
(
	FTM_CHAR_PTR 	pConfigFileName
);

FTM_RET	FTOM_saveConfigToFile
(
	FTM_CHAR_PTR	pFileName
);

FTM_RET	FTOM_loadConfig
(
	FTM_CONFIG_PTR	pConfig
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
	FTOM_SERVICE_TYPE	xService,
	FTOM_MSG_PTR		pMsg
);

FTM_RET	FTOM_setMessageCallback
(
	FTOM_MSG_TYPE 	xMsg, 
	FTOM_ON_MESSAGE_CALLBACK	fMessageCB,
	FTM_VOID_PTR				pData,
	FTOM_ON_MESSAGE_CALLBACK _PTR_	pOldCB,
	FTM_VOID_PTR _PTR_			pOldData
);

FTM_RET	FTOM_NOTIFY_SNMPTrap
(
	FTM_CHAR_PTR 		pTrapMsg
);

FTM_RET	FTOM_serverSync
(
	FTM_BOOL			bAutoRegister
);

FTM_RET	FTOM_getDefaultUpdateInterval
(
	FTM_ULONG_PTR	pulUpdateInterval
);

FTM_RET	FTOM_addNodeCreationLog
(
	FTOM_NODE_PTR	pNode
);

FTM_RET	FTOM_addNodeRemovalLog
(
	FTM_CHAR_PTR	pDID
);

FTM_RET	FTOM_addEPCreationLog
(
	FTOM_EP_PTR		pEP
);

FTM_RET	FTOM_addEPRemovalLog
(
	FTM_CHAR_PTR	pEPID
);

FTM_RET	FTOM_addEventCreationLog
(
	FTOM_EVENT_PTR	pEvent
);

FTM_RET	FTOM_addEventRemovalLog
(
	FTM_CHAR_PTR	pID
);

FTM_RET	FTOM_addActionCreationLog
(
	FTOM_ACTION_PTR	pAction
);

FTM_RET	FTOM_addActionRemovalLog
(
	FTM_CHAR_PTR	pID
);

FTM_RET	FTOM_addRuleCreationLog
(
	FTOM_RULE_PTR	pRule
);

FTM_RET	FTOM_addRuleRemovalLog
(
	FTM_CHAR_PTR	pID
);

FTM_RET	FTOM_addRuleCreationLog
(
	FTOM_RULE_PTR	pRule
);

FTM_RET	FTOM_addRuleRemovalLog
(
	FTM_CHAR_PTR	pID
);

FTM_RET	FTOM_getLogCount
(
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTOM_getLogsAt
(
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_LOG_PTR		pLogs,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTOM_removeLogsFrom
(
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulCount
);

#endif

