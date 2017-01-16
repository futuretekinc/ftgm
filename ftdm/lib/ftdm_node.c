#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftdm.h"
#include "ftdm_node.h"
#include "ftdm_sqlite.h"
#include "ftm_mem.h"
#include "ftdm_log.h"

typedef	struct FTDM_NODE_STRUCT
{
	FTDM_PTR	pFTDM;
	FTM_NODE	xInfo;
}	FTDM_NODE, _PTR_ FTDM_NODE_PTR;

FTM_RET    FTDM_NODE_create
(   
	FTDM_PTR	pFTDM,
	FTM_NODE_PTR	pInfo,
	FTM_BOOL		bWithDB,
	FTDM_NODE_PTR _PTR_ ppNode
)   
{
	ASSERT(pInfo != NULL);

	FTM_RET    		xRet;
	FTDM_NODE_PTR	pNode;
	FTDM_DBIF_PTR	pDBIF;

	xRet = FTM_NODE_isValid(pInfo);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	if (bWithDB)
	{
		xRet = FTDM_getDBIF(pFTDM, &pDBIF);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to get DB interface!\n");
			return	xRet;
		}

		xRet = FTDM_DBIF_addNode(pDBIF, pInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to create node[%s] to DB.\n", pInfo->pDID);
			return	xRet;
		}
	}

	pNode = (FTDM_NODE_PTR)FTM_MEM_malloc(sizeof(FTDM_NODE));
	if (pNode == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory!\n");
		if (bWithDB)
		{
			FTDM_DBIF_deleteNode(pDBIF, pInfo->pDID);
		}
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memset(pNode, 0, sizeof(FTDM_NODE));
	memcpy(&pNode->xInfo, pInfo, sizeof(FTM_NODE));

	pNode->pFTDM = pFTDM;

	*ppNode = pNode;

	return  FTM_RET_OK;
}	  

FTM_RET 	FTDM_NODE_destroy
(
	FTDM_NODE_PTR _PTR_ ppNode,
	FTM_BOOL			bWithDB
)
{
	ASSERT(ppNode != NULL);
	FTM_RET	xRet;

	if (bWithDB)
	{
		FTDM_DBIF_PTR	pDBIF;

		xRet = FTDM_getDBIF((*ppNode)->pFTDM, &pDBIF);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to get DB interface!\n");
			return	xRet;
		}

		xRet = FTDM_DBIF_deleteNode(pDBIF, (*ppNode)->xInfo.pDID);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to remove node[%s] from DB[%08x].\n", (*ppNode)->xInfo.pDID);
		}
	}

	FTM_MEM_free(*ppNode);
	*ppNode = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODE_init
(
	FTDM_NODE_PTR	pNode
)
{
	ASSERT(pNode != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODE_final
(
	FTDM_NODE_PTR	pNode
)
{
	ASSERT(pNode != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODE_getInfo
(
	FTDM_NODE_PTR	pNode,
	FTM_NODE_PTR	pInfo
)
{
	ASSERT(pNode != NULL);
	ASSERT(pInfo != NULL);

	memcpy(pInfo, &pNode->xInfo, sizeof(FTM_NODE));

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODE_setInfo
(
	FTDM_NODE_PTR	pNode,
	FTM_NODE_PTR	pInfo
)
{
	ASSERT(pNode != NULL);
	ASSERT(pInfo != NULL);
	
	FTM_RET	xRet;

	xRet = FTM_NODE_isValid(pInfo);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	memcpy(&pNode->xInfo, pInfo, sizeof(FTM_NODE));

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODE_isInfoChanged
(
	FTDM_NODE_PTR	pNode,
	FTM_NODE_PTR	pInfo,
	FTM_BOOL_PTR	pbChanged
)
{
	ASSERT(pNode != NULL);
	ASSERT(pInfo != NULL);
	
	*pbChanged = FTM_FALSE;

	if ((strcmp(pNode->xInfo.pDID, pInfo->pDID) != 0) ||
	    (pNode->xInfo.xType != pInfo->xType) || 
		(strcmp(pNode->xInfo.pModel, pInfo->pModel) != 0) ||
		(strcmp(pNode->xInfo.pName, pInfo->pName) != 0) ||
		(pNode->xInfo.xFlags != pInfo->xFlags) ||
		(strcmp(pNode->xInfo.pLocation, pInfo->pLocation) != 0) ||
		(pNode->xInfo.ulReportInterval != pInfo->ulReportInterval) ||
		(pNode->xInfo.ulTimeout != pInfo->ulTimeout))
	{
		*pbChanged = FTM_TRUE;	
		return	FTM_RET_OK;
	}

	switch(pNode->xInfo.xType)
	{
	case	FTM_NODE_TYPE_SNMP:
		{	
			if ((pNode->xInfo.xOption.xSNMP.ulVersion != pInfo->xOption.xSNMP.ulVersion) ||
			 	(strcmp(pNode->xInfo.xOption.xSNMP.pURL, pInfo->xOption.xSNMP.pURL) != 0) ||
				(strcmp(pNode->xInfo.xOption.xSNMP.pCommunity, pInfo->xOption.xSNMP.pCommunity) != 0) ||
				(strcmp(pNode->xInfo.xOption.xSNMP.pMIB, pInfo->xOption.xSNMP.pMIB) != 0) ||
				(pNode->xInfo.xOption.xSNMP.ulMaxRetryCount != pInfo->xOption.xSNMP.ulMaxRetryCount))
			{
				*pbChanged = FTM_TRUE;	
			}
		}
		break;

	case	FTM_NODE_TYPE_MODBUS_OVER_TCP:
		{	
			if ((pNode->xInfo.xOption.xMB.ulVersion != pInfo->xOption.xMB.ulVersion) ||
				(strcmp(pNode->xInfo.xOption.xMB.pURL, pInfo->xOption.xMB.pURL) != 0) ||
				(pNode->xInfo.xOption.xMB.ulPort != pInfo->xOption.xMB.ulPort) ||
				(pNode->xInfo.xOption.xMB.ulSlaveID != pInfo->xOption.xMB.ulSlaveID))
			{
				*pbChanged = FTM_TRUE;	
			}
		}
		break;

	default:
		{
			return	FTM_RET_INVALID_TYPE;
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODE_getID
(
	FTDM_NODE_PTR	pNode,
	FTM_CHAR_PTR	pBuff,
	FTM_ULONG		ulBuffLen
)
{
	ASSERT(pNode != NULL);
	ASSERT(pBuff != NULL);

	if (ulBuffLen < strlen(pNode->xInfo.pDID) + 1)
	{
		return	FTM_RET_BUFFER_TOO_SMALL;	
	}

	strncpy(pBuff, pNode->xInfo.pDID, ulBuffLen);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODE_getURL
(
	FTDM_NODE_PTR	pNode,
	FTM_CHAR_PTR	pBuff,
	FTM_ULONG		ulBuffLen
)
{
	ASSERT(pNode != NULL);
	ASSERT(pBuff != NULL);

	switch(pNode->xInfo.xType)
	{
	case	FTM_NODE_TYPE_SNMP:
		{	
			if (ulBuffLen < strlen(pNode->xInfo.xOption.xSNMP.pURL) + 1)
			{
				return	FTM_RET_BUFFER_TOO_SMALL;	
			}

			strncpy(pBuff, pNode->xInfo.xOption.xSNMP.pURL, ulBuffLen);
		}
		break;

	case	FTM_NODE_TYPE_MODBUS_OVER_TCP:
		{	
			if (ulBuffLen < strlen(pNode->xInfo.xOption.xMB.pURL) + 1)
			{
				return	FTM_RET_BUFFER_TOO_SMALL;	
			}

			strncpy(pBuff, pNode->xInfo.xOption.xMB.pURL, ulBuffLen);
		}
		break;

	default:
		return	FTM_RET_INVALID_TYPE;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_NODE_getType
(
	FTDM_NODE_PTR	pNode,
	FTM_NODE_TYPE_PTR	pType
)
{
	ASSERT(pNode != NULL);
	ASSERT(pType != NULL);

	*pType = pNode->xInfo.xType;

	return	FTM_RET_OK;
}

FTM_BOOL FTDM_NODE_seeker
(
	const FTM_VOID_PTR pElement,
	const FTM_VOID_PTR pKey
)
{
	FTDM_NODE_PTR	pNode = (FTDM_NODE_PTR)pElement;
	FTM_CHAR_PTR	pDID = (FTM_CHAR_PTR)pKey;

	return	(strcmp(pNode->xInfo.pDID, pDID) == 0);
}

FTM_BOOL FTDM_NODE_comparator
(
	const FTM_VOID_PTR pElement1,
	const FTM_VOID_PTR pElement2
)
{
	FTDM_NODE_PTR	pNode1 = (FTDM_NODE_PTR)pElement1;
	FTDM_NODE_PTR	pNode2 = (FTDM_NODE_PTR)pElement2;

	return	strcasecmp(pNode1->xInfo.pDID, pNode2->xInfo.pDID);
}

