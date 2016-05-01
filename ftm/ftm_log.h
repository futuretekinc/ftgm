#ifndef	_FTM_LOG_H_
#define	_FTM_LOG_H_

#include "ftm_types.h"
#include "ftm_time.h"

typedef	enum	FTM_LOG_TYPE_ENUM
{
	FTM_LOG_TYPE_EVENT
}	FTM_LOG_TYPE, _PTR_ FTM_LOG_TYPE_PTR;

typedef	struct
{
	FTM_CHAR	pRuleID[FTM_ID_LEN+1];
	FTM_BOOL	bOccurred;
}	FTM_LOG_EVENT_PARAMS, _PTR_ FTM_LOG_EVENT_PARAMS_PTR;
	
typedef	struct FTM_LOG_STRUCT
{
	FTM_LOG_TYPE	xType;
	FTM_TIME		xTime;
	union
	{
		FTM_LOG_EVENT_PARAMS	xEvent;
	}	xParams;
}	FTM_LOG, _PTR_ FTM_LOG_PTR;

#endif
