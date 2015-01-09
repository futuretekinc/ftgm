#include <stdlib.h>
#include <string.h>
#include "ftnm.h"
#include "ftnm_config.h"
#include "libconfig.h"

FTM_RET	FTNM_CFG_init(FTNM_CFG_PTR pConfig)
{
	ASSERT(pConfig != NULL);

	strcpy(pConfig->xClient.xNetwork.pServerIP, "127.0.0.1");
	pConfig->xClient.xNetwork.usPort = 8888;

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
	memcpy(pDestCfg, pSrcCfg, sizeof(FTNM_CFG));

	return	FTM_RET_OK;
}

FTM_RET FTNM_CFG_show(FTNM_CFG_PTR pConfig)
{
	return	FTM_RET_OK;
}
