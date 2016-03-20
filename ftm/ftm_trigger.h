#ifndef	_FTM_TRIGGER_H_
#define	_FTM_TRIGGER_H_

#include "ftm_types.h"
#include "ftm_ep.h"
#include "ftm_action.h"
#include "ftm_time.h"
#include "ftm_list.h"

typedef	enum
{
	FTM_TRIGGER_TYPE_ABOVE = 0,
	FTM_TRIGGER_TYPE_BELOW,
	FTM_TRIGGER_TYPE_INCLUDE,
	FTM_TRIGGER_TYPE_EXCEPT,
	FTM_TRIGGER_TYPE_CHANGE,
}	FTM_TRIGGER_TYPE, _PTR_ FTM_TRIGGER_TYPE_PTR;

typedef	FTM_ULONG	FTM_TRIGGER_ID, _PTR_ FTM_TRIGGER_ID_PTR;

typedef	struct
{
	FTM_TRIGGER_ID		xID;
	FTM_TRIGGER_TYPE	xType;
	FTM_EP_ID			xEPID;
	union
	{
		struct
		{
			FTM_TIME		xDetectionTime;
			FTM_TIME		xHoldingTime;
		} xCommon;
		struct
		{
			FTM_TIME		xDetectionTime;
			FTM_TIME		xHoldingTime;
			FTM_EP_DATA		xValue;		
		} xAbove, xBelow;
		struct
		{
			FTM_TIME		xDetectionTime;
			FTM_TIME		xHoldingTime;
			FTM_EP_DATA		xUpper;		
			FTM_EP_DATA		xLower;		
		} xInclude, xExcept;
	}	xParams;
}	FTM_TRIGGER, _PTR_ FTM_TRIGGER_PTR;

FTM_RET	FTM_TRIGGER_init(FTM_VOID); 
FTM_RET	FTM_TRIGGER_final(FTM_VOID); 
FTM_RET	FTM_TRIGGER_createCopy(FTM_TRIGGER_PTR pSrc, FTM_TRIGGER_PTR _PTR_ ppEvent); 
FTM_RET	FTM_TRIGGER_createAbove(FTM_TRIGGER_ID xEventID, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData, FTM_TRIGGER_PTR _PTR_ ppEvent); 
FTM_RET	FTM_TRIGGER_createBelow(FTM_TRIGGER_ID xEventID, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData, FTM_TRIGGER_PTR _PTR_ ppEvent); 
FTM_RET	FTM_TRIGGER_createInclude(FTM_TRIGGER_ID xEventID, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pUpper, FTM_EP_DATA_PTR pLower, FTM_TRIGGER_PTR _PTR_ ppEvent); 
FTM_RET	FTM_TRIGGER_createExcept(FTM_TRIGGER_ID xEventID, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pUpper, FTM_EP_DATA_PTR pLower, FTM_TRIGGER_PTR _PTR_ ppEvent); 
FTM_RET	FTM_TRIGGER_createChange(FTM_TRIGGER_ID xEventID, FTM_EP_ID xEPID, FTM_TRIGGER_PTR _PTR_ ppEvent);
FTM_RET	FTM_TRIGGER_destroy(FTM_TRIGGER_PTR pEvent);

FTM_RET	FTM_TRIGGER_count(FTM_ULONG_PTR pulCount);
FTM_RET	FTM_TRIGGER_get(FTM_TRIGGER_ID xEventID, FTM_TRIGGER_PTR _PTR_ ppEvent);
FTM_RET	FTM_TRIGGER_getAt(FTM_ULONG ulIndex, FTM_TRIGGER_PTR _PTR_ ppEvent);

FTM_RET	FTM_TRIGGER_occurred(FTM_TRIGGER_PTR pEvent, FTM_EP_DATA_PTR pCurrData, FTM_BOOL_PTR pResult);

FTM_CHAR_PTR	FTM_TRIGGER_typeString(FTM_TRIGGER_TYPE xType);
FTM_RET	FTM_TRIGGER_conditionToString(FTM_TRIGGER_PTR pEvent, FTM_CHAR_PTR pBuff, FTM_ULONG ulLen);

#endif
