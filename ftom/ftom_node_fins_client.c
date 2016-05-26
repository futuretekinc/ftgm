#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ftom.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"
#include "ftom_node_class.h"
#include "ftom_node_fins_client_hhtw.h"

static 
FTOM_NODE_CLASS_PTR	pFINSClasses[1] =
{
	&xHHTWNodeClass 
};

FTM_RET	FTOM_NODE_FINSC_create
(
	FTM_NODE_PTR pInfo, 
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppNode != NULL);

	FTM_INT	i;
	FTOM_NODE_FINSC_PTR	pNode;
	FTOM_NODE_CLASS_PTR	pClass = NULL;

	if (pInfo->xType != FTM_NODE_TYPE_FINS)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	for(i = 0 ; i < sizeof(pFINSClasses) / sizeof(FTOM_NODE_CLASS_PTR) ; i++)
	{
		if(strcasecmp(pInfo->pModel, pFINSClasses[i]->pModel) == 0)
		{
			pClass = pFINSClasses[i];	
			break;
		}
	}

	if (pClass == NULL)
	{
		ERROR("Class not found!\n");
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	pNode = (FTOM_NODE_FINSC_PTR)FTM_MEM_malloc(sizeof(FTOM_NODE_FINSC));
	if (pNode == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memcpy(&pNode->xCommon.xInfo, pInfo, sizeof(FTM_NODE));
	FTM_LOCK_create(&pNode->pLock);

	pNode->xCommon.pClass = pClass;

	*ppNode = (FTOM_NODE_PTR)pNode;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_FINSC_destroy
(
	FTOM_NODE_FINSC_PTR _PTR_ ppNode
)
{
	ASSERT(ppNode != NULL);

	FTM_LOCK_destroy(&(*ppNode)->pLock);

	FTM_MEM_free(*ppNode);

	*ppNode = NULL;

	return	FTM_RET_OK;
}

