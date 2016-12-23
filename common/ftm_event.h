#ifndef	_FTM_EVENT_H_
#define	_FTM_EVENT_H_

#include "ftm_types.h"
#include "ftm_ep.h"
#include "ftm_action.h"
#include "ftm_time.h"
#include "ftm_list.h"
#include "ftm_value.h"

typedef	FTM_ULONG	FTM_EVENT_FIELD, _PTR_ FTM_EVENT_FIELD_PTR;

#define	FTM_EVENT_FIELD_ID			(1 << 0)
#define	FTM_EVENT_FIELD_TYPE			(1 << 1)
#define	FTM_EVENT_FIELD_NAME			(1 << 2)
#define	FTM_EVENT_FIELD_EPID			(1 << 3)
#define	FTM_EVENT_FIELD_DETECT_TIME	(1 << 4)
#define	FTM_EVENT_FIELD_HOLD_TIME		(1 << 5)
#define	FTM_EVENT_FIELD_VALUE			(1 << 6)
#define	FTM_EVENT_FIELD_LOWER			(1 << 7)
#define	FTM_EVENT_FIELD_UPPER			(1 << 8)
#define	FTM_EVENT_FIELD_CONDITION		(FTM_EVENT_FIELD_DETECT_TIME \
										| FTM_EVENT_FIELD_HOLD_TIME\
										| FTM_EVENT_FIELD_VALUE\
										| FTM_EVENT_FIELD_LOWER\
										| FTM_EVENT_FIELD_UPPER)
#define	FTM_EVENT_FIELD_ALL			(0xFFFF)

typedef	enum
{
	FTM_EVENT_TYPE_NONE	= 0,
	FTM_EVENT_TYPE_ABOVE,
	FTM_EVENT_TYPE_BELOW,
	FTM_EVENT_TYPE_INCLUDE,
	FTM_EVENT_TYPE_EXCEPT,
	FTM_EVENT_TYPE_CHANGE,
	FTM_EVENT_TYPE_UNKNOWN
}	FTM_EVENT_TYPE, _PTR_ FTM_EVENT_TYPE_PTR;

typedef	struct
{
	FTM_CHAR			pID[FTM_ID_LEN+1];
	FTM_EVENT_TYPE	xType;
	FTM_CHAR			pName[FTM_NAME_LEN+1];
	FTM_CHAR			pEPID[FTM_EPID_LEN+1];
	union
	{
		struct
		{
			FTM_ULONG		ulDetectionTime;
			FTM_ULONG		ulHoldingTime;
		} xCommon;
		struct
		{
			FTM_ULONG		ulDetectionTime;
			FTM_ULONG		ulHoldingTime;
			FTM_VALUE		xValue;		
		} xAbove, xBelow;
		struct
		{
			FTM_ULONG		ulDetectionTime;
			FTM_ULONG		ulHoldingTime;
			FTM_VALUE		xUpper;		
			FTM_VALUE		xLower;		
		} xInclude, xExcept;
	}	xParams;
}	FTM_EVENT, _PTR_ FTM_EVENT_PTR;

FTM_RET	FTM_EVENT_init
(
	FTM_VOID
); 

FTM_RET	FTM_EVENT_final
(
	FTM_VOID
); 

FTM_RET	FTM_EVENT_setDefault
(
	FTM_EVENT_PTR pEvent
); 

FTM_RET	FTM_EVENT_create
(
	FTM_EVENT_PTR pEvent
); 

FTM_RET	FTM_EVENT_createAbove
(
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pEPID,
	FTM_VALUE_PTR	pValue, 
	FTM_EVENT_PTR _PTR_ ppEvent
); 

FTM_RET	FTM_EVENT_createBelow
(
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pEPID,
	FTM_VALUE_PTR	pValue, 
	FTM_EVENT_PTR _PTR_ ppEvent
); 

FTM_RET	FTM_EVENT_createInclude
(
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pEPID,
	FTM_VALUE_PTR	pUppder, 
	FTM_VALUE_PTR	pLower, 
	FTM_EVENT_PTR _PTR_ ppEvent
); 

FTM_RET	FTM_EVENT_createExcept
(
	FTM_CHAR_PTR	xID, 
	FTM_CHAR_PTR	pEPID,
	FTM_VALUE_PTR	pUppder, 
	FTM_VALUE_PTR	pLower, 
	FTM_EVENT_PTR _PTR_ ppEvent
); 

FTM_RET	FTM_EVENT_createChange
(
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pEPID,
	FTM_EVENT_PTR _PTR_ ppEvent
);

FTM_RET	FTM_EVENT_destroy
(
	FTM_EVENT_PTR pEvent
);

FTM_RET	FTM_EVENT_append
(
	FTM_EVENT_PTR pEvent
);

FTM_RET	FTM_EVENT_remove
(
	FTM_EVENT_PTR pEvent
);

FTM_RET	FTM_EVENT_count
(
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTM_EVENT_get
(
	FTM_CHAR_PTR	pID,
	FTM_EVENT_PTR _PTR_ ppEvent
);

FTM_RET	FTM_EVENT_getAt
(
	FTM_ULONG 		ulIndex, 
	FTM_EVENT_PTR _PTR_ ppEvent
);

FTM_RET	FTM_EVENT_occurred
(
	FTM_EVENT_PTR pEvent, 
	FTM_VALUE_PTR	pCurrData, 
	FTM_BOOL_PTR 	pResult
);

FTM_RET	FTM_EVENT_strToType
(
	FTM_CHAR_PTR	pString,
	FTM_EVENT_TYPE_PTR pType
);

FTM_CHAR_PTR	FTM_EVENT_typeString
(
	FTM_EVENT_TYPE xType
);

FTM_RET	FTM_EVENT_conditionToString
(
	FTM_EVENT_PTR pEvent, 
	FTM_CHAR_PTR 	pBuff, 
	FTM_ULONG 		ulLen
);

#endif
