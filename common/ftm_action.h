#ifndef	_FTM_ACTION_H_
#define	_FTM_ACTION_H_

#include "ftm_types.h"
#include "ftm_ep.h"

typedef	FTM_ULONG	FTM_ACTION_FIELD, _PTR_ FTM_ACTION_FIELD_PTR;

#define	FTM_ACTION_FIELD_ID				(1 << 0)
#define	FTM_ACTION_FIELD_TYPE			(1 << 1)
#define	FTM_ACTION_FIELD_NAME			(1 << 2)
#define	FTM_ACTION_FIELD_EPID			(1 << 3)
#define	FTM_ACTION_FIELD_VALUE			(1 << 4)
#define	FTM_ACTION_FIELD_ACTION			(1 << 5)
#define	FTM_ACTION_FIELD_ALL			(0xFFFF)

typedef	enum
{
	FTM_ACTION_TYPE_NONE,
	FTM_ACTION_TYPE_SET,
	FTM_ACTION_TYPE_SMS,
	FTM_ACTION_TYPE_PUSH,
	FTM_ACTION_TYPE_MAIL
}	FTM_ACTION_TYPE, _PTR_ FTM_ACTION_TYPE_PTR;

typedef	union
{
	struct
	{
		FTM_CHAR	pEPID[FTM_EPID_LEN+1];
		FTM_VALUE	xValue;
	}	xSet;
}	FTM_ACTION_PARAMS, _PTR_ FTM_ACTION_PARAMS_PTR;

typedef	struct
{
	FTM_CHAR			pID[FTM_ID_LEN+1];
	FTM_ACTION_TYPE		xType;
	FTM_CHAR			pName[FTM_NAME_LEN+1];
	FTM_ACTION_PARAMS	xParams;
}	FTM_ACTION, _PTR_ FTM_ACTION_PTR;

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
	FTM_CHAR_PTR	pID,
	FTM_ACTION_TYPE xType, 
	FTM_CHAR_PTR 	pTargetID, 
	FTM_VALUE_PTR	pValue, 
	FTM_ACTION_PTR _PTR_ ppAction
);

FTM_RET	FTM_ACTION_destroy
(
	FTM_ACTION_PTR 	ppAction
);

FTM_RET	FTM_ACTION_setDefault
(
	FTM_ACTION_PTR	pAction
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
	FTM_CHAR_PTR	pID,
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