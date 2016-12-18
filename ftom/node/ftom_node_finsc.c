#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ftom.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"
#include "ftom_node_class.h"
#include "ftom_node_finsc.h"
#include "ftom_node_hhtw.h"

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
		ERROR2(xRet, "Class[%s] not found!\n", pInfo->pModel);
		return	xRet;
	}

	pNode = (FTOM_NODE_FINSC_PTR)FTM_MEM_malloc(sizeof(FTOM_NODE_FINSC));
	if (pNode == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR2(xRet, "Not enough memory!\n");
		return	xRet;
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

FTM_RET	FTOM_NODE_FINSC_set
(
	FTOM_NODE_FINSC_PTR		pNode,
	FTM_NODE_FIELD			xFields,
	FTM_NODE_PTR			pInfo
)
{
	ASSERT(pNode != NULL);
	ASSERT(pInfo != NULL);

	if (xFields & FTM_NODE_FIELD_FLAGS)
	{
		pNode->xCommon.xInfo.xFlags = pInfo->xFlags;
	}

	if (xFields & FTM_NODE_FIELD_NAME)
	{
		strcpy(pNode->xCommon.xInfo.pName, pInfo->pName);
	}

	if (xFields & FTM_NODE_FIELD_LOCATION)
	{
		strcpy(pNode->xCommon.xInfo.pLocation, pInfo->pLocation);
	}

	if (xFields & FTM_NODE_FIELD_INTERVAL)
	{
		pNode->xCommon.xInfo.ulReportInterval = pInfo->ulReportInterval;
	}

	if (xFields & FTM_NODE_FIELD_TIMEOUT)
	{
		pNode->xCommon.xInfo.ulTimeout = pInfo->ulTimeout;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_FINSC_printOpts
(
	FTOM_NODE_FINSC_PTR	pNode
)
{
	ASSERT(pNode != NULL);

	MESSAGE("%16s   %10s - %lu\n", "", "Version", 	pNode->xCommon.xInfo.xOption.xFINS.ulVersion);	
	MESSAGE("%16s   %10s - %s\n", "", "DestIP", 	pNode->xCommon.xInfo.xOption.xFINS.pDIP);
	MESSAGE("%16s   %10s - %lu\n","", "DestPort",	pNode->xCommon.xInfo.xOption.xFINS.ulDP);
	MESSAGE("%16s   %10s - %lu\n","", "SrcPort", 	pNode->xCommon.xInfo.xOption.xFINS.ulSP);
	MESSAGE("%16s   %10s - %02x:%02x:%02x\n","", "DestAddr",	
			(FTM_UINT8)((pNode->xCommon.xInfo.xOption.xFINS.ulDA >> 16) & 0xFF),
			(FTM_UINT8)((pNode->xCommon.xInfo.xOption.xFINS.ulDA >>  8) & 0xFF),
			(FTM_UINT8)((pNode->xCommon.xInfo.xOption.xFINS.ulDA >>  0) & 0xFF));
	MESSAGE("%16s   %10s - %02x:%02x:%02x\n","", "SrcAddr", 
			(FTM_UINT8)((pNode->xCommon.xInfo.xOption.xFINS.ulSA >> 16) & 0xFF),
			(FTM_UINT8)((pNode->xCommon.xInfo.xOption.xFINS.ulSA >>  8) & 0xFF),
			(FTM_UINT8)((pNode->xCommon.xInfo.xOption.xFINS.ulSA >>  0) & 0xFF));
	MESSAGE("%16s   %10s - %lu\n","", "ServerID", 	pNode->xCommon.xInfo.xOption.xFINS.ulServerID);

	return	FTM_RET_OK;
}

