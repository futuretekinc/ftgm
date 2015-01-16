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

	memset(pConfig, 0, sizeof(FTNM_CFG));

	strcpy(pConfig->xClient.xNetwork.pServerIP, FTDM_DEFAULT_SERVER_IP);
	pConfig->xClient.xNetwork.usPort = FTDM_DEFAULT_SERVER_PORT;

	pConfig->xServer.usPort			= FTNM_DEFAULT_SERVER_PORT;
	pConfig->xServer.ulMaxSession	= FTNM_DEFAULT_SERVER_SESSION_COUNT	;

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

	pSection = config_lookup(&xConfig, "server");
	if (pSection != NULL)
	{
		config_setting_t	*pField;

		pField = config_setting_get_member(pSection, "max_session");
		if (pField != NULL)
		{
			pConfig->xServer.ulMaxSession = (FTM_ULONG)config_setting_get_int(pField);
		}
	
		pField = config_setting_get_member(pSection, "port");
		if (pField != NULL)
		{
			pConfig->xServer.usPort = (FTM_ULONG)config_setting_get_int(pField);
		}
	}

	pSection = config_lookup(&xConfig, "client");
	if (pSection != NULL)
	{
		config_setting_t	*pField;

		pField = config_setting_get_member(pSection, "server_ip");
		if (pField != NULL)
		{
			strncpy(pConfig->xClient.xNetwork.pServerIP, config_setting_get_string(pField), FTDMC_SERVER_IP_LEN);
		}
	
		pField = config_setting_get_member(pSection, "port");
		if (pField != NULL)
		{
			pConfig->xClient.xNetwork.usPort = (FTM_ULONG)config_setting_get_int(pField);
		}
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
	FTM_ULONG	ulCount;

	ASSERT(pConfig != NULL);

	MESSAGE("\n[ SERVER CONFIGURATION ]\n");
	MESSAGE("%16s : %d\n", "PORT", pConfig->xServer.usPort);
	MESSAGE("%16s : %lu\n", "MAX SESSION", pConfig->xServer.ulMaxSession);

	MESSAGE("\n[ DATA MANAGER CONNECTION CONFIGURATION ]\n");
	MESSAGE("%16s : %s\n", "SERVER", pConfig->xClient.xNetwork.pServerIP);
	MESSAGE("%16s : %d\n", "PORT", pConfig->xClient.xNetwork.usPort);

	if (FTM_LIST_count(&pConfig->xSNMPC.xMIBList, &ulCount) == FTM_RET_OK)
	{
		FTM_ULONG i;

		MESSAGE("\n[ MIBs ]\n");
		for(i = 0 ; i < ulCount ; i++)
		{
			FTM_VOID_PTR	pValue;

			if (FTM_LIST_getAt(&pConfig->xSNMPC.xMIBList, i, &pValue) == FTM_RET_OK)
			{
				MESSAGE("%16d : %s\n", i+1, (FTM_CHAR_PTR)pValue);
			}
		}
	}

	return	FTM_RET_OK;
}
