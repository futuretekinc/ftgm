#ifndef	__FTOM_NODE_MODBUS_CLIENT_H__
#define	__FTOM_NODE_MODBUS_CLIENT_H__

#include <semaphore.h>
#include "ftom.h"
#include "ftm_timer.h"
#include "ftom_node.h"
#include "modbus/modbus-tcp.h"
#include "ftm_lock.h"

typedef	struct 
{
	FTOM_NODE				xCommon;
	modbus_t				*pMB;
	FTM_LOCK				xLock;

	struct 
	{
		FTM_ULONG			ulRequest;
		FTM_ULONG			ulResponse;
	}	xStatistics;	
}	FTOM_NODE_MBC, _PTR_ FTOM_NODE_MBC_PTR;

typedef	FTM_RET	(*FTOM_NODE_MBC_GET)(FTOM_NODE_MBC_PTR	pMBC, FTOM_EP_PTR pEP, FTM_EP_DATA_PTR PData);
typedef	FTM_RET	(*FTOM_NODE_MBC_SET)(FTOM_NODE_MBC_PTR	pMBC, FTOM_EP_PTR pEP, FTM_EP_DATA_PTR PData);

typedef	struct
{
	FTM_CHAR			pModel[FTM_NAME_LEN+1];
	FTOM_NODE_MBC_GET	fSet;
	FTOM_NODE_MBC_SET	fGet;
}	FTOM_NODE_MBC_DESCRIPT, _PTR_ FTOM_NODE_MBC_DESCRIPT_PTR;

FTM_RET	FTOM_NODE_MBC_getClass
(
	FTM_CHAR_PTR		pModel,
	FTOM_NODE_CLASS_PTR	_PTR_ ppClass
);

FTM_RET	FTOM_NODE_MBC_create
(
	FTM_NODE_PTR 		pInfo, 
	FTOM_NODE_PTR _PTR_ ppNode
);

FTM_RET	FTOM_NODE_MBC_destroy
(
	FTOM_NODE_MBC_PTR _PTR_ pNode
);

FTM_RET	FTOM_NODE_MBC_set
(
	FTOM_NODE_MBC_PTR	pNode,
	FTM_NODE_FIELD		xFields,
	FTM_NODE_PTR		pInfo
);

FTM_RET	FTOM_NODE_MBC_printOpts
(
	FTOM_NODE_MBC_PTR	pNode
);
#endif

