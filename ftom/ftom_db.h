#ifndef	__FTOM_DB_H__
#define	__FTOM_DB_H__

#include "ftm.h"
#include "ftom.h"

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

FTM_RET FTOM_DB_NODE_getDIDList
(
	FTM_DID_PTR		pIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
);

FTM_RET	FTOM_DB_NODE_getInfo
(
	FTM_CHAR_PTR	pDID,
	FTM_NODE_PTR	pInfo
);

FTM_RET	FTOM_DB_NODE_getInfoAt
(
	FTM_ULONG		ulIndex,
	FTM_NODE_PTR	pInfo
);

FTM_RET	FTOM_DB_NODE_setInfo
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

FTM_RET FTOM_DB_EP_getEPIDList
(
	FTM_EPID_PTR	pEPIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
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
	FTM_ULONG_PTR 	pulCount,
	FTM_BOOL_PTR	pbRemain
);

FTM_RET	FTOM_DB_EP_getDataListWithTime
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG 		ulBegin, 
	FTM_ULONG 		ulEnd, 
	FTM_BOOL		bAscending,
	FTM_EP_DATA_PTR pDataList, 
	FTM_ULONG 		ulMaxCount, 
	FTM_ULONG_PTR 	pulCount,
	FTM_BOOL_PTR	pbRemain
);

FTM_RET	FTOM_DB_EP_getDataInfo
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR 	pulBeginTime, 
	FTM_ULONG_PTR 	pulEndTime, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTOM_DB_EP_setDataLimit
(
	FTM_CHAR_PTR		pEPID,
	FTM_EP_LIMIT_PTR	pLimit
);

FTM_RET	FTOM_DB_EP_getDataCount
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR 	ulCount
);

FTM_RET	FTOM_DB_EP_getDataCountWithTime
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulStart,
	FTM_ULONG		ulEnd,
	FTM_ULONG_PTR	pulCount
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

FTM_RET FTOM_DB_TRIGGER_getIDList
(
	FTM_ID_PTR		pIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
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

FTM_RET FTOM_DB_ACTION_getIDList
(
	FTM_ID_PTR		pIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
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

FTM_RET FTOM_DB_RULE_getIDList
(
	FTM_ID_PTR		pIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
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

#endif

