#ifndef	__FTDM_PARAMS_H__
#define	__FTDM_PARAMS_H__

#include "ftm_types.h"
#include "ftm_trigger.h"

#define	FTDM_RESP_PARAMS_MAX_SIZE	4096

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
}	FTDM_REQ_PARAMS, _PTR_ FTDM_REQ_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
}	FTDM_RESP_PARAMS, _PTR_ FTDM_RESP_PARAMS_PTR;

typedef	struct 
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_NODE		xNodeInfo;
}	FTDM_REQ_NODE_ADD_PARAMS,	_PTR_ FTDM_REQ_NODE_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
}	FTDM_RESP_NODE_ADD_PARAMS, _PTR_ FTDM_RESP_NODE_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pDID[FTM_DID_LEN+1];
}	FTDM_REQ_NODE_DEL_PARAMS, _PTR_ FTDM_REQ_NODE_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
}	FTDM_RESP_NODE_DEL_PARAMS, _PTR_ FTDM_RESP_NODE_DEL_PARAMS_PTR;

typedef struct	
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
}	FTDM_REQ_NODE_COUNT_PARAMS, _PTR_ FTDM_REQ_NODE_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_ULONG			nCount;
}	FTDM_RESP_NODE_COUNT_PARAMS, _PTR_ FTDM_RESP_NODE_COUNT_PARAMS_PTR;

typedef struct	
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_ULONG			nIndex;
}	FTDM_REQ_NODE_GET_AT_PARAMS, _PTR_ FTDM_REQ_NODE_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_NODE		xNodeInfo;
}	FTDM_RESP_NODE_GET_AT_PARAMS, _PTR_ FTDM_RESP_NODE_GET_AT_PARAMS_PTR;

typedef struct	
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pDID[FTM_DID_LEN+1];
}	FTDM_REQ_NODE_GET_PARAMS, _PTR_ 	FTDM_REQ_NODE_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_NODE		xNodeInfo;
}	FTDM_RESP_NODE_GET_PARAMS, _PTR_ FTDM_RESP_NODE_GET_PARAMS_PTR;

typedef struct	
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_NODE_FIELD		xFields;
	FTM_NODE			xNodeInfo;
}	FTDM_REQ_NODE_SET_PARAMS, _PTR_ 	FTDM_REQ_NODE_SET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
}	FTDM_RESP_NODE_SET_PARAMS, _PTR_ FTDM_RESP_NODE_SET_PARAMS_PTR;

typedef struct	
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_ULONG			ulIndex;
	FTM_ULONG			ulCount;
}	FTDM_REQ_NODE_GET_DID_LIST_PARAMS, _PTR_ 	FTDM_REQ_NODE_GET_DID_LIST_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_ULONG			ulCount;
	FTM_DID				pDIDs[];
}	FTDM_RESP_NODE_GET_DID_LIST_PARAMS, _PTR_ FTDM_RESP_NODE_GET_DID_LIST_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pDID[FTM_DID_LEN+1];
}	FTDM_REQ_GET_NODE_TYPE_PARAMS, _PTR_	FTDM_REQ_GET_NODE_TYPE_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_NODE_TYPE		xType;
}	FTDM_RESP_GET_NODE_TYPE_PARAMS, _PTR_	FTDM_RESP_GET_NODE_TYPE_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pDID[FTM_DID_LEN+1];
}	FTDM_REQ_GET_NODE_URL_PARAMS, _PTR_	FTDM_REQ_GET_NODE_URL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_INT 			nURLLen;
	FTM_CHAR			pURL[]; 
}	FTDM_RESP_GET_NODE_URL_PARAMS, _PTR_	FTDM_RESP_GET_NODE_URL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pDID[FTM_DID_LEN+1];
	FTM_INT			nURLLen;
	FTM_CHAR			pURL[];
}	FTDM_REQ_SET_NODE_URL_PARAMS, _PTR_ FTDM_REQ_SET_NODE_URL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
}	FTDM_RESP_SET_NODE_URL_PARAMS, _PTR_	FTDM_RESP_SET_NODE_URL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pDID[FTM_DID_LEN+1];
}	FTDM_REQ_GET_NODE_LOCATION_PARAMS, _PTR_	FTDM_REQ_GET_NODE_LOCATION_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_INT 			nLocationLen;
	FTM_CHAR			pLocation[]; 
}	FTDM_RESP_GET_NODE_LOCATION_PARAMS, _PTR_	FTDM_RESP_GET_NODE_LOCATION_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pDID[FTM_DID_LEN+1];
	FTM_INT			nLocationLen;
	FTM_CHAR			pLocation[];
}	FTDM_REQ_SET_NODE_LOCATION_PARAMS, _PTR_	FTDM_REQ_SET_NODE_LOCATION_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
}	FTDM_RESP_SET_NODE_LOCATION_PARAMS, _PTR_	FTDM_RESP_SET_NODE_LOCATION_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_EP     	xInfo;
}	FTDM_REQ_EP_ADD_PARAMS, _PTR_ FTDM_REQ_EP_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
}	FTDM_RESP_EP_ADD_PARAMS, _PTR_	FTDM_RESP_EP_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
}	FTDM_REQ_EP_DEL_PARAMS, _PTR_	FTDM_REQ_EP_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
}	FTDM_RESP_EP_DEL_PARAMS, _PTR_ 	FTDM_RESP_EP_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_EP_TYPE		xType;
}	FTDM_REQ_EP_COUNT_PARAMS, _PTR_ 	FTDM_REQ_EP_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_ULONG			nCount;
}	FTDM_RESP_EP_COUNT_PARAMS, _PTR_	FTDM_RESP_EP_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
}	FTDM_REQ_EP_GET_PARAMS, _PTR_ 	FTDM_REQ_EP_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_EP				xInfo;
}	FTDM_RESP_EP_GET_PARAMS, _PTR_	FTDM_RESP_EP_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_ULONG			nIndex;
}	FTDM_REQ_EP_GET_AT_PARAMS, _PTR_	FTDM_REQ_EP_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_EP				xInfo;
}	FTDM_RESP_EP_GET_AT_PARAMS, _PTR_	FTDM_RESP_EP_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pEPID[FTM_ID_LEN+1];
	FTM_EP_FIELD		xFields;
	FTM_EP				xInfo;
}	FTDM_REQ_EP_SET_PARAMS, _PTR_ 	FTDM_REQ_EP_SET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
}	FTDM_RESP_EP_SET_PARAMS, _PTR_	FTDM_RESP_EP_SET_PARAMS_PTR;

typedef struct	
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_ULONG			ulIndex;
	FTM_ULONG			ulCount;
}	FTDM_REQ_EP_GET_EPID_LIST_PARAMS, _PTR_ 	FTDM_REQ_EP_GET_EPID_LIST_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_ULONG			ulCount;
	FTM_EPID			pEPIDs[];
}	FTDM_RESP_EP_GET_EPID_LIST_PARAMS, _PTR_ FTDM_RESP_EP_GET_EPID_LIST_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_EP_TYPE			xType;
	FTM_EP_CLASS		xInfo;
}	FTDM_REQ_EP_CLASS_ADD_PARAMS, _PTR_ FTDM_REQ_EP_CLASS_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
}	FTDM_RESP_EP_CLASS_ADD_PARAMS, _PTR_	FTDM_RESP_EP_CLASS_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_EP_TYPE		xType;
}	FTDM_REQ_EP_CLASS_DEL_PARAMS, _PTR_ FTDM_REQ_EP_CLASS_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
}	FTDM_RESP_EP_CLASS_DEL_PARAMS, _PTR_	FTDM_RESP_EP_CLASS_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_EP_TYPE		xEPClass;
}	FTDM_REQ_EP_CLASS_GET_PARAMS, _PTR_ FTDM_REQ_EP_CLASS_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_EP_CLASS	xInfo;
}	FTDM_RESP_EP_CLASS_GET_PARAMS, _PTR_	FTDM_RESP_EP_CLASS_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
}	FTDM_REQ_EP_CLASS_COUNT_PARAMS, _PTR_ 	FTDM_REQ_EP_CLASS_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_ULONG			nCount;
}	FTDM_RESP_EP_CLASS_COUNT_PARAMS, _PTR_	FTDM_RESP_EP_CLASS_COUNT_PARAMS_PTR;


typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_ULONG			nIndex;
}	FTDM_REQ_EP_CLASS_GET_AT_PARAMS, _PTR_	FTDM_REQ_EP_CLASS_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_EP_CLASS	xInfo;
}	FTDM_RESP_EP_CLASS_GET_AT_PARAMS, _PTR_	FTDM_RESP_EP_CLASS_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
	FTM_EP_DATA			xData;
}	FTDM_REQ_EP_DATA_ADD_PARAMS, _PTR_ FTDM_REQ_EP_DATA_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
}	FTDM_RESP_EP_DATA_ADD_PARAMS, _PTR_	FTDM_RESP_EP_DATA_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
}	FTDM_REQ_EP_DATA_INFO_PARAMS, _PTR_ FTDM_REQ_EP_DATA_INFO_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
	FTM_ULONG			ulBeginTime;
	FTM_ULONG			ulEndTime;
	FTM_ULONG			ulCount;
}	FTDM_RESP_EP_DATA_INFO_PARAMS, _PTR_ FTDM_RESP_EP_DATA_INFO_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
	FTM_EP_LIMIT		xLimit;
}	FTDM_REQ_EP_DATA_SET_LIMIT_PARAMS, _PTR_ FTDM_REQ_EP_DATA_SET_LIMIT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				xRet;
}	FTDM_RESP_EP_DATA_SET_LIMIT_PARAMS, _PTR_ FTDM_RESP_EP_DATA_SET_LIMIT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
	FTM_ULONG			nStartIndex;
	FTM_ULONG			nCount; 
}	FTDM_REQ_EP_DATA_GET_PARAMS, _PTR_	FTDM_REQ_EP_DATA_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_BOOL			bRemain;
	FTM_ULONG			nCount; 
	FTM_EP_DATA 		pData[];
}	FTDM_RESP_EP_DATA_GET_PARAMS, _PTR_ FTDM_RESP_EP_DATA_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
	FTM_ULONG 			nBeginTime; 
	FTM_ULONG 			nEndTime; 
	FTM_BOOL			bAscending;
	FTM_ULONG			nCount; 
}	FTDM_REQ_EP_DATA_GET_WITH_TIME_PARAMS, _PTR_	FTDM_REQ_EP_DATA_GET_WITH_TIME_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_BOOL			bRemain;
	FTM_ULONG			nCount; 
	FTM_EP_DATA 		pData[];
}	FTDM_RESP_EP_DATA_GET_WITH_TIME_PARAMS, _PTR_ FTDM_RESP_EP_DATA_GET_WITH_TIME_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
	FTM_INT				nIndex; 
	FTM_ULONG			nCount;
}	FTDM_REQ_EP_DATA_DEL_PARAMS, _PTR_ FTDM_REQ_EP_DATA_DEL_PARAMS_PTR; 

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_ULONG			ulCount;
}	FTDM_RESP_EP_DATA_DEL_PARAMS, _PTR_ 	FTDM_RESP_EP_DATA_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
	FTM_ULONG 			nBeginTime;
	FTM_ULONG 			nEndTime; 
}	FTDM_REQ_EP_DATA_DEL_WITH_TIME_PARAMS, _PTR_ FTDM_REQ_EP_DATA_DEL_WITH_TIME_PARAMS_PTR; 

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_ULONG			ulCount;
}	FTDM_RESP_EP_DATA_DEL_WITH_TIME_PARAMS, _PTR_ 	FTDM_RESP_EP_DATA_DEL_WITH_TIME_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
}	FTDM_REQ_EP_DATA_COUNT_PARAMS, _PTR_ FTDM_REQ_EP_DATA_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_ULONG			nCount;
}	FTDM_RESP_EP_DATA_COUNT_PARAMS, _PTR_ FTDM_RESP_EP_DATA_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
	FTM_ULONG 			nBeginTime;
	FTM_ULONG 			nEndTime; 
}	FTDM_REQ_EP_DATA_COUNT_WITH_TIME_PARAMS, _PTR_ FTDM_REQ_EP_DATA_COUNT_WITH_TIME_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_ULONG			nCount;
}	FTDM_RESP_EP_DATA_COUNT_WITH_TIME_PARAMS, _PTR_ FTDM_RESP_EP_DATA_COUNT_WITH_TIME_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_TRIGGER    		xTrigger;
}	FTDM_REQ_TRIGGER_ADD_PARAMS, _PTR_ FTDM_REQ_TRIGGER_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_CHAR			pTriggerID[FTM_ID_LEN+1];
}	FTDM_RESP_TRIGGER_ADD_PARAMS, _PTR_	FTDM_RESP_TRIGGER_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pTriggerID[FTM_ID_LEN+1];
}	FTDM_REQ_TRIGGER_DEL_PARAMS, _PTR_	FTDM_REQ_TRIGGER_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
}	FTDM_RESP_TRIGGER_DEL_PARAMS, _PTR_ 	FTDM_RESP_TRIGGER_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
}	FTDM_REQ_TRIGGER_COUNT_PARAMS, _PTR_ 	FTDM_REQ_TRIGGER_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_ULONG			nCount;
}	FTDM_RESP_TRIGGER_COUNT_PARAMS, _PTR_	FTDM_RESP_TRIGGER_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pTriggerID[FTM_ID_LEN+1];
}	FTDM_REQ_TRIGGER_GET_PARAMS, _PTR_ 	FTDM_REQ_TRIGGER_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_TRIGGER			xTrigger;
}	FTDM_RESP_TRIGGER_GET_PARAMS, _PTR_	FTDM_RESP_TRIGGER_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_ULONG			nIndex;
}	FTDM_REQ_TRIGGER_GET_AT_PARAMS, _PTR_	FTDM_REQ_TRIGGER_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_TRIGGER			xTrigger;
}	FTDM_RESP_TRIGGER_GET_AT_PARAMS, _PTR_	FTDM_RESP_TRIGGER_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pTriggerID[FTM_ID_LEN+1];
	FTM_TRIGGER_FIELD	xFields;
	FTM_TRIGGER			xTrigger;
}	FTDM_REQ_TRIGGER_SET_PARAMS, _PTR_ 	FTDM_REQ_TRIGGER_SET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_TRIGGER			xTrigger;
}	FTDM_RESP_TRIGGER_SET_PARAMS, _PTR_	FTDM_RESP_TRIGGER_SET_PARAMS_PTR;

typedef struct	
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_ULONG			ulIndex;
	FTM_ULONG			ulCount;
}	FTDM_REQ_TRIGGER_GET_ID_LIST_PARAMS, _PTR_ 	FTDM_REQ_TRIGGER_GET_ID_LIST_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_ULONG			ulCount;
	FTM_ID				pIDs[];
}	FTDM_RESP_TRIGGER_GET_ID_LIST_PARAMS, _PTR_ FTDM_RESP_TRIGGER_GET_ID_LIST_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_ACTION     		xAction;
}	FTDM_REQ_ACTION_ADD_PARAMS, _PTR_ FTDM_REQ_ACTION_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
}	FTDM_RESP_ACTION_ADD_PARAMS, _PTR_	FTDM_RESP_ACTION_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pActionID[FTM_ID_LEN+1];
}	FTDM_REQ_ACTION_DEL_PARAMS, _PTR_	FTDM_REQ_ACTION_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
}	FTDM_RESP_ACTION_DEL_PARAMS, _PTR_ 	FTDM_RESP_ACTION_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
}	FTDM_REQ_ACTION_COUNT_PARAMS, _PTR_ 	FTDM_REQ_ACTION_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_ULONG			nCount;
}	FTDM_RESP_ACTION_COUNT_PARAMS, _PTR_	FTDM_RESP_ACTION_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pActionID[FTM_ID_LEN+1];
}	FTDM_REQ_ACTION_GET_PARAMS, _PTR_ 	FTDM_REQ_ACTION_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_ACTION			xAction;
}	FTDM_RESP_ACTION_GET_PARAMS, _PTR_	FTDM_RESP_ACTION_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_ULONG			nIndex;
}	FTDM_REQ_ACTION_GET_AT_PARAMS, _PTR_	FTDM_REQ_ACTION_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_ACTION			xAction;
}	FTDM_RESP_ACTION_GET_AT_PARAMS, _PTR_	FTDM_RESP_ACTION_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pActionID[FTM_ID_LEN+1];
	FTM_ACTION_FIELD	xFields;
	FTM_ACTION			xAction;
}	FTDM_REQ_ACTION_SET_PARAMS, _PTR_ 	FTDM_REQ_ACTION_SET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_ACTION			xAction;
}	FTDM_RESP_ACTION_SET_PARAMS, _PTR_	FTDM_RESP_ACTION_SET_PARAMS_PTR;

typedef struct	
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_ULONG			ulIndex;
	FTM_ULONG			ulCount;
}	FTDM_REQ_ACTION_GET_ID_LIST_PARAMS, _PTR_ 	FTDM_REQ_ACTION_GET_ID_LIST_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_ULONG			ulCount;
	FTM_ID				pIDs[];
}	FTDM_RESP_ACTION_GET_ID_LIST_PARAMS, _PTR_ FTDM_RESP_ACTION_GET_ID_LIST_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RULE     		xRule;
}	FTDM_REQ_RULE_ADD_PARAMS, _PTR_ FTDM_REQ_RULE_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
}	FTDM_RESP_RULE_ADD_PARAMS, _PTR_	FTDM_RESP_RULE_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pRuleID[FTM_ID_LEN+1];
}	FTDM_REQ_RULE_DEL_PARAMS, _PTR_	FTDM_REQ_RULE_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
}	FTDM_RESP_RULE_DEL_PARAMS, _PTR_ 	FTDM_RESP_RULE_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
}	FTDM_REQ_RULE_COUNT_PARAMS, _PTR_ 	FTDM_REQ_RULE_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_ULONG			nCount;
}	FTDM_RESP_RULE_COUNT_PARAMS, _PTR_	FTDM_RESP_RULE_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pRuleID[FTM_ID_LEN+1];
}	FTDM_REQ_RULE_GET_PARAMS, _PTR_ 	FTDM_REQ_RULE_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_RULE			xRule;
}	FTDM_RESP_RULE_GET_PARAMS, _PTR_	FTDM_RESP_RULE_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_ULONG			nIndex;
}	FTDM_REQ_RULE_GET_AT_PARAMS, _PTR_	FTDM_REQ_RULE_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_RULE			xRule;
}	FTDM_RESP_RULE_GET_AT_PARAMS, _PTR_	FTDM_RESP_RULE_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_CHAR			pRuleID[FTM_ID_LEN+1];
	FTM_RULE_FIELD		xFields;
	FTM_RULE			xRule;
}	FTDM_REQ_RULE_SET_PARAMS, _PTR_ 	FTDM_REQ_RULE_SET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_RULE			xRule;
}	FTDM_RESP_RULE_SET_PARAMS, _PTR_	FTDM_RESP_RULE_SET_PARAMS_PTR;

typedef struct	
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_ULONG			ulIndex;
	FTM_ULONG			ulCount;
}	FTDM_REQ_RULE_GET_ID_LIST_PARAMS, _PTR_ 	FTDM_REQ_RULE_GET_ID_LIST_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_ULONG			ulCount;
	FTM_ID				pIDs[];
}	FTDM_RESP_RULE_GET_ID_LIST_PARAMS, _PTR_ FTDM_RESP_RULE_GET_ID_LIST_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_LOG     		xLog;
}	FTDM_REQ_LOG_ADD_PARAMS, _PTR_ FTDM_REQ_LOG_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
}	FTDM_RESP_LOG_ADD_PARAMS, _PTR_	FTDM_RESP_LOG_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_ULONG			ulIndex;
	FTM_ULONG			ulCount;
}	FTDM_REQ_LOG_DEL_PARAMS, _PTR_	FTDM_REQ_LOG_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_ULONG			ulCount;
}	FTDM_RESP_LOG_DEL_PARAMS, _PTR_ 	FTDM_RESP_LOG_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
}	FTDM_REQ_LOG_COUNT_PARAMS, _PTR_ 	FTDM_REQ_LOG_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_ULONG			nCount;
}	FTDM_RESP_LOG_COUNT_PARAMS, _PTR_	FTDM_RESP_LOG_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_ULONG			ulIndex;
	FTM_ULONG			ulCount;
}	FTDM_REQ_LOG_GET_PARAMS, _PTR_ 	FTDM_REQ_LOG_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_ULONG			ulCount;
	FTM_LOG				pLogs[];
}	FTDM_RESP_LOG_GET_PARAMS, _PTR_	FTDM_RESP_LOG_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_ULONG			ulIndex;
}	FTDM_REQ_LOG_GET_AT_PARAMS, _PTR_	FTDM_REQ_LOG_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				xRet;
	FTM_LOG				xLog;
}	FTDM_RESP_LOG_GET_AT_PARAMS, _PTR_	FTDM_RESP_LOG_GET_AT_PARAMS_PTR;

#endif
