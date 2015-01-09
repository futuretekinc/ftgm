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

	FTM_LIST_init(&pConfig->xSNMPC.xMIBList);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CFG_final(FTNM_CFG_PTR pConfig)
{
	FTM_ULONG i, ulCount;

	FTM_LIST_count(&pConfig->xSNMPC.xMIBList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_VOID_PTR pValue;

		if (FTM_LIST_getAt(&pConfig->xSNMPC.xMIBList, i, &pValue) == FTM_RET_OK)
		{
			FTM_MEM_free(pValue);
		}
	}

	FTM_LIST_destroy(&pConfig->xSNMPC.xMIBList);

	return	FTM_RET_OK;
}

FTM_RET FTNM_CFG_load(FTNM_CFG_PTR pConfig, FTM_CHAR_PTR pFileName)
{
	config_t			xConfig;
	config_setting_t	*pSection;
	
	ASSERT((pConfig != NULL) && (pFileName != NULL));

	config_init(&xConfig);
	if (config_read_file(&xConfig, pFileName) == CONFIG_FALSE)
	{
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	pSection = config_lookup(&xConfig, "snmpc");
	if (pSection != NULL)
	{
		config_setting_t	*pField;

		pField = config_setting_get_member(pSection, "mibs");
		if (pField != NULL)
		{
			FTM_INT	i, nCount = config_setting_length(pField);
			
			for(i = 0 ; i < nCount ; i++)
			{
				const char * pMIBFileName = config_setting_get_string_elem(pField, i);	
				if (pMIBFileName != NULL)
				{
					FTM_CHAR_PTR pBuff = (FTM_CHAR_PTR)FTM_MEM_malloc(strlen(pMIBFileName)+1);
					if (pBuff != NULL)
					{
						strcpy(pBuff, pMIBFileName);
						FTM_LIST_append(&pConfig->xSNMPC.xMIBList, pBuff);
					}
				}
			
			}
		
		}
	}
	config_destroy(&xConfig);

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
