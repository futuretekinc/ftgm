#ifndef	__FTDM_NODE_INFO_H__
#define	__FTDM_NODE_INFO_H__

#include "ftm_types.h"
#include "ftm_object.h"
#include "ftdm_config.h"

#define	FTDM_DT_ROOT	0x00000001

FTM_RET	FTDM_initNodeInfo
(
	FTDM_CFG_NODE_PTR	pConfig
);

FTM_RET	FTDM_finalNodeInfo
(
 	FTM_VOID
);

#endif

