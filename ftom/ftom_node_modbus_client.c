#include <stdlib.h>
#include "ftom.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"
#include "ftom_node_class.h"
#include "ftom_node_modbus_client_hhtw.h"

static
FTOM_NODE_CLASS_PTR	pClasses[] =
{
	&xNodeModbusClientHHTW
};

FTM_RET	FTOM_NODE_MBC_create
(
	FTM_NODE_PTR pInfo, 
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppNode != NULL);

	FTM_INT		i;
	FTOM_NODE_MBC_PTR	pNode;
	FTOM_NODE_CLASS_PTR	pClass = NULL;

	if (pInfo->xType != FTM_NODE_TYPE_MODBUS_OVER_TCP)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	for(i = 0 ; i < sizeof(pClasses) / sizeof(FTOM_NODE_CLASS_PTR) ; i++)
	{
		if (strcasecmp(pClasses[i]->pModel, pInfo->pModel) == 0)
		{
			pClass = pClasses[i];	
			break;
		}
	}

	if (pClass == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	pNode = (FTOM_NODE_MBC_PTR)FTM_MEM_malloc(sizeof(FTOM_NODE_MBC));
	if (pNode == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memcpy(&pNode->xCommon.xInfo, pInfo, sizeof(FTM_NODE));

	pNode->xCommon.pClass = pClass;

	*ppNode = (FTOM_NODE_PTR)pNode;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_MBC_destroy
(
	FTOM_NODE_MBC_PTR _PTR_ ppNode
)
{
	ASSERT(ppNode != NULL);

	FTM_MEM_free(*ppNode);

	*ppNode = NULL;

	return	FTM_RET_OK;
}

