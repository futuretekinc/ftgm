#ifndef	__FTDM_NODE_H__
#define	__FTDM_NODE_H__

#include "ftm.h"
#include "ftdm_config.h"

#define	FTDM_DT_ROOT	0x00000001

typedef	struct
{
	FTM_NODE	xInfo;
}	FTDM_NODE, _PTR_ FTDM_NODE_PTR;

FTM_RET	FTDM_NODE_create
(
	FTM_NODE_PTR		pInfo,
	FTDM_NODE_PTR _PTR_ ppNode
);

FTM_RET FTDM_NODE_destroy
(
	FTDM_NODE_PTR _PTR_ ppNode
);

FTM_RET FTDM_NODE_destroy2
(
	FTDM_NODE_PTR _PTR_ ppNode
);

FTM_RET	FTDM_NODE_setInfo
(
	FTDM_NODE_PTR	pNode,
	FTM_NODE_PTR	pInfo
);

#endif

