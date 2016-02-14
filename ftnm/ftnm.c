#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "ftnm.h"
#include "ftdm_client.h"
#include "ftnm_node.h"
#include "ftnm_snmpc.h"
#include "ftnm_ep.h"
#include "ftnm_ep_class.h"
#include "ftnm_server.h"
#include "ftnm_dmc.h"

FTM_VOID_PTR	FTNM_task(FTM_VOID_PTR pData);
FTM_RET			FTNM_taskInit(FTNM_CONTEXT_PTR pCTX);
FTM_RET			FTNM_taskConnect(FTNM_CONTEXT_PTR pCTX);
FTM_RET			FTNM_taskSync(FTNM_CONTEXT_PTR pCTX);
FTM_RET			FTNM_taskRunChild(FTNM_CONTEXT_PTR pCTX);
FTM_RET			FTNM_taskWait(FTNM_CONTEXT_PTR pCTX);

FTM_RET	FTNM_init(FTNM_CONTEXT_PTR pCTX, FTM_CHAR_PTR pConfigFileName)
{
	FTM_RET	nRet;

	nRet = FTNM_NODE_init(pCTX);
	if (nRet != FTM_RET_OK)
	{
		ERROR("Node manager initialization failed.\n");
		return	nRet;
	}

	nRet = FTNM_EP_init(pCTX);
	if (nRet != FTM_RET_OK)
	{
		ERROR("EP manager initialization failed.\n");
		FTNM_NODE_final(pCTX);
		return	nRet;
	}

	nRet = FTNM_EP_CLASS_INFO_init();
	if (nRet != FTM_RET_OK)
	{
		ERROR("EP manager initialization failed.\n");
		FTNM_EP_final(pCTX);
		FTNM_NODE_final(pCTX);
		return	nRet;
	}

	FTNM_DMC_init(&pCTX->xDMC);
	FTNM_DMC_loadConfig(&pCTX->xDMC, pConfigFileName);

	FTNM_SRV_init(&pCTX->xServer);
	FTNM_SRV_loadConfig(&pCTX->xServer, pConfigFileName);

	FTNM_SNMPC_init(&pCTX->xSNMPC);
	FTNM_SNMPC_loadConfig(&pCTX->xSNMPC, pConfigFileName);

	TRACE("FTNM initialization done.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTNM_final(FTNM_CONTEXT_PTR pCTX)
{
	FTNM_SNMPC_final(&pCTX->xSNMPC);
	FTNM_SRV_final(&pCTX->xServer);
	FTNM_DMC_final(&pCTX->xDMC);

	FTNM_EP_final(pCTX);
	FTNM_NODE_final(pCTX);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_showConfig(FTNM_CONTEXT_PTR pCTX)
{
	FTNM_DMC_CFG_show(&pCTX->xDMC.xConfig);
	FTNM_SRV_CFG_show(&pCTX->xServer.xConfig);
	FTNM_SNMPC_CFG_show(&pCTX->xSNMPC.xConfig);

	return	FTM_RET_OK;
}

FTM_RET FTNM_run(FTNM_CONTEXT_PTR pCTX)
{
	if (pthread_create(&pCTX->xPThread, NULL, FTNM_task, (FTM_VOID_PTR)pCTX) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET FTNM_waitingForFinished(FTNM_CONTEXT_PTR pCTX)
{
	pthread_join(pCTX->xPThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTNM_task(FTM_VOID_PTR pData)
{
	ASSERT (pData != NULL);
	
	FTNM_CONTEXT_PTR	pCTX = (FTNM_CONTEXT_PTR)pData;

	pCTX->xState = FTNM_STATE_CREATED;

	while(FTM_TRUE)
	{
		switch(pCTX->xState)
		{
		case	FTNM_STATE_CREATED:
			{
				FTNM_taskInit(pCTX);
			}
			break;

		case	FTNM_STATE_INITIALIZED:
			{
				FTNM_SRV_run(&pCTX->xServer);
				FTNM_taskConnect(pCTX);
			}
			break;

		case	FTNM_STATE_CONNECTED:
			{
				FTNM_taskSync(pCTX);
			}
			break;

		case	FTNM_STATE_SYNCHRONIZED:
			{
				FTNM_taskRunChild(pCTX);	
			}
			break;

		case	FTNM_STATE_PROCESS_FINISHED:
			{
				FTNM_taskWait(pCTX);	
			}
			break;
		}
		
		usleep(1000);
	}

	return	0;
}

FTM_RET	FTNM_taskInit(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTM_RET	nRet;

	TRACE("FTNM_taskInit\n");
	nRet = FTDMC_init(&pCTX->xDMC.xConfig);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}
	pCTX->xState = FTNM_STATE_INITIALIZED;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_taskConnect(FTNM_CONTEXT_PTR pCTX)
{
	FTM_RET			nRet;

	nRet = FTDMC_connect(&pCTX->xDMC.xSession, 
		inet_addr(pCTX->xDMC.xConfig.xNetwork.pServerIP),
		pCTX->xDMC.xConfig.xNetwork.usPort);
	if (nRet != FTM_RET_OK)
	{
		usleep(1000000);
		return	nRet;	
	}

	pCTX->xState = FTNM_STATE_CONNECTED;

	return	nRet;
}

FTM_RET	FTNM_taskSync(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTM_RET			nRet;
	FTM_ULONG		ulCount, i;

	nRet = FTDMC_NODE_INFO_count(&pCTX->xDMC.xSession, &ulCount);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_NODE_INFO	xNodeInfo;
		FTNM_NODE_PTR	pNode;

		nRet = FTDMC_NODE_INFO_getAt(&pCTX->xDMC.xSession, i, &xNodeInfo);	
		if (nRet != FTM_RET_OK)
		{
			ERROR("FTDMC_NODE_INFO_getAt(%08lx, %d, &xNodeInfo) = %08lx\n",
					pCTX->xDMC.xSession.hSock, i, nRet);
			continue;	
		}

		nRet = FTNM_NODE_create(pCTX, &xNodeInfo, &pNode);
		if (nRet != FTM_RET_OK)
		{
			ERROR("FTNM_NODE_create(xNode, &pNode) = %08lx\n", nRet);
			continue;	
		}

		TRACE("Node[%s] creating success.\n", pNode->xInfo.pDID);
	}

	nRet = FTDMC_EP_CLASS_INFO_count(&pCTX->xDMC.xSession, &ulCount);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_EP_CLASS_INFO	xEPClassInfo;

		nRet = FTDMC_EP_CLASS_INFO_getAt(&pCTX->xDMC.xSession, i, &xEPClassInfo);
		if (nRet != FTM_RET_OK)
		{
			ERROR("FTDMC_EP_CLASS_INFO_getAt(%08lx, %d, &xEPInfo) = %08lx\n",
					pCTX->xDMC.xSession.hSock, i, nRet);
			continue;
		}

		nRet = FTNM_EP_CLASS_INFO_create(&xEPClassInfo);
		if (nRet != FTM_RET_OK)
		{
			ERROR("FTNM_EP_CLASS_append(&xEPClassInfo) = %08lx\n", nRet);
			continue;	
		}
	}

	nRet = FTDMC_EP_INFO_count(&pCTX->xDMC.xSession, 0, &ulCount);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTNM_NODE_PTR	pNode;
		FTM_EP_INFO	xEPInfo;
		FTNM_EP_PTR	pEP;

		nRet = FTDMC_EP_INFO_getAt(&pCTX->xDMC.xSession, i, &xEPInfo);
		if (nRet != FTM_RET_OK)
		{
			ERROR("FTDMC_EP_INFO_getAt(%08lx, %d, &xEPInfo) = %08lx\n",
					pCTX->xDMC.xSession.hSock, i, nRet);
			continue;
		}

		nRet = FTNM_EP_create(pCTX, &xEPInfo, &pEP);
		if (nRet != FTM_RET_OK)
		{
			ERROR("FTNM_EP_create(xEP, &pNode) = %08lx\n", nRet);
			continue;	
		}

		if (FTNM_NODE_get(pCTX, xEPInfo.pDID, &pNode) == FTM_RET_OK)
		{
			FTNM_NODE_linkEP(pCTX, pNode, pEP);
		}
		
		TRACE("EP[%08lx] creating success.\n", pEP->xInfo.xEPID);
	}


	pCTX->xState = FTNM_STATE_SYNCHRONIZED;
	return	FTM_RET_OK;
}

FTM_RET	FTNM_taskRunChild(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTNM_NODE_PTR	pNode;
	FTM_ULONG		i, ulCount;

	FTNM_NODE_count(pCTX, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTNM_NODE_getAt(pCTX, i, &pNode) == FTM_RET_OK)
		{
			FTNM_NODE_run(pCTX, pNode);
			usleep(100000);
		}
	}
	
	pCTX->xState = FTNM_STATE_PROCESS_FINISHED;
	return	FTM_RET_OK;
}

FTM_RET			FTNM_taskWait(FTNM_CONTEXT_PTR pCTX)
{
	usleep(1000000);
	return	FTM_RET_OK;
}


