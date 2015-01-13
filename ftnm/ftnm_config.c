#include <stdlib.h>
#include <string.h>
#include "ftnm.h"
#include "ftnm_config.h"
#include "libconfig.h"

FTM_RET	FTNM_CFG_create(FTNM_CFG_PTR _PTR_ ppConfig)
{
	FTNM_CFG_PTR	pConfig;

	ASSERT(ppConfig != NULL);

	pConfig = FTM_MEM_calloc(1, sizeof(FTNM_CFG));
	if (pConfig == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	FTNM_CFG_init(pConfig);

	*ppConfig = pConfig;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CFG_copyCreate(FTNM_CFG_PTR _PTR_ ppConfig, FTNM_CFG_PTR pOldConfig)
{
	FTNM_CFG_PTR	pConfig;

	ASSERT((pOldConfig != NULL) && (ppConfig != NULL));

	pConfig = FTM_MEM_calloc(1, sizeof(FTNM_CFG));
	if (pConfig == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	if (FTNM_CFG_copy(pConfig, pOldConfig) != FTM_RET_OK)
	{
		FTM_MEM_free(pConfig);
		return	FTM_RET_INTERNAL_ERROR;
	}

	*ppConfig = pConfig;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CFG_destroy(FTNM_CFG_PTR pConfig)
{
	ASSERT(pConfig != NULL);

	FTNM_CFG_final(pConfig);
	FTM_MEM_free(pConfig);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CFG_init(FTNM_CFG_PTR pConfig)
{
	ASSERT(pConfig != NULL);

	strcpy(pConfig->xClient.xNetwork.pServerIP, "127.0.0.1");
	pConfig->xClient.xNetwork.usPort = 8888;

	pConfig->xServer.usPort			= 8889;
	pConfig->xServer.ulMaxSession	= 10;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CFG_final(FTNM_CFG_PTR pConfig)
{
	return	FTM_RET_OK;
}

FTM_RET FTNM_CFG_load(FTNM_CFG_PTR pConfig, FTM_CHAR_PTR pFileName)
{
	return	FTM_RET_OK;
}

FTM_RET FTNM_CFG_copy(FTNM_CFG_PTR pDestCfg, FTNM_CFG_PTR pSrcCfg)
{
	ASSERT((pDestCfg != NULL) && (pSrcCfg != NULL));

	memcpy(pDestCfg, pSrcCfg, sizeof(FTNM_CFG));

	return	FTM_RET_OK;
}

FTM_RET FTNM_CFG_show(FTNM_CFG_PTR pConfig)
{
	ASSERT(pConfig != NULL);

	MESSAGE("Server Configuration\n");
	MESSAGE("%16s : %d\n", "PORT", pConfig->xServer.usPort);
	MESSAGE("%16s : %lu\n", "MAX SESSION", pConfig->xServer.ulMaxSession);

	return	FTM_RET_OK;
}
