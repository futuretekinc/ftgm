#ifndef	_FTOM_CGI_H_
#define	_FTOM_CGI_H_

#include <syslog.h>
#include "ftm.h"
#include "ftom.h"
#include "ftom_client.h"
#include "qdecoder.h"
#include "mxml.h"
#include "cJSON.h"

typedef	struct FTOM_CGI_COMMAND_STRUCT
{
	FTM_CHAR_PTR	pName;
	FTM_RET			(*fService)(FTOM_CLIENT_PTR pClient, qentry_t *req);
} FTOM_CGI_COMMAND, _PTR_ FTOM_CGI_COMMAND_PTR;

FTM_RET	FTOM_CGI_finish
(
	qentry_t _PTR_ pReq,
	cJSON _PTR_ pRoot, 
	FTM_RET xRet
);

FTM_RET	FTOM_CGI_node
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t *pReq
);

FTM_RET	FTOM_CGI_ep
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t *pReq
);

FTM_RET	FTOM_CGI_data
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t *pReq
);

FTM_RET	FTOM_CGI_trigger
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t *pReq
);

FTM_RET	FTOM_CGI_action
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t *pReq
);

FTM_RET	FTOM_CGI_rule
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t *pReq
);

FTM_RET	FTOM_CGI_log
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t *pReq
);

FTM_RET	FTOM_CGI_discovery
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t *pReq
);

const 
char *FTOM_CGI_whitespaceCB
(
	mxml_node_t *node,
	int			where
);

FTM_RET	FTOM_CGI_getUSHORT
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pName,
	FTM_USHORT_PTR	pusValue,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getULONG
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pName,
	FTM_ULONG_PTR	pulValue,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getSTRING
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pName,
	FTM_CHAR_PTR	pBuff,
	FTM_ULONG		ulBuffLen,
	FTM_BOOL	bAllowEmpty
);

FTM_RET FTOM_CGI_getNodeType
(
	qentry_t *pReq, 
	FTM_NODE_TYPE_PTR pType,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getNodeOptSNMP
(
	qentry_t *pReq,
	FTM_NODE_OPT_SNMP_PTR pSNMP,
	FTM_BOOL bAllowEmpty
);

FTM_RET	FTOM_CGI_getEPID
(
	qentry_t *pReq, 
	FTM_CHAR_PTR pEPID,
	FTM_BOOL	bAllowEmpty
);

FTM_RET FTOM_CGI_getEPType
(
	qentry_t *pReq, 
	FTM_EP_TYPE_PTR pType,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getEPFlags
(
	qentry_t *pReq, 
	FTM_EP_FLAG_PTR	pFlags,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getModel
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pModel,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getName
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pName,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getUnit
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pUnit,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getLocation
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pLocation,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getEnable
(
	qentry_t *pReq, 
	FTM_BOOL_PTR	pEnable	,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getTimeout
(
	qentry_t *pReq, 
	FTM_ULONG_PTR	pTimeout,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getInterval
(
	qentry_t *pReq, 
	FTM_ULONG_PTR	pInterval,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getReportInterval
(
	qentry_t *pReq, 
	FTM_ULONG_PTR	pInterval,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getDID
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pDID,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getLimit
(
	qentry_t *pReq,
	FTM_EP_LIMIT_PTR pLimit,
	FTM_BOOL	bAllowEmpty
);

FTM_RET FTOM_CGI_getIndex
(
	qentry_t *pReq, 
	FTM_ULONG_PTR pulIndex,
	FTM_BOOL	bAllowEmpty
);

FTM_RET FTOM_CGI_getCount
(
	qentry_t *pReq, 
	FTM_ULONG_PTR pulCount,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getIPString
(
	qentry_t *pReq,
	FTM_CHAR_PTR	pBuff,
	FTM_ULONG		ulBuffLen,
	FTM_BOOL bAllowEmpty
);

FTM_RET	FTOM_CGI_getPort
(
	qentry_t *pReq, 
	FTM_USHORT_PTR	pusPort,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getRetry
(
	qentry_t *pReq, 
	FTM_ULONG_PTR	pulRetry,
	FTM_BOOL	bAllowEmpty
);

FTM_RET FTOM_CGI_getTriggerID
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pTriggerID,
	FTM_BOOL	bAllowEmpty
);

FTM_RET FTOM_CGI_getTriggerType
(
	qentry_t *pReq, 
	FTM_TRIGGER_TYPE_PTR pType,
	FTM_BOOL	bAllowEmpty
);

FTM_RET FTOM_CGI_getBeginTime
(
	qentry_t *pReq, 
	FTM_ULONG_PTR	pulTime,
	FTM_BOOL	bAllowEmpty
);

FTM_RET FTOM_CGI_getEndTime
(
	qentry_t *pReq, 
	FTM_ULONG_PTR	pulTime,
	FTM_BOOL	bAllowEmpty
);

FTM_RET FTOM_CGI_getDetectTime
(
	qentry_t *pReq, 
	FTM_ULONG_PTR	pulTime,
	FTM_BOOL	bAllowEmpty
);

FTM_RET FTOM_CGI_getHoldTime
(
	qentry_t *pReq, 
	FTM_ULONG_PTR pulTime,
	FTM_BOOL	bAllowEmpty
);

FTM_RET FTOM_CGI_getValue
(
	qentry_t *pReq, 
	FTM_VALUE_TYPE	xType,
	FTM_VALUE_PTR	pValue,
	FTM_BOOL	bAllowEmpty
);

FTM_RET FTOM_CGI_getLowerValue
(
	qentry_t *pReq, 
	FTM_VALUE_TYPE	xType,
	FTM_VALUE_PTR	pValue,
	FTM_BOOL	bAllowEmpty
);

FTM_RET FTOM_CGI_getUpperValue
(
	qentry_t *pReq, 
	FTM_VALUE_TYPE	xType,
	FTM_VALUE_PTR	pValue,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getActionID
(
	qentry_t *pReq,
	FTM_CHAR_PTR	pActionID,
	FTM_BOOL	bAllowEmpry
);

FTM_RET	FTOM_CGI_getActionType
(
	qentry_t *pReq,
	FTM_ACTION_TYPE_PTR	pType,
	FTM_BOOL	bAllowEmpry
);

FTM_RET	FTOM_CGI_getActionParams
(
	qentry_t *pReq,
	FTM_ACTION_TYPE	xType,
	FTM_ACTION_PARAMS_PTR	pParams,
	FTM_BOOL	bAllowEmpry
);

FTM_RET	FTOM_CGI_getRuleID
(
	qentry_t *pReq,
	FTM_CHAR_PTR	pRuleID,
	FTM_BOOL	bAllowEmpry
);

FTM_RET	FTOM_CGI_getRuleState
(
	qentry_t *pReq,
	FTM_RULE_STATE_PTR	pState,
	FTM_BOOL	bAllowEmpty
);

FTM_RET	FTOM_CGI_getRuleTrigger
(
	qentry_t 	*pReq,
	FTM_INT		nIndex,
	FTM_CHAR_PTR pTriggerID,
	FTM_ULONG	ulIDLen,
	FTM_BOOL	bAllowEmpty
);
 
FTM_RET	FTOM_CGI_getRuleAction
(
	qentry_t 	*pReq,
	FTM_INT		nIndex,
	FTM_CHAR_PTR pActionID,
	FTM_ULONG	ulIDLen,
	FTM_BOOL	bAllowEmpty
);
 
FTM_RET	FTOM_CGI_addNodeInfoToObject
(
	cJSON _PTR_ 	pRoot, 
	FTM_NODE_PTR 	pNodeInfo, 
	FTM_NODE_FIELD 	xFields
);

FTM_RET	FTOM_CGI_addEPInfoToObject
(
	cJSON _PTR_ 		pObject,
	FTM_EP_PTR			pEPInfo,
	FTM_EP_FIELD		xFields
);

#undef	TRACE
#define	TRACE(format, ...) 	syslog(LOG_INFO, format, ## __VA_ARGS__)

#endif
