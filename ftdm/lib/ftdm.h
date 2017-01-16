#ifndef	__FTDM_H__
#define	__FTDM_H__

#include "ftm.h"
#include "ftdm_types.h"

struct FTDM_DBIF_STRUCT ;
struct FTDM_NODEM_STRUCT;
struct FTDM_EPM_STRUCT;
//struct FTDM_TRIGGERM_STRUCT;
struct FTDM_ACTIONM_STRUCT;
struct FTDM_RULEM_STRUCT;
struct FTDM_LOGGER_STRUCT;
struct FTDM_CONTEXT_STRUCT;

#include "ftdm_cmd.h"
#include "ftdm_config.h"
#include "ftdm_node.h"
#include "ftdm_ep.h"
#include "ftdm_ep_management.h"
#include "ftdm_ep_class.h"
#include "ftdm_server.h"
#include "ftdm_logger.h"
#include "ftdm_modules.h"
#include "ftdm_sqlite.h"
#include "ftdm_action.h"
#include "ftdm_trigger.h"
#include "ftdm_rule.h"

typedef	struct FTDM_CONTEXT_STRUCT _PTR_ FTDM_CONTEXT_PTR;

FTM_RET	FTDM_create
(
	FTDM_CONTEXT_PTR _PTR_	ppFTDM
);

FTM_RET	FTDM_destroy
(
	FTDM_CONTEXT_PTR _PTR_	ppFTDM
);

FTM_RET	FTDM_init
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM
);

FTM_RET	FTDM_final
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM
);

FTM_RET FTDM_loadConfig
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_CONFIG_PTR 		pConfig
);

FTM_RET	FTDM_loadFromFile
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_CHAR_PTR		pFileName
);

FTM_RET 	FTDM_loadObjectFromConfig
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_CONFIG_PTR		pConfig
);

FTM_RET	FTDM_loadObjectFromFile
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_CHAR_PTR		pFileName
);

FTM_RET	FTDM_loadObjectFromDB
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM
);

FTM_RET	FTDM_saveObjectToDB
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM
);

FTM_RET	FTDM_setDebugLevel
(
	FTM_ULONG		ulLevel
);

FTM_RET	FTDM_getSIS
(
	struct FTDM_CONTEXT_STRUCT _PTR_ pFTDM,
	FTDM_SIS_PTR _PTR_ ppSIS
);

FTM_RET	FTDM_getDBIF
(
	struct FTDM_CONTEXT_STRUCT _PTR_ pFTDM,
	struct FTDM_DBIF_STRUCT _PTR_ _PTR_ ppDBIF
);

FTM_RET	FTDM_getNodeM
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	struct FTDM_NODEM_STRUCT _PTR_ _PTR_ ppNodeM
);

FTM_RET	FTDM_getEPM
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	struct FTDM_EPM_STRUCT _PTR_ _PTR_ ppEPM
);

FTM_RET	FTDM_removeInvalidData
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM
);	

FTM_RET	FTDM_showNodeList
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM
);

FTM_RET	FTDM_createNode
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_NODE_PTR		xInfo,
	FTDM_NODE_PTR _PTR_ pNode
);

FTM_RET	FTDM_deleteNode
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_CHAR_PTR		pID
);

FTM_RET FTDM_isNodeExist
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
 	FTM_CHAR_PTR	pDID,
	FTM_BOOL_PTR	pExist
);

FTM_RET	FTDM_getNodeCount
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_ULONG_PTR		pulCount
);

FTM_RET	FTDM_getNode
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_CHAR_PTR		pID,
	FTDM_NODE_PTR _PTR_	ppNode
);

FTM_RET	FTDM_getNodeAt
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_ULONG			ulIndex,
	FTDM_NODE_PTR _PTR_	ppNode
);

FTM_RET	FTDM_getNodeIDList
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_DID_PTR		pDIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTDM_createEP
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_EP_PTR			xInfo,
	FTDM_EP_PTR _PTR_ 	pEP
);

FTM_RET	FTDM_deleteEP
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_CHAR_PTR		pID
);

FTM_RET	FTDM_getEPCount
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_EP_TYPE			xType,
	FTM_ULONG_PTR		pulCount
);

FTM_RET	FTDM_getEP
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_CHAR_PTR		pID,
	FTDM_EP_PTR _PTR_	ppEP
);

FTM_RET	FTDM_getEPAt
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_ULONG			ulIndex,
	FTDM_EP_PTR _PTR_	ppEP
);

FTM_RET	FTDM_getEPIDList
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_DID_PTR		pDIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTDM_createAction
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_ACTION_PTR			xInfo,
	FTDM_ACTION_PTR _PTR_ 	pAction
);

FTM_RET	FTDM_deleteAction
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_CHAR_PTR		pID
);

FTM_RET	FTDM_getActionCount
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_ULONG_PTR		pulCount
);

FTM_RET	FTDM_getAction
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_CHAR_PTR		pID,
	FTDM_ACTION_PTR _PTR_	ppAction
);

FTM_RET	FTDM_getActionAt
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_ULONG			ulIndex,
	FTDM_ACTION_PTR _PTR_	ppAction
);

FTM_RET	FTDM_getActionIDList
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_DID_PTR		pDIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTDM_createTrigger
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_TRIGGER_PTR			xInfo,
	FTDM_TRIGGER_PTR _PTR_ 	pTrigger
);

FTM_RET	FTDM_deleteTrigger
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_CHAR_PTR		pID
);

FTM_RET	FTDM_getTriggerCount
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_ULONG_PTR		pulCount
);

FTM_RET	FTDM_getTrigger
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_CHAR_PTR		pID,
	FTDM_TRIGGER_PTR _PTR_	ppTrigger
);

FTM_RET	FTDM_getTriggerAt
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_ULONG			ulIndex,
	FTDM_TRIGGER_PTR _PTR_	ppTrigger
);

FTM_RET	FTDM_getTriggerIDList
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_DID_PTR		pDIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTDM_createRule
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_RULE_PTR			xInfo,
	FTDM_RULE_PTR _PTR_ 	pRule
);

FTM_RET	FTDM_deleteRule
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_CHAR_PTR		pID
);

FTM_RET	FTDM_getRuleCount
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_ULONG_PTR		pulCount
);

FTM_RET	FTDM_getRule
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_CHAR_PTR		pID,
	FTDM_RULE_PTR _PTR_	ppRule
);

FTM_RET	FTDM_getRuleAt
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_ULONG			ulIndex,
	FTDM_RULE_PTR _PTR_	ppRule
);

FTM_RET	FTDM_getRuleIDList
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_DID_PTR		pDIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTDM_addLog
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_LOG_PTR	pLog
);

FTM_RET	FTDM_deleteLog
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulDeletedCount
);

FTM_RET	FTDM_getLogCount
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTDM_getLog
(
	struct FTDM_CONTEXT_STRUCT	_PTR_ pFTDM,
	FTM_ULONG		ulIndex,
	FTM_LOG_PTR		pLogList,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
);

	
#endif

