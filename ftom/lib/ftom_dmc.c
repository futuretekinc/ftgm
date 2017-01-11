#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "libconfig.h"
#include "ftom.h"
#include "ftdm_client.h"
#include "ftom_message_queue.h"
#include "ftom_server.h"
#include "ftom_dmc.h"

#undef	__MODULE__
#define	__MODULE__	FTOM_TRACE_MODULE_DMC

static FTM_VOID_PTR	FTOM_DMC_process
(
	FTM_VOID_PTR 	pData
);

static FTM_RET	FTOM_DMC_onEPData
(
	FTOM_DMC_PTR	pDMC,
	FTOM_MSG_EP_DATA_PTR	pMsg
);

FTM_RET	FTOM_DMC_create
(
	FTOM_DMC_PTR _PTR_ 	ppDMC
)
{
	ASSERT(ppDMC != NULL);

	FTM_RET	xRet;
	FTOM_DMC_PTR	pDMC;

	pDMC = (FTOM_DMC_PTR)FTM_MEM_malloc(sizeof(FTOM_DMC));
	if (pDMC == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_DMC_init(pDMC);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pDMC);
		return	xRet;	
	}

	*ppDMC = pDMC;

	return	FTM_RET_OK;
}
	
FTM_RET	FTOM_DMC_destroy
(
	FTOM_DMC_PTR _PTR_ 	ppDMC
)
{
	ASSERT(ppDMC != NULL);

	FTOM_DMC_final(*ppDMC );

	FTM_MEM_free(*ppDMC);

	*ppDMC = NULL;

	return	FTM_RET_OK;
}

FTM_RET FTOM_DMC_init
(
	FTOM_DMC_PTR 	pDMC
)
{
	ASSERT(pDMC != NULL);
	FTM_RET	xRet;

	memset(pDMC, 0, sizeof(FTOM_DMC));

	strcpy(pDMC->xConfig.xNetwork.pServerIP, FTDM_DEFAULT_SERVER_IP);
	pDMC->xConfig.xNetwork.usPort = FTDM_DEFAULT_SERVER_PORT;

	xRet = FTDM_CLIENT_create(&pDMC->pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create FTDM client!\n");
		return	xRet;
	}

	FTOM_MSGQ_init(&pDMC->xMsgQ);

	return	FTM_RET_OK;
}

FTM_RET FTOM_DMC_final
(
	FTOM_DMC_PTR pDMC
)
{
	ASSERT(pDMC != NULL);
	FTM_RET	xRet;

	if (pDMC->pClient != NULL)
	{
		xRet = FTDM_CLIENT_destroy(&pDMC->pClient);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to destroy FTDM client!\n");
		}
	}

	FTOM_MSGQ_final(&pDMC->xMsgQ);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_DMC_start
(
	FTOM_DMC_PTR pDMC
)
{
	ASSERT(pDMC != NULL);

	if (pDMC->bStop)
	{
		return	FTM_RET_ALREADY_STARTED;	
	}


	if (pthread_create(&pDMC->xThread, NULL, FTOM_DMC_process, pDMC) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_DMC_stop
(
	FTOM_DMC_PTR pDMC
)
{
	ASSERT(pDMC != NULL);

	if (pDMC->bStop)
	{
		return	FTM_RET_NOT_START;	
	}

	pDMC->bStop = FTM_TRUE;
	pthread_join(pDMC->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_DMC_isConnected
(
	FTOM_DMC_PTR 	pDMC,
	FTM_BOOL_PTR	pbConnected
)
{
	ASSERT(pDMC != NULL);

	*pbConnected = pDMC->bConnected;

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTOM_DMC_process
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTM_RET			xRet;
	FTOM_DMC_PTR	pDMC = (FTOM_DMC_PTR)pData;

	pDMC->bStop = FTM_FALSE;
		
	while(!pDMC->bStop)
	{
		FTM_BOOL	bConnected;

		FTDM_CLIENT_isConnected(pDMC->pClient, &bConnected);
		if (!bConnected)
		{
			xRet = FTDM_CLIENT_connect(pDMC->pClient, inet_addr(pDMC->xConfig.xNetwork.pServerIP), pDMC->xConfig.xNetwork.usPort);
			if (xRet != FTM_RET_OK)
			{
				pDMC->bConnected = FTM_FALSE;
				TRACE("DB connection failed.\n");	
				usleep(1000000);
			}
			else
			{
				if (pDMC->fServiceCB != NULL)
				{
					pDMC->fServiceCB(pDMC->xServiceID, FTOM_MSG_TYPE_CONNECTED, NULL);	
				}
				pDMC->bConnected = FTM_TRUE;
			}
		}
		else
		{
			FTOM_MSG_PTR	pMsg= NULL;

			while(FTOM_MSGQ_timedPop(&pDMC->xMsgQ, 100, &pMsg) == FTM_RET_OK)
			{
				switch(pMsg->xType)
				{
				case	FTOM_MSG_TYPE_INITIALIZE_DONE:
					{
					}
					break;

				case	FTOM_MSG_TYPE_EP_DATA:
					{
						FTOM_DMC_onEPData(pDMC, (FTOM_MSG_EP_DATA_PTR)pMsg);
					}
					break;

				case	FTOM_MSG_TYPE_QUIT:
					{
						pDMC->bStop = FTM_TRUE;	
					}
					break;

				default:
					{
						TRACE("Message[%08x] not supported.\n", pMsg->xType);
					}
					
				}

				FTOM_MSG_destroy(&pMsg);
			}
		}
	}

	return	0;
}

FTM_RET	FTOM_DMC_setServiceCallback
(
	FTOM_DMC_PTR 	pDMC, 
	FTOM_SERVICE_ID xServiceID, 
	FTOM_SERVICE_CB pServiceCB
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pServiceCB != NULL);

	pDMC->xServiceID = xServiceID;
	pDMC->fServiceCB = pServiceCB;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_DMC_sendMessage
(
	FTOM_DMC_PTR pDMC, 
	FTOM_MSG_PTR pMsg
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pMsg != NULL);

	return	FTOM_MSGQ_push(&pDMC->xMsgQ, pMsg);
}

FTM_RET	FTOM_DMC_onEPData
(
	FTOM_DMC_PTR	pDMC,
	FTOM_MSG_EP_DATA_PTR	pMsg
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pMsg != NULL);
	
	FTM_INT	i;

	for(i = 0 ; i < pMsg->ulCount; i++)
	{
		FTDM_CLIENT_EP_DATA_append(pDMC->pClient, pMsg->pEPID, &pMsg->pData[i]);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_DMC_NODE_add
(
	FTOM_DMC_PTR 	pDMC, 
	FTM_NODE_PTR 		pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pInfo != NULL);
	
	FTM_RET	xRet;

	xRet = FTDM_CLIENT_NODE_append(pDMC->pClient, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to add node to DB!\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DMC_NODE_remove
(
	FTOM_DMC_PTR 	pDMC, 
	FTM_CHAR_PTR	pDID
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pDID != NULL);

	return	FTDM_CLIENT_NODE_remove(pDMC->pClient, pDID);
}

FTM_RET	FTOM_DMC_NODE_count
(
	FTOM_DMC_PTR	pDMC,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pulCount != NULL);

	return	FTDM_CLIENT_NODE_count(pDMC->pClient, pulCount);
}

FTM_RET	FTOM_DMC_NODE_get
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pDID,
	FTM_NODE_PTR	pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pDID != NULL);
	ASSERT(pInfo != NULL);

	return	FTDM_CLIENT_NODE_get(pDMC->pClient, pDID, pInfo);
}

FTM_RET	FTOM_DMC_NODE_getAt
(
	FTOM_DMC_PTR	pDMC,
	FTM_ULONG		ulIndex,
	FTM_NODE_PTR	pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pInfo != NULL);

	return	FTDM_CLIENT_NODE_getAt(pDMC->pClient, ulIndex, pInfo);
}

FTM_RET	FTOM_DMC_NODE_getDIDList
(
	FTOM_DMC_PTR	pDMC,
	FTM_DID_PTR		pDIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pDIDs != NULL);
	ASSERT(pulCount != NULL);

	return	FTDM_CLIENT_NODE_getDIDList(pDMC->pClient, pDIDs, 0, ulCount,  &ulCount);
}

FTM_RET	FTOM_DMC_NODE_set
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pDID,
	FTM_NODE_FIELD	xFields,
	FTM_NODE_PTR	pInfo
)
{	
	ASSERT(pDMC != NULL);
	ASSERT(pDID != NULL);
	ASSERT(pInfo != NULL);

	return	FTDM_CLIENT_NODE_set(pDMC->pClient, pDID, xFields, pInfo);
}

FTM_RET	FTOM_DMC_EP_add
(
	FTOM_DMC_PTR 	pDMC, 
	FTM_EP_PTR 		pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pInfo != NULL);

	return	FTDM_CLIENT_EP_append(pDMC->pClient, pInfo);
}

FTM_RET	FTOM_DMC_EP_remove
(
	FTOM_DMC_PTR 	pDMC, 
	FTM_CHAR_PTR	pEPID
)
{
	ASSERT(pDMC != NULL);

	return	FTDM_CLIENT_EP_remove(pDMC->pClient, pEPID);
}

FTM_RET	FTOM_DMC_EP_count
(
	FTOM_DMC_PTR	pDMC,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pDMC != NULL);

	return	FTDM_CLIENT_EP_count(pDMC->pClient, 0, pulCount);
}

FTM_RET	FTOM_DMC_EP_getEPIDList
(
	FTOM_DMC_PTR	pDMC,
	FTM_EPID_PTR	pEPIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pEPIDs != NULL);
	ASSERT(pulCount != NULL);

	return	FTDM_CLIENT_EP_getEPIDList(pDMC->pClient, pEPIDs, ulIndex, ulMaxCount, pulCount);
}

FTM_RET	FTOM_DMC_EP_get
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_PTR		pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pEPID != NULL);
	ASSERT(pInfo != NULL);

	return	FTDM_CLIENT_EP_get(pDMC->pClient, pEPID, pInfo);
}

FTM_RET	FTOM_DMC_EP_getAt
(
	FTOM_DMC_PTR	pDMC,
	FTM_ULONG		ulIndex,
	FTM_EP_PTR		pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pInfo != NULL);

	return	FTDM_CLIENT_EP_getAt(pDMC->pClient, ulIndex, pInfo);
}

FTM_RET	FTOM_DMC_EP_set
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_FIELD	xFields,
	FTM_EP_PTR		pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pEPID != NULL);
	ASSERT(pInfo != NULL);

	return	FTDM_CLIENT_EP_set(pDMC->pClient, pEPID, xFields, pInfo);
}

FTM_RET	FTOM_DMC_EP_DATA_add
(
	FTOM_DMC_PTR 	pDMC, 
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR pData
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pData != NULL);

	return	FTDM_CLIENT_EP_DATA_append(pDMC->pClient, pEPID, pData);
}

FTM_RET	FTOM_DMC_EP_DATA_remove
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulDeletedCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pEPID != NULL);
	ASSERT(pulDeletedCount != NULL);

	return	FTDM_CLIENT_EP_DATA_remove(pDMC->pClient, pEPID, ulIndex, ulCount, pulDeletedCount);
}

FTM_RET	FTOM_DMC_EP_DATA_removeWithTime
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulBegin,
	FTM_ULONG		ulEnd,
	FTM_ULONG_PTR	pulDeletedCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pEPID != NULL);
	ASSERT(pulDeletedCount != NULL);

	return	FTDM_CLIENT_EP_DATA_removeWithTime(pDMC->pClient, pEPID, ulBegin, ulEnd, pulDeletedCount);
}


FTM_RET	FTOM_DMC_EP_DATA_count
(
	FTOM_DMC_PTR 	pDMC, 
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pulCount != NULL);

	return	FTDM_CLIENT_EP_DATA_count(pDMC->pClient, pEPID, pulCount);
}

FTM_RET	FTOM_DMC_EP_DATA_countWithTime
(
	FTOM_DMC_PTR 	pDMC, 
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulStart,
	FTM_ULONG		ulEnd,
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pulCount != NULL);

	return	FTDM_CLIENT_EP_DATA_countWithTime(pDMC->pClient, pEPID, ulStart, ulEnd, pulCount);
}

FTM_RET FTOM_DMC_EP_DATA_get
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulStartIndex,
	FTM_EP_DATA_PTR	pData,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR 	pulCount,
	FTM_BOOL_PTR	pbRemain
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pEPID != NULL);
	ASSERT(pData != NULL);
	ASSERT(pulCount != NULL);

	return	FTDM_CLIENT_EP_DATA_get(pDMC->pClient, pEPID, ulStartIndex, pData, ulMaxCount, pulCount, pbRemain);
}

FTM_RET FTOM_DMC_EP_DATA_getWithTime
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulBegin,
	FTM_ULONG		ulEnd,
	FTM_BOOL		bAscending,
	FTM_EP_DATA_PTR	pData,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR 	pulCount,
	FTM_BOOL_PTR	pbRemain
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pEPID != NULL);
	ASSERT(pData != NULL);
	ASSERT(pulCount != NULL);

	return	FTDM_CLIENT_EP_DATA_getWithTime(pDMC->pClient, pEPID, ulBegin, ulEnd, bAscending, pData, ulMaxCount, pulCount, pbRemain);
}

FTM_RET FTOM_DMC_EP_DATA_del
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pEPID != NULL);
	ASSERT(pulCount != NULL);

	return	FTDM_CLIENT_EP_DATA_remove(pDMC->pClient, pEPID, ulIndex, ulCount, pulCount);
}

FTM_RET FTOM_DMC_EP_DATA_delWithTime
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulStart,
	FTM_ULONG		ulEnd,
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pEPID != NULL);
	ASSERT(pulCount != NULL);

	return	FTDM_CLIENT_EP_DATA_removeWithTime(pDMC->pClient, pEPID, ulStart, ulEnd, pulCount);
}

FTM_RET FTOM_DMC_EP_DATA_info
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR 	pulBeginTime, 
	FTM_ULONG_PTR 	pulEndTime, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pDMC != NULL);

	return	FTDM_CLIENT_EP_DATA_info(pDMC->pClient, pEPID, pulBeginTime, pulEndTime, pulCount);
}

FTM_RET FTOM_DMC_EP_DATA_setLimit
(
	FTOM_DMC_PTR		pDMC,
	FTM_CHAR_PTR		pEPID,
	FTM_EP_LIMIT_PTR	pLimit
)
{
	ASSERT(pDMC != NULL);

	return	FTDM_CLIENT_EP_DATA_setLimit(pDMC->pClient, pEPID, pLimit);
}

FTM_RET	FTOM_DMC_EP_CLASS_count
(
	FTOM_DMC_PTR	pDMC,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pulCount != NULL);

	return	FTDM_CLIENT_EP_CLASS_count(pDMC->pClient, pulCount);
}

FTM_RET	FTOM_DMC_EP_CLASS_getAt
(
	FTOM_DMC_PTR	pDMC,
	FTM_ULONG		ulIndex,
	FTM_EP_CLASS_PTR	pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pInfo != NULL);

	return	FTDM_CLIENT_EP_CLASS_getAt(pDMC->pClient, ulIndex, pInfo);
}

FTM_RET	FTOM_DMC_EVENT_add
(
	FTOM_DMC_PTR	pDMC,
	FTM_EVENT_PTR	pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pInfo != NULL);

	return	FTDM_CLIENT_EVENT_add(pDMC->pClient, pInfo);
}

FTM_RET	FTOM_DMC_EVENT_remove
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pID
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pID != NULL);

	return	FTDM_CLIENT_EVENT_del(pDMC->pClient, pID);
}

FTM_RET	FTOM_DMC_EVENT_count
(
	FTOM_DMC_PTR	pDMC,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pulCount != NULL);
	
	return	FTDM_CLIENT_EVENT_count(pDMC->pClient, pulCount);
}

FTM_RET	FTOM_DMC_EVENT_getIDList
(
	FTOM_DMC_PTR	pDMC,
	FTM_ID_PTR		pIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pIDs != NULL);
	ASSERT(pulCount != NULL);

	return	FTDM_CLIENT_EVENT_getIDList(pDMC->pClient, pIDs, ulIndex, ulMaxCount, pulCount);
}

FTM_RET	FTOM_DMC_EVENT_get
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pID,
	FTM_EVENT_PTR	pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pID != NULL);
	ASSERT(pInfo != NULL);

	return	FTDM_CLIENT_EVENT_get(pDMC->pClient, pID, pInfo);
}

FTM_RET	FTOM_DMC_EVENT_getAt
(
	FTOM_DMC_PTR	pDMC,
	FTM_ULONG		ulIndex,
	FTM_EVENT_PTR	pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pInfo != NULL);

	return	FTDM_CLIENT_EVENT_getAt(pDMC->pClient, ulIndex, pInfo);
}

FTM_RET	FTOM_DMC_EVENT_set
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pID,
	FTM_EVENT_FIELD	xFields,
	FTM_EVENT_PTR	pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pID != NULL);
	ASSERT(pInfo != NULL);

	return	FTDM_CLIENT_EVENT_set(pDMC->pClient, pID, xFields, pInfo);
}

FTM_RET	FTOM_DMC_ACTION_add
(
	FTOM_DMC_PTR	pDMC,
	FTM_ACTION_PTR	pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pInfo != NULL);

	return	FTDM_CLIENT_ACTION_add(pDMC->pClient, pInfo);
}

FTM_RET	FTOM_DMC_ACTION_remove
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pID
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pID != NULL);

	return	FTDM_CLIENT_ACTION_del(pDMC->pClient, pID);
}

FTM_RET	FTOM_DMC_ACTION_count
(
	FTOM_DMC_PTR	pDMC,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pulCount != NULL);
	
	return	FTDM_CLIENT_ACTION_count(pDMC->pClient, pulCount);
}

FTM_RET	FTOM_DMC_ACTION_get
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pID,
	FTM_ACTION_PTR	pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pID != NULL);
	ASSERT(pInfo != NULL);

	return	FTDM_CLIENT_ACTION_get(pDMC->pClient, pID, pInfo);
}

FTM_RET	FTOM_DMC_ACTION_getAt
(
	FTOM_DMC_PTR	pDMC,
	FTM_ULONG		ulIndex,
	FTM_ACTION_PTR	pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pInfo != NULL);

	return	FTDM_CLIENT_ACTION_getAt(pDMC->pClient, ulIndex, pInfo);
}

FTM_RET	FTOM_DMC_ACTION_set
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pID,
	FTM_ACTION_FIELD	xFields,
	FTM_ACTION_PTR	pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pID != NULL);
	ASSERT(pInfo != NULL);

	return	FTDM_CLIENT_ACTION_set(pDMC->pClient, pID, xFields, pInfo);
}

FTM_RET	FTOM_DMC_ACTION_getIDList
(
	FTOM_DMC_PTR	pDMC,
	FTM_ID_PTR		pIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pIDs != NULL);
	ASSERT(pulCount != NULL);

	return	FTDM_CLIENT_ACTION_getIDList(pDMC->pClient, pIDs, ulIndex, ulMaxCount, pulCount);
}

FTM_RET	FTOM_DMC_RULE_add
(
	FTOM_DMC_PTR	pDMC,
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pInfo != NULL);

	return	FTDM_CLIENT_RULE_add(pDMC->pClient, pInfo);
}

FTM_RET	FTOM_DMC_RULE_remove
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pID
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pID != NULL);

	return	FTDM_CLIENT_RULE_del(pDMC->pClient, pID);
}

FTM_RET	FTOM_DMC_RULE_count
(
	FTOM_DMC_PTR	pDMC,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pulCount != NULL);
	
	return	FTDM_CLIENT_RULE_count(pDMC->pClient, pulCount);
}

FTM_RET	FTOM_DMC_RULE_get
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pID,
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pID != NULL);
	ASSERT(pInfo != NULL);

	return	FTDM_CLIENT_RULE_get(pDMC->pClient, pID, pInfo);
}

FTM_RET	FTOM_DMC_RULE_getAt
(
	FTOM_DMC_PTR	pDMC,
	FTM_ULONG		ulIndex,
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pInfo != NULL);

	return	FTDM_CLIENT_RULE_getAt(pDMC->pClient, ulIndex, pInfo);
}

FTM_RET	FTOM_DMC_RULE_set
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pID,
	FTM_RULE_FIELD	xFields,
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pID != NULL);
	ASSERT(pInfo != NULL);

	return	FTDM_CLIENT_RULE_set(pDMC->pClient, pID, xFields, pInfo);
}

FTM_RET	FTOM_DMC_RULE_getIDList
(
	FTOM_DMC_PTR	pDMC,
	FTM_ID_PTR		pIDs,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pIDs != NULL);
	ASSERT(pulCount != NULL);

	return	FTDM_CLIENT_RULE_getIDList(pDMC->pClient, pIDs, ulIndex, ulMaxCount, pulCount);
}

/*********************************************************************
 *
 *********************************************************************/
FTM_RET	FTOM_DMC_LOG_add
(
	FTOM_DMC_PTR	pDMC,
	FTM_LOG_PTR		pLog
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pLog != NULL);

	return	FTDM_CLIENT_LOG_add(pDMC->pClient, pLog);
}

FTM_RET	FTOM_DMC_LOG_del
(
	FTOM_DMC_PTR	pDMC,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulDeletedCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pulDeletedCount != NULL);

	return	FTDM_CLIENT_LOG_del(pDMC->pClient, ulIndex, ulCount, pulDeletedCount);
}

FTM_RET	FTOM_DMC_LOG_count
(
	FTOM_DMC_PTR	pDMC,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pulCount != NULL);
	
	return	FTDM_CLIENT_LOG_count(pDMC->pClient, pulCount);
}

FTM_RET	FTOM_DMC_LOG_get
(
	FTOM_DMC_PTR	pDMC,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_LOG_PTR		pLogs,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pLogs != NULL);
	ASSERT(pulCount != NULL);

	return	FTDM_CLIENT_LOG_get(pDMC->pClient, ulIndex, ulCount, pLogs, pulCount);
}

FTM_RET	FTOM_DMC_LOG_getAt
(
	FTOM_DMC_PTR	pDMC,
	FTM_ULONG		ulIndex,
	FTM_LOG_PTR		pLog
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pLog != NULL);

	return	FTDM_CLIENT_LOG_getAt(pDMC->pClient, ulIndex, pLog);
}

/*******************************************************************************
 *
 *******************************************************************************/
FTM_RET FTOM_DMC_loadConfig
(
	FTOM_DMC_PTR 	pDMC, 
	FTM_CONFIG_PTR	pConfig
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pConfig != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_ITEM		xSection;

	xRet = FTM_CONFIG_getItem(pConfig, "client", &xSection);
	if (xRet == FTM_RET_OK)
	{
		FTM_CHAR	pValue[256];

		memset(pValue, 0, sizeof(pValue) - 1);
		xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "server_ip", pValue, sizeof(pValue) - 1);
		if (xRet == FTM_RET_OK)
		{
			strncpy(pDMC->xConfig.xNetwork.pServerIP, pValue, FTDM_CLIENT_SERVER_IP_LEN);
		}
	
		xRet = FTM_CONFIG_ITEM_getItemUSHORT(&xSection, "port", &pDMC->xConfig.xNetwork.usPort);
	}

	return	FTM_RET_OK;
}

FTM_RET FTOM_DMC_loadFromFile
(
	FTOM_DMC_PTR pDMC, 
	FTM_CHAR_PTR pFileName
)
{
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_PTR		pConfig;

	xRet = FTM_CONFIG_create(pFileName, &pConfig, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	FTOM_DMC_loadConfig(pDMC, pConfig);

	FTM_CONFIG_destroy(&pConfig);

	return	FTM_RET_OK;
}

FTM_RET FTOM_DMC_saveConfig
(
	FTOM_DMC_PTR 	pDMC, 
	FTM_CONFIG_PTR	pConfig
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pConfig != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_ITEM		xSection;

	xRet = FTM_CONFIG_getItem(pConfig, "client", &xSection);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_CONFIG_addItem(pConfig, "client", &xSection);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}
	}

	FTM_CONFIG_ITEM_setItemString(&xSection, "server_ip", pDMC->xConfig.xNetwork.pServerIP);
	FTM_CONFIG_ITEM_setItemUSHORT(&xSection, "port", pDMC->xConfig.xNetwork.usPort);

	return	FTM_RET_OK;
}

FTM_RET FTOM_DMC_showConfig(FTOM_DMC_PTR pDMC)
{
	ASSERT(pDMC != NULL);

	MESSAGE("\n[ DATA MANAGER CONNECTION CONFIGURATION ]\n");
	MESSAGE("%16s : %s\n", "SERVER", pDMC->xConfig.xNetwork.pServerIP);
	MESSAGE("%16s : %d\n", "PORT", pDMC->xConfig.xNetwork.usPort);

	return	FTM_RET_OK;
}

