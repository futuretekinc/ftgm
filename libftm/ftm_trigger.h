#ifndef	_FTM_TRIGGER_H_
#define	_FTM_TRIGGER_H_

#include "ftm_types.h"
#include "ftm_ep.h"
#include "ftm_action.h"
#include "ftm_time.h"
#include "ftm_list.h"
#include "ftm_value.h"

typedef	FTM_ULONG	FTM_TRIGGER_FIELD, _PTR_ FTM_TRIGGER_FIELD_PTR;

#define	FTM_TRIGGER_FIELD_ID			(1 << 0)
#define	FTM_TRIGGER_FIELD_TYPE			(1 << 1)
#define	FTM_TRIGGER_FIELD_NAME			(1 << 2)
#define	FTM_TRIGGER_FIELD_EPID			(1 << 3)
#define	FTM_TRIGGER_FIELD_DETECT_TIME	(1 << 4)
#define	FTM_TRIGGER_FIELD_HOLD_TIME		(1 << 5)
#define	FTM_TRIGGER_FIELD_VALUE			(1 << 6)
#define	FTM_TRIGGER_FIELD_LOWER			(1 << 7)
#define	FTM_TRIGGER_FIELD_UPPER			(1 << 8)
#define	FTM_TRIGGER_FIELD_CONDITION		(FTM_TRIGGER_FIELD_DETECT_TIME \
										| FTM_TRIGGER_FIELD_HOLD_TIME\
										| FTM_TRIGGER_FIELD_VALUE\
										| FTM_TRIGGER_FIELD_LOWER\
										| FTM_TRIGGER_FIELD_UPPER)
#define	FTM_TRIGGER_FIELD_ALL			(0xFFFF)

typedef	enum
{
	FTM_TRIGGER_TYPE_NONE	= 0,
	FTM_TRIGGER_TYPE_ABOVE,
	FTM_TRIGGER_TYPE_BELOW,
	FTM_TRIGGER_TYPE_INCLUDE,
	FTM_TRIGGER_TYPE_EXCEPT,
	FTM_TRIGGER_TYPE_CHANGE,
	FTM_TRIGGER_TYPE_UNKNOWN
}	FTM_TRIGGER_TYPE, _PTR_ FTM_TRIGGER_TYPE_PTR;

typedef	struct
{
	FTM_CHAR			pID[FTM_ID_LEN+1];
	FTM_TRIGGER_TYPE	xType;
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
}	FTM_TRIGGER, _PTR_ FTM_TRIGGER_PTR;

FTM_RET	FTM_TRIGGER_init
(
	FTM_VOID
); 

FTM_RET	FTM_TRIGGER_final
(
	FTM_VOID
); 

FTM_RET	FTM_TRIGGER_setDefault
(
	FTM_TRIGGER_PTR pTrigger
); 

FTM_RET	FTM_TRIGGER_create
(
	FTM_TRIGGER_PTR pTrigger
); 

FTM_RET	FTM_TRIGGER_createAbove
(
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pEPID,
	FTM_VALUE_PTR	pValue, 
	FTM_TRIGGER_PTR _PTR_ ppTrigger
); 

FTM_RET	FTM_TRIGGER_createBelow
(
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pEPID,
	FTM_VALUE_PTR	pValue, 
	FTM_TRIGGER_PTR _PTR_ ppTrigger
); 

FTM_RET	FTM_TRIGGER_createInclude
(
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pEPID,
	FTM_VALUE_PTR	pUppder, 
	FTM_VALUE_PTR	pLower, 
	FTM_TRIGGER_PTR _PTR_ ppTrigger
); 

FTM_RET	FTM_TRIGGER_createExcept
(
	FTM_CHAR_PTR	xID, 
	FTM_CHAR_PTR	pEPID,
	FTM_VALUE_PTR	pUppder, 
	FTM_VALUE_PTR	pLower, 
	FTM_TRIGGER_PTR _PTR_ ppTrigger
); 

FTM_RET	FTM_TRIGGER_createChange
(
	FTM_CHAR_PTR	pID,
	FTM_CHAR_PTR	pEPID,
	FTM_TRIGGER_PTR _PTR_ ppTrigger
);

FTM_RET	FTM_TRIGGER_destroy
(
	FTM_TRIGGER_PTR pTrigger
);

FTM_RET	FTM_TRIGGER_append
(
	FTM_TRIGGER_PTR pTrigger
);

FTM_RET	FTM_TRIGGER_remove
(
	FTM_TRIGGER_PTR pTrigger
);

FTM_RET	FTM_TRIGGER_count
(
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTM_TRIGGER_get
(
	FTM_CHAR_PTR	pID,
	FTM_TRIGGER_PTR _PTR_ ppTrigger
);

FTM_RET	FTM_TRIGGER_getAt
(
	FTM_ULONG 		ulIndex, 
	FTM_TRIGGER_PTR _PTR_ ppTrigger
);

FTM_RET	FTM_TRIGGER_occurred
(
	FTM_TRIGGER_PTR pTrigger, 
	FTM_VALUE_PTR	pCurrData, 
	FTM_BOOL_PTR 	pResult
);

FTM_RET	FTM_TRIGGER_strToType
(
	FTM_CHAR_PTR	pString,
	FTM_TRIGGER_TYPE_PTR pType
);

FTM_CHAR_PTR	FTM_TRIGGER_typeString
(
	FTM_TRIGGER_TYPE xType
);

FTM_RET	FTM_TRIGGER_conditionToString
(
	FTM_TRIGGER_PTR pTrigger, 
	FTM_CHAR_PTR 	pBuff, 
	FTM_ULONG 		ulLen
);

#endif
