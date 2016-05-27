#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ftom.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"
#include "ftom_node_class.h"
#include "ftom_node_fins_client.h"
#include "ftom_node_fins_client_hhtw.h"

static 
FTOM_NODE_CLASS_PTR	pClasses[1] =
{
	&xHHTWNodeClass 
};

FTM_RET	FTOM_NODE_FINSC_getClass
(
	FTM_CHAR_PTR	pModel,
	FTOM_NODE_CLASS_PTR	_PTR_ ppClass
)
{
	FTM_INT	i;

	for(i = 0 ; i < sizeof(pClasses) / sizeof(FTOM_NODE_CLASS_PTR) ; i++)
	{
		if(strcasecmp(pModel, pClasses[i]->pModel) == 0)
		{
			*ppClass = pClasses[i];	
			return	FTM_RET_OK;
		}
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

FTM_RET	FTOM_NODE_FINSC_create
(
	FTM_NODE_PTR pInfo, 
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppNode != NULL);

	FTM_RET	xRet;
	FTOM_NODE_FINSC_PTR	pNode;
	FTOM_NODE_CLASS_PTR	pClass = NULL;

	xRet = FTOM_NODE_FINSC_getClass(pInfo->pModel, &pClass);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Class[%s] not found!\n", pInfo->pModel);
		return	xRet;
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

