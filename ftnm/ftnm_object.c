#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "ftnm.h"
#include "ftnm_object.h"
#include "ftnm_snmp_client.h"
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

FTM_RET	FTNM_createNodeSNMP(FTDM_DEVICE_INFO_PTR pInfo, FTNM_NODE_PTR _PTR_ ppNode)
{
	FTNM_NODE_PTR			pNode;
	FTNM_SNMP_CONTEXT_PTR	pSNMP;

	pNode = (FTNM_NODE_PTR)calloc(1, sizeof(FTNM_NODE));
	if (pNode == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pSNMP = (FTNM_SNMP_CONTEXT_PTR)calloc(1, sizeof(FTNM_SNMP_CONTEXT));
	if (pSNMP == NULL)
	{
		free(pNode);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pNode->xType = FTNM_NODE_TYPE_SNMP;

	memcpy(&pNode->xInfo, pInfo, sizeof(FTDM_DEVICE_INFO));
	list_init(&pNode->xEPList);

	pSNMP->xInfo.nVersion = SNMP_VERSION_2c;
	strcpy(pSNMP->xInfo.pPeerName, pInfo->pURL);
	strcpy(pSNMP->xInfo.pCommunity, "public");
	list_init(&pSNMP->xOIDList);

	pNode->pData = pSNMP;
	pNode->nUpdateInterval = 10;
	list_append(&xNodeList, pNode);

	if (ppNode != NULL)
	{
		*ppNode = pNode;	
	}

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

	if (pNode->pData != NULL)
	{
		free(pNode->pData);
		pNode->pData = NULL;
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
	FTM_INT	nRet;

	if (pNode == NULL) 
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	nRet =  pthread_create(&pNode->xPThread, NULL, &FTNM_threadMain, (FTM_VOID_PTR)pNode);
	if (nRet != 0)
	{
		TRACE("pthread_create failed [nRet = %d]\n", nRet);
		pNode->xPThread = 0;
		return	FTM_RET_CANT_CREATE_THREAD;
	}

	TRACE("pthread_create success [xPThread = %08lx]\n", pNode->xPThread);

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

static FTM_VOID_PTR	FTNM_threadMain(FTM_VOID_PTR pParams)
{
	FTNM_NODE_PTR	pNode = (FTNM_NODE_PTR)pParams;
	time_t			xBaseTime, xCurrentTime;

	MESSAGE("%s[%d] : pNode = %08lx, pDID = %s\n", __func__, __LINE__, pNode, pNode->xInfo.pDID);
	xBaseTime = time(NULL);
	while(1)
	{
		MESSAGE("%s[%d] : pDID = %s,  xType = %d\n", __func__, __LINE__, pNode->xInfo.pDID, pNode->xType);
		switch(pNode->xType)
		{
		case	FTNM_NODE_TYPE_SNMP:
			{
					MESSAGE("%s[%d] : pDID = %s\n", __func__, __LINE__, pNode->xInfo.pDID);
				if (FTNM_snmpIsRunning(pNode) != FTM_BOOL_TRUE)
				{
					MESSAGE("%s[%d] : pDID = %s\n", __func__, __LINE__, pNode->xInfo.pDID);

					FTNM_snmpClientAsyncCall(pNode);
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

