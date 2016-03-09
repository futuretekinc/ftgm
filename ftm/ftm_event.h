#ifndef	_FTM_EVENT_H_
#define	_FTM_EVENT_H_

#include "ftm_types.h"
#include "ftm_ep.h"
#include "ftm_act.h"
#include "ftm_time.h"

typedef	enum
{
	FTM_EVENT_TYPE_ABOVE = 0,
	FTM_EVENT_TYPE_BELOW,
	FTM_EVENT_TYPE_INCLUDE,
	FTM_EVENT_TYPE_EXCEPT,
	FTM_EVENT_TYPE_CHANGE,
}	FTM_EVENT_TYPE, _PTR_ FTM_EVENT_TYPE_PTR;

typedef	FTM_ULONG	FTM_EVENT_ID, _PTR_ FTM_EVENT_ID_PTR;

typedef	struct
{
	FTM_EVENT_ID	xID;
	FTM_EVENT_TYPE	xType;
	FTM_EP_ID		xEPID;
	union
	{
		struct
		{
			FTM_EP_DATA		xValue;		
		} xAbove, xBelow;
		struct
		{
			FTM_EP_DATA		xUpper;		
			FTM_EP_DATA		xLower;		
		} xInclude, xExcept;
	}	xParams;
	FTM_ACT_ID		xActID;
	FTM_TIME		xDetectionTime;
	FTM_TIME		xHoldingTime;
}	FTM_EVENT, _PTR_ FTM_EVENT_PTR;

FTM_RET	FTM_EVENT_init(FTM_VOID);
FTM_RET	FTM_EVENT_final(FTM_VOID);

FTM_RET	FTM_EVENT_createCopy(FTM_EVENT_PTR pSrc, FTM_EVENT_PTR _PTR_ ppEvent);
FTM_RET	FTM_EVENT_createAbove(FTM_EVENT_ID xEventID, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData, FTM_EVENT_PTR _PTR_ ppEvent);
FTM_RET	FTM_EVENT_createBelow(FTM_EVENT_ID xEventID, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData, FTM_EVENT_PTR _PTR_ ppEvent);
FTM_RET	FTM_EVENT_createInclude(FTM_EVENT_ID xEventID, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pUpper, FTM_EP_DATA_PTR pLower, FTM_EVENT_PTR _PTR_ ppEvent);
FTM_RET	FTM_EVENT_createExcept(FTM_EVENT_ID xEventID, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pUpper, FTM_EP_DATA_PTR pLower, FTM_EVENT_PTR _PTR_ ppEvent);
FTM_RET	FTM_EVENT_createChange(FTM_EVENT_ID xEventID, FTM_EP_ID xEPID, FTM_EVENT_PTR _PTR_ ppEvent);
FTM_RET	FTM_EVENT_destroy(FTM_EVENT_PTR pEvent);

FTM_RET	FTM_EVENT_count(FTM_ULONG_PTR pulCount);
FTM_RET	FTM_EVENT_get(FTM_EVENT_ID xEventID, FTM_EVENT_PTR _PTR_ ppEvent);
FTM_RET	FTM_EVENT_getAt(FTM_ULONG ulIndex, FTM_EVENT_PTR _PTR_ ppEvent);

FTM_RET	FTM_EVENT_occurred(FTM_EVENT_PTR pEvent, FTM_EP_DATA_PTR pCurrData, FTM_BOOL_PTR pResult);

FTM_CHAR_PTR	FTM_EVENT_typeString(FTM_EVENT_TYPE xType);
FTM_RET	FTM_EVENT_conditionToString(FTM_EVENT_PTR pEvent, FTM_CHAR_PTR pBuff, FTM_ULONG ulLen);
#endif
