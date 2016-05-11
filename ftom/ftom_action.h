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

	
FTM_RET	FTOM_ACTION_init
(
	FTM_VOID
);

FTM_RET	FTOM_ACTION_final
(
	FTM_VOID
);

FTM_RET	FTOM_ACTION_create
(
	FTM_ACTION_PTR	pInfo,
	FTOM_ACTION_PTR _PTR_ ppActionM
);

FTM_RET	FTOM_ACTION_createFromDB
(
	FTM_CHAR_PTR	pID,
	FTOM_ACTION_PTR _PTR_ ppActionM
);

FTM_RET	FTOM_ACTION_destroy
(
	FTOM_ACTION_PTR _PTR_ ppActionM
);

FTM_RET	FTOM_ACTION_start
(
	FTM_VOID
);

FTM_RET	FTOM_ACTION_stop
(
	FTM_VOID
);

FTM_RET	FTOM_ACTION_count
(
	FTM_ULONG_PTR 		pulCount
);

FTM_RET	FTOM_ACTION_get
(
	FTM_CHAR_PTR		pActionID,
	FTOM_ACTION_PTR _PTR_ ppActor
);

FTM_RET	FTOM_ACTION_getAt
(
	FTM_ULONG 			ulIndex, 
	FTOM_ACTION_PTR _PTR_ ppActor
);

FTM_RET	FTOM_ACTION_setInfo
(
	FTOM_ACTION_PTR		pAction,
	FTM_ACTION_FIELD	xFields,
	FTM_ACTION_PTR		xInfo
);

FTM_RET	FTOM_ACTION_active
(
	FTM_CHAR_PTR		pActionID,
	FTM_BOOL			bActivate
);

FTM_RET	FTOM_ACTION_printList
(
	FTM_VOID
);

#endif
