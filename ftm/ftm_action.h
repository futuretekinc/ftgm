#ifndef	_FTM_ACTION_H_
#define	_FTM_ACTION_H_

#include "ftm_types.h"
#include "ftm_ep.h"

typedef	FTM_ULONG	FTM_ACTION_ID, _PTR_ FTM_ACTION_ID_PTR;

typedef	enum
{
	FTM_ACTION_TYPE_SET,
	FTM_ACTION_TYPE_SMS,
	FTM_ACTION_TYPE_PUSH,
	FTM_ACTION_TYPE_MAIL
}	FTM_ACTION_TYPE, _PTR_ FTM_ACTION_TYPE_PTR;
	
typedef	struct
{
	FTM_ACTION_ID	xID;
	FTM_ACTION_TYPE	xType;
	union
	{
		struct
		{
			FTM_CHAR	pEPID[FTM_EPID_LEN+1];
			FTM_EP_DATA	xValue;
		}	xSet;
	}	xParams;
}	FTM_ACTION, _PTR_ FTM_ACTION_PTR;
typedef	FTM_ULONG	FTM_ACTION_ID, _PTR_ FTM_ACTION_ID_PTR;

FTM_RET	FTM_ACTION_init
(
	FTM_VOID
);

FTM_RET	FTM_ACTION_final
(
	FTM_VOID
);

FTM_RET	FTM_ACTION_create
(
	FTM_ACTION_PTR 	pAction
);

FTM_RET	FTM_ACTION_createSet
(
	FTM_ACTION_ID 	xID, 
	FTM_ACTION_TYPE xType, 
	FTM_CHAR_PTR 	pTargetID, 
	FTM_EP_DATA_PTR pValue, 
	FTM_ACTION_PTR _PTR_ ppAction
);

FTM_RET	FTM_ACTION_destroy
(
	FTM_ACTION_PTR 	ppAction
);

FTM_RET	FTM_ACTION_append
(
	FTM_ACTION_PTR pAction
);

FTM_RET FTM_ACTION_remove
(
	FTM_ACTION_PTR pAction
);

FTM_RET FTM_ACTION_count
(
	FTM_ULONG_PTR 	pulCount
);

FTM_RET FTM_ACTION_get
(
	FTM_ACTION_ID 	xID, 
	FTM_ACTION_PTR _PTR_ ppAction
);

FTM_RET FTM_ACTION_getAt
(
	FTM_ULONG 	ulIndex, 
	FTM_ACTION_PTR _PTR_ ppAction
);

FTM_CHAR_PTR	FTM_ACTION_typeString
(
	FTM_ACTION_TYPE xType
);
#endif
