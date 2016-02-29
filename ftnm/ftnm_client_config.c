#define	_XOPEN_SOURCE
#include <string.h>
#include "libconfig.h"
#include "ftm.h"
#include "ftnm_client_config.h"

FTM_RET	FTNMC_initConfig(FTNM_CFG_CLIENT_PTR pConfig)
{
	if (pConfig == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	strcpy(pConfig->xNetwork.pServerIP, "127.0.0.1");
	pConfig->xNetwork.usPort = 8888;

	return	FTM_RET_OK;
}

FTM_RET	FTNMC_loadConfig(FTNM_CFG_CLIENT_PTR pConfig, FTM_CHAR_PTR pFileName)
{
	config_t			xConfig;
	config_setting_t	*pSection;

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

	config_destroy(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET FTNMC_finalConfig(FTNM_CFG_CLIENT_PTR pConfig)
{
	if (pConfig == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	return	FTM_RET_OK;
}
