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

FTM_RET	FTNM_init(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTNM_EP_init();
	FTNM_NODE_init();
	FTNM_EP_CLASS_INFO_init();

	FTNM_DMC_init(&pCTX->xDMC);
	FTNM_SRV_init(&pCTX->xServer);
	FTNM_SNMPC_init(&pCTX->xSNMPC);

	TRACE("FTNM initialization done.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTNM_final(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTNM_SNMPC_final(&pCTX->xSNMPC);
	FTNM_SRV_final(&pCTX->xServer);
	FTNM_DMC_final(&pCTX->xDMC);

	FTNM_EP_CLASS_INFO_final();
	FTNM_NODE_final();
	FTNM_EP_final();

	return	FTM_RET_OK;
}

FTM_RET	FTNM_loadConfig(FTNM_CONTEXT_PTR pCTX, FTM_CHAR_PTR pFileName)
{
	ASSERT(pCTX != NULL);
	ASSERT(pFileName != NULL);

	FTNM_DMC_loadConfig(&pCTX->xDMC, pFileName);
	FTNM_SRV_loadConfig(&pCTX->xServer, pFileName);
	FTNM_SNMPC_loadConfig(&pCTX->xSNMPC, pFileName);

	TRACE("FTNM was loaded configuration.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTNM_showConfig(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTNM_DMC_showConfig(&pCTX->xDMC);
	FTNM_SRV_showConfig(&pCTX->xServer);
	FTNM_SNMPC_showConfig(&pCTX->xSNMPC);

	return	FTM_RET_OK;
}

FTM_RET FTNM_run(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	if (pthread_create(&pCTX->xPThread, NULL, FTNM_task, (FTM_VOID_PTR)pCTX) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET FTNM_waitingForFinished(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

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
	}

	return	0;
}

FTM_RET	FTNM_taskInit(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTM_RET	xRet;

	xRet = FTDMC_init(&pCTX->xDMC.xConfig);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}
	pCTX->xState = FTNM_STATE_INITIALIZED;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_taskConnect(FTNM_CONTEXT_PTR pCTX)
{
	FTM_RET			xRet;

	xRet = FTDMC_connect(&pCTX->xDMC.xSession, 
		inet_addr(pCTX->xDMC.xConfig.xNetwork.pServerIP),
		pCTX->xDMC.xConfig.xNetwork.usPort);
	if (xRet != FTM_RET_OK)
	{
		usleep(1000000);
		return	xRet;	
	}

	pCTX->xState = FTNM_STATE_CONNECTED;

	return	xRet;
}

FTM_RET	FTNM_taskSync(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTM_RET			xRet;
	FTM_ULONG		ulCount, i;

	xRet = FTDMC_NODE_INFO_count(&pCTX->xDMC.xSession, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_NODE_INFO	xNodeInfo;
		FTNM_NODE_PTR	pNode;

		xRet = FTDMC_NODE_INFO_getAt(&pCTX->xDMC.xSession, i, &xNodeInfo);	
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTDMC_NODE_INFO_getAt(%08lx, %d, &xNodeInfo) = %08lx\n",
					pCTX->xDMC.xSession.hSock, i, xRet);
			continue;	
		}

		xRet = FTNM_NODE_create(&xNodeInfo, &pNode);
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTNM_NODE_create(xNode, &pNode) = %08lx\n", xRet);
			continue;	
		}

		TRACE("Node[%s] creating success.\n", pNode->xInfo.pDID);
	}

	xRet = FTDMC_EP_CLASS_INFO_count(&pCTX->xDMC.xSession, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_EP_CLASS_INFO	xEPClassInfo;

		xRet = FTDMC_EP_CLASS_INFO_getAt(&pCTX->xDMC.xSession, i, &xEPClassInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTDMC_EP_CLASS_INFO_getAt(%08lx, %d, &xEPInfo) = %08lx\n",
					pCTX->xDMC.xSession.hSock, i, xRet);
			continue;
		}

		xRet = FTNM_EP_CLASS_INFO_create(&xEPClassInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTNM_EP_CLASS_append(&xEPClassInfo) = %08lx\n", xRet);
			continue;	
		}
	}

	xRet = FTDMC_EP_INFO_count(&pCTX->xDMC.xSession, 0, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTNM_NODE_PTR	pNode;
		FTM_EP_INFO	xEPInfo;
		FTNM_EP_PTR	pEP;

		xRet = FTDMC_EP_INFO_getAt(&pCTX->xDMC.xSession, i, &xEPInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTDMC_EP_INFO_getAt(%08lx, %d, &xEPInfo) = %08lx\n",
					pCTX->xDMC.xSession.hSock, i, xRet);
			continue;
		}

		xRet = FTNM_EP_create(&xEPInfo, &pEP);
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTNM_EP_create(xEP, &pNode) = %08lx\n", xRet);
			continue;	
		}

		if (FTNM_NODE_get(xEPInfo.pDID, &pNode) == FTM_RET_OK)
		{
			FTNM_NODE_linkEP(pNode, pEP);
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

	TRACE("FTNM_taskRunChild\n");
	FTNM_NODE_count(&ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTNM_NODE_getAt(i, &pNode) == FTM_RET_OK)
		{
			FTNM_NODE_run(pNode);
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


