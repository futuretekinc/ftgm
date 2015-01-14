#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "ftnm.h"
#include "ftdm_client.h"
#include "ftnm_node_snmpc.h"

FTM_VOID_PTR	FTNM_DMC_task(FTM_VOID_PTR pData);
FTM_RET			FTNM_DMC_taskInit(FTNM_CONTEXT_PTR pContext);
FTM_RET			FTNM_DMC_taskConnect(FTNM_CONTEXT_PTR pContext);
FTM_RET			FTNM_DMC_taskSync(FTNM_CONTEXT_PTR pContext);
FTM_RET			FTNM_DMC_taskRunChild(FTNM_CONTEXT_PTR pContext);
FTM_RET			FTNM_DMC_taskWait(FTNM_CONTEXT_PTR pContext);

FTM_RET	FTNM_init(FTNM_CONTEXT_PTR pContext, FTNM_CFG_PTR pConfig)
{
	FTM_RET	nRet;

	ASSERT((pContext != NULL) && (pConfig != NULL));

	memset(pContext, 0, sizeof(FTNM_CONTEXT));

	nRet = FTNM_NODE_MNGR_init();
	if (nRet != FTM_RET_OK)
	{
		ERROR("Node manager initialization failed.\n");
		return	nRet;
	}

	nRet = FTNM_EP_init();
	if (nRet != FTM_RET_OK)
	{
		ERROR("EP manager initialization failed.\n");
		FTNM_NODE_MNGR_final();
		return	nRet;
	}

	nRet = FTNM_EP_CLASS_INFO_init();
	if (nRet != FTM_RET_OK)
	{
		ERROR("EP manager initialization failed.\n");
		FTNM_EP_final();
		FTNM_NODE_MNGR_final();
		return	nRet;
	}

	FTNM_CFG_copyCreate(&pContext->pConfig, pConfig);

	FTNM_SNMPC_init();

	TRACE("FTNM initialization done.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTNM_final(FTNM_CONTEXT_PTR pContext)
{
	FTNM_CFG_destroy(pContext->pConfig);

	FTNM_EP_final();
	FTNM_NODE_MNGR_final();

	return	FTM_RET_OK;
}

FTM_RET FTNM_DMC_run(FTNM_CONTEXT_PTR pContext)
{
	ASSERT(pContext != NULL);

	if (pthread_create(&pContext->xDMC.xThread, NULL, FTNM_DMC_task, (FTM_VOID_PTR)pContext) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	pthread_join(pContext->xDMC.xThread, NULL);
	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTNM_DMC_task(FTM_VOID_PTR pData)
{
	FTNM_CONTEXT_PTR	pContext = (FTNM_CONTEXT_PTR)pData;

	pContext->xDMC.xState = FTNM_DMC_STATE_CREATED;

	while(1)
	{
		switch(pContext->xDMC.xState)
		{
		case	FTNM_DMC_STATE_CREATED:
			{
				FTNM_DMC_taskInit(pContext);
			}
			break;

		case	FTNM_DMC_STATE_INITIALIZED:
			{
				FTNM_DMC_taskConnect(pContext);
			}
			break;

		case	FTNM_DMC_STATE_CONNECTED:
			{
				FTNM_DMC_taskSync(pContext);
			}
			break;

		case	FTNM_DMC_STATE_SYNCHRONIZED:
			{
				FTNM_DMC_taskRunChild(pContext);	
			}
			break;

		case	FTNM_DMC_STATE_PROCESS_FINISHED:
			{
				FTNM_DMC_taskWait(pContext);	
			}
			break;
		}
		
		usleep(1000);
	}

	return	0;
}

FTM_RET	FTNM_DMC_taskInit(FTNM_CONTEXT_PTR pContext)
{
	FTM_RET	nRet;

	ASSERT(pContext != NULL);

	nRet = FTDMC_init(&pContext->pConfig->xClient);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}
	pContext->xDMC.xState = FTNM_DMC_STATE_INITIALIZED;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_DMC_taskConnect(FTNM_CONTEXT_PTR pContext)
{
	FTM_RET			nRet;

	nRet = FTDMC_connect(&pContext->xDMC.xSession, 
		inet_addr(pContext->pConfig->xClient.xNetwork.pServerIP),
		pContext->pConfig->xClient.xNetwork.usPort);
	if (nRet != FTM_RET_OK)
	{
		usleep(1000000);
		return	nRet;	
	}

	pContext->xDMC.xState = FTNM_DMC_STATE_CONNECTED;

	return	nRet;
}

FTM_RET	FTNM_DMC_taskSync(FTNM_CONTEXT_PTR pContext)
{
	FTM_RET			nRet;
	FTM_ULONG		ulCount, i;

	ASSERT(pContext != NULL);

	nRet = FTDMC_getNodeInfoCount(&pContext->xDMC.xSession, &ulCount);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_NODE_INFO	xNodeInfo;
		FTNM_NODE_PTR	pNode;

		nRet = FTDMC_getNodeInfoByIndex(&pContext->xDMC.xSession, i, &xNodeInfo);	
		if (nRet != FTM_RET_OK)
		{
			ERROR("FTDMC_getNodeInfoByIndex(%08lx, %d, &xNodeInfo) = %08lx\n",
					pContext->xDMC.xSession.hSock, i, nRet);
			continue;	
		}

		nRet = FTNM_NODE_create(&xNodeInfo, &pNode);
		if (nRet != FTM_RET_OK)
		{
			ERROR("FTNM_NODE_create(xNode, &pNode) = %08lx\n", nRet);
			continue;	
		}

		TRACE("Node[%s] creating success.\n", pNode->xInfo.pDID);
	}

	nRet = FTDMC_getEPInfoCount(&pContext->xDMC.xSession, 0, &ulCount);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTNM_NODE_PTR	pNode;
		FTM_EP_INFO	xEPInfo;
		FTNM_EP_PTR	pEP;

		nRet = FTDMC_getEPInfoByIndex(&pContext->xDMC.xSession, i, &xEPInfo);
		if (nRet != FTM_RET_OK)
		{
			ERROR("FTDMC_getEPInfoByIndex(%08lx, %d, &xEPInfo) = %08lx\n",
					pContext->xDMC.xSession.hSock, i, nRet);
			continue;
		}

		nRet = FTNM_EP_create(&xEPInfo, &pEP);
		if (nRet != FTM_RET_OK)
		{
			ERROR("FTNM_EP_create(xEP, &pNode) = %08lx\n", nRet);
			continue;	
		}

		if (FTNM_NODE_get(xEPInfo.pDID, &pNode) == FTM_RET_OK)
		{
			FTNM_NODE_linkEP(pNode, pEP);
		}
		
		TRACE("EP[%08lx] creating success.\n", pEP->xInfo.xEPID);
	}

	nRet = FTDMC_getEPClassInfoCount(&pContext->xDMC.xSession, &ulCount);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTNM_NODE_PTR		pNode;
		FTM_EP_CLASS_INFO	xEPClassInfo;
		FTNM_EP_PTR			pEP;

		nRet = FTDMC_getEPClassInfoByIndex(&pContext->xDMC.xSession, i, &xEPClassInfo);
		if (nRet != FTM_RET_OK)
		{
			ERROR("FTDMC_getEPInfoByIndex(%08lx, %d, &xEPInfo) = %08lx\n",
					pContext->xDMC.xSession.hSock, i, nRet);
			continue;
		}

		nRet = FTNM_EP_CLASS_INFO_create(&xEPClassInfo);
		if (nRet != FTM_RET_OK)
		{
			ERROR("FTNM_EP_CLASS_append(&xEPClassInfo) = %08lx\n", nRet);
			continue;	
		}
	}


	pContext->xDMC.xState = FTNM_DMC_STATE_SYNCHRONIZED;
	return	FTM_RET_OK;
}

FTM_RET	FTNM_DMC_taskRunChild(FTNM_CONTEXT_PTR pContext)
{
	FTM_RET			nRet;
	FTNM_NODE_PTR	pNode;
	FTM_ULONG		i, ulCount;

	FTNM_NODE_count(&ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTNM_NODE_getAt(i, &pNode) == FTM_RET_OK)
		{
			FTNM_NODE_run(pNode);
		}
	}
	
	pContext->xDMC.xState = FTNM_DMC_STATE_PROCESS_FINISHED;
	return	FTM_RET_OK;
}

FTM_RET			FTNM_DMC_taskWait(FTNM_CONTEXT_PTR pContext)
{
	usleep(1000000);
	return	FTM_RET_OK;
}
