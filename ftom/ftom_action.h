#ifndef	_FTOM_ACTION_H_
#define	_FTOM_ACTION_H_

#include <pthread.h>
#include "ftm.h"
#include "ftom.h"


typedef	struct
{
	FTM_ACTION	xInfo;
	FTM_TIMER	xTimer;
}	FTOM_ACTION, _PTR_ FTOM_ACTION_PTR;

FTM_RET	FTOM_ACTION_create
(
	FTM_ACTION_PTR 		pInfo,
	FTOM_ACTION_PTR _PTR_ ppAction
);

FTM_RET	FTOM_ACTION_destroy
(
	FTOM_ACTION_PTR _PTR_ ppAction
);

FTM_RET	FTOM_ACTION_init
(
	FTOM_ACTION_PTR pAction,
	FTM_ACTION_PTR 	pInfo
);

FTM_RET	FTOM_ACTION_final
(
	FTOM_ACTION_PTR pAction
);

#endif
