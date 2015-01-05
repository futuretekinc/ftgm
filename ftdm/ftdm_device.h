#ifndef	__FTM_DEVICE_H__
#define	__FTM_DEVICE_H__

#include "ftm_types.h"
#include "ftm_object.h"

#define	FTDM_DT_ROOT	0x00000001

FTM_RET	FTDM_initNode
(
 	FTM_VOID
);

FTM_RET	FTDM_finalNode
(
 	FTM_VOID
);

FTM_RET	FTDM_insertNode
(
 	FTM_NODE_INFO_PTR pNode
);

FTM_RET	FTDM_removeNode
(
 	FTM_NODE_INFO_PTR pNode
);

#endif

