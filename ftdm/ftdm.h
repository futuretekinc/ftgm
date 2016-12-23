#ifndef	__FTDM_H__
#define	__FTDM_H__

#include "ftm.h"
#include "ftdm_types.h"
#include "ftdm_cmd.h"
#include "ftdm_config.h"
#include "ftdm_node.h"
#include "ftdm_node_management.h"
#include "ftdm_ep.h"
#include "ftdm_ep_management.h"
#include "ftdm_ep_class.h"
#include "ftdm_server.h"
#include "ftdm_logger.h"
#include "ftdm_rule.h"
#include "ftdm_action.h"
#include "ftdm_event.h"
#include "ftdm_modules.h"
#include "ftdm_dbif.h"

typedef struct FTDM_CONTEXT_STRUCT
{
	FTDM_DBIF_PTR	pDBIF;
	FTDM_NODEM_PTR	pNodeM;	
	FTDM_EPM_PTR	pEPM;	
	FTDM_LOGGER_PTR	pLogger;
} FTDM_CONTEXT, _PTR_ FTDM_CONTEXT_PTR;

FTM_RET	FTDM_init
(
	FTM_VOID
);

FTM_RET	FTDM_final
(
	FTM_VOID
);

FTM_RET FTDM_loadConfig
(
	FTM_CONFIG_PTR		pConfig
);

FTM_RET	FTDM_loadObjectFromFile
(
	FTM_CHAR_PTR		pFileName
);

FTM_RET	FTDM_saveObjectToDB
(
	FTM_VOID
);

FTM_RET	FTDM_setDebugLevel
(
	FTM_ULONG		ulLevel
);

FTM_RET	FTDM_getServer
(
	FTDM_SERVER_PTR _PTR_ ppServer
);
	
FTM_RET	FTDM_removeInvalidData
(
	FTM_VOID
);	

FTM_RET	FTDM_createAction
(
	FTM_ACTION_PTR 	pInfo,
	FTDM_ACTION_PTR	_PTR_ ppAction
);

FTM_RET	FTDM_destroyAction
(
	FTM_CHAR_PTR	pActionID
);

FTM_RET	FTDM_loadActionConfig
(
	FTM_CONFIG_PTR		pConfig
);

FTM_RET	FTDM_loadActionFromFile
(
	FTM_CHAR_PTR			pFileName
);

FTM_RET	FTDM_loadActionFromDB
(
	FTM_VOID
);

FTM_RET	FTDM_saveActionToDB
(
	FTM_VOID
);

FTM_RET	FTDM_createRule
(
	FTM_RULE_PTR 	pInfo,
	FTDM_RULE_PTR _PTR_ ppRule
);

FTM_RET	FTDM_destroyRule
(
	FTM_CHAR_PTR	pRuleID
);

FTM_RET	FTDM_loadRuleConfig
(
	FTM_CONFIG_PTR	pConfig
);

FTM_RET	FTDM_loadRuleFromDB
(
	FTM_VOID
);

FTM_RET	FTDM_saveRuleToDB
(
	FTM_VOID
);

FTM_RET	FTDM_createEvent
(
	FTM_EVENT_PTR 	pInfo,
	FTDM_EVENT_PTR _PTR_ ppEvent
);

FTM_RET	FTDM_destroyEvent
(
	FTM_CHAR_PTR	pID
);

FTM_RET	FTDM_loadEventConfig
(
	FTM_CONFIG_PTR	pConfig
);

FTM_RET	FTDM_loadEventFromDB
(
	FTM_VOID
);

FTM_RET	FTDM_saveEventToDB
(
	FTM_VOID
);
#endif

