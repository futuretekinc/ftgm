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
#include "ftom_logger.h"

#undef	__MODULE__
#define	__MODULE__	FTOM_TRACE_MODULE_NODE

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
FTM_VOID_PTR FTOM_NODE_threadMain
(
	FTM_VOID_PTR pData
);

static
FTM_RET	FTOM_NODE_message
(
	FTOM_NODE_PTR	pNode,
	FTOM_MSG_PTR	pMsg
);

static
FTM_RET	FTOM_NODE_message
(
	FTOM_NODE_PTR	pNode,
	FTOM_MSG_PTR	pBaseMsg
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
		FTOM_NODE_destroy(&pNode, FTM_FALSE);	
	}

	FTM_LIST_destroy(pNodeList);

	pNodeList = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_create
(
	FTM_NODE_PTR 	pInfo, 
	FTM_BOOL		bNew,
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
			ERROR2(FTM_RET_ALREADY_EXIST_OBJECT, "Failed to create node[%s]!\n", pInfo->pDID);
			return	FTM_RET_ALREADY_EXIST_OBJECT;	
		}
	}
	else
	{
		xRet = FTOM_NODE_get(pInfo->pDID, &pNode);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTM_RET_ALREADY_EXIST_OBJECT;	
			ERROR2(xRet, "Failed to create node.\n");
			return	xRet;
		}
	}

	xRet = FTOM_NODE_CLASS_get(pInfo->pModel, pInfo->xType, &pClass);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get class information for node[%s]!\n", pInfo->pModel);
		return	xRet;	
	}

	ASSERT(pClass->fCreate != NULL);
	ASSERT(pClass->fDestroy != NULL);

	xRet = pClass->fCreate(pInfo, &pNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create Node[%s]!\n", pInfo->pModel);
		return	xRet;	
	}

	xRet = FTM_LIST_init(&pNode->xEPList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to initialize list.\n");
		goto error1;
	}
	
	xRet = FTOM_MSGQ_init(&pNode->xMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to initialize message queue.\n");
		goto error2;
	}

	xRet= FTM_LOCK_init(&pNode->xLock);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to initialize lock!\n");
		goto error3;
	}

	if (bNew)
	{
		xRet = FTOM_DB_NODE_add(&pNode->xInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to add node[%s] to DB.\n", pNode->xInfo.pDID);
		}

		FTOM_LOG_createNode(&pNode->xInfo);
	}

	pNode->bStop = FTM_TRUE;
	pNode->xState = FTOM_NODE_STATE_INITIALIZED;

	if ((pNode->pClass != NULL) & (pNode->pClass->fInit != NULL))
	{
		pNode->pClass->fInit(pNode);
	}

	pNode->xState = FTOM_NODE_STATE_CREATED;

	FTM_LIST_append(pNodeList, pNode);
	TRACE("Append Node : %08x, %08x\n", pNodeList, pNode);

	*ppNode = pNode;

	return	FTM_RET_OK;


error3:
	FTOM_MSGQ_final(&pNode->xMsgQ);

error2:
	FTM_LIST_final(&pNode->xEPList);

error1:
	pClass->fDestroy(&pNode);

	return	xRet;
}

FTM_RET	FTOM_NODE_destroy
(
	FTOM_NODE_PTR _PTR_	ppNode,
	FTM_BOOL			bStorage
)
{
	ASSERT(ppNode != NULL);

	FTM_RET			xRet;
	FTOM_EP_PTR		pEP;
	FTM_ULONG		ulCount;
	
	FTOM_NODE_stop((*ppNode));

	if ((*ppNode)->pClass->fFinal!= NULL)
	{
		(*ppNode)->pClass->fFinal(*ppNode);
	}

	if (bStorage)
	{
		xRet = FTOM_DB_NODE_remove((*ppNode)->xInfo.pDID);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to remove Node[%s] from DB.\n", (*ppNode)->xInfo.pDID);
		}
		FTOM_LOG_destroyNode(&(*ppNode)->xInfo);
	}

	while((FTM_LIST_count(&(*ppNode)->xEPList, &ulCount) == FTM_RET_OK) && (ulCount != 0))
	{
		xRet = FTM_LIST_getFirst(&(*ppNode)->xEPList, (FTM_VOID_PTR _PTR_)&pEP);
		if (xRet == FTM_RET_OK)
		{
			FTOM_NODE_unlinkEP((*ppNode), pEP);
			FTOM_EP_destroy(&pEP, bStorage);
		}
	}

	xRet = FTM_LIST_final(&(*ppNode)->xEPList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to finalize EP list of node[%s]!\n", (*ppNode)->xInfo.pDID);	
	}

	xRet = FTM_LOCK_final(&(*ppNode)->xLock);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to finalize lock of node[%s]!\n", (*ppNode)->xInfo.pDID);	
	}

	FTM_LIST_remove(pNodeList, (*ppNode));

	if ((*ppNode)->pClass->fDestroy != NULL)
	{
		xRet = (*ppNode)->pClass->fDestroy(ppNode);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to destroy node[%s]!\n", (*ppNode)->xInfo.pDID);	
		}
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

FTM_RET	FTOM_NODE_getAttr
(
	FTOM_NODE_PTR	pNode,
	FTM_NODE_PTR	pInfo
)
{
	ASSERT(pNode != NULL);
	ASSERT(pInfo != NULL);

	memcpy(pInfo, &pNode->xInfo, sizeof(FTM_NODE));

	return	FTM_RET_OK;
}

FTM_RET FTOM_NODE_setAttr
(
	FTOM_NODE_PTR	pNode, 
	FTM_NODE_FIELD	xFields,
	FTM_NODE_PTR 	pInfo
)
{
	ASSERT(pNode != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET			xRet;
	
	if (pNode->pClass == NULL)
	{
		xRet = FTM_RET_NO_CLASS_INFO;
		ERROR2(xRet, "No class information for node[%s]\n", pNode->xInfo.pDID);
		goto error;
	}

	if (pNode->pClass->fSet == NULL)
	{
		xRet = FTM_RET_FUNCTION_NOT_SUPPORTED;
		ERROR2(xRet, "Failed to set node[%s] information!\n", pNode->xInfo.pDID);
		goto error;
	}

	xRet = pNode->pClass->fSet(pNode, xFields, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set node[%s] information!\n", pNode->xInfo.pDID);
		goto error;	
	}

	xRet = FTOM_DB_NODE_setInfo(pNode->xInfo.pDID, xFields, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set node[%s].\n", pNode->xInfo.pDID);
	}

	return	FTM_RET_OK;

error:
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

	FTM_RET	xRet;

	if (pNode->pClass == NULL)
	{
		ERROR2(FTM_RET_NO_CLASS_INFO, "No class information for node[%s]\n", pNode->xInfo.pDID);
		return	FTM_RET_NO_CLASS_INFO;
	}

	if (pNode->pClass->fAttachEP == NULL)
	{
		ERROR2(FTM_RET_FUNCTION_NOT_SUPPORTED, "Failed to set node[%s] information!\n", pNode->xInfo.pDID);
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;
	}

	xRet = pNode->pClass->fAttachEP(pNode, pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to attach EP[%s] to Node[%s].\n", pEP->xInfo.pEPID, pNode->xInfo.pDID);
		return	xRet;	
	}

	FTM_LOCK_set(&pNode->xLock);

	FTM_LIST_append(&pNode->xEPList, pEP);

	FTM_LOCK_reset(&pNode->xLock);

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

	FTM_RET	xRet;

	FTOM_EP_stop(pEP, FTM_TRUE);

	if (pNode->pClass == NULL)
	{
		ERROR2(FTM_RET_NO_CLASS_INFO, "No class information for node[%s]\n", pNode->xInfo.pDID);
		return	FTM_RET_NO_CLASS_INFO;
	}

	if (pNode->pClass->fDetachEP == NULL)
	{
		ERROR2(FTM_RET_FUNCTION_NOT_SUPPORTED, "Failed to set node[%s] information!\n", pNode->xInfo.pDID);
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;
	}

	xRet = pNode->pClass->fDetachEP(pNode, pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to detach EP[%s] to Node[%s].\n", pEP->xInfo.pEPID, pNode->xInfo.pDID);
		return	xRet;	
	}

	FTM_LOCK_set(&pNode->xLock);

	FTM_LIST_remove(&pNode->xEPList, pEP);
	
	FTM_LOCK_reset(&pNode->xLock);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_getEPID
(
	FTOM_NODE_PTR	pNode,
	FTM_EP_TYPE		xEPType,
	FTM_ULONG		ulIndex,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulMaxLen
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEPID != NULL);

	FTM_RET	xRet;

	if ((pNode->pClass == NULL) || (pNode->pClass->fGetEPID == NULL))
	{
		ERROR2(FTM_RET_FUNCTION_NOT_SUPPORTED, "Not supported function getEPID!\n");
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	xRet = pNode->pClass->fGetEPID(pNode, xEPType, ulIndex, pEPID, ulMaxLen);	
	
	return	xRet;
}

FTM_RET	FTOM_NODE_getEPName
(
	FTOM_NODE_PTR	pNode,
	FTM_EP_TYPE		xEPType,
	FTM_ULONG		ulIndex,
	FTM_CHAR_PTR	pName,
	FTM_ULONG		ulMaxLen
)
{
	ASSERT(pNode != NULL);
	ASSERT(pName != NULL);

	FTM_RET	xRet;

	if ((pNode->pClass == NULL) || (pNode->pClass->fGetEPName == NULL))
	{
		ERROR2(FTM_RET_FUNCTION_NOT_SUPPORTED, "Not supported function getEPName!\n");
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	xRet = pNode->pClass->fGetEPName(pNode, xEPType, ulIndex, pName, ulMaxLen);	
	
	return	xRet;
}

FTM_RET	FTOM_NODE_getEPState
(
	FTOM_NODE_PTR	pNode,
	FTM_EP_TYPE		xEPType,
	FTM_ULONG		ulIndex,
	FTM_BOOL_PTR	pbEnable
)
{
	ASSERT(pNode != NULL);
	ASSERT(pbEnable != NULL);

	FTM_RET	xRet;

	if ((pNode->pClass == NULL) || (pNode->pClass->fGetEPState == NULL))
	{
		ERROR2(FTM_RET_FUNCTION_NOT_SUPPORTED, "Not supported function getEPState!\n");
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	xRet = pNode->pClass->fGetEPState(pNode, xEPType, ulIndex, pbEnable);	
	
	return	xRet;
}

FTM_RET	FTOM_NODE_getEPUpdateInterval
(
	FTOM_NODE_PTR	pNode,
	FTM_EP_TYPE		xEPType,
	FTM_ULONG		ulIndex,
	FTM_ULONG_PTR	pulUpdateInterval
)
{
	ASSERT(pNode != NULL);
	ASSERT(pulUpdateInterval != NULL);

	FTM_RET	xRet;

	
	if ((pNode->pClass == NULL) || (pNode->pClass->fGetEPInterval == NULL))
	{
		ERROR2(FTM_RET_FUNCTION_NOT_SUPPORTED, "Not supported function getEPInterval!\n");
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	xRet = pNode->pClass->fGetEPInterval(pNode, xEPType, ulIndex, pulUpdateInterval);	

	
	return	xRet;
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

	FTM_RET	xRet;


	if ((pNode->pClass == NULL) || (pNode->pClass->fGetEPData == NULL))
	{
		ERROR2(FTM_RET_FUNCTION_NOT_SUPPORTED, "Not supported function getEPID!\n");
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	pNode->xStatistics.ulGetCount++;
	xRet = pNode->pClass->fGetEPData(pNode, pEP, pData);
	if (xRet != FTM_RET_OK)
	{
		pNode->xStatistics.ulGetError++;
	}


	return	xRet;
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

	FTM_RET	xRet = FTM_RET_FUNCTION_NOT_SUPPORTED;	

	if ((pNode->pClass == NULL) || (pNode->pClass->fSetEPData == NULL))
	{
		ERROR2(FTM_RET_FUNCTION_NOT_SUPPORTED, "Not supported function getEPID!\n");
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	pNode->xStatistics.ulGetCount++;
	xRet = pNode->pClass->fSetEPData(pNode, pEP, pData);
	if (xRet != FTM_RET_OK)
	{
		pNode->xStatistics.ulSetError++;
	}

	return	xRet;
}

FTM_RET	FTOM_NODE_getEPCount
(
	FTOM_NODE_PTR 	pNode, 
	FTM_EP_TYPE		xType,	
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pNode != NULL);
	ASSERT(pulCount != NULL);
	
	FTM_RET		xRet;
	FTM_ULONG	ulCount = 0;

	xRet = FTM_LIST_count(&pNode->xEPList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get EP count!\n");
		return	xRet;	
	}

	if (ulCount == 0)
	{
		if ((pNode->pClass != NULL) && (pNode->pClass->fGetEPCount != NULL))
		{
			xRet = pNode->pClass->fGetEPCount(pNode, xType, &ulCount);	
		}
	}

	if (xRet == FTM_RET_OK)
	{
		*pulCount = ulCount;	
	}

	return	xRet;
}

FTM_RET	FTOM_NODE_getEPDataAsync
(
	FTOM_NODE_PTR	pNode,
	FTOM_EP_PTR		pEP
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);

	if (pNode->pClass == NULL)
	{
		ERROR2(FTM_RET_NOT_SUPPORTED_NODE_CLASS, "Not supported node class!\n");
		return	FTM_RET_NOT_SUPPORTED_NODE_CLASS;
	}
	
	if (pNode->pClass->fGetEPDataAsync == NULL)
	{
		ERROR2(FTM_RET_FUNCTION_NOT_SUPPORTED, "Asynchronous data get function not supported!\n");
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pNode->pClass->fGetEPDataAsync(pNode, pEP);
}

FTM_RET	FTOM_NODE_setEPDataAsync
(
	FTOM_NODE_PTR	pNode,
	FTOM_EP_PTR		pEP,
	FTM_EP_DATA_PTR pData
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);

	if (pNode->pClass == NULL)
	{
		ERROR2(FTM_RET_NOT_SUPPORTED_NODE_CLASS, "Not supported node class!\n");
		return	FTM_RET_NOT_SUPPORTED_NODE_CLASS;
	}
	
	if (pNode->pClass->fSetEPDataAsync == NULL)
	{
		ERROR2(FTM_RET_FUNCTION_NOT_SUPPORTED, "Asynchronous data set function not supported!\n");
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;	
	}

	return	pNode->pClass->fSetEPDataAsync(pNode, pEP, pData);
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
		xRet = FTM_RET_ALREADY_STARTED;
		ERROR2(xRet, "Node is already started.\n");
		return	xRet;
	}

	if ((pNode->pClass != NULL) && (pNode->pClass->fPrestart != NULL))
	{
		pNode->pClass->fPrestart(pNode);
	}

	if ((pNode->pClass != NULL) && (pNode->pClass->fStart != NULL))
	{
		return	pNode->pClass->fStart(pNode);
	}
	else
	{
		FTM_ULONG	ulCount = 0;

		if ((pNode->pClass != NULL) && (pNode->pClass->fProcess != NULL))
		{
		
			xRet = pNode->pClass->fProcess(pNode);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to run process!\n");
				return	xRet;
			}
		}
		else
		{
			nRet = pthread_create(&pNode->xThread, NULL, FTOM_NODE_threadMain, pNode);
			if (nRet != 0)
			{
				xRet = FTM_RET_THREAD_CREATION_ERROR;
				ERROR2(xRet, "Node start failed.[%d]\n", nRet);
				return	xRet;
			}
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

		if ((pNode->pClass != NULL) && (pNode->pClass->fPoststart != NULL))
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
		return	FTM_RET_OK;
	}

	if ((pNode->pClass != NULL) && (pNode->pClass->fStop != NULL))
	{
		return	pNode->pClass->fStop(pNode);
	}
	else
	{
		FTM_ULONG	ulCount;
		if ((pNode->pClass != NULL) && (pNode->pClass->fPrestop != NULL))
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
			ERROR2(xRet, "NODE[%s] : Can't create quit message!\n", pNode->xInfo.pDID);
			pthread_cancel(pNode->xThread);
		}
	
		
		pthread_join(pNode->xThread, NULL);
		pNode->xThread = 0;
	
		if ((pNode->pClass != NULL) && (pNode->pClass->fPoststop != NULL))
		{
			pNode->pClass->fPoststop(pNode);
		}
	}

	TRACE("Node[%s] is stopped!\n", pNode->xInfo.pDID);
	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_isRun
(
	FTOM_NODE_PTR	pNode,
	FTM_BOOL_PTR	pbRun
)
{
	ASSERT(pNode != NULL);
	ASSERT(pbRun != NULL);

	*pbRun = !pNode->bStop;

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTOM_NODE_threadMain
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
			FTOM_NODE_message(pNode, pMsg);

			FTOM_MSG_destroy(&pMsg);

			FTM_TIMER_remainMS(&xLoopTimer, &ulRemainTime);
		}
	
		FTM_TIMER_addS(&xLoopTimer, FTOM_NODE_LOOP_INTERVAL);
	} 

	pNode->xState= FTOM_NODE_STATE_STOP;
	TRACE("Node[%s] stopped.\n", pNode->xInfo.pDID);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_message
(
	FTOM_NODE_PTR	pNode,
	FTOM_MSG_PTR	pBaseMsg
)
{
	ASSERT(pNode != NULL);
	ASSERT(pBaseMsg != NULL);
	FTM_RET	xRet;

	switch(pBaseMsg->xType)
	{
	case	FTOM_MSG_TYPE_QUIT:
		{	
			pNode->bStop = FTM_TRUE;
			
			xRet = FTM_RET_OK;
		}
		break;

	default:
		{
			xRet = FTM_RET_INVALID_MESSAGE_TYPE;

			ERROR2(xRet, "Invalid message type[%08x]\n", pBaseMsg->xType);
		}
	}

	return	xRet;
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

	if ((pNode->pClass != NULL) && (pNode->pClass->fPrintOpts != NULL))
	{
		pNode->pClass->fPrintOpts(pNode);	
	}

	MESSAGE("%16s : %s\n", "State", 	FTOM_NODE_stateToStr(pNode->xState));
	MESSAGE("%16s : %lu\n", "Report Interval", 	pNode->xInfo.ulReportInterval);
	MESSAGE("%16s : %lu\n", "Timeout", 	pNode->xInfo.ulTimeout);
	FTOM_NODE_getEPCount(pNode, 0, &ulEPCount);
	MESSAGE("%16s : %lu\n", "EPs",		ulEPCount);
	for(j = 0; j < ulEPCount ; j++)
	{
		if (FTOM_NODE_getEPAt(pNode, j, &pEP) == FTM_RET_OK)
		{
			MESSAGE("%16s   %d - %16s\n", "", j+1, pEP->xInfo.pEPID);
		}
	}
	MESSAGE("%16s : %s\n", "Register", 		pNode->xServer.bRegistered?"True":"False");	
	MESSAGE("%16s : %lu\n", "Get Count", 	pNode->xStatistics.ulGetCount);
	MESSAGE("%16s : %lu\n", "Set Count", 	pNode->xStatistics.ulSetCount);
	MESSAGE("%16s : %lu\n", "Tx Count", 	pNode->xStatistics.ulTxCount);
	MESSAGE("%16s : %lu\n", "Rx Count", 	pNode->xStatistics.ulRxCount);
	MESSAGE("%16s : %lu\n", "Get Error", 	pNode->xStatistics.ulGetError);
	MESSAGE("%16s : %lu\n", "Set Error", 	pNode->xStatistics.ulSetError);
	MESSAGE("%16s : %lu\n", "Tx Error", 	pNode->xStatistics.ulTxError);
	MESSAGE("%16s : %lu\n", "Rx Error", 	pNode->xStatistics.ulRxError);
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

		FTOM_NODE_getEPCount(pNode, 0, &ulEPCount);
		MESSAGE("%3lu\n", ulCount);
	}

	return	FTM_RET_OK;
}

