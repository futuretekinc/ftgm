#ifndef	__FTOM_PARAMS_H__
#define	__FTOM_PARAMS_H__

#include "ftom_types.h"
#include "ftom_server_cmd.h"
#include "ftom_msg.h"

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
}	FTOM_REQ_PARAMS, _PTR_ FTOM_REQ_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
}	FTOM_RESP_PARAMS, _PTR_ FTOM_RESP_PARAMS_PTR;

typedef	struct 
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_NODE		xNodeInfo;
}	FTOM_REQ_NODE_CREATE_PARAMS,	_PTR_ FTOM_REQ_NODE_CREATE_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_CHAR			pDID[FTM_DID_LEN + 1];
}	FTOM_RESP_NODE_CREATE_PARAMS, _PTR_ FTOM_RESP_NODE_CREATE_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pDID[FTM_DID_LEN+1];
}	FTOM_REQ_NODE_DESTROY_PARAMS, _PTR_ FTOM_REQ_NODE_DESTROY_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
}	FTOM_RESP_NODE_DESTROY_PARAMS, _PTR_ FTOM_RESP_NODE_DESTROY_PARAMS_PTR;

typedef struct	
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
}	FTOM_REQ_NODE_COUNT_PARAMS, _PTR_ FTOM_REQ_NODE_COUNT_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_ULONG			ulCount;
}	FTOM_RESP_NODE_COUNT_PARAMS, _PTR_ FTOM_RESP_NODE_COUNT_PARAMS_PTR;

typedef struct	
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pDID[FTM_DID_LEN+1];
}	FTOM_REQ_NODE_GET_PARAMS, _PTR_ FTOM_REQ_NODE_GET_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_NODE		xNodeInfo;
}	FTOM_RESP_NODE_GET_PARAMS, _PTR_ FTOM_RESP_NODE_GET_PARAMS_PTR;

typedef struct	
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_ULONG			ulIndex;
}	FTOM_REQ_NODE_GET_AT_PARAMS, _PTR_ FTOM_REQ_NODE_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_NODE		xNodeInfo;
}	FTOM_RESP_NODE_GET_AT_PARAMS, _PTR_ FTOM_RESP_NODE_GET_AT_PARAMS_PTR;

typedef struct	
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pDID[FTM_DID_LEN+1];
	FTM_NODE_FIELD		xFields;
	FTM_NODE			xInfo;
}	FTOM_REQ_NODE_SET_PARAMS, _PTR_ FTOM_REQ_NODE_SET_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_NODE			xInfo;
}	FTOM_RESP_NODE_SET_PARAMS, _PTR_ FTOM_RESP_NODE_SET_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_EP     			xInfo;
}	FTOM_REQ_EP_CREATE_PARAMS, _PTR_ FTOM_REQ_EP_CREATE_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_EP     			xInfo;
}	FTOM_RESP_EP_CREATE_PARAMS, _PTR_	FTOM_RESP_EP_CREATE_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
}	FTOM_REQ_EP_DESTROY_PARAMS, _PTR_	FTOM_REQ_EP_DESTROY_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
}	FTOM_RESP_EP_DESTROY_PARAMS, _PTR_ 	FTOM_RESP_EP_DESTROY_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_EP_TYPE		xType;
}	FTOM_REQ_EP_COUNT_PARAMS, _PTR_ 	FTOM_REQ_EP_COUNT_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_ULONG			nCount;
}	FTOM_RESP_EP_COUNT_PARAMS, _PTR_	FTOM_RESP_EP_COUNT_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_EP_TYPE		xType;
	FTM_ULONG			ulMaxCount;
}	FTOM_REQ_EP_GET_LIST_PARAMS, _PTR_ 	FTOM_REQ_EP_GET_LIST_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_ULONG			ulCount;
	FTM_CHAR			pEPIDList[][FTM_EPID_LEN+1];
}	FTOM_RESP_EP_GET_LIST_PARAMS, _PTR_	FTOM_RESP_EP_GET_LIST_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
}	FTOM_REQ_EP_GET_PARAMS, _PTR_ 	FTOM_REQ_EP_GET_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_EP			xInfo;
}	FTOM_RESP_EP_GET_PARAMS, _PTR_	FTOM_RESP_EP_GET_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_ULONG			ulIndex;
}	FTOM_REQ_EP_GET_AT_PARAMS, _PTR_	FTOM_REQ_EP_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_EP			xInfo;
}	FTOM_RESP_EP_GET_AT_PARAMS, _PTR_	FTOM_RESP_EP_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pEPID[FTM_ID_LEN+1];
	FTM_EP_FIELD		xFields;
	FTM_EP				xInfo;
}	FTOM_REQ_EP_SET_PARAMS, _PTR_ 	FTOM_REQ_EP_SET_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
}	FTOM_RESP_EP_SET_PARAMS, _PTR_	FTOM_RESP_EP_SET_PARAMS_PTR;

typedef struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
}	FTOM_REQ_EP_REG_NOTIFY_RECEIVER_PARAMS, _PTR_ FTOM_REQ_EP_REG_NOTIFY_RECEIVER_PARAMS_PTR;

typedef struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
}	FTOM_RESP_EP_REG_NOTIFY_RECEIVER_PARAMS, _PTR_ FTOM_RESP_EP_REG_NOTIFY_RECEIVER_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
	FTM_EP_DATA			xData;
}	FTOM_REQ_EP_DATA_ADD_PARAMS, _PTR_ FTOM_REQ_EP_DATA_ADD_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
}	FTOM_RESP_EP_DATA_ADD_PARAMS, _PTR_	FTOM_RESP_EP_DATA_ADD_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
}	FTOM_REQ_EP_DATA_INFO_PARAMS, _PTR_ FTOM_REQ_EP_DATA_INFO_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_ULONG			ulBeginTime;
	FTM_ULONG			ulEndTime;
	FTM_ULONG			ulCount;
}	FTOM_RESP_EP_DATA_INFO_PARAMS, _PTR_	FTOM_RESP_EP_DATA_INFO_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
}	FTOM_REQ_EP_DATA_GET_LAST_PARAMS, _PTR_	FTOM_REQ_EP_DATA_GET_LAST_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_EP_DATA 		xData;
}	FTOM_RESP_EP_DATA_GET_LAST_PARAMS, _PTR_ FTOM_RESP_EP_DATA_GET_LAST_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
	FTM_ULONG			nStartIndex;
	FTM_ULONG			nCount; 
}	FTOM_REQ_EP_DATA_GET_LIST_PARAMS, _PTR_	FTOM_REQ_EP_DATA_GET_LIST_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_ULONG			nCount; 
	FTM_EP_DATA 		pData[];
}	FTOM_RESP_EP_DATA_GET_LIST_PARAMS, _PTR_ FTOM_RESP_EP_DATA_GET_LIST_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
	FTM_ULONG 			ulIndex; 
	FTM_ULONG			ulCount;
}	FTOM_REQ_EP_DATA_DEL_PARAMS, _PTR_ FTOM_REQ_EP_DATA_DEL_PARAMS_PTR; 

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_ULONG			ulCount;
}	FTOM_RESP_EP_DATA_DEL_PARAMS, _PTR_ 	FTOM_RESP_EP_DATA_DEL_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
	FTM_ULONG 			ulBegin; 
	FTM_ULONG			ulEnd;
}	FTOM_REQ_EP_DATA_DEL_WITH_TIME_PARAMS, _PTR_ FTOM_REQ_EP_DATA_DEL_WITH_TIME_PARAMS_PTR; 

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_ULONG			ulCount;
}	FTOM_RESP_EP_DATA_DEL_WITH_TIME_PARAMS, _PTR_ 	FTOM_RESP_EP_DATA_DEL_WITH_TIME_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
}	FTOM_REQ_EP_DATA_COUNT_PARAMS, _PTR_ FTOM_REQ_EP_DATA_COUNT_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_ULONG			ulCount;
}	FTOM_RESP_EP_DATA_COUNT_PARAMS, _PTR_ FTOM_RESP_EP_DATA_COUNT_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
}	FTOM_REQ_EP_DATA_TYPE_PARAMS, _PTR_ FTOM_REQ_EP_DATA_TYPE_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_EP_DATA_TYPE	xType;
}	FTOM_RESP_EP_DATA_TYPE_PARAMS, _PTR_ FTOM_RESP_EP_DATA_TYPE_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTOM_MSG			xMsg;
}	FTOM_RESP_NOTIFY_PARAMS, _PTR_ FTOM_RESP_NOTIFY_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_TRIGGER			xTrigger;
}	FTOM_REQ_TRIGGER_ADD_PARAMS, _PTR_ FTOM_REQ_TRIGGER_ADD_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_CHAR			pTriggerID[FTM_ID_LEN+1];
}	FTOM_RESP_TRIGGER_ADD_PARAMS, _PTR_ FTOM_RESP_TRIGGER_ADD_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pTriggerID[FTM_ID_LEN+1];
}	FTOM_REQ_TRIGGER_DEL_PARAMS, _PTR_ FTOM_REQ_TRIGGER_DEL_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
}	FTOM_RESP_TRIGGER_DEL_PARAMS, _PTR_ FTOM_RESP_TRIGGER_DEL_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
}	FTOM_REQ_TRIGGER_COUNT_PARAMS, _PTR_ FTOM_REQ_TRIGGER_COUNT_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_ULONG			ulCount;
}	FTOM_RESP_TRIGGER_COUNT_PARAMS, _PTR_ FTOM_RESP_TRIGGER_COUNT_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pTriggerID[FTM_ID_LEN+1];
}	FTOM_REQ_TRIGGER_GET_PARAMS, _PTR_ FTOM_REQ_TRIGGER_GET_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_TRIGGER			xTrigger;
}	FTOM_RESP_TRIGGER_GET_PARAMS, _PTR_ FTOM_RESP_TRIGGER_GET_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_ULONG			ulIndex;
}	FTOM_REQ_TRIGGER_GET_AT_PARAMS, _PTR_ FTOM_REQ_TRIGGER_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_TRIGGER			xTrigger;
}	FTOM_RESP_TRIGGER_GET_AT_PARAMS, _PTR_ FTOM_RESP_TRIGGER_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pTriggerID[FTM_ID_LEN+1];
	FTM_TRIGGER_FIELD	xFields;
	FTM_TRIGGER			xTrigger;
}	FTOM_REQ_TRIGGER_SET_PARAMS, _PTR_ FTOM_REQ_TRIGGER_SET_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
}	FTOM_RESP_TRIGGER_SET_PARAMS, _PTR_ FTOM_RESP_TRIGGER_SET_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_ACTION			xAction;
}	FTOM_REQ_ACTION_ADD_PARAMS, _PTR_ FTOM_REQ_ACTION_ADD_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_CHAR			pActionID[FTM_ID_LEN+1];
}	FTOM_RESP_ACTION_ADD_PARAMS, _PTR_ FTOM_RESP_ACTION_ADD_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pActionID[FTM_ID_LEN+1];
}	FTOM_REQ_ACTION_DEL_PARAMS, _PTR_ FTOM_REQ_ACTION_DEL_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
}	FTOM_RESP_ACTION_DEL_PARAMS, _PTR_ FTOM_RESP_ACTION_DEL_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
}	FTOM_REQ_ACTION_COUNT_PARAMS, _PTR_ FTOM_REQ_ACTION_COUNT_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_ULONG			ulCount;
}	FTOM_RESP_ACTION_COUNT_PARAMS, _PTR_ FTOM_RESP_ACTION_COUNT_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pActionID[FTM_ID_LEN+1];
}	FTOM_REQ_ACTION_GET_PARAMS, _PTR_ FTOM_REQ_ACTION_GET_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_ACTION			xAction;
}	FTOM_RESP_ACTION_GET_PARAMS, _PTR_ FTOM_RESP_ACTION_GET_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_ULONG			ulIndex;
}	FTOM_REQ_ACTION_GET_AT_PARAMS, _PTR_ FTOM_REQ_ACTION_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_ACTION			xAction;
}	FTOM_RESP_ACTION_GET_AT_PARAMS, _PTR_ FTOM_RESP_ACTION_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pActionID[FTM_ID_LEN+1];
	FTM_ACTION_FIELD	xFields;
	FTM_ACTION			xAction;
}	FTOM_REQ_ACTION_SET_PARAMS, _PTR_ FTOM_REQ_ACTION_SET_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
}	FTOM_RESP_ACTION_SET_PARAMS, _PTR_ FTOM_RESP_ACTION_SET_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RULE			xRule;
}	FTOM_REQ_RULE_ADD_PARAMS, _PTR_ FTOM_REQ_RULE_ADD_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_CHAR			pRuleID[FTM_ID_LEN+1];
}	FTOM_RESP_RULE_ADD_PARAMS, _PTR_ FTOM_RESP_RULE_ADD_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pRuleID[FTM_ID_LEN+1];
}	FTOM_REQ_RULE_DEL_PARAMS, _PTR_ FTOM_REQ_RULE_DEL_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
}	FTOM_RESP_RULE_DEL_PARAMS, _PTR_ FTOM_RESP_RULE_DEL_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
}	FTOM_REQ_RULE_COUNT_PARAMS, _PTR_ FTOM_REQ_RULE_COUNT_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_ULONG			ulCount;
}	FTOM_RESP_RULE_COUNT_PARAMS, _PTR_ FTOM_RESP_RULE_COUNT_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pRuleID[FTM_ID_LEN+1];
}	FTOM_REQ_RULE_GET_PARAMS, _PTR_ FTOM_REQ_RULE_GET_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_RULE			xRule;
}	FTOM_RESP_RULE_GET_PARAMS, _PTR_ FTOM_RESP_RULE_GET_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_ULONG			ulIndex;
}	FTOM_REQ_RULE_GET_AT_PARAMS, _PTR_ FTOM_REQ_RULE_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_RULE			xRule;
}	FTOM_RESP_RULE_GET_AT_PARAMS, _PTR_ FTOM_RESP_RULE_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pRuleID[FTM_ID_LEN+1];
	FTM_RULE_FIELD		xFields;
	FTM_RULE			xRule;
}	FTOM_REQ_RULE_SET_PARAMS, _PTR_ FTOM_REQ_RULE_SET_PARAMS_PTR;

typedef	struct
{
	FTM_ULONG			ulReqID;
	FTOM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
}	FTOM_RESP_RULE_SET_PARAMS, _PTR_ FTOM_RESP_RULE_SET_PARAMS_PTR;

#endif
