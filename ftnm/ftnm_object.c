#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "ftnm.h"
#include "ftnm_object.h"
#include "simclist.h"


static FTM_VOID_PTR	FTNM_threadMain(FTM_VOID_PTR pParams);
static FTM_INT	FTNM_nodeSeeker(const void *pElement, const void *pKey) ;

list_t	xNodeList;

FTM_RET FTNM_initNodeManager(void)
{
	if (list_init(&xNodeList) < 0)
	{
		return	FTM_RET_INTERNAL_ERROR;	
	}

    list_attributes_seeker(&xNodeList, FTNM_nodeSeeker);

	return	FTM_RET_OK;
}

FTM_RET FTNM_finalNodeManager(void)
{
	while (0 != list_size(&xNodeList))
	{
		FTNM_NODE_PTR pNode = list_get_at(&xNodeList, 0);
		if (pNode != NULL)
		{
			FTM_CHAR	pDID[FTDM_DEVICE_ID_LEN+1];

			memset(pDID, 0, sizeof(pDID));
			strncpy(pDID, pNode->xInfo.pDID, FTDM_DEVICE_ID_LEN);

			FTNM_destroyNode(pDID);	
		}
	}
	list_destroy(&xNodeList);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_createNodeSNMP(FTDM_DEVICE_INFO_PTR pInfo)
{
	FTNM_NODE_SNMP_PTR	pNode;

	pNode = (FTNM_NODE_SNMP_PTR)calloc(1, sizeof(FTNM_NODE_SNMP));
	if (pNode == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pNode->xType = FTNM_NODE_TYPE_SNMP;

	memcpy(&pNode->xInfo, pInfo, sizeof(FTDM_DEVICE_INFO));
	list_init(&pNode->xEPList);

	pNode->xSNMP.nVersion = SNMP_VERSION_2c;
	pNode->xSNMP.pPeerName= "10.0.1.35";
	pNode->xSNMP.pCommunity= "public";
	list_init(&pNode->xSNMP.xOIDList);

	pNode->nUpdateInterval = 10;
	list_append(&xNodeList, pNode);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_destroyNode(FTM_CHAR_PTR	pDID)
{
	FTM_INT	i;
	FTNM_NODE_PTR	pNode;

	if (FTNM_getNode(pDID, &pNode) != FTM_RET_OK)
	{
		ERROR("Node not found[DID = %s]\n", pDID);
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	if (list_delete(&xNodeList, pNode) != 0)
	{
		ERROR("object not found[DID = %s]\n", pDID);
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	for(i = 0 ; i < list_size(&pNode->xEPList) ; i++)
	{
		FTDM_EP_INFO *pEP = list_get_at(&pNode->xEPList, i);
		if (pEP != NULL)
		{
			free(pEP);	
		}
	}

	list_destroy(&pNode->xEPList);
	free(pNode);

	return	FTM_RET_OK;
}

FTM_RET FTNM_getNode(FTDM_CHAR_PTR pDID, FTNM_NODE_PTR _PTR_ ppNode)
{
	FTNM_NODE_PTR	pNode;

	pNode = (FTNM_NODE_PTR)list_seek(&xNodeList, pDID);
	if (pNode == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	*ppNode = pNode;
	return	FTM_RET_OK;
}


FTM_RET	FTNM_startNode(FTNM_NODE_PTR pNode)
{
	if (pNode == NULL) 
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	pthread_create(&pNode->xPThread, NULL, &FTNM_threadMain, (FTM_VOID_PTR)pNode);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_stopNode(FTNM_NODE_PTR pNode)
{
	if (pNode == NULL) 
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	pthread_exit(&pNode->xPThread);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_restartNode(FTNM_NODE_PTR pNode)
{
	FTM_RET	nRet;

	nRet = FTNM_stopNode(pNode);
	if (nRet != FTM_RET_OK)
	{	
		return	nRet;
	}

	nRet = FTNM_startNode(pNode);
	if (nRet != FTM_RET_OK)
	{	
		return	nRet;
	}

	return	FTM_RET_OK;
}

FTM_RET FTNM_addEP(FTDM_EP_INFO_PTR pInfo)
{
	FTM_RET				nRet;
	FTNM_NODE_PTR		pNode;
	FTDM_EP_INFO_PTR	pEPInfo;

	if (pInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	nRet = FTNM_getNode(pInfo->pDID, &pNode);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;
	}

	pEPInfo = (FTDM_EP_INFO_PTR)calloc(1, sizeof(FTDM_EP_INFO));
	if (pEPInfo == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}
	memcpy(pEPInfo, pInfo, sizeof(FTDM_EP_INFO));

	list_append(&pNode->xEPList, pEPInfo);
	
	return	FTM_RET_OK;
}
static FTM_INT nTemp = 0;

static FTM_VOID_PTR	FTNM_threadMain(FTM_VOID_PTR pParams)
{
	FTNM_NODE_PTR	pNode = (FTNM_NODE_PTR)pParams;
	time_t			xBaseTime, xCurrentTime;

	xBaseTime = time(NULL);
	while(1)
	{
		switch(pNode->xType)
		{
		case	FTNM_NODE_TYPE_SNMP:
			{
				if (FTNM_snmpIsRunning(&((FTNM_NODE_SNMP_PTR)pNode)->xSNMP) != FTM_BOOL_TRUE)
				{
						FTNM_snmpClientAsyncCall(&((FTNM_NODE_SNMP_PTR)pNode)->xSNMP);
				}
			}
			break;
		}
		xCurrentTime = time(NULL);
		xBaseTime += pNode->nUpdateInterval;
		sleep(xBaseTime - xCurrentTime);
	}
	
	pthread_exit((FTM_VOID_PTR)0);
}

static FTM_INT	FTNM_nodeSeeker(const void *pElement, const void *pKey) 
{
	FTNM_NODE_PTR	pNode = (FTNM_NODE_PTR)pElement;
	FTM_CHAR_PTR	pDID  = (FTM_CHAR_PTR)pKey;

	if (strncmp(pNode->xInfo.pDID, pDID, FTDM_DEVICE_ID_LEN) == 0)
	{
		return 1;
	}

	return 0;
}

