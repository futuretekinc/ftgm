#ifndef	__FTDM_H__
#define	__FTDM_H__

#include "ftm.h"
#include "ftdm_types.h"


#include "ftdm_cmd.h"
#include "ftdm_config.h"
#include "ftdm_node.h"
#include "ftdm_ep.h"
#include "ftdm_ep_class.h"
#include "ftdm_server.h"
#include "ftdm_logger.h"
#include "ftdm_modules.h"
#include "ftdm_sqlite.h"
#include "ftdm_action.h"
#include "ftdm_trigger.h"
#include "ftdm_rule.h"


FTM_RET	FTDM_create
(
	FTDM_PTR _PTR_	ppFTDM
);

FTM_RET	FTDM_destroy
(
	FTDM_PTR _PTR_	ppFTDM
);

FTM_RET	FTDM_init
(
	FTDM_PTR	pFTDM
);

FTM_RET	FTDM_final
(
	FTDM_PTR	pFTDM
);

FTM_RET FTDM_loadConfig
(
	FTDM_PTR	pFTDM,
	FTM_CONFIG_PTR	pConfig
);

FTM_RET	FTDM_loadFromFile
(
	FTDM_PTR	pFTDM,
	FTM_CHAR_PTR	pFileName
);

FTM_RET 	FTDM_loadObjectFromConfig
(
	FTDM_PTR	pFTDM,
	FTM_CONFIG_PTR		pConfig
);

FTM_RET	FTDM_loadObjectFromFile
(
	FTDM_PTR	pFTDM,
	FTM_CHAR_PTR		pFileName
);

FTM_RET	FTDM_loadObjectFromDB
(
	FTDM_PTR	pFTDM
);

FTM_RET	FTDM_saveObjectToDB
(
	FTDM_PTR	pFTDM
);

FTM_RET	FTDM_setDebugLevel
(
	FTM_ULONG		ulLevel
);

FTM_RET	FTDM_getSIS
(
	FTDM_PTR	pFTDM,
	FTDM_SIS_PTR _PTR_ ppSIS
);

FTM_RET	FTDM_getDBIF
(
	FTDM_PTR	pFTDM,
	FTDM_DBIF_PTR _PTR_ ppDBIF
);

FTM_RET	FTDM_getLogger
(
	FTDM_PTR	pFTDM,
	FTDM_LOGGER_PTR _PTR_ ppLogger
);

FTM_RET	FTDM_removeInvalidData
(
	FTDM_PTR	pFTDM
);	

FTM_RET	FTDM_showNodeList
(
	FTDM_PTR	pFTDM
);

FTM_RET	FTDM_createNode
(
	FTDM_PTR		pFTDM,
	FTM_NODE_PTR	xInfo,
	FTDM_NODE_PTR _PTR_ pNode
);

FTM_RET	FTDM_deleteNode
(
	FTDM_PTR	pFTDM,
	FTM_CHAR_PTR	pID
);

FTM_RET FTDM_isNodeExist
(
	FTDM_PTR	pFTDM,
 	FTM_CHAR_PTR	pDID,
	FTM_BOOL_PTR	pExist
);

FTM_RET	FTDM_getNodeCount
(
	FTDM_PTR	pFTDM,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTDM_getNode
(
	FTDM_PTR	pFTDM,
	FTM_CHAR_PTR	pID,
	FTDM_NODE_PTR _PTR_	ppNode
);

FTM_RET	FTDM_getNodeAt
(
	FTDM_PTR	pFTDM,
	FTM_ULONG	ulIndex,
	FTDM_NODE_PTR _PTR_	ppNode
);

FTM_RET	FTDM_getNodeIDList
(
	FTDM_PTR		pFTDM,
	FTM_DID_PTR		pDIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTDM_createEP
(
	FTDM_PTR	pFTDM,
	FTM_EP_PTR			xInfo,
	FTDM_EP_PTR _PTR_ 	pEP
);

FTM_RET	FTDM_deleteEP
(
	FTDM_PTR	pFTDM,
	FTM_CHAR_PTR		pID
);

FTM_RET	FTDM_getEPCount
(
	FTDM_PTR	pFTDM,
	FTM_EP_TYPE			xType,
	FTM_ULONG_PTR		pulCount
);

FTM_RET	FTDM_getEP
(
	FTDM_PTR	pFTDM,
	FTM_CHAR_PTR		pID,
	FTDM_EP_PTR _PTR_	ppEP
);

FTM_RET	FTDM_getEPAt
(
	FTDM_PTR	pFTDM,
	FTM_ULONG			ulIndex,
	FTDM_EP_PTR _PTR_	ppEP
);

FTM_RET	FTDM_getEPIDList
(
	FTDM_PTR	pFTDM,
	FTM_DID_PTR		pDIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTDM_createAction
(
	FTDM_PTR	pFTDM,
	FTM_ACTION_PTR			xInfo,
	FTDM_ACTION_PTR _PTR_ 	pAction
);

FTM_RET	FTDM_deleteAction
(
	FTDM_PTR	pFTDM,
	FTM_CHAR_PTR		pID
);

FTM_RET	FTDM_getActionCount
(
	FTDM_PTR	pFTDM,
	FTM_ULONG_PTR		pulCount
);

FTM_RET	FTDM_getAction
(
	FTDM_PTR	pFTDM,
	FTM_CHAR_PTR		pID,
	FTDM_ACTION_PTR _PTR_	ppAction
);

FTM_RET	FTDM_getActionAt
(
	FTDM_PTR	pFTDM,
	FTM_ULONG			ulIndex,
	FTDM_ACTION_PTR _PTR_	ppAction
);

FTM_RET	FTDM_getActionIDList
(
	FTDM_PTR	pFTDM,
	FTM_DID_PTR		pDIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTDM_createTrigger
(
	FTDM_PTR	pFTDM,
	FTM_TRIGGER_PTR			xInfo,
	FTDM_TRIGGER_PTR _PTR_ 	pTrigger
);

FTM_RET	FTDM_deleteTrigger
(
	FTDM_PTR	pFTDM,
	FTM_CHAR_PTR		pID
);

FTM_RET	FTDM_getTriggerCount
(
	FTDM_PTR	pFTDM,
	FTM_ULONG_PTR		pulCount
);

FTM_RET	FTDM_getTrigger
(
	FTDM_PTR	pFTDM,
	FTM_CHAR_PTR		pID,
	FTDM_TRIGGER_PTR _PTR_	ppTrigger
);

FTM_RET	FTDM_getTriggerAt
(
	FTDM_PTR	pFTDM,
	FTM_ULONG			ulIndex,
	FTDM_TRIGGER_PTR _PTR_	ppTrigger
);

FTM_RET	FTDM_getTriggerIDList
(
	FTDM_PTR	pFTDM,
	FTM_DID_PTR		pDIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTDM_createRule
(
	FTDM_PTR	pFTDM,
	FTM_RULE_PTR			xInfo,
	FTDM_RULE_PTR _PTR_ 	pRule
);

FTM_RET	FTDM_deleteRule
(
	FTDM_PTR	pFTDM,
	FTM_CHAR_PTR		pID
);

FTM_RET	FTDM_getRuleCount
(
	FTDM_PTR	pFTDM,
	FTM_ULONG_PTR		pulCount
);

FTM_RET	FTDM_getRule
(
	FTDM_PTR	pFTDM,
	FTM_CHAR_PTR		pID,
	FTDM_RULE_PTR _PTR_	ppRule
);

FTM_RET	FTDM_getRuleAt
(
	FTDM_PTR	pFTDM,
	FTM_ULONG			ulIndex,
	FTDM_RULE_PTR _PTR_	ppRule
);

FTM_RET	FTDM_getRuleIDList
(
	FTDM_PTR	pFTDM,
	FTM_DID_PTR		pDIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTDM_addLog
(
	FTDM_PTR	pFTDM,
	FTM_LOG_PTR	pLog
);

FTM_RET	FTDM_deleteLog
(
	FTDM_PTR	pFTDM,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulDeletedCount
);

FTM_RET	FTDM_getLogCount
(
	FTDM_PTR	pFTDM,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTDM_getLog
(
	FTDM_PTR	pFTDM,
	FTM_ULONG		ulIndex,
	FTM_LOG_PTR		pLogList,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
);

	
#endif

