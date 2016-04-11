#include "ftom_discovery.h"
#include "ftm_list.h"

static
FTM_RET	FTOM_DISCOVERY_requestInformation
(
	FTOM_DISCOVERY_PTR	pDiscovery,
	FTM_CHAR_PTR		pNetwork,
	FTM_USHORT			usPort
);

static 
FTM_RET	FTOM_DISCOVERY_infoCB
(
	FTOM_PTR					pOM,
	FTOM_MSG_DISCOVERY_INFO_PTR	pMsg,
	FTM_VOID_PTR				pObj
);

static 
FTM_VOID_PTR FTM_DISCOVERY_process
(
	FTM_VOID_PTR	pData
);

static
FTM_BOOL	FTOM_DISCOVERY_seekNode
(
	const FTM_VOID_PTR	pElement,
	const FTM_VOID_PTR	pKey
);

FTM_RET	FTOM_DISCOVERY_init
(
	FTOM_DISCOVERY_PTR	pDiscovery,
	FTOM_PTR			pOM
)
{
	ASSERT(pOM != NULL);

	TRACE("DISCOVERY init\n");
	pDiscovery->bStop 		= FTM_TRUE;
	pDiscovery->bInProgress = FTM_FALSE;
	pDiscovery->ulTimeout	= 2;
	pDiscovery->ulRetryCount= 3;
	pDiscovery->ulLoopCount	= 0;
	FTOM_MSGQ_create(&pDiscovery->pMsgQ);
	FTM_TIMER_init(&pDiscovery->xTimer, 0);
	FTM_LIST_init(&pDiscovery->xNodeList);
	FTM_LIST_init(&pDiscovery->xInfoList);

	FTM_LIST_setSeeker(&pDiscovery->xInfoList, FTOM_DISCOVERY_seekNode);

	pDiscovery->pOM = pOM;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_DISCOVERY_final
(
	FTOM_DISCOVERY_PTR	pDiscovery
)
{
	FTOM_DISCOVERY_INFO_PTR pInfo = NULL;

	FTM_LIST_iteratorStart(&pDiscovery->xInfoList);
	while(FTM_LIST_iteratorNext(&pDiscovery->xInfoList, (FTM_VOID_PTR _PTR_)&pInfo) == FTM_RET_OK)
	{
		FTM_LIST_remove(&pDiscovery->xInfoList, pInfo);
		FTM_MEM_free(pInfo);
	}

	FTOM_MSGQ_destroy(&pDiscovery->pMsgQ);
	FTM_LIST_final(&pDiscovery->xInfoList);
	FTM_LIST_final(&pDiscovery->xNodeList);
	FTM_LIST_final(&pDiscovery->xEPList);
	pDiscovery->pOM = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_DISCOVERY_start
(
	FTOM_DISCOVERY_PTR	pDiscovery
)
{
	ASSERT(pDiscovery != NULL);

	FTM_RET	xRet;
	FTM_INT	nRet;

	TRACE("DISCOVERY start\n");
	if (!pDiscovery->bStop)
	{
		return	FTM_RET_ALREADY_STARTED;
	}

	xRet = FTOM_setMessageCallback(pDiscovery->pOM, FTOM_MSG_TYPE_DISCOVERY_INFO, (FTOM_ON_MESSAGE_CALLBACK)FTOM_DISCOVERY_infoCB, (FTM_VOID_PTR)pDiscovery, &pDiscovery->fOldCB, &pDiscovery->pOldData);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	nRet = pthread_create(&pDiscovery->xThread, NULL, FTM_DISCOVERY_process, pDiscovery);
	if (nRet != 0)
	{
		ERROR("Discovery thread creation failed[%d].\n", nRet);
		return	FTM_RET_CANT_CREATE_THREAD;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_DISCOVERY_stop
(
	FTOM_DISCOVERY_PTR	pDiscovery
)
{
	ASSERT(pDiscovery != NULL);

	if (pDiscovery->bStop)
	{
		return	FTM_RET_NOT_START;
	}

	pDiscovery->bStop = FTM_TRUE;
	pthread_join(pDiscovery->xThread, NULL);

	FTOM_setMessageCallback(pDiscovery->pOM, FTOM_MSG_TYPE_DISCOVERY_INFO, pDiscovery->fOldCB, pDiscovery->pOldData, NULL, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTM_DISCOVERY_process
(
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);

	FTOM_DISCOVERY_PTR	pDiscovery = (FTOM_DISCOVERY_PTR)pData;
	FTM_RET		xRet;

	pDiscovery->bStop = FTM_FALSE;

	while(!pDiscovery->bStop)
	{
		FTOM_MSG_PTR	pCommonMsg;

		xRet = FTOM_MSGQ_timedPop(pDiscovery->pMsgQ, 100000, &pCommonMsg);
		if (xRet == FTM_RET_OK)
		{
			switch(pCommonMsg->xType)
			{
			case	FTOM_MSG_TYPE_DISCOVERY:
				{
					FTOM_MSG_DISCOVERY_PTR	pMsg = (FTOM_MSG_DISCOVERY_PTR)pCommonMsg;

					if (!pDiscovery->bInProgress)
					{
						pDiscovery->bInProgress = FTM_TRUE;
						FTM_TIMER_init(&pDiscovery->xTimer, pDiscovery->ulTimeout * 1000000);
						FTOM_DISCOVERY_requestInformation(pDiscovery, pMsg->pNetwork, pMsg->usPort);
						pDiscovery->bInProgress = FTM_TRUE;
						pDiscovery->ulLoopCount = 1;
					}
				}
				break;

			case	FTOM_MSG_TYPE_DISCOVERY_INFO:
				{
					FTOM_MSG_DISCOVERY_INFO_PTR	pMsg = (FTOM_MSG_DISCOVERY_INFO_PTR)pCommonMsg;
					FTOM_NODE_PTR	pNode;

					TRACE("Discovery Node : %s\n", pMsg->pDID);
					xRet = FTOM_getNode(pDiscovery->pOM, pMsg->pDID, &pNode);
					if (xRet != FTM_RET_OK)
					{
						WARN("Unknown Node[%s]\n", pMsg->pDID);	
					}
					else
					{ 
						FTM_ULONG	i, ulCount;

						for(i = 0 ; i < pMsg->ulCount ; i++)
						{
							xRet = FTOM_discoveryEPCount(pDiscovery->pOM, pMsg->pIP, pMsg->pTypes[i], &ulCount);
							if (xRet == FTM_RET_OK)
							{
								TRACE("EP[%08x] is %d\n", pMsg->pTypes[i], ulCount);	
							}
						}	
					}
					//FTM_NODE	xNodeInfo;
					//FTOM_DISCOVERY_getNodeInfo(pMsg->pDID, &xNodeInfO);
				}
				break;

			default:
				{
					ERROR("Not supported msg[%08x]\n", pCommonMsg->xType);	
				}
			}

			FTOM_MSG_destroy(&pCommonMsg);
		}

		if (pDiscovery->bInProgress && FTM_TIMER_isExpired(&pDiscovery->xTimer))
		{
			if (pDiscovery->ulLoopCount < pDiscovery->ulRetryCount)
			{
				FTM_TIMER_init(&pDiscovery->xTimer, pDiscovery->ulTimeout * 1000000);
				FTOM_DISCOVERY_requestInformation(pDiscovery, "255.255.255.255", 1234);
				pDiscovery->ulLoopCount++;
			}
			else
			{
				pDiscovery->bInProgress = FTM_FALSE;
			}
		}

	}

	return	0;
}

FTM_RET	FTOM_DISCOVERY_call
(
	FTOM_DISCOVERY_PTR	pDiscovery,
	FTM_CHAR_PTR		pNetwork,
	FTM_USHORT			usPort
)
{
	ASSERT(pDiscovery != NULL);
	ASSERT(pNetwork != NULL);

	FTOM_MSG_DISCOVERY_PTR	pMsg;
	FTM_RET		xRet;
	
	xRet = FTOM_MSG_createDiscovery(pNetwork, usPort, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Discovery message creation failed[%08x].\n", xRet);
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(pDiscovery->pMsgQ, (FTOM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Discovery message push failed[%08x].\n", xRet);
		FTOM_MSG_destroy((FTOM_MSG_PTR _PTR_ )&pMsg);
		return	xRet;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_DISCOVERY_getNodeInfoCount
(
	FTOM_DISCOVERY_PTR	pDiscovery,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pDiscovery != NULL);
	ASSERT(pulCount != NULL);

	FTM_LIST_count(&pDiscovery->xNodeList, pulCount);	

	return	FTM_RET_OK;
}

FTM_RET	FTOM_DISCOVERY_getNodeInfoAt
(
	FTOM_DISCOVERY_PTR	pDiscovery,
	FTM_ULONG			ulIndex,
	FTM_NODE_PTR		pNodeInfo
)
{
	ASSERT(pDiscovery != NULL);
	ASSERT(pNodeInfo != NULL);

	FTM_RET	xRet;	
	FTM_NODE_PTR	pItem;

	xRet = FTM_LIST_getAt(&pDiscovery->xNodeList, ulIndex, (FTM_VOID_PTR _PTR_)&pItem);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	memcpy(pNodeInfo, pItem, sizeof(FTM_NODE));

	return	FTM_RET_OK;
}

FTM_RET	FTOM_DISCOVERY_infoCB
(
	FTOM_PTR					pOM,
	FTOM_MSG_DISCOVERY_INFO_PTR	pMsg,
	FTM_VOID_PTR				pObj
)
{
	ASSERT(pOM != NULL);
	ASSERT(pMsg != NULL);
	ASSERT(pObj != NULL);
	
	FTOM_DISCOVERY_PTR		pDiscovery = (FTOM_DISCOVERY_PTR)pObj;
	FTOM_DISCOVERY_INFO_PTR	pInfo;
	FTOM_MSG_DISCOVERY_INFO_PTR pNewMsg;
	FTM_RET					xRet;

	xRet = FTM_LIST_get(&pDiscovery->xInfoList, pMsg->pDID, (FTM_VOID_PTR _PTR_)&pInfo);
	if (xRet == FTM_RET_OK)
	{
		return	FTM_RET_OK;
	}

	pInfo = (FTOM_DISCOVERY_INFO_PTR)FTM_MEM_malloc(sizeof(FTOM_DISCOVERY_INFO) + sizeof(FTM_EP_TYPE) * pMsg->ulCount);
	if (pInfo == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	strcpy(pInfo->pDID, pMsg->pDID);
	memcpy(pInfo->pEPTypes, pMsg->pTypes, sizeof(FTM_EP_TYPE)*pMsg->ulCount);
	pInfo->ulCount = pMsg->ulCount;

	xRet = FTM_LIST_append(&pDiscovery->xInfoList, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR("List append failed!\n");
		FTM_MEM_free(pInfo);
		return	xRet;	
	}

	xRet = FTOM_MSG_createDiscoveryInfo(pMsg->pName, pMsg->pDID, pMsg->pIP, pMsg->pTypes, pMsg->ulCount, &pNewMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Discovery info message creation failed[%08x].\n", xRet);
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(pDiscovery->pMsgQ, (FTOM_MSG_PTR)pNewMsg);
	if (xRet != FTM_RET_OK)
	{
		FTOM_MSG_destroy((FTOM_MSG_PTR _PTR_)&pNewMsg);
		ERROR("Message push failed[%08x]\n", xRet);
		return	xRet;	
	}

	return	FTM_RET_OK;		
}

FTM_RET	FTOM_DISCOVERY_requestInformation
(
	FTOM_DISCOVERY_PTR	pDiscovery,
	FTM_CHAR_PTR		pNetwork,
	FTM_USHORT			usPort
)
{
	FTM_CHAR_PTR	pMsg = "Hello?";
	FTM_INT			nSockFD;
	FTM_INT			nRet;
	FTM_INT			nBroadcast = 1;
	FTM_INT			nBytes;
	struct sockaddr_in xDestAddr;

	TRACE("Discovery request!\n");
	nSockFD = socket(PF_INET,SOCK_DGRAM,0);
	if(nSockFD == -1)
	{
		return	FTM_RET_ERROR;
	}

	nRet = setsockopt(nSockFD, SOL_SOCKET, SO_BROADCAST, &nBroadcast,sizeof(nBroadcast));
	if (nRet == -1)
	{
		ERROR("setsocketopt error!\n");
		return	FTM_RET_ERROR;
	}

	xDestAddr.sin_family = AF_INET;
	xDestAddr.sin_port = htons(usPort);
	xDestAddr.sin_addr.s_addr = inet_addr(pNetwork);
	memset(xDestAddr.sin_zero,'\0',sizeof(xDestAddr.sin_zero));
	
	nBytes = sendto(nSockFD, pMsg, strlen(pMsg) , 0, (struct sockaddr *)&xDestAddr, sizeof(xDestAddr));
	if (nBytes == -1)
	{
		ERROR("Packet send failed.\n");
		return	FTM_RET_ERROR;
	}

	close(nSockFD);

	return	FTM_RET_OK;

}

FTM_BOOL	FTOM_DISCOVERY_seekNode
(
	const FTM_VOID_PTR	pElement,
	const FTM_VOID_PTR	pKey
)
{
	ASSERT(pElement != NULL);
	ASSERT(pKey != NULL);

	FTOM_DISCOVERY_INFO_PTR 	pInfo = (FTOM_DISCOVERY_INFO_PTR)pElement;
	FTM_CHAR_PTR				pDID = (FTM_CHAR_PTR)pKey;

	return	(strcmp(pInfo->pDID, pDID) == 0);
}



