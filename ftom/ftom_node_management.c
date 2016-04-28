#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ftm.h"
#include "ftom.h"
#include "ftom_node_management.h"
#include "ftom_ep.h"

static FTM_RET FTOM_NODEM_init
(
	FTOM_PTR		pOM,
	FTOM_NODEM_PTR pNodeM
);

static FTM_RET FTOM_NODEM_final
(
	FTOM_NODEM_PTR pNodeM
);

static FTM_INT	FTOM_NODEM_seek
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);

static FTM_INT	FTOM_NODEM_comparator
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);

/********************************************************************************
 *	NODE Manager
 ********************************************************************************/

FTM_RET	FTOM_NODEM_create
(
	FTOM_PTR		pOM,
	FTOM_NODEM_PTR _PTR_ ppNodeM
)
{
	ASSERT(ppNodeM != NULL);

	FTM_RET			xRet;
	FTOM_NODEM_PTR	pNodeM;

	pNodeM = (FTOM_NODEM_PTR)FTM_MEM_malloc(sizeof(FTOM_NODEM));
	if (pNodeM == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_NODEM_init(pOM, pNodeM);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pNodeM);
		return	xRet;	
	}

	*ppNodeM = pNodeM;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODEM_destroy
(
	FTOM_NODEM_PTR _PTR_ ppNodeM
)
{
	ASSERT(ppNodeM != NULL);
	
	FTM_RET	xRet;

	xRet = FTOM_NODEM_final(*ppNodeM);
	if (xRet != FTM_RET_OK)
	{
		WARN("Node manager finalize failed.");
	}

	FTM_MEM_free(*ppNodeM);

	*ppNodeM = NULL;

	return	FTM_RET_OK;
}

FTM_RET FTOM_NODEM_init
(
	FTOM_PTR		pOM,
	FTOM_NODEM_PTR pNodeM
)
{
	ASSERT(pNodeM != NULL);

	FTM_RET	xRet;

	xRet = FTM_LIST_create(&pNodeM->pList);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}
	pNodeM->pOM = pOM;
	FTM_LIST_setSeeker(pNodeM->pList, FTOM_NODEM_seek);
	FTM_LIST_setComparator(pNodeM->pList, FTOM_NODEM_comparator);

	return	FTM_RET_OK;
}

FTM_RET FTOM_NODEM_final
(
	FTOM_NODEM_PTR	pNodeM
)
{	
	ASSERT(pNodeM != NULL);
	
	FTOM_NODE_PTR	pNode = NULL;

	FTM_LIST_iteratorStart(pNodeM->pList);
	while(FTM_LIST_iteratorNext(pNodeM->pList, (FTM_VOID_PTR _PTR_)&pNode) == FTM_RET_OK)
	{
		FTOM_NODE_destroy(&pNode);	
	}

	FTM_LIST_destroy(pNodeM->pList);
	pNodeM->pList = NULL;

	return	FTM_RET_OK;
}

FTM_RET FTOM_NODEM_getNode
(
	FTOM_NODEM_PTR	pNodeM,
	FTM_CHAR_PTR pDID, 
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pDID != NULL);
	ASSERT(ppNode != NULL);

	FTM_RET			xRet;
	FTOM_NODE_PTR	pNode;
	
	xRet = FTM_LIST_get(pNodeM->pList, (FTM_VOID_PTR)pDID, (FTM_VOID_PTR _PTR_)&pNode);
	if (xRet == FTM_RET_OK)
	{
		*ppNode = pNode;
	}

	return	xRet;
}

FTM_RET FTOM_NODEM_getNodeAt
(
	FTOM_NODEM_PTR	pNodeM,
	FTM_ULONG ulIndex, 
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(ppNode != NULL);

	FTM_RET			xRet;
	FTOM_NODE_PTR	pNode;

	xRet = FTM_LIST_getAt(pNodeM->pList, ulIndex, (FTM_VOID_PTR _PTR_)&pNode);
	if (xRet == FTM_RET_OK)
	{
		*ppNode = pNode;
	}

	return	xRet;
}

FTM_RET FTOM_NODEM_setNode
(
	FTOM_NODEM_PTR	pNodeM,
	FTM_CHAR_PTR 	pDID, 
	FTM_NODE_FIELD	xFields,
	FTM_NODE_PTR 	pInfo
)
{
	ASSERT(pNodeM != NULL);
	ASSERT(pDID != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET			xRet;
	FTOM_NODE_PTR	pNode;
	
	xRet = FTM_LIST_get(pNodeM->pList, (FTM_VOID_PTR)pDID, (FTM_VOID_PTR _PTR_)&pNode);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	if (xFields & FTM_NODE_FIELD_FLAGS)
	{
		pNode->xInfo.xFlags = pInfo->xFlags;
	}

	if (xFields & FTM_NODE_FIELD_LOCATION)
	{
		strcpy(pNode->xInfo.pLocation, pInfo->pLocation);
	}

	if (xFields & FTM_NODE_FIELD_INTERVAL)
	{
		pNode->xInfo.ulInterval = pInfo->ulInterval;
	}

	if (xFields & FTM_NODE_FIELD_TIMEOUT)
	{
		pNode->xInfo.ulTimeout = pInfo->ulTimeout;
	}

	if (xFields & FTM_NODE_FIELD_SNMP_VERSION)
	{
		pNode->xInfo.xOption.xSNMP.ulVersion = pInfo->xOption.xSNMP.ulVersion ;
	}

	if (xFields & FTM_NODE_FIELD_SNMP_URL)
	{
		strcpy(pNode->xInfo.xOption.xSNMP.pURL, pInfo->xOption.xSNMP.pURL);
	}

	if (xFields & FTM_NODE_FIELD_SNMP_COMMUNITY)
	{
		strcpy(pNode->xInfo.xOption.xSNMP.pCommunity, pInfo->xOption.xSNMP.pCommunity);
	}

	if (xFields & FTM_NODE_FIELD_SNMP_MIB)
	{
		strcpy(pNode->xInfo.xOption.xSNMP.pMIB, pInfo->xOption.xSNMP.pMIB);
	}

	if (xFields & FTM_NODE_FIELD_SNMP_MAX_RETRY)
	{
		pNode->xInfo.xOption.xSNMP.ulMaxRetryCount = pInfo->xOption.xSNMP.ulMaxRetryCount;
	}

	if (xFields & FTM_NODE_FIELD_MQTT_VERSION)
	{
		pNode->xInfo.xOption.xMQTT.ulVersion = pInfo->xOption.xMQTT.ulVersion;
	}

	if (xFields & FTM_NODE_FIELD_MQTT_URL)
	{
		strcpy(pNode->xInfo.xOption.xMQTT.pURL, pInfo->xOption.xMQTT.pURL);
	}

	if (xFields & FTM_NODE_FIELD_MQTT_TOPIC)
	{
		strcpy(pNode->xInfo.xOption.xMQTT.pTopic, pInfo->xOption.xMQTT.pTopic);
	}

	if (xFields & FTM_NODE_FIELD_LORA_VERION)
	{
		pNode->xInfo.xOption.xLoRa.ulVersion = pInfo->xOption.xLoRa.ulVersion;
	}

	if (xFields & FTM_NODE_FIELD_LORA_DEVICE)
	{
		strcpy(pNode->xInfo.xOption.xLoRa.pDevice, pInfo->xOption.xLoRa.pDevice);
	}


	return	xRet;
}
FTM_RET	FTOM_NODEM_countNode
(	
	FTOM_NODEM_PTR	pNodeM,
	FTM_ULONG_PTR 	pulCount
)
{
	return	FTM_LIST_count(pNodeM->pList, pulCount);
}

FTM_RET	FTOM_NODEM_attachNode
(
	FTOM_NODEM_PTR	pNodeM,
	FTOM_NODE_PTR	pNode
)
{
	ASSERT(pNodeM != NULL);
	ASSERT(pNode != NULL);
	
	FTM_RET	xRet;

	xRet = FTM_LIST_seek(pNodeM->pList, pNode->xInfo.pDID);
	if (xRet == FTM_RET_OK)
	{
		return	FTM_RET_ALREADY_ATTACHED;	
	}

	xRet = FTM_LIST_append(pNodeM->pList, pNode);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pNode->pNodeM = pNodeM;

	return	FTM_RET_OK;
}


FTM_RET	FTOM_NODEM_detachNode
(
	FTOM_NODEM_PTR	pNodeM,
	FTOM_NODE_PTR	pNode
)
{
	ASSERT(pNodeM != NULL);
	ASSERT(pNode != NULL);

	FTM_RET	xRet;

	if (pNode->pNodeM != pNodeM)
	{
		return	FTM_RET_NOT_ATTACHED;	
	}

	xRet = FTM_LIST_remove(pNodeM->pList, pNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR("pNode is not attached.");
	}

	pNode->pNodeM = NULL;

	return	FTM_RET_OK;

}

FTM_INT	FTOM_NODEM_seek
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
)
{
	FTOM_NODE_PTR	pNode = (FTOM_NODE_PTR)pElement;
	FTM_CHAR_PTR	pDID = (FTM_CHAR_PTR)pIndicator;

	if ((pElement == NULL) || (pIndicator == NULL))
	{
		return	0;	
	}

	return	(strcasecmp(pNode->xInfo.pDID, pDID) == 0);
}

FTM_INT	FTOM_NODEM_comparator
(
	const FTM_VOID_PTR pElement1, 
	const FTM_VOID_PTR pElement2
)
{
	FTOM_NODE_PTR	pNode1 = (FTOM_NODE_PTR)pElement1;
	FTOM_NODE_PTR	pNode2 = (FTOM_NODE_PTR)pElement2;

	if ((pElement1 == NULL) || (pElement2 == NULL))
	{
		return	0;	
	}

	return	strcasecmp(pNode1->xInfo.pDID, pNode2->xInfo.pDID);
}


