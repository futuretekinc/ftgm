#include <stdlib.h>
#include <string.h>
#include "ftm_debug.h"
#include "ftnm_config.h"
#include "libconfig.h"

static FTNM_CONFIG_PTR	_pAppConfig = NULL;

FTM_RET FTNM_configInit(void)
{
	_pAppConfig = (FTNM_CONFIG_PTR)calloc(1, sizeof(FTNM_CONFIG));
	if (_pAppConfig == NULL)
	{
		return FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	list_init(&_pAppConfig->xEPList);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_configFinal(void)
{
	if (_pAppConfig != NULL)
	{
		list_destroy(&_pAppConfig->xEPList);

		if (_pAppConfig->pName != NULL)
		{
			free(_pAppConfig->pName);	
		}

		free(_pAppConfig);
		_pAppConfig = NULL;
	}

	return	FTM_RET_OK;
}

FTM_RET FTNM_configLoad(FTM_CHAR_PTR pFileName)
{
	config_t			xConfig;
	config_setting_t	*pEPInfoConfigs;
	FTM_INT				i;

	if (_pAppConfig == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	config_init(&xConfig);

	if (CONFIG_TRUE != config_read_file(&xConfig, pFileName))
	{
		return	FTM_RET_CONFIG_LOAD_FAILED;	
	}

	pEPInfoConfigs = config_lookup(&xConfig, "ep_infos");
	for(i = 0 ; ; i++)
	{
		config_setting_t	*pEPInfoConfig;
		FTNM_EP_INFO_PTR	pEPInfo;
		FTM_CHAR_PTR		pType;
		FTM_CHAR_PTR		pOIDPrefix;

		pEPInfoConfig = config_setting_get_elem(pEPInfoConfigs, i);
		if (pEPInfoConfig == NULL)
		{
			break;	
		}

		if (CONFIG_TRUE != config_setting_lookup_string(pEPInfoConfig, "type", (const char **)&pType))
		{
			continue;
		}

		if (CONFIG_TRUE != config_setting_lookup_string(pEPInfoConfig, "oid_prefix", (const char **)&pOIDPrefix))
		{
			continue;
		}

		pEPInfo = (FTNM_EP_INFO_PTR)calloc(1, sizeof(FTNM_EP_INFO));
		if (pEPInfo == NULL)
		{
			break;	
		}

		pEPInfo->pType 		= strdup(pType);
		pEPInfo->pOIDPrefix = strdup(pOIDPrefix);

		
		list_append(&_pAppConfig->xEPList, pEPInfo);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_getAppName(FTM_CHAR_PTR pBuff, FTM_INT nLen)
{
	if (_pAppConfig == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;
	}

	strncpy(pBuff, _pAppConfig->pName, nLen);

	return	FTM_RET_OK;
}
