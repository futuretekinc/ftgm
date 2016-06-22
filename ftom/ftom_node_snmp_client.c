#include <string.h>
#include "ftom.h"
#include "ftom_node_class.h"
#include "ftom_node_snmp_client.h"
#include "ftom_node_snmp_client_gen.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"

FTM_ULONG		active_hosts = 0;


static 
FTOM_NODE_CLASS_PTR	pClasses[] =
{
	&xGeneralSNMP
};

FTM_RET	FTOM_NODE_SNMPC_getClass
(
	FTM_CHAR_PTR	pModel,
	FTOM_NODE_CLASS_PTR	_PTR_ ppClass
)
{
	FTM_INT	i;

	if (strlen(pModel) == 0)
	{
		*ppClass = &xGeneralSNMP;	
		return	FTM_RET_OK;
	}

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

FTM_RET	FTOM_NODE_SNMPC_create
(
	FTM_NODE_PTR pInfo, 
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppNode != NULL);
	
	FTM_RET	xRet;
	FTOM_NODE_SNMPC_PTR	pNode;
	FTOM_NODE_CLASS_PTR	pClass = NULL;

	xRet = FTOM_NODE_SNMPC_getClass(pInfo->pModel, &pClass);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Class[%s] not found!\n", pInfo->pModel);
		return	xRet;
	}

	pNode = (FTOM_NODE_SNMPC_PTR)FTM_MEM_malloc(sizeof(FTOM_NODE_SNMPC));
	if (pNode == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memcpy(&pNode->xCommon.xInfo, pInfo, sizeof(FTM_NODE));

	pNode->xCommon.pClass = pClass;
	if (strlen(pNode->xCommon.xInfo.pName) == 0)
	{
		snprintf(pNode->xCommon.xInfo.pName, FTM_NAME_LEN, "node_%s", pNode->xCommon.xInfo.pDID);
	}

	*ppNode = (FTOM_NODE_PTR)pNode;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_SNMPC_destroy
(
	FTOM_NODE_SNMPC_PTR _PTR_ ppNode
)
{
	ASSERT(ppNode != NULL);

	FTM_MEM_free(*ppNode);

	*ppNode = NULL;

	return	FTM_RET_OK;
}

static FTM_CHAR_PTR	pOIDNamePrefix[] =
{
	"",
	"temp",
	"humi",
	"vlt",
	"curr",
	"di",
	"do",
	"cnt",
	"prs",
	"dsc",
	"dev"
};

FTM_RET		FTOM_NODE_SNMPC_getOID
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTM_ULONG 			ulType, 
	FTM_ULONG 			ulIndex, 
	oid 				*pOID, 
	size_t 				*pnOIDLen
)
{
	ASSERT(pNode != NULL);
	ASSERT(pOID != NULL);
	ASSERT(pnOIDLen != NULL);

	FTM_CHAR	pBuff[1024];

	sprintf(pBuff, "%s:%sValue.%lu", pNode->xCommon.xInfo.xOption.xSNMP.pMIB, pOIDNamePrefix[ulType], ulIndex);

	return	FTOM_SNMPC_getOID(pBuff, pOID, pnOIDLen);
}
