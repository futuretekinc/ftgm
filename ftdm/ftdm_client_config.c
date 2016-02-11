#define	_XOPEN_SOURCE
#include <string.h>
#include "libconfig.h"
#include "ftdm_client_config.h"
#include "ftm_object.h"
#include "ftm_debug.h"
#include "ftm_error.h"

FTM_RET	FTDMC_CFG_init(FTDMC_CFG_PTR pConfig)
{
	if (pConfig == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	strcpy(pConfig->xNetwork.pServerIP, "127.0.0.1");
	pConfig->xNetwork.usPort = 8888;
	FTM_LIST_init(&pConfig->xDiagnostic.xEPList);
	return	FTM_RET_OK;
}

FTM_RET	FTDMC_CFG_load(FTDMC_CFG_PTR pConfig, FTM_CHAR_PTR pFileName)
{
	config_t			xConfig;
	config_setting_t	*pSection;
	config_setting_t 	*pTypeStringSetting;
	config_setting_t 	*pEPTestSetting;
	FTM_INT				i;

	if ((pConfig == NULL) || (pFileName == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}


	config_init(&xConfig);

	if (CONFIG_TRUE != config_read_file(&xConfig, pFileName))
	{
		ERROR("Configuration loading failed.[FILE = %s]\n", pFileName);
			return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	pTypeStringSetting = config_lookup(&xConfig, "type_string");
	if (pTypeStringSetting)
	{
		for( i = 0 ; i < config_setting_length(pTypeStringSetting) ; i++)
		{
			config_setting_t	*pElement;

			pElement = config_setting_get_elem(pTypeStringSetting, i);
			if (pElement != NULL)
			{
				FTM_INT		 nType = config_setting_get_int_elem(pElement, 0);	
				FTM_CHAR_PTR pTypeString = (FTM_CHAR_PTR)config_setting_get_string_elem(pElement, 1);	

				if (pTypeString != NULL)
				{
					FTM_appendEPTypeString(nType, pTypeString);	
				}
			}
		}
	}

	pSection = config_lookup(&xConfig, "default");
	if (pSection)
	{
		config_setting_t *pServerSetting;

		pServerSetting = config_setting_get_member(pSection, "server");
		if (pServerSetting)
		{
			config_setting_t *pIPSetting;
			config_setting_t *pPortSetting;

			pIPSetting = config_setting_get_member(pServerSetting, "ip");
			if (pIPSetting)
			{
				strncpy(pConfig->xNetwork.pServerIP, 
						config_setting_get_string(pIPSetting),
						sizeof(pConfig->xNetwork.pServerIP) - 1);	
			}

			pPortSetting = config_setting_get_member(pServerSetting, "port");
			if (pPortSetting)
			{
				pConfig->xNetwork.usPort = config_setting_get_int(pPortSetting);
			}
		}
	}

	pEPTestSetting = config_lookup(&xConfig, "ep_test");
	if (pEPTestSetting)
	{
		config_setting_t *pEPIDListSetting;
		config_setting_t *pStartTimeSetting;
		config_setting_t *pEndTimeSetting;

		pEPIDListSetting = config_setting_get_member(pEPTestSetting, "epid");
		if (pEPIDListSetting != 0)
		{
			FTM_LIST_init(&pConfig->xDiagnostic.xEPList);
			for( i = 0 ; i < config_setting_length(pEPIDListSetting) ; i++)
			{
				config_setting_t *pElement;
				
				pElement = config_setting_get_elem(pEPIDListSetting, i);	
				if (pElement != NULL)
				{
					FTM_INT	nEPID = config_setting_get_int(pElement);	
					FTM_LIST_append(&pConfig->xDiagnostic.xEPList, (void *)nEPID);
				}
			}
		}

		pStartTimeSetting = config_setting_get_member(pEPTestSetting, "start_time");
		if (pStartTimeSetting != 0)
		{
			strptime(config_setting_get_string(pStartTimeSetting), 
					"%Y-%m-%d %H:%M:%S", &pConfig->xDiagnostic.xStartTM);
		}
	
		pEndTimeSetting = config_setting_get_member(pEPTestSetting, "end_time");
		if (pEndTimeSetting != 0)
		{
			strptime(config_setting_get_string(pEndTimeSetting), 
					"%Y-%m-%d %H:%M:%S", &pConfig->xDiagnostic.xEndTM);
		}

		
	}
	

	config_destroy(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET FTDMC_CFG_final(FTDMC_CFG_PTR pConfig)
{
	if (pConfig == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	FTM_LIST_final(&pConfig->xDiagnostic.xEPList);

	return	FTM_RET_OK;
}
