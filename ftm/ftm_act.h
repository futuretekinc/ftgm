#ifndef	_FTM_aCT_H_
#define	_FTM_aCT_H_

#include "ftm_types.h"
#include "ftm_ep.h"

typedef	FTM_ULONG	FTM_ACT_ID, _PTR_ FTM_ACT_ID_PTR;

typedef	enum 
{
	FTM_ACT_TYPE_ON = 0,
	FTM_ACT_TYPE_OFF,
	FTM_ACT_TYPE_BLINK
} FTM_ACT_TYPE, _PTR_ FTM_ACT_TYPE_PTR;

typedef	struct
{
	FTM_ACT_ID		xID;
	FTM_ACT_TYPE	xType;
	FTM_EPID		xTargetID;
}	FTM_ACT, _PTR_ FTM_ACT_PTR;

FTM_RET	FTM_ACT_init(FTM_VOID);
FTM_RET	FTM_ACT_final(FTM_VOID);

FTM_RET	FTM_ACT_create(FTM_ACT_ID xID, FTM_ACT_TYPE xType, FTM_EPID xTargetID, FTM_ACT_PTR _PTR_ ppActor);
FTM_RET	FTM_ACT_destroy(FTM_ACT_PTR ppActor);

FTM_RET FTM_ACT_count(FTM_ULONG_PTR pulCount);
FTM_RET FTM_ACT_get(FTM_ACT_ID xID, FTM_ACT_PTR _PTR_ ppActor);
FTM_RET FTM_ACT_getAt(FTM_ULONG ulIndex, FTM_ACT_PTR _PTR_ ppActor);

#endif
