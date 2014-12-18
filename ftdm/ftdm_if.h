#ifndef	__FTDM_IF_H__
#define	__FTDM_IF_H__

typedef	struct 
{
	FTGM_DEVICE_ID 		xDID; 
	FTGM_DEVICE_TYPE 	xType;
	FTGM_DEVICE_URL		xURL; 
	FTGM_DEVICE_LOC		xLocation;
}	FTDM_REQ_DEVICE_INSERT_PARAMS;

typedef	struct
{
	FTGM_RET			nRet;
}	FTDM_REP_DEVICE_INSERT_PARAMS;

typedef struct	
{
	FTGM_DEVICE_ID		xDID;
}	FTDM_REQ_DEVICE_INFO_PARAMS;

typedef	struct
{
	FTGM_RET			nRet;
	FTGM_DEVICE_INFO	xInfo;
}	FTDM_REP_DEVICE_INFO_PARAMS;

typedef	struct
{
	FTGM_DEVICE_ID		xDID;
}	FTDM_REQ_DEVICE_TYPE_PARAMS;

typedef	struct
{
	FTGM_RET			nRet;
	FTGM_DEVICE_TYPE	xType;
}	FTDM_REP_DEVICE_TYPE_PARAMS;

typedef	struct
{
	FTGM_DEVICE_ID 			xDID;
}	FTDM_REQ_DEVICE_REMOVE_PARAMS;

typedef	struct
{
	FTGM_RET			nRet;
}	FTDM_REP_DEVICE_REMOVE_PARAMS;

typedef	struct
{
	FTGM_DEVICE_ID 		xDID ;
}	FTDM_REQ_DEVICE_URL_PARAMS;

typedef	struct
{
	FTGM_RET			nRet;
	FTGM_INT 			nURLLen;
	FTGM_CHAR			pURL[]; 
}	FTDM_REP_DEVICE_URL_PARAMS;

typedef	struct
{
	FTGM_DEVICE_ID 		xDID;
	FTGM_INT			nURLLen;
	FTGM_CHAR			pURL[];
}	FTDM_REQ_DEVICE_URL_SET_PARAMS;

typedef	struct
{
	FTGM_RET			nRet;
}	FTDM_REP_DEVICE_URL_SET_PARAMS;

typedef	struct
{
	FTGM_DEVICE_ID 		xDID; 
}	FTDM_REQ_DEVICE_LOCATION_PARAMS;

typedef	struct
{
	FTGM_RET			nRet;
	FTGM_INT 			nLocationLen;
	FTGM_CHAR			pLocation[]; 
}	FTDM_REP_DEVICE_LOCATION_PARAMS;

typedef	struct
{
	FTGM_DEVICE_ID 		xDID; 
	FTGM_INT			nLocationLen;
	FTGM_CHAR			pLocation[];
}	FTDM_REQ_DEVICE_LOCATION_SET_PARAMS;

typedef	struct
{
	FTGM_RET			nRet;
}	FTDM_REP_DEVICE_LOCATION_SET_PARAMS;

typedef	struct
{
	FTGM_EP_ID 			xEPID;
	FTGM_EP_INFO     	xInfo;
}	FTDM_REQ_EP_INSERT_PARAMS;

typedef	struct
{
	FTGM_RET			nRet;
}	FTDM_REP_EP_INSERT_PARAMS;

typedef	struct
{
	FTGM_EP_ID 			xEPID;
}	FTDM_REQ_EP_REMOVE_PARAMS;

typedef	struct
{
	FTGM_RET			nRet;
}	FTDM_REP_EP_REMOVE_PARAMS;

typedef	struct
{
	FTGM_EP_ID			xEPID;
}	FTDM_REQ_EP_INFO_PARAMS;

typedef	struct
{
	FTGM_RET			nRet;
	FTGM_EP_INFO		xInfo;
}	FTDM_REP_EP_INFO_PARAMS;

typedef	struct
{
	FTGM_EP_ID 			xEPID; 
	FTGM_ULONG			nTime;
	FTGM_ULONG 			nValue;
}	FTDM_REQ_EP_DATA_APPEND_PARAMS;

typedef	struct
{
	FTGM_RET			nRet;
}	FTDM_REP_EP_DATA_APPEND_PARAMS;

typedef	struct
{
	FTGM_EP_ID 			xEPID; 
	FTGM_ULONG 			nBeginTime; 
	FTGM_ULONG 			nEndTime; 
	FTGM_ULONG			nCount; 
}	FTDM_REQ_EP_DATA_PARAMS;

typedef	struct
{
	FTGM_RET			nRet;
	FTGM_ULONG			nCount; 
	FTGM_EP_DATA 		pData[];
}	FTDM_REP_EP_DATA_PARAMS;

typedef	struct
{
	FTGM_EP_ID 			xEPID;
	FTGM_ULONG 			nBeginTime;
	FTGM_ULONG 			nEndTime; 
	FTGM_ULONG			nCount;
}	FTDM_REQ_EP_DATA_REMOVE_PARAMS; 

typedef	struct
{
	FTGM_RET			nRet;
}	FTDM_REP_EP_DATA_REMOVE_PARAMS;

#endif
