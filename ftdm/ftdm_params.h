#ifndef	__FTDM_PARAMS_H__
#define	__FTDM_PARAMS_H__

#include "ftm_types.h"
#include "ftm_event.h"
typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
}	FTDM_REQ_PARAMS, _PTR_ FTDM_REQ_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET			nRet;
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
	FTM_RET				nRet;
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
	FTM_RET				nRet;
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
	FTM_RET				nRet;
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
	FTM_RET				nRet;
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
	FTM_RET				nRet;
	FTM_NODE		xNodeInfo;
}	FTDM_RESP_NODE_GET_PARAMS, _PTR_ FTDM_RESP_NODE_GET_PARAMS_PTR;

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
	FTM_RET				nRet;
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
	FTM_RET			nRet;
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
	FTM_RET			nRet;
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
	FTM_RET			nRet;
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
	FTM_RET			nRet;
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
	FTM_RET				nRet;
}	FTDM_RESP_EP_ADD_PARAMS, _PTR_	FTDM_RESP_EP_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_EP_ID 			xEPID;
}	FTDM_REQ_EP_DEL_PARAMS, _PTR_	FTDM_REQ_EP_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET			nRet;
}	FTDM_RESP_EP_DEL_PARAMS, _PTR_ 	FTDM_RESP_EP_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_EP_CLASS		xClass;
}	FTDM_REQ_EP_COUNT_PARAMS, _PTR_ 	FTDM_REQ_EP_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
	FTM_ULONG			nCount;
}	FTDM_RESP_EP_COUNT_PARAMS, _PTR_	FTDM_RESP_EP_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_EP_ID			xEPID;
}	FTDM_REQ_EP_GET_PARAMS, _PTR_ 	FTDM_REQ_EP_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
	FTM_EP			xInfo;
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
	FTM_RET			nRet;
	FTM_EP		xInfo;
}	FTDM_RESP_EP_GET_AT_PARAMS, _PTR_	FTDM_RESP_EP_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_EP_CLASS		xClass;
	FTM_EP_CLASS_INFO	xInfo;
}	FTDM_REQ_EP_CLASS_INFO_ADD_PARAMS, _PTR_ FTDM_REQ_EP_CLASS_INFO_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
}	FTDM_RESP_EP_CLASS_INFO_ADD_PARAMS, _PTR_	FTDM_RESP_EP_CLASS_INFO_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_EP_CLASS		xClass;
}	FTDM_REQ_EP_CLASS_INFO_DEL_PARAMS, _PTR_ FTDM_REQ_EP_CLASS_INFO_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
}	FTDM_RESP_EP_CLASS_INFO_DEL_PARAMS, _PTR_	FTDM_RESP_EP_CLASS_INFO_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_EP_CLASS		xEPClass;
}	FTDM_REQ_EP_CLASS_INFO_GET_PARAMS, _PTR_ FTDM_REQ_EP_CLASS_INFO_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
	FTM_EP_CLASS_INFO	xInfo;
}	FTDM_RESP_EP_CLASS_INFO_GET_PARAMS, _PTR_	FTDM_RESP_EP_CLASS_INFO_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
}	FTDM_REQ_EP_CLASS_INFO_COUNT_PARAMS, _PTR_ 	FTDM_REQ_EP_CLASS_INFO_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
	FTM_ULONG			nCount;
}	FTDM_RESP_EP_CLASS_INFO_COUNT_PARAMS, _PTR_	FTDM_RESP_EP_CLASS_INFO_COUNT_PARAMS_PTR;


typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_ULONG			nIndex;
}	FTDM_REQ_EP_CLASS_INFO_GET_AT_PARAMS, _PTR_	FTDM_REQ_EP_CLASS_INFO_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
	FTM_EP_CLASS_INFO	xInfo;
}	FTDM_RESP_EP_CLASS_INFO_GET_AT_PARAMS, _PTR_	FTDM_RESP_EP_CLASS_INFO_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_EP_ID			xEPID;
	FTM_EP_DATA			xData;
}	FTDM_REQ_EP_DATA_ADD_PARAMS, _PTR_ FTDM_REQ_EP_DATA_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET			nRet;
}	FTDM_RESP_EP_DATA_ADD_PARAMS, _PTR_	FTDM_RESP_EP_DATA_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_EP_ID			xEPID;
}	FTDM_REQ_EP_DATA_INFO_PARAMS, _PTR_ FTDM_REQ_EP_DATA_INFO_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			ulLen;
	FTM_RET				nRet;
	FTM_ULONG			ulBeginTime;
	FTM_ULONG			ulEndTime;
	FTM_ULONG			ulCount;
}	FTDM_RESP_EP_DATA_INFO_PARAMS, _PTR_ FTDM_RESP_EP_DATA_INFO_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_EP_ID			xEPID;
	FTM_ULONG			nStartIndex;
	FTM_ULONG			nCount; 
}	FTDM_REQ_EP_DATA_GET_PARAMS, _PTR_	FTDM_REQ_EP_DATA_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
	FTM_ULONG			nCount; 
	FTM_EP_DATA 		pData[];
}	FTDM_RESP_EP_DATA_GET_PARAMS, _PTR_ FTDM_RESP_EP_DATA_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_EP_ID			xEPID;
	FTM_ULONG 			nBeginTime; 
	FTM_ULONG 			nEndTime; 
	FTM_ULONG			nCount; 
}	FTDM_REQ_EP_DATA_GET_WITH_TIME_PARAMS, _PTR_	FTDM_REQ_EP_DATA_GET_WITH_TIME_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
	FTM_ULONG			nCount; 
	FTM_EP_DATA 		pData[];
}	FTDM_RESP_EP_DATA_GET_WITH_TIME_PARAMS, _PTR_ FTDM_RESP_EP_DATA_GET_WITH_TIME_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_EP_ID			xEPID;
	FTM_ULONG 			nIndex; 
	FTM_ULONG			nCount;
}	FTDM_REQ_EP_DATA_DEL_PARAMS, _PTR_ FTDM_REQ_EP_DATA_DEL_PARAMS_PTR; 

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
}	FTDM_RESP_EP_DATA_DEL_PARAMS, _PTR_ 	FTDM_RESP_EP_DATA_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_EP_ID			xEPID;
	FTM_ULONG 			nBeginTime;
	FTM_ULONG 			nEndTime; 
}	FTDM_REQ_EP_DATA_DEL_WITH_TIME_PARAMS, _PTR_ FTDM_REQ_EP_DATA_DEL_WITH_TIME_PARAMS_PTR; 

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
}	FTDM_RESP_EP_DATA_DEL_WITH_TIME_PARAMS, _PTR_ 	FTDM_RESP_EP_DATA_DEL_WITH_TIME_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_EP_ID			xEPID;
}	FTDM_REQ_EP_DATA_COUNT_PARAMS, _PTR_ FTDM_REQ_EP_DATA_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
	FTM_ULONG			nCount;
}	FTDM_RESP_EP_DATA_COUNT_PARAMS, _PTR_ FTDM_RESP_EP_DATA_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_EP_ID			xEPID;
	FTM_ULONG 			nBeginTime;
	FTM_ULONG 			nEndTime; 
}	FTDM_REQ_EP_DATA_COUNT_WITH_TIME_PARAMS, _PTR_ FTDM_REQ_EP_DATA_COUNT_WITH_TIME_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
	FTM_ULONG			nCount;
}	FTDM_RESP_EP_DATA_COUNT_WITH_TIME_PARAMS, _PTR_ FTDM_RESP_EP_DATA_COUNT_WITH_TIME_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_EVENT     		xEvent;
}	FTDM_REQ_EVENT_ADD_PARAMS, _PTR_ FTDM_REQ_EVENT_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
}	FTDM_RESP_EVENT_ADD_PARAMS, _PTR_	FTDM_RESP_EVENT_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_EVENT_ID		xID;
}	FTDM_REQ_EVENT_DEL_PARAMS, _PTR_	FTDM_REQ_EVENT_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET			nRet;
}	FTDM_RESP_EVENT_DEL_PARAMS, _PTR_ 	FTDM_RESP_EVENT_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
}	FTDM_REQ_EVENT_COUNT_PARAMS, _PTR_ 	FTDM_REQ_EVENT_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
	FTM_ULONG			nCount;
}	FTDM_RESP_EVENT_COUNT_PARAMS, _PTR_	FTDM_RESP_EVENT_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_EVENT_ID		xID;
}	FTDM_REQ_EVENT_GET_PARAMS, _PTR_ 	FTDM_REQ_EVENT_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
	FTM_EVENT			xEvent;
}	FTDM_RESP_EVENT_GET_PARAMS, _PTR_	FTDM_RESP_EVENT_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_ULONG			nIndex;
}	FTDM_REQ_EVENT_GET_AT_PARAMS, _PTR_	FTDM_REQ_EVENT_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
	FTM_EVENT			xEvent;
}	FTDM_RESP_EVENT_GET_AT_PARAMS, _PTR_	FTDM_RESP_EVENT_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_ACT     		xAct;
}	FTDM_REQ_ACT_ADD_PARAMS, _PTR_ FTDM_REQ_ACT_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
}	FTDM_RESP_ACT_ADD_PARAMS, _PTR_	FTDM_RESP_ACT_ADD_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_ACT_ID			xID;
}	FTDM_REQ_ACT_DEL_PARAMS, _PTR_	FTDM_REQ_ACT_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
}	FTDM_RESP_ACT_DEL_PARAMS, _PTR_ 	FTDM_RESP_ACT_DEL_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
}	FTDM_REQ_ACT_COUNT_PARAMS, _PTR_ 	FTDM_REQ_ACT_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
	FTM_ULONG			nCount;
}	FTDM_RESP_ACT_COUNT_PARAMS, _PTR_	FTDM_RESP_ACT_COUNT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_ACT_ID			xID;
}	FTDM_REQ_ACT_GET_PARAMS, _PTR_ 	FTDM_REQ_ACT_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
	FTM_ACT				xAct;
}	FTDM_RESP_ACT_GET_PARAMS, _PTR_	FTDM_RESP_ACT_GET_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_ULONG			nIndex;
}	FTDM_REQ_ACT_GET_AT_PARAMS, _PTR_	FTDM_REQ_ACT_GET_AT_PARAMS_PTR;

typedef	struct
{
	FTDM_CMD			xCmd;
	FTM_ULONG			nLen;
	FTM_RET				nRet;
	FTM_ACT				xAct;
}	FTDM_RESP_ACT_GET_AT_PARAMS, _PTR_	FTDM_RESP_ACT_GET_AT_PARAMS_PTR;

#endif
