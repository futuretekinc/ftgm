#ifndef	_FTM_EVENT_H_
#define	_FTM_EVENT_H_

#include "ftm_types.h"
#include "ftm_endpoint.h"

typedef	enum
{
	FTM_EVENT_TYPE_ABOVE = 0,
	FTM_EVENT_TYPE_BELOW,
	FTM_EVENT_TYPE_INCLUDE,
	FTM_EVENT_TYPE_EXCEPT,
	FTM_EVENT_TYPE_CHANGE,
	FTM_EVENT_TYPE_AND,
	FTM_EVENT_TYPE_OR,
}	FTM_EVENT_TYPE, _PTR_ FTM_EVENT_TYPE_PTR;

typedef	FTM_ULONG	FTM_EVENT_ID, _PTR_ FTM_EVENT_ID_PTR;

typedef	struct
{
	FTM_EVENT_ID	xID;
	FTM_EVENT_TYPE	xType;
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
		struct
		{
			FTM_EVENT_ID	xID1;
			FTM_EVENT_ID	xID2;
		} xAnd, xOr;
	}	xParams;
}	FTM_EVENT, _PTR_ FTM_EVENT_PTR;

FTM_RET	FTM_EVENT_createAbove(FTM_EVENT_ID xEventID, FTM_EP_DATA_PTR pData, FTM_EVENT_PTR _PTR_ ppEvent);
FTM_RET	FTM_EVENT_createBelow(FTM_EVENT_ID xEventID, FTM_EP_DATA_PTR pData, FTM_EVENT_PTR _PTR_ ppEvent);
FTM_RET	FTM_EVENT_createInclude(FTM_EVENT_ID xEventID, FTM_EP_DATA_PTR pUpper, FTM_EP_DATA_PTR pLower, FTM_EVENT_PTR _PTR_ ppEvent);
FTM_RET	FTM_EVENT_createExcept(FTM_EVENT_ID xEventID, FTM_EP_DATA_PTR pUpper, FTM_EP_DATA_PTR pLower, FTM_EVENT_PTR _PTR_ ppEvent);
FTM_RET	FTM_EVENT_createChange(FTM_EVENT_ID xEventID, FTM_EVENT_PTR _PTR_ ppEvent);
FTM_RET	FTM_EVENT_createAnd(FTM_EVENT_ID xEventID, FTM_EVENT_ID xEventID1, FTM_EVENT_ID xEventID2, FTM_EVENT_PTR _PTR_ ppEvent);
FTM_RET	FTM_EVENT_createOr(FTM_EVENT_ID xEventID, FTM_EVENT_ID xEventID1, FTM_EVENT_ID xEventID2, FTM_EVENT_PTR _PTR_ ppEvent);
FTM_RET	FTM_EVENT_destroy(FTM_EVENT_PTR pEvent);

FTM_RET	FTM_EVENT_get(FTM_EVENT_ID xEventID, FTM_EVENT_PTR _PTR_ ppEvent);

FTM_RET	FTM_EVENT_occurred(FTM_EVENT_PTR pEvent, FTM_EP_DATA_PTR pPrevData, FTM_EP_DATA_PTR pCurrData, FTM_BOOL_PTR pResult);
#endif
