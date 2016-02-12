#include <stdlib.h>
#include <string.h>
#include "ftnm.h"
#include "ftnm_config.h"
#include "libconfig.h"

FTM_RET	FTNM_CFG_SERVER_create(FTNM_CFG_SERVER_PTR _PTR_ ppConfig)
{
	ASSERT(ppConfig != NULL);

	FTNM_CFG_SERVER_PTR	pConfig;

	pConfig = FTM_MEM_calloc(1, sizeof(FTNM_CFG_SERVER));
	if (pConfig == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	FTNM_CFG_SERVER_init(pConfig);

	*ppConfig = pConfig;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CFG_SERVER_copyCreate(FTNM_CFG_SERVER_PTR _PTR_ ppConfig, FTNM_CFG_SERVER_PTR pOldConfig)
{
	ASSERT(pOldConfig != NULL);
	ASSERT(ppConfig != NULL);

	FTNM_CFG_SERVER_PTR	pConfig;

	pConfig = FTM_MEM_calloc(1, sizeof(FTNM_CFG_SERVER));
	if (pConfig == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	if (FTNM_CFG_SERVER_copy(pConfig, pOldConfig) != FTM_RET_OK)
	{
		FTM_MEM_free(pConfig);
		return	FTM_RET_INTERNAL_ERROR;
	}

	*ppConfig = pConfig;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CFG_SERVER_destroy(FTNM_CFG_SERVER_PTR pConfig)
{
	ASSERT(pConfig != NULL);

	FTNM_CFG_SERVER_final(pConfig);
	FTM_MEM_free(pConfig);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CFG_SERVER_init(FTNM_CFG_SERVER_PTR pConfig)
{
	ASSERT(pConfig != NULL);

	memset(pConfig, 0, sizeof(FTNM_CFG_SERVER));

	pConfig->usPort			= FTNM_DEFAULT_SERVER_PORT;
	pConfig->ulMaxSession	= FTNM_DEFAULT_SERVER_SESSION_COUNT	;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CFG_SERVER_final(FTNM_CFG_SERVER_PTR pConfig)
{
	return	FTM_RET_OK;
}

FTM_RET FTNM_CFG_SERVER_load(FTNM_CFG_SERVER_PTR pConfig, FTM_CHAR_PTR pFileName)
{
	ASSERT(pConfig != NULL);
	ASSERT(pFileName != NULL);

	config_t			xConfig;
	config_setting_t	*pSection;
	

	config_init(&xConfig);
	if (config_read_file(&xConfig, pFileName) == CONFIG_FALSE)
	{
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	pSection = config_lookup(&xConfig, "server");
	if (pSection != NULL)
	{
		config_setting_t	*pField;

		pField = config_setting_get_member(pSection, "max_session");
		if (pField != NULL)
		{
			pConfig->ulMaxSession = (FTM_ULONG)config_setting_get_int(pField);
		}
	
		pField = config_setting_get_member(pSection, "port");
		if (pField != NULL)
		{
			pConfig->usPort = (FTM_ULONG)config_setting_get_int(pField);
		}
	}

	config_destroy(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET FTNM_CFG_SERVER_copy(FTNM_CFG_SERVER_PTR pDestCfg, FTNM_CFG_SERVER_PTR pSrcCfg)
{
	ASSERT(pDestCfg != NULL);
	ASSERT(pSrcCfg != NULL);

	memcpy(pDestCfg, pSrcCfg, sizeof(FTNM_CFG_SERVER));

	return	FTM_RET_OK;
}

FTM_RET FTNM_CFG_SERVER_show(FTNM_CFG_SERVER_PTR pConfig)
{
	ASSERT(pConfig != NULL);

	MESSAGE("\n[ SERVER CONFIGURATION ]\n");
	MESSAGE("%16s : %d\n", "PORT", pConfig->usPort);
	MESSAGE("%16s : %lu\n", "MAX SESSION", pConfig->ulMaxSession);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CFG_DMC_create(FTDMC_CFG_PTR _PTR_ ppConfig)
{
	FTDMC_CFG_PTR	pConfig;

	ASSERT(ppConfig != NULL);

	pConfig = FTM_MEM_calloc(1, sizeof(FTDMC_CFG));
	if (pConfig == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	FTNM_CFG_DMC_init(pConfig);

	*ppConfig = pConfig;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CFG_DMC_copyCreate(FTDMC_CFG_PTR _PTR_ ppConfig, FTDMC_CFG_PTR pOldConfig)
{
	ASSERT(pOldConfig != NULL);
	ASSERT(ppConfig != NULL);

	FTDMC_CFG_PTR	pConfig;

	pConfig = FTM_MEM_calloc(1, sizeof(FTDMC_CFG));
	if (pConfig == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	if (FTNM_CFG_DMC_copy(pConfig, pOldConfig) != FTM_RET_OK)
	{
		FTM_MEM_free(pConfig);
		return	FTM_RET_INTERNAL_ERROR;
	}

	*ppConfig = pConfig;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CFG_DMC_destroy(FTDMC_CFG_PTR pConfig)
{
	ASSERT(pConfig != NULL);

	FTNM_CFG_DMC_final(pConfig);
	FTM_MEM_free(pConfig);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CFG_DMC_init(FTDMC_CFG_PTR pConfig)
{
	ASSERT(pConfig != NULL);

	memset(pConfig, 0, sizeof(FTDMC_CFG));

	strcpy(pConfig->xNetwork.pServerIP, FTDM_DEFAULT_SERVER_IP);
	pConfig->xNetwork.usPort = FTDM_DEFAULT_SERVER_PORT;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CFG_DMC_final(FTDMC_CFG_PTR pConfig)
{
	return	FTM_RET_OK;
}

FTM_RET FTNM_CFG_DMC_load(FTDMC_CFG_PTR pConfig, FTM_CHAR_PTR pFileName)
{
	ASSERT(pConfig != NULL);
	ASSERT(pFileName != NULL);

	config_t			xConfig;
	config_setting_t	*pSection;
	
	config_init(&xConfig);
	if (config_read_file(&xConfig, pFileName) == CONFIG_FALSE)
	{
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	pSection = config_lookup(&xConfig, "client");
	if (pSection != NULL)
	{
		config_setting_t	*pField;

		pField = config_setting_get_member(pSection, "server_ip");
		if (pField != NULL)
		{
			strncpy(pConfig->xNetwork.pServerIP, config_setting_get_string(pField), FTDMC_SERVER_IP_LEN);
		}
	
		pField = config_setting_get_member(pSection, "port");
		if (pField != NULL)
		{
			pConfig->xNetwork.usPort = (FTM_ULONG)config_setting_get_int(pField);
		}
	}

	config_destroy(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET FTNM_CFG_DMC_copy(FTDMC_CFG_PTR pDestCfg, FTDMC_CFG_PTR pSrcCfg)
{
	ASSERT(pDestCfg != NULL);
	ASSERT(pSrcCfg != NULL);

	memcpy(pDestCfg, pSrcCfg, sizeof(FTDMC_CFG));

	return	FTM_RET_OK;
}

FTM_RET FTNM_CFG_DMC_show(FTDMC_CFG_PTR pConfig)
{
	ASSERT(pConfig != NULL);

	MESSAGE("\n[ DATA MANAGER CONNECTION CONFIGURATION ]\n");
	MESSAGE("%16s : %s\n", "SERVER", pConfig->xNetwork.pServerIP);
	MESSAGE("%16s : %d\n", "PORT", pConfig->xNetwork.usPort);

	return	FTM_RET_OK;
}


FTM_RET	FTNM_CFG_SNMPC_create(FTNM_CFG_SNMPC_PTR _PTR_ ppConfig)
{
	ASSERT(ppConfig != NULL);

	FTNM_CFG_SNMPC_PTR	pConfig;

	pConfig = FTM_MEM_calloc(1, sizeof(FTNM_CFG_SNMPC));
	if (pConfig == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	FTNM_CFG_SNMPC_init(pConfig);

	*ppConfig = pConfig;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CFG_SNMPC_copyCreate(FTNM_CFG_SNMPC_PTR _PTR_ ppConfig, FTNM_CFG_SNMPC_PTR pOldConfig)
{
	ASSERT(pOldConfig != NULL);
	ASSERT(ppConfig != NULL);

	FTNM_CFG_SNMPC_PTR	pConfig;

	pConfig = FTM_MEM_calloc(1, sizeof(FTNM_CFG_SNMPC));
	if (pConfig == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	if (FTNM_CFG_SNMPC_copy(pConfig, pOldConfig) != FTM_RET_OK)
	{
		FTM_MEM_free(pConfig);
		return	FTM_RET_INTERNAL_ERROR;
	}

	*ppConfig = pConfig;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CFG_SNMPC_destroy(FTNM_CFG_SNMPC_PTR pConfig)
{
	ASSERT(pConfig != NULL);

	FTNM_CFG_SNMPC_final(pConfig);
	FTM_MEM_free(pConfig);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CFG_SNMPC_init(FTNM_CFG_SNMPC_PTR pConfig)
{
	ASSERT(pConfig != NULL);

	memset(pConfig, 0, sizeof(FTNM_CFG_SNMPC));

	strcpy(pConfig->pName, "ftnm");
	FTM_LIST_init(&pConfig->xMIBList);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CFG_SNMPC_final(FTNM_CFG_SNMPC_PTR pConfig)
{
	FTM_ULONG i, ulCount;

	FTM_LIST_count(&pConfig->xMIBList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_VOID_PTR pValue;

		if (FTM_LIST_getAt(&pConfig->xMIBList, i, &pValue) == FTM_RET_OK)
		{
			FTM_MEM_free(pValue);
		}
	}

	FTM_LIST_destroy(&pConfig->xMIBList);

	return	FTM_RET_OK;
}

FTM_RET FTNM_CFG_SNMPC_load(FTNM_CFG_SNMPC_PTR pConfig, FTM_CHAR_PTR pFileName)
{
	ASSERT(pConfig != NULL);
	ASSERT(pFileName != NULL);

	config_t			xConfig;
	config_setting_t	*pSection;
	
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
					printf("Hello\n");
					FTM_CHAR_PTR pBuff = (FTM_CHAR_PTR)FTM_MEM_malloc(strlen(pMIBFileName)+1);
					printf("Hello\n");
					if (pBuff != NULL)
					{
						strcpy(pBuff, pMIBFileName);
						FTM_LIST_append(&pConfig->xMIBList, pBuff);
					}
				}
			
			}
		
		}
	}
	config_destroy(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET FTNM_CFG_SNMPC_copy(FTNM_CFG_SNMPC_PTR pDestCfg, FTNM_CFG_SNMPC_PTR pSrcCfg)
{
	ASSERT(pDestCfg != NULL);
	ASSERT(pSrcCfg != NULL);

	memcpy(pDestCfg, pSrcCfg, sizeof(FTNM_CFG_SNMPC));

	return	FTM_RET_OK;
}

FTM_RET FTNM_CFG_SNMPC_show(FTNM_CFG_SNMPC_PTR pConfig)
{
	ASSERT(pConfig != NULL);

	FTM_ULONG	ulCount;

	if (FTM_LIST_count(&pConfig->xMIBList, &ulCount) == FTM_RET_OK)
	{
		FTM_ULONG i;

		MESSAGE("\n[ MIBs ]\n");
		for(i = 0 ; i < ulCount ; i++)
		{
			FTM_VOID_PTR	pValue;

			if (FTM_LIST_getAt(&pConfig->xMIBList, i, &pValue) == FTM_RET_OK)
			{
				MESSAGE("%16d : %s\n", i+1, (FTM_CHAR_PTR)pValue);
			}
		}
	}

	return	FTM_RET_OK;
}
