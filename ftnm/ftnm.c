#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "ftnm.h"
#include "ftdm_client.h"
#include "ftnm_node_snmpc.h"
#include "ftnm_server.h"

FTM_VOID_PTR	FTNM_task(FTM_VOID_PTR pData);
FTM_RET			FTNM_taskInit(FTNM_DMC_PTR pDMC);
FTM_RET			FTNM_taskConnect(FTNM_DMC_PTR pDMC);
FTM_RET			FTNM_taskSync(FTNM_DMC_PTR pDMC);
FTM_RET			FTNM_taskRunChild(FTNM_DMC_PTR pDMC);
FTM_RET			FTNM_taskWait(FTNM_DMC_PTR pDMC);

static	FTNM_CFG_SERVER xServerConfig;
static	FTNM_CFG_SNMPC	xSNMPCConfig;
static	FTNM_DMC		xDMC;	

FTM_RET	FTNM_init(FTM_CHAR_PTR pConfigFileName)
{
	FTM_RET	nRet;

	memset(&xServerConfig, 0, sizeof(FTNM_CFG_SERVER));
	memset(&xSNMPCConfig, 0, sizeof(FTNM_CFG_SNMPC));
	memset(&xDMC, 0, sizeof(FTNM_DMC));

	FTNM_CFG_SERVER_init(&xServerConfig);
	FTNM_CFG_DMC_init(&xDMC.xConfig);
	FTNM_CFG_SNMPC_init(&xSNMPCConfig);

	FTNM_CFG_SERVER_load(&xServerConfig, pConfigFileName);
	FTNM_CFG_DMC_load(&xDMC.xConfig, pConfigFileName);
	FTNM_CFG_SNMPC_load(&xSNMPCConfig, pConfigFileName);

	nRet = FTNM_NODE_init();
	if (nRet != FTM_RET_OK)
	{
		ERROR("Node manager initialization failed.\n");
		return	nRet;
	}

	nRet = FTNM_EP_init();
	if (nRet != FTM_RET_OK)
	{
		ERROR("EP manager initialization failed.\n");
		FTNM_NODE_final();
		return	nRet;
	}

	nRet = FTNM_EP_CLASS_INFO_init();
	if (nRet != FTM_RET_OK)
	{
		ERROR("EP manager initialization failed.\n");
		FTNM_EP_final();
		FTNM_NODE_final();
		return	nRet;
	}

	FTNM_SNMPC_init(&xSNMPCConfig);

	TRACE("FTNM initialization done.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTNM_final(void)
{
	FTNM_CFG_SERVER_final(&xServerConfig);
	FTNM_CFG_DMC_final(&xDMC.xConfig);
	FTNM_CFG_SNMPC_final(&xSNMPCConfig);

	FTNM_EP_final();
	FTNM_NODE_final();

	return	FTM_RET_OK;
}

FTM_RET	FTNM_showConfig(FTM_VOID)
{
	FTNM_CFG_SERVER_show(&xServerConfig);
	FTNM_CFG_DMC_show(&xDMC.xConfig);
	FTNM_CFG_SNMPC_show(&xSNMPCConfig);

	return	FTM_RET_OK;
}

FTM_RET FTNM_run(void)
{
	pthread_t	xServer;

	FTNMS_run(&xServerConfig, &xServer);

	if (pthread_create(&xDMC.xThread, NULL, FTNM_task, (FTM_VOID_PTR)&xDMC) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	pthread_join(xDMC.xThread, NULL);
	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTNM_task(FTM_VOID_PTR pData)
{
	ASSERT (pData != NULL);

	FTNM_DMC_PTR	pDMC = (FTNM_DMC_PTR)pData;

	pDMC->xState = FTNM_STATE_CREATED;

	while(1)
	{
		switch(pDMC->xState)
		{
		case	FTNM_STATE_CREATED:
			{
				FTNM_taskInit(pDMC);
			}
			break;

		case	FTNM_STATE_INITIALIZED:
			{
				FTNM_taskConnect(pDMC);
			}
			break;

		case	FTNM_STATE_CONNECTED:
			{
				FTNM_taskSync(pDMC);
			}
			break;

		case	FTNM_STATE_SYNCHRONIZED:
			{
				FTNM_taskRunChild(pDMC);	
			}
			break;

		case	FTNM_STATE_PROCESS_FINISHED:
			{
				FTNM_taskWait(pDMC);	
			}
			break;
		}
		
		usleep(1000);
	}

	return	0;
}

FTM_RET	FTNM_taskInit(FTNM_DMC_PTR pDMC)
{
	FTM_RET	nRet;

	ASSERT(pDMC != NULL);

	TRACE("FTNM_taskInit\n");
	nRet = FTDMC_init(&pDMC->xConfig);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}
	pDMC->xState = FTNM_STATE_INITIALIZED;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_taskConnect(FTNM_DMC_PTR pDMC)
{
	FTM_RET			nRet;

	nRet = FTDMC_connect(&pDMC->xSession, 
		inet_addr(pDMC->xConfig.xNetwork.pServerIP),
		pDMC->xConfig.xNetwork.usPort);
	if (nRet != FTM_RET_OK)
	{
		usleep(1000000);
		return	nRet;	
	}

	pDMC->xState = FTNM_STATE_CONNECTED;

	return	nRet;
}

FTM_RET	FTNM_taskSync(FTNM_DMC_PTR pDMC)
{
	FTM_RET			nRet;
	FTM_ULONG		ulCount, i;

	ASSERT(pDMC != NULL);

	nRet = FTDMC_NODE_INFO_count(&pDMC->xSession, &ulCount);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_NODE_INFO	xNodeInfo;
		FTNM_NODE_PTR	pNode;

		nRet = FTDMC_NODE_INFO_getAt(&pDMC->xSession, i, &xNodeInfo);	
		if (nRet != FTM_RET_OK)
		{
			ERROR("FTDMC_NODE_INFO_getAt(%08lx, %d, &xNodeInfo) = %08lx\n",
					pDMC->xSession.hSock, i, nRet);
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

	nRet = FTDMC_EP_CLASS_INFO_count(&pDMC->xSession, &ulCount);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_EP_CLASS_INFO	xEPClassInfo;

		nRet = FTDMC_EP_CLASS_INFO_getAt(&pDMC->xSession, i, &xEPClassInfo);
		if (nRet != FTM_RET_OK)
		{
			ERROR("FTDMC_EP_CLASS_INFO_getAt(%08lx, %d, &xEPInfo) = %08lx\n",
					pDMC->xSession.hSock, i, nRet);
			continue;
		}

		nRet = FTNM_EP_CLASS_INFO_create(&xEPClassInfo);
		if (nRet != FTM_RET_OK)
		{
			ERROR("FTNM_EP_CLASS_append(&xEPClassInfo) = %08lx\n", nRet);
			continue;	
		}
	}

	nRet = FTDMC_EP_INFO_count(&pDMC->xSession, 0, &ulCount);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTNM_NODE_PTR	pNode;
		FTM_EP_INFO	xEPInfo;
		FTNM_EP_PTR	pEP;

		nRet = FTDMC_EP_INFO_getAt(&pDMC->xSession, i, &xEPInfo);
		if (nRet != FTM_RET_OK)
		{
			ERROR("FTDMC_EP_INFO_getAt(%08lx, %d, &xEPInfo) = %08lx\n",
					pDMC->xSession.hSock, i, nRet);
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


	pDMC->xState = FTNM_STATE_SYNCHRONIZED;
	return	FTM_RET_OK;
}

FTM_RET	FTNM_taskRunChild(FTNM_DMC_PTR pDMC)
{
	ASSERT(pDMC != NULL);

	FTNM_NODE_PTR	pNode;
	FTM_ULONG		i, ulCount;

	FTNM_NODE_count(&ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTNM_NODE_getAt(i, &pNode) == FTM_RET_OK)
		{
			FTNM_NODE_run(pNode);
			usleep(100000);
		}
	}
	
	pDMC->xState = FTNM_STATE_PROCESS_FINISHED;
	return	FTM_RET_OK;
}

FTM_RET			FTNM_taskWait(FTNM_DMC_PTR pDMC)
{
	usleep(1000000);
	return	FTM_RET_OK;
}

FTM_RET	FTNM_DMC_EP_DATA_set(FTNM_EP_PTR pEP)
{
	return	FTDMC_EP_DATA_append(&xDMC.xSession, pEP->xInfo.xEPID, &pEP->xData);
}

FTM_RET FTNM_DMC_EP_DATA_setINT(FTM_EPID xEPID, FTM_ULONG ulTime, FTM_INT nValue)
{
	FTM_EP_DATA	xEPData;

	xEPData.ulTime = ulTime;
	xEPData.xType = FTM_EP_DATA_TYPE_INT;
	xEPData.xValue.nValue = nValue;

	return FTDMC_EP_DATA_append(&xDMC.xSession, xEPID, &xEPData);
}

FTM_RET FTNM_DMC_EP_DATA_setULONG(FTM_EPID xEPID, FTM_ULONG ulTime, FTM_ULONG ulValue)
{
	FTM_EP_DATA	xEPData;

	xEPData.ulTime = ulTime;
	xEPData.xType = FTM_EP_DATA_TYPE_ULONG;
	xEPData.xValue.ulValue = ulValue;

	return FTDMC_EP_DATA_append(&xDMC.xSession, xEPID, &xEPData);
}

FTM_RET FTNM_DMC_EP_DATA_setFLOAT(FTM_EPID xEPID, FTM_ULONG ulTime, FTM_DOUBLE fValue)
{
	FTM_EP_DATA	xEPData;

	xEPData.ulTime = ulTime;
	xEPData.xType = FTM_EP_DATA_TYPE_FLOAT;
	xEPData.xValue.fValue = fValue;

	return FTDMC_EP_DATA_append(&xDMC.xSession, xEPID, &xEPData);
}

FTM_RET	FTNM_DMC_EP_DATA_count(FTM_EPID xEPID, FTM_ULONG_PTR pulCount)
{
	return	FTDMC_EP_DATA_count(&xDMC.xSession, xEPID, pulCount);
}

FTM_RET FTNM_DMC_EP_DATA_info
(
	FTM_EPID 		xEPID, 
	FTM_ULONG_PTR 	pulBeginTime, 
	FTM_ULONG_PTR 	pulEndTime, 
	FTM_ULONG_PTR 	pulCount
)
{
	return	FTDMC_EP_DATA_info(&xDMC.xSession, xEPID, pulBeginTime, pulEndTime, pulCount);
}


