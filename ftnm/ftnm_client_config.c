#define	_XOPEN_SOURCE
#include <string.h>
#include "libconfig.h"
#include "ftm.h"
#include "ftnm_client_config.h"

static FTNM_CFG_CLIENT _defaultConfig =
{
	.xNetwork =
	{
		.pServerIP  = FTNM_DEFAULT_SERVER_IP,
		.usPort		= FTNM_DEFAULT_SERVER_PORT
	}
};

FTM_RET	FTNMC_loadConfig(FTNM_CFG_CLIENT_PTR pConfig, FTM_CHAR_PTR pFileName)
{
	ASSERT(pConfig != NULL);
	ASSERT(pFileName != NULL);

	config_t			xConfig;
	config_setting_t	*pSection;

	memcpy(pConfig, &_defaultConfig, sizeof(FTNM_CFG_CLIENT));

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

