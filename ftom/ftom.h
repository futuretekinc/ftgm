#ifndef	__FTOM_H__
#define	__FTOM_H__

#include "ftm.h"
#include "ftom_types.h"
#include "ftom_dmc.h"
#include "ftom_client.h"
#include "ftom_server.h"
#include "ftom_snmpc.h"
#include "ftom_snmptrapd.h"
#include "ftom_node.h"
#include "ftom_msg.h"

#define	FTOM_DEFAULT_SERVER_SESSION_COUNT	10

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

typedef	struct FTOM_EPM_STRUCT 		_PTR_ FTOM_EPM_PTR;
typedef	struct FTOM_STRUCT 			_PTR_ FTOM_PTR;
typedef struct FTOM_TRIGGERM_STRUCT _PTR_ FTOM_TRIGGERM_PTR;
typedef struct FTOM_ACTIONM_STRUCT 	_PTR_ FTOM_ACTIONM_PTR;
typedef struct FTOM_RULEM_STRUCT 	_PTR_ FTOM_RULEM_PTR;

typedef	FTM_RET	(*FTOM_ON_MESSAGE_CALLBACK)(FTOM_PTR pOM, FTOM_MSG_PTR pMsg);

typedef	struct FTOM_STRUCT
{
	FTOM_CONFIG			xConfig;

	FTOM_STATE			xState;
	pthread_t			xThread;

	FTM_BOOL			bStop;

	FTOM_NODEM_PTR		pNodeM;
	FTOM_EPM_PTR		pEPM;
	FTOM_MSG_QUEUE_PTR	pMsgQ;
	FTOM_TRIGGERM_PTR	pTriggerM;
	FTOM_ACTIONM_PTR	pActionM;
	FTOM_RULEM_PTR		pRuleM;
	FTM_SHELL			xShell;

	FTOM_ON_MESSAGE_CALLBACK	onMessage[FTOM_MSG_TYPE_MAX];
}	FTOM, _PTR_ FTOM_PTR;


FTM_RET	FTOM_create
(
	FTOM_PTR _PTR_ ppOM
);

FTM_RET	FTOM_destroy
(
	FTOM_PTR _PTR_ ppOM
);

FTM_RET	FTOM_init
(
	FTOM_PTR pOM
);

FTM_RET	FTOM_final
(
	FTOM_PTR pOM
);

FTM_RET	FTOM_loadFromFile
(
	FTOM_PTR 	pOM, 
	FTM_CHAR_PTR 	pConfigFileName
);

FTM_RET	FTOM_showConfig
(
	FTOM_PTR 	pOM
);

FTM_RET FTOM_start
(
	FTOM_PTR 	pOM
);

FTM_RET FTOM_stop
(
	FTOM_PTR 	pOM
);

FTM_RET FTOM_waitingForFinished
(
	FTOM_PTR 	pOM
);

FTM_RET	FTOM_getDID
(
	FTOM_PTR 		pOM, 
	FTM_CHAR_PTR 	pBuff, 
	FTM_ULONG 		ulBuffLen
);

FTM_RET	FTOM_createNode
(
	FTOM_PTR		pOM,
	FTM_NODE_PTR	pInfo,
	FTOM_NODE_PTR _PTR_	ppNode
);

FTM_RET	FTOM_destroyNode
(
	FTOM_PTR		pOM,
	FTOM_NODE_PTR _PTR_	ppNode
);

FTM_RET	FTOM_countNode
(
	FTOM_PTR		pOM,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTOM_getNode
(
	FTOM_PTR		pOM,
	FTM_CHAR		pDID[FTM_DID_LEN + 1],
	FTOM_NODE_PTR _PTR_ ppNode
);

FTM_RET	FTOM_getNodeAt
(
	FTOM_PTR		pOM,
	FTM_ULONG		ulIndex,
	FTOM_NODE_PTR _PTR_ ppNode
);

FTM_RET	FTOM_createEP
(
	FTOM_PTR pOM, 
	FTM_EP_PTR pInfo
);

FTM_RET	FTOM_destroyEP
(
	FTOM_PTR 	pOM, 
	FTM_EP_ID 	xEPID
);

FTM_RET	FTOM_getEPDataList
(
	FTOM_PTR 		pOM, 
	FTM_EP_ID 		xEPID, 
	FTM_ULONG 		ulStart, 
	FTM_EP_DATA_PTR pDataList, 
	FTM_ULONG 		ulMaxCount, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTOM_getEPDataInfo
(
	FTOM_PTR 		pOM, 
	FTM_EP_ID 		xEPID, 
	FTM_ULONG_PTR 	pulBeginTime, 
	FTM_ULONG_PTR 	pulEndTime, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTOM_getEPDataCount
(
	FTOM_PTR 		pOM, 
	FTM_EP_ID 		xEPID, 
	FTM_ULONG_PTR 	ulCount
);

FTM_RET	FTOM_NOTIFY_rule
(
	FTOM_PTR 		pOM,
	FTM_RULE_ID		xRuleID,
	FTM_RULE_STATE	xRuleState
);

FTM_RET	FTOM_NOTIFY_SNMPTrap
(
	FTOM_PTR 	pOM, 
	FTM_CHAR_PTR 		pTrapMsg
);

FTM_RET	FTOM_setEPData
(
	FTOM_PTR 			pOM, 
	FTM_EP_ID 			xEPID, 
	FTM_EP_DATA_PTR 	pData
);

FTM_RET	FTOM_saveEPData
(
	FTOM_PTR 			pOM, 
	FTM_EP_ID 			xEPID, 
	FTM_EP_DATA_PTR 	pData
);

FTM_RET	FTOM_sendEPData
(
	FTOM_PTR		pOM,
	FTM_EP_ID 		xEPID, 
	FTM_EP_DATA_PTR	pData,
	FTM_ULONG		ulCount
);

FTM_RET	FTOM_helloNode
(
	FTOM_PTR		pOM
);

FTM_RET	FTOM_sendAlert
(
	FTOM_PTR		pOM,
	FTM_EP_ID 		xEPID, 
	FTM_EP_DATA_PTR	pData
);

FTM_RET	FTOM_sendDiscovery
(
	FTOM_PTR		pOM,
	FTM_CHAR_PTR	pName,
	FTM_CHAR_PTR	pDID,
	FTM_EP_TYPE_PTR	pTypes,
	FTM_ULONG		ulCount
);
#endif

