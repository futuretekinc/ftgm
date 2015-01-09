#include "ftnm.h"
#include "ftdm_client.h"

FTDMC_SESSION	xClientSession;

FTM_RET	FTNM_init(FTNM_CONTEXT_PTR pContext, FTNM_CFG_PTR pConfig)
{
	FTM_RET	nRet;

	ASSERT((pContext != NULL) && (pConfig != NULL));

	nRet = FTNM_NODE_MNGR_init();
	if (nRet != FTM_RET_OK)
	{
		return	nRet;
	}

	nRet = FTNM_EP_init();
	if (nRet != FTM_RET_OK)
	{
		FTNM_NODE_MNGR_final();
		return	nRet;
	}

	nRet = FTDMC_init(&pConfig->xClient);
	if (nRet != FTM_RET_OK)
	{
		FTNM_EP_final();
		FTNM_NODE_MNGR_final();
		return	nRet;
	}

	pContext->pConfig = pConfig;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_final(FTNM_CONTEXT_PTR pContext)
{
	FTDMC_final();
	FTNM_EP_final();
	FTNM_NODE_MNGR_final();

	return	FTM_RET_OK;
}

FTM_RET	FTNM_run(FTNM_CONTEXT_PTR pContext)
{
	while(1)
	{
	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_connectDM(FTNM_CONTEXT_PTR pContext)
{
	FTM_RET			nRet;

	nRet = FTDMC_connect(&pContext->xSession, 
		inet_addr(pContext->pConfig->xClient.xNetwork.pServerIP),
		pContext->pConfig->xClient.xNetwork.usPort);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	return	nRet;
}

