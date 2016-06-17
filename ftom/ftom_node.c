#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "ftm.h"
#include "ftom.h"
#include "ftom_node.h"
#include "ftom_ep.h"
#include "ftm_list.h"
#include "ftom_node_class.h"
#include "ftom_message_queue.h"

static
FTM_INT	FTOM_NODE_seeker
(
	const FTM_VOID_PTR pElement, 
	const FTM_VOID_PTR pIndicator
);

static
FTM_INT	FTOM_NODE_comparator
(
	const FTM_VOID_PTR pElement1, 
	const FTM_VOID_PTR pElement2
);

static 
FTM_RET	FTOM_NODE_lock
(
	FTOM_NODE_PTR pNode
);

static 
FTM_RET	FTOM_NODE_unlock
(
	FTOM_NODE_PTR pNode
);

static 
FTM_VOID_PTR FTOM_NODE_process
(
	FTM_VOID_PTR pData
);

static
FTM_LIST_PTR	pNodeList = NULL;

/********************************************************************************
 *	NODE Manager
 ********************************************************************************/
FTM_RET	FTOM_NODE_init
(
	FTM_VOID
)
{
	FTM_RET	xRet ;

	if (pNodeList != NULL)
	{
		return	FTM_RET_ALREADY_INITIALIZED;	
	}

	xRet = FTM_LIST_create(&pNodeList);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTM_LIST_setSeeker(pNodeList, FTOM_NODE_seeker);
	FTM_LIST_setComparator(pNodeList, FTOM_NODE_comparator);

	return	FTM_RET_OK;
}

FTM_RET FTOM_NODE_final
(
	FTM_VOID
)
{
	FTOM_NODE_PTR	pNode = NULL;
	
	if (pNodeList == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}
	FTM_LIST_iteratorStart(pNodeList);
	while(FTM_LIST_iteratorNext(pNodeList, (FTM_VOID_PTR _PTR_)&pNode) == FTM_RET_OK)
	{
		FTM_LIST_remove(pNodeList, pNode);
		FTOM_NODE_destroy(&pNode);	
	}

	FTM_LIST_destroy(pNodeList);

	pNodeList = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_create
(
	FTM_NODE_PTR 	pInfo, 
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppNode != NULL);

	FTM_RET			xRet;
	FTOM_NODE_PTR	pNode;
	FTOM_NODE_CLASS_PTR	pClass = NULL;

	if (strlen(pInfo->pDID) == 0)
	{
		FTM_INT	i;

		for(i = 0 ; i < 10 ; i++)
		{
			FTM_makeID(pInfo->pDID, FTM_ID_LEN);

			xRet = FTOM_NODE_get(pInfo->pDID, &pNode);
			if (xRet != FTM_RET_OK)
			{
				break;
			}
		}

		if (i == 10)
		{
			return	FTM_RET_ALREADY_EXIST_OBJECT;	
		}
	}
	else
	{
		xRet = FTOM_NODE_get(pInfo->pDID, &pNode);
		if (xRet == FTM_RET_OK)
		{
			return	FTM_RET_ALREADY_EXIST_OBJECT;	
		}
	}

	xRet = FTOM_NODE_CLASS_get(pInfo->pModel, pInfo->xType, &pClass);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node[%s] not found!\n", pInfo->pModel);
		return	xRet;	
	}

	ASSERT(pClass->fCreate != NULL);

	xRet = pClass->fCreate(pInfo, &pNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node[%s] creation failed!\n", pInfo->pModel);
		return	xRet;	
	}

	xRet = FTOM_DB_NODE_add(pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node[%s] failed to create to DB[%08x].\n", pInfo->pDID, xRet);
		FTM_MEM_free(pNode);
		return	xRet;	
	}

	xRet = FTM_LIST_init(&pNode->xEPList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("List initialize failed[%08x].\n", xRet);
		FTM_MEM_free(pNode);

		return	xRet;	
	}
	
	xRet = FTOM_MSGQ_init(&pNode->xMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message queue initialize failed[%08x].\n", xRet);
		FTM_MEM_free(pNode);
		return	xRet;	
	}

	pthread_mutex_init(&pNode->xMutexLock, NULL);
	
	pNode->bStop = FTM_TRUE;
	pNode->xState = FTOM_NODE_STATE_INITIALIZED;

	if (pNode->pClass->fInit != NULL)
	{
		pNode->pClass->fInit(pNode);
	}

	pNode->xState = FTOM_NODE_STATE_CREATED;

	FTM_LIST_append(pNodeList, pNode);

	*ppNode = pNode;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_createFromDB
(
	FTM_CHAR_PTR	pDID,
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pDID != NULL);
	ASSERT(ppNode != NULL);

	FTM_RET			xRet;
	FTM_NODE		xInfo;
	FTOM_NODE_PTR	pNode;
	FTOM_NODE_CLASS_PTR	pClass = NULL;

	xRet = FTM_LIST_get(pNodeList, pDID, (FTM_VOID_PTR _PTR_)&pNode);
	if (xRet == FTM_RET_OK)
	{
		return	FTM_RET_ALREADY_EXIST_OBJECT;	
	}

	xRet = FTOM_DB_NODE_getInfo(pDID, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node[%s] get info failed!\n", pDID);
		return	xRet;	
	}

	xRet = FTOM_NODE_CLASS_get(xInfo.pModel, xInfo.xType, &pClass);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node[%s] not found!\n", xInfo.pModel);
		return	xRet;	
	}

	ASSERT(pClass->fCreate != NULL);

	xRet = pClass->fCreate(&xInfo, &pNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Class[%s] creation failed!\n", xInfo.pModel);
		return	xRet;	
	}

	xRet = FTM_LIST_init(&pNode->xEPList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("List initialize failed[%08x].\n", xRet);
		FTM_MEM_free(pNode);

		return	xRet;	
	}
	
	xRet = FTOM_MSGQ_init(&pNode->xMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message queue initialize failed[%08x].\n", xRet);
		FTM_MEM_free(pNode);
		return	xRet;	
	}

	pthread_mutex_init(&pNode->xMutexLock, NULL);
	
	pNode->bStop = FTM_TRUE;
	pNode->xState = FTOM_NODE_STATE_INITIALIZED;

	if (pNode->pClass->fInit != NULL)
	{
		pNode->pClass->fInit(pNode);
	}

	pNode->xState = FTOM_NODE_STATE_CREATED;

	FTM_LIST_append(pNodeList, pNode);

	*ppNode = pNode;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_destroy
(
	FTOM_NODE_PTR _PTR_	ppNode
)
{
	ASSERT(ppNode != NULL);

	FTM_RET			xRet;
	FTOM_EP_PTR		pEP;

	if ((*ppNode)->pClass->fFinal!= NULL)
	{
		(*ppNode)->pClass->fFinal(*ppNode);
	}

	xRet = FTOM_DB_NODE_remove((*ppNode)->xInfo.pDID);
	if (xRet != FTM_RET_OK)
	{
		INFO("Node[%s] failed to remove from DB[%08x]\n", (*ppNode)->xInfo.pDID, xRet);	
	}

	FTOM_NODE_lock((*ppNode));

	FTM_LIST_iteratorStart(&(*ppNode)->xEPList);
	while(FTM_LIST_iteratorNext(&(*ppNode)->xEPList, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
	{
		FTOM_EP_detach(pEP);
	}
	FTM_LIST_final(&(*ppNode)->xEPList);

	FTOM_NODE_unlock((*ppNode));

	pthread_mutex_destroy(&(*ppNode)->xMutexLock);

	FTM_LIST_remove(pNodeList, (*ppNode));

	if ((*ppNode)->pClass->fDestroy != NULL)
	{
		(*ppNode)->pClass->fDestroy(ppNode);
	}

	*ppNode = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_count
(
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pulCount != NULL);

	return	FTM_LIST_count(pNodeList, pulCount);
}


FTM_RET FTOM_NODE_get
(
	FTM_CHAR_PTR pDID, 
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pDID != NULL);
	ASSERT(ppNode != NULL);

	FTM_RET			xRet;
	FTOM_NODE_PTR	pNode;
	
	xRet = FTM_LIST_get(pNodeList, (FTM_VOID_PTR)pDID, (FTM_VOID_PTR _PTR_)&pNode);
	if (xRet == FTM_RET_OK)
	{
		*ppNode = pNode;
	}

	return	xRet;
}

FTM_RET FTOM_NODE_getAt
(
	FTM_ULONG ulIndex, 
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(ppNode != NULL);

	FTM_RET			xRet;
	FTOM_NODE_PTR	pNode;

	xRet = FTM_LIST_getAt(pNodeList, ulIndex, (FTM_VOID_PTR _PTR_)&pNode);
	if (xRet == FTM_RET_OK)
	{
		*ppNode = pNode;
	}

	return	xRet;
}

FTM_RET FTOM_NODE_set
(
	FTM_CHAR_PTR 	pDID, 
	FTM_NODE_FIELD	xFields,
	FTM_NODE_PTR 	pInfo
)
{
	ASSERT(pDID != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET			xRet;
	FTOM_NODE_PTR	pNode;
	
	xRet = FTM_LIST_get(pNodeList, (FTM_VOID_PTR)pDID, (FTM_VOID_PTR _PTR_)&pNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node[%s] not found.[%08x]\n", pDID, xRet);
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	xRet = FTOM_DB_NODE_set(pDID, xFields, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node[%s] failed to set[%08x]\n", pDID, xRet);
		return	xRet;	
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
		pNode->xInfo.ulReportInterval = pInfo->ulReportInterval;
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

FTM_RET	FTOM_NODE_linkEP
(
	FTOM_NODE_PTR pNode, 
	FTOM_EP_PTR pEP
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);

	FTOM_NODE_lock(pNode);

	FTM_LIST_append(&pNode->xEPList, pEP);
	FTOM_EP_attach(pEP, pNode);

	FTOM_NODE_unlock(pNode);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_unlinkEP
(
	FTOM_NODE_PTR pNode, 
	FTOM_EP_PTR pEP
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);

	FTOM_NODE_lock(pNode);

	FTOM_EP_detach(pEP);
	FTM_LIST_remove(&pNode->xEPList, pEP);
	
	FTOM_NODE_unlock(pNode);

	return	FTM_RET_OK;
}

FTM_RET FTOM_NODE_getEPData
(
	FTOM_NODE_PTR pNode, 
	FTOM_EP_PTR pEP, 
	FTM_EP_DATA_PTR pData
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);

	if (pNode->pClass->fGetEPData == NULL)
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pNode->pClass->fGetEPData(pNode, pEP, pData);
}

FTM_RET	FTOM_NODE_setEPData
(
	FTOM_NODE_PTR pNode, 
	FTOM_EP_PTR pEP, 
	FTM_EP_DATA_PTR pData
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);

	if (pNode->pClass->fSetEPData == NULL)
	{
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pNode->pClass->fSetEPData(pNode, pEP, pData);
}

FTM_RET	FTOM_NODE_getEPCount
(
	FTOM_NODE_PTR pNode, 
	FTM_ULONG_PTR pulCount
)
{
	ASSERT(pNode != NULL);
	ASSERT(pulCount != NULL);

	return FTM_LIST_count(&pNode->xEPList, pulCount);
}

FTM_RET	FTOM_NODE_getEP
(
	FTOM_NODE_PTR pNode, 
	FTM_CHAR_PTR	pEPID,
	FTOM_EP_PTR _PTR_ ppEP
)
{
	ASSERT(pNode != NULL);
	ASSERT(ppEP != NULL);

	return	FTM_LIST_get(&pNode->xEPList, pEPID, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET	FTOM_NODE_getEPAt
(
	FTOM_NODE_PTR pNode, 
	FTM_ULONG ulIndex, 
	FTOM_EP_PTR _PTR_ ppEP
)
{
	ASSERT(pNode != NULL);
	ASSERT(ppEP != NULL);

	return	FTM_LIST_getAt(&pNode->xEPList, ulIndex, (FTM_VOID_PTR _PTR_)ppEP);
}

FTM_RET	FTOM_NODE_start
(
	FTOM_NODE_PTR pNode
)
{
	ASSERT(pNode != NULL);

	FTM_INT	nRet;
	FTM_RET	xRet;

	if (!pNode->bStop)
	{
		ERROR("Node is already started.\n");
		return	FTM_RET_ALREADY_STARTED;
	}

	if (pNode->pClass->fStart != NULL)
	{
		return	pNode->pClass->fStart(pNode);
	}
	else
	{
		FTM_ULONG	ulCount = 0;

		if (pNode->pClass->fPrestart != NULL)
		{
			pNode->pClass->fPrestart(pNode);
		}



		if (pNode->pClass->fProcess != NULL)
		{
			nRet = pthread_create(&pNode->xThread, NULL, pNode->pClass->fProcess, pNode);
		}
		else
		{
			TRACE("Run default process\n");
			nRet = pthread_create(&pNode->xThread, NULL, FTOM_NODE_process, pNode);
		}

		if (nRet != 0)
		{
			ERROR("Node start failed.[%d]\n", nRet);
			return	FTM_RET_THREAD_CREATION_ERROR;
		}

		xRet = FTM_LIST_count(&pNode->xEPList, &ulCount);
		if (xRet == FTM_RET_OK)
		{
			FTM_INT	i;
	
			for(i = 0 ; i < ulCount ; i++)
			{
				FTOM_EP_PTR	pEP;
	
				xRet = FTM_LIST_getAt(&pNode->xEPList, i, (FTM_VOID_PTR _PTR_)&pEP);
				if (xRet == FTM_RET_OK)
				{
					FTOM_EP_start(pEP);
				}
			}
		}

		if (pNode->pClass->fPoststart != NULL)
		{
			pNode->pClass->fPoststart(pNode);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET FTOM_NODE_stop
(
	FTOM_NODE_PTR pNode
)
{
	ASSERT(pNode != NULL);
	FTM_RET	xRet;
	FTOM_MSG_PTR	pMsg = NULL;

	if (pNode->bStop)
	{
		ERROR("Node[%s] not started.\n", pNode->xInfo.pDID);
		return	FTM_RET_NOT_START;
	}

	if (pNode->pClass->fStop != NULL)
	{
		return	pNode->pClass->fStop(pNode);
	}
	else
	{
		FTM_ULONG	ulCount;
		if (pNode->pClass->fPrestop != NULL)

		{
			pNode->pClass->fPrestop(pNode);
		}
	
		xRet = FTM_LIST_count(&pNode->xEPList, &ulCount);
		if (xRet == FTM_RET_OK)
		{
			FTM_INT	i;

			for(i = 0 ; i < ulCount ; i++)
			{
				FTOM_EP_PTR	pEP;

				xRet = FTM_LIST_getAt(&pNode->xEPList, i, (FTM_VOID_PTR _PTR_)&pEP);
				if (xRet == FTM_RET_OK)
				{
					FTOM_EP_stop(pEP, FTM_TRUE);
				}
			}
		}

		xRet = FTOM_MSG_createQuit(&pMsg);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTOM_MSGQ_push(&pNode->xMsgQ, pMsg);
			if (xRet != FTM_RET_OK)
			{
				FTOM_MSG_destroy(&pMsg);	
			}
		}
		else
		{
			ERROR("NODE[%s] : Can't create quit message!\n", pNode->xInfo.pDID);
			pthread_cancel(pNode->xThread);
		}
	
		
		pthread_join(pNode->xThread, NULL);
		pNode->xThread = 0;
	
		if (pNode->pClass->fPoststop != NULL)
		{
			pNode->pClass->fPoststop(pNode);
		}
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_NODE_process
(
	FTM_VOID_PTR pData
)
{
	FTOM_NODE_PTR pNode = (FTOM_NODE_PTR)pData;
	FTOM_MSG_PTR	pMsg;
	FTM_TIMER		xLoopTimer;

	TRACE("Node[%s] started.\n", pNode->xInfo.pDID);

	FTM_TIMER_initS(&xLoopTimer, 0);
	FTM_TIMER_initS(&pNode->xReportTimer, 0);

	pNode->xState= FTOM_NODE_STATE_RUN;
	pNode->bStop = FTM_FALSE;

	while(!pNode->bStop)
	{
		FTM_ULONG		ulRemainTime = 0;

		if (FTM_TIMER_isExpired(&pNode->xReportTimer))
		{
			FTM_TIMER_addS(&pNode->xReportTimer, pNode->xInfo.ulReportInterval);
		}

		FTM_TIMER_remainMS(&xLoopTimer, &ulRemainTime);
		while (!pNode->bStop && (FTOM_MSGQ_timedPop(&pNode->xMsgQ, ulRemainTime, &pMsg) == FTM_RET_OK))
		{
			TRACE("Message received[%08x]\n", pMsg->xType);
			switch(pMsg->xType)
			{
			case	FTOM_MSG_TYPE_QUIT:
				{	
					pNode->bStop = FTM_TRUE;
				}
				break;

			default:
				{
					WARN("Invalid message[%08x]\n", pMsg->xType);	
				}
			}

			FTOM_MSG_destroy(&pMsg);

			FTM_TIMER_remainMS(&xLoopTimer, &ulRemainTime);
		}
	
		FTM_TIMER_addS(&xLoopTimer, FTOM_NODE_LOOP_INTERVAL);
	} 

	pNode->xState= FTOM_NODE_STATE_STOP;
	TRACE("Node[%s] stopped.\n", pNode->xInfo.pDID);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_lock
(
	FTOM_NODE_PTR pNode
)
{
	ASSERT(pNode != NULL);

	pthread_mutex_lock(&pNode->xMutexLock);

	return	FTM_RET_OK;
}


FTM_RET	FTOM_NODE_unlock
(
	FTOM_NODE_PTR pNode
)
{
	ASSERT(pNode != NULL);

	pthread_mutex_unlock(&pNode->xMutexLock);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_setReportInterval
(
	FTOM_NODE_PTR	pNode,
	FTM_ULONG		ulInterval
)
{
	ASSERT(pNode != NULL);

	FTM_RET	xRet;

	xRet = FTM_isValidInterval(ulInterval);
	if (xRet == FTM_RET_OK)
	{
		pNode->xInfo.ulReportInterval = ulInterval;
	}

	return	xRet;
}

FTM_RET	FTOM_NODE_getReportInterval
(
	FTOM_NODE_PTR	pNode,
	FTM_ULONG_PTR	pulInterval
)
{
	ASSERT(pNode != NULL);
	ASSERT(pulInterval != NULL);
		
	*pulInterval = pNode->xInfo.ulReportInterval;

	return	FTM_RET_OK;
}

FTM_CHAR_PTR	FTOM_NODE_stateToStr
(
	FTOM_NODE_STATE xState
)
{
	switch(xState)
	{
	case	FTOM_NODE_STATE_CREATED:		return	"CREATED";
	case	FTOM_NODE_STATE_INITIALIZED: 	return	"INITIALIZED";
	case	FTOM_NODE_STATE_RUN:			return	"RUN";
	case	FTOM_NODE_STATE_FINALIZED:		return	"FINALIZED";
	}

	return	"UNKNOWN";
}

FTM_INT	FTOM_NODE_seeker
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

FTM_INT	FTOM_NODE_comparator
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

FTM_RET	FTOM_NODE_print
(
	FTOM_NODE_PTR	pNode
)
{
	ASSERT(pNode != NULL);
	FTM_INT		j;
	FTOM_EP_PTR	pEP;
	FTM_ULONG	ulEPCount;

	MESSAGE("\n# Node Information\n");
	MESSAGE("%16s : %s\n", "DID", 		pNode->xInfo.pDID);
	MESSAGE("%16s : %s\n", "Model",		pNode->xInfo.pModel);
	MESSAGE("%16s : %s\n", "Name",		pNode->xInfo.pName);
	MESSAGE("%16s : %s\n", "Type", 		FTM_NODE_typeString(pNode->xInfo.xType));
	switch(pNode->xInfo.xType)
	{
	case	FTM_NODE_TYPE_SNMP:	
		{
			MESSAGE("%16s   %10s - %s\n", "", "Version", 	FTM_SNMP_versionString(pNode->xInfo.xOption.xSNMP.ulVersion));	
			MESSAGE("%16s   %10s - %s\n", "", "URL", 		pNode->xInfo.xOption.xSNMP.pURL);
			MESSAGE("%16s   %10s - %s\n", "", "Community", 	pNode->xInfo.xOption.xSNMP.pCommunity);
			MESSAGE("%16s   %10s - %s\n", "", "MIB", 		pNode->xInfo.xOption.xSNMP.pMIB);
			MESSAGE("%16s   %10s - %lu\n","", "Retry", 		pNode->xInfo.xOption.xSNMP.ulMaxRetryCount);
		}
		break;
#if 0
	case	FTM_NODE_TYPE_MODBUS_OVER_TCP:
		{
			MESSAGE("%16s   %10s - %d\n", "", "Version", 	pNode->xInfo.xOption.xMB.ulVersion);	
			MESSAGE("%16s   %10s - %s\n", "", "URL", 		pNode->xInfo.xOption.xMB.pURL);
			MESSAGE("%16s   %10s - %lu\n","", "Port", 		pNode->xInfo.xOption.xMB.ulPort);
			MESSAGE("%16s   %10s - %lu\n","", "SlaveID", 	pNode->xInfo.xOption.xMB.ulSlaveID);
		}
		break;
#endif
	case	FTM_NODE_TYPE_FINS:
		{
			MESSAGE("%16s   %10s - %lu\n", "", "Version", 	pNode->xInfo.xOption.xFINS.ulVersion);	
			MESSAGE("%16s   %10s - %s\n", "", "DestIP", 	pNode->xInfo.xOption.xFINS.pDIP);
			MESSAGE("%16s   %10s - %lu\n","", "DestPort",	pNode->xInfo.xOption.xFINS.ulDP);
			MESSAGE("%16s   %10s - %lu\n","", "SrcPort", 	pNode->xInfo.xOption.xFINS.ulSP);
			MESSAGE("%16s   %10s - %02x:%02x:%02x\n","", "DestAddr",	
					(FTM_UINT8)((pNode->xInfo.xOption.xFINS.ulDA >> 16) & 0xFF),
					(FTM_UINT8)((pNode->xInfo.xOption.xFINS.ulDA >>  8) & 0xFF),
					(FTM_UINT8)((pNode->xInfo.xOption.xFINS.ulDA >>  0) & 0xFF));
			MESSAGE("%16s   %10s - %02x:%02x:%02x\n","", "SrcAddr", 
					(FTM_UINT8)((pNode->xInfo.xOption.xFINS.ulSA >> 16) & 0xFF),
					(FTM_UINT8)((pNode->xInfo.xOption.xFINS.ulSA >>  8) & 0xFF),
					(FTM_UINT8)((pNode->xInfo.xOption.xFINS.ulSA >>  0) & 0xFF));
			MESSAGE("%16s   %10s - %lu\n","", "ServerID", 	pNode->xInfo.xOption.xFINS.ulServerID);
		}
		break;
	}

	MESSAGE("%16s : %s\n", "State", 	FTOM_NODE_stateToStr(pNode->xState));
	MESSAGE("%16s : %lu\n", "Report Interval", 	pNode->xInfo.ulReportInterval);
	MESSAGE("%16s : %lu\n", "Timeout", 	pNode->xInfo.ulTimeout);
	FTOM_NODE_getEPCount(pNode, &ulEPCount);
	MESSAGE("%16s : %lu\n", "EPs",		ulEPCount);
	for(j = 0; j < ulEPCount ; j++)
	{
		if (FTOM_NODE_getEPAt(pNode, j, &pEP) == FTM_RET_OK)
		{
			MESSAGE("%16s   %d - %16s\n", "", j+1, pEP->xInfo.pEPID);
		}
	}
	MESSAGE("%16s : %lu\n", "Tx Count", 		pNode->xStatistics.ulTxCount);
	MESSAGE("%16s : %lu\n", "Rx Count", 		pNode->xStatistics.ulRxCount);
	MESSAGE("%16s : %lu\n", "Tx Error", 		pNode->xStatistics.ulTxError);
	MESSAGE("%16s : %lu\n", "Rx Error", 		pNode->xStatistics.ulRxError);
	MESSAGE("%16s : %lu\n", "Invalid Frame", pNode->xStatistics.ulInvalidFrame);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_printList
(
	FTM_VOID
)
{
	FTM_INT			i;
	FTM_ULONG		ulCount;
	FTOM_NODE_PTR	pNode;

	MESSAGE("\n# Node Information\n");
	MESSAGE("%16s %16s %16s %16s %16s %8s %8s %s\n", "DID", "MODEL", "NAME", "TYPE", "STATE", "REPORT", "TIMEOUT", "EPs");

	FTM_LIST_count(pNodeList, &ulCount);

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_ULONG	ulEPCount;

		FTOM_NODE_getAt(i, &pNode);
		MESSAGE("%16s ", pNode->xInfo.pDID);
		MESSAGE("%16s ", pNode->xInfo.pModel);
		MESSAGE("%16s ", pNode->xInfo.pName);
		MESSAGE("%16s ", FTM_NODE_typeString(pNode->xInfo.xType));
		MESSAGE("%16s ", FTOM_NODE_stateToStr(pNode->xState));
		MESSAGE("%8lu ", pNode->xInfo.ulReportInterval);
		MESSAGE("%8lu ", pNode->xInfo.ulTimeout);

		FTOM_NODE_getEPCount(pNode, &ulEPCount);
		MESSAGE("%3lu\n", ulCount);
	}

	return	FTM_RET_OK;
}

