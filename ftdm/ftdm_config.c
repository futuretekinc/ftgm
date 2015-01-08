#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_object.h"
#include "ftdm_config.h"

extern char *program_invocation_short_name;

FTM_RET	FTDM_initServerConfig(FTDM_SERVER_CONFIG_PTR pServerConfig)
{
	FTM_CHAR	pBuff[1024];

	if (pServerConfig == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	memset(pServerConfig, 0, sizeof(FTDM_CONFIG));

	FTDM_initEPInfoConfig(&pServerConfig->xConfig.xEPInfo);
	FTDM_initEPClassInfoConfig(&pServerConfig->xConfig.xEPClassInfo);


	pServerConfig->xNetwork.usPort 		= FTDM_SERVER_DEFAULT_PORT;
	pServerConfig->xNetwork.ulMaxSession	= FTDM_SERVER_DEFAULT_MAX_SESSION;

	pServerConfig->xDebug.ulPrintOutMode = 2;
	sprintf(pBuff, "/var/log/%s", program_invocation_short_name);
	pServerConfig->xDebug.xTrace.pFileName = strdup(pBuff);
	pServerConfig->xDebug.xError.pFileName = strdup(pBuff);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_destroyServerConfig(FTDM_SERVER_CONFIG_PTR pConfig)
{
	FTDM_destroyEPInfoConfig(&pConfig->xConfig.xEPInfo);
	FTDM_destroyEPClassInfoConfig(&pConfig->xConfig.xEPClassInfo);

	if (pConfig->xConfig.xDatabase.pFileName != NULL)
	{
		free(pConfig->xConfig.xDatabase.pFileName);	
		pConfig->xConfig.xDatabase.pFileName = NULL;
	}

	if (pConfig->xDebug.xTrace.pFileName != NULL)
	{
		free(pConfig->xDebug.xTrace.pFileName);	
		pConfig->xDebug.xTrace.pFileName = NULL;
	}

	if (pConfig->xDebug.xError.pFileName != NULL)
	{
		free(pConfig->xDebug.xError.pFileName);	
		pConfig->xDebug.xError.pFileName = NULL;
	}

	return	FTM_RET_OK;
}

FTM_RET FTDM_loadServerConfig(FTDM_SERVER_CONFIG_PTR pConfig, FTM_CHAR_PTR pFileName)
{
	config_t			xConfig;
	config_setting_t	*pSection;

	config_init(&xConfig);

	if (CONFIG_TRUE != config_read_file(&xConfig, pFileName))
	{
		ERROR("Configuration loading failed.[FILE = %s]\n", pFileName);
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	pSection = config_lookup(&xConfig, "application");
	if (pSection)
	{
		config_setting_t *pDBSetting;
		config_setting_t *pNetworkSetting;
		config_setting_t *pDebugSetting;

		TRACE("Application section found.\n");

		pDBSetting = config_setting_get_member(pSection, "database");
		if (pDBSetting)
		{
			config_setting_t *pDBFileSetting;

			TRACE("DB setting found.\n");
			pDBFileSetting = config_setting_get_member(pDBSetting, "file");
			if (pDBFileSetting)
			{
				if (pConfig->xConfig.xDatabase.pFileName != 0)
				{	
					free(pConfig->xConfig.xDatabase.pFileName);
					pConfig->xConfig.xDatabase.pFileName = NULL;
				}

				pConfig->xConfig.xDatabase.pFileName = strdup(config_setting_get_string(pDBFileSetting));	
			}
		}

		pNetworkSetting = config_setting_get_member(pSection, "network");
		if (pNetworkSetting)
		{
			config_setting_t *pPortSetting;
			config_setting_t *pMaxSessionSetting;

			TRACE("Network setting found.\n");
			pPortSetting = config_setting_get_member(pNetworkSetting, "port");
			if (pPortSetting)
			{
				pConfig->xNetwork.usPort = config_setting_get_int(pPortSetting);
			}

			pMaxSessionSetting = config_setting_get_member(pNetworkSetting, "max_session");
			if (pPortSetting)
			{
				pConfig->xNetwork.ulMaxSession = config_setting_get_int(pMaxSessionSetting);
			}
		}

		pDebugSetting = config_setting_get_member(pSection, "debug");
		if (pDebugSetting)
		{
			config_setting_t	*pModeSetting;

			TRACE("Debug setting found.\n");
			pModeSetting = config_setting_get_member(pDebugSetting, "mode");
			if (pModeSetting)
			{
				pConfig->xDebug.ulPrintOutMode = (FTM_ULONG)config_setting_get_int(pModeSetting);
			}
		}
	}
	TRACE("Application section found.\n");

	pSection = config_lookup(&xConfig, "endpoint");
	if (pSection)
	{
		FTM_INT	i;
		config_setting_t	*pClassInfoSetting;

		pClassInfoSetting = config_setting_get_member(pSection, "class_info");
		for( i = 0 ; i < config_setting_length(pClassInfoSetting) ; i++)
		{
			config_setting_t	*pElement;	

			pElement = config_setting_get_elem(pClassInfoSetting, i);
			if (pElement != NULL)
			{
				config_setting_t		*pItem;	
				FTM_EP_CLASS_INFO_PTR	pEPClassInfo;

				pEPClassInfo = (FTM_EP_CLASS_INFO_PTR)calloc(1, sizeof(FTM_EP_CLASS_INFO));
				if (pEPClassInfo == NULL)
				{
					break;	
				}

				pItem = config_setting_get_member(pElement, "class");	
				if (pItem == NULL)
				{
					continue;
				}
				pEPClassInfo->xClass = (FTM_ULONG)config_setting_get_int(pItem);	
			
				pItem = config_setting_get_member(pElement, "id");
				if (pItem != NULL)
				{
					strncpy(pEPClassInfo->xOIDs.pID, config_setting_get_string(pItem), sizeof(pEPClassInfo->xOIDs.pID) - 1);
				}

				pItem = config_setting_get_member(pElement, "type");
				if (pItem != NULL)
				{
					strncpy(pEPClassInfo->xOIDs.pType, config_setting_get_string(pItem), sizeof(pEPClassInfo->xOIDs.pType) - 1);
				}

				pItem = config_setting_get_member(pElement, "name");
				if (pItem != NULL)
				{
					strncpy(pEPClassInfo->xOIDs.pName, config_setting_get_string(pItem), sizeof(pEPClassInfo->xOIDs.pName) - 1);
				}

				pItem = config_setting_get_member(pElement, "sn");
				if (pItem != NULL)
				{
					strncpy(pEPClassInfo->xOIDs.pSN, config_setting_get_string(pItem), sizeof(pEPClassInfo->xOIDs.pSN) - 1);
				}

				pItem = config_setting_get_member(pElement, "state");
				if (pItem != NULL)
				{
					strncpy(pEPClassInfo->xOIDs.pState, config_setting_get_string(pItem), sizeof(pEPClassInfo->xOIDs.pState) - 1);
				}

				pItem = config_setting_get_member(pElement, "value");
				if (pItem != NULL)
				{
					strncpy(pEPClassInfo->xOIDs.pValue, config_setting_get_string(pItem), sizeof(pEPClassInfo->xOIDs.pValue) - 1);
				}

				pItem = config_setting_get_member(pElement, "time");
				if (pItem != NULL)
				{
					strncpy(pEPClassInfo->xOIDs.pTime, config_setting_get_string(pItem), sizeof(pEPClassInfo->xOIDs.pTime) - 1);
				}
			}

		}

	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_showServerConfig(FTDM_SERVER_CONFIG_PTR pConfig)
{
	//FTDM_EP_TYPE_INFO_PTR	pTypeInfo;
	TRACE("############ CONFIGURATION #############\n");
	TRACE("[ APPLICATION ]\n");
	TRACE("[ NETWORK ]\n");
	TRACE("\t%-16s : %d\n", "PORT", pConfig->xNetwork.usPort);
	TRACE("\t%-16s : %d\n", "MAX SESSION", pConfig->xNetwork.ulMaxSession);
/*
	list_iterator_start(&pConfig->xConfig.xEP.xInfoList);
	while((pTypeInfo = list_iterator_next(&pConfig->xConfig.xEP.xInfoList)) != 0)
	{
		free(pTypeInfo);	
	}
	
	list_destroy(&pConfig->xConfig.xEP.xInfoList);
*/
	FTDM_showEPClassInfoConfig(&pConfig->xConfig.xEPClassInfo);	
	return	FTM_RET_OK;
}

FTM_RET	FTDM_initDBConfig(FTDM_DB_CONFIG_PTR pConfig)
{
	FTM_CHAR	pBuff[1024];

	if (pConfig == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	/* set default configuration */
	sprintf(pBuff, "%s.db", program_invocation_short_name);
	pConfig->pFileName = strdup(pBuff);

	return	FTM_RET_OK;
}

FTM_RET FTDM_destroyDBConfig(FTDM_DB_CONFIG_PTR pConfig)
{
	if (pConfig == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	if (pConfig->pFileName != NULL)
	{
		free(pConfig->pFileName);
		pConfig->pFileName = NULL;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_initEPInfoConfig(FTDM_EP_INFO_CONFIG_PTR pConfig)
{
	if (pConfig == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	list_init(&pConfig->xInfos);

	return	FTM_RET_OK;
}

FTM_RET FTDM_destroyEPInfoConfig(FTDM_EP_INFO_CONFIG_PTR pConfig)
{
	FTM_EP_INFO_PTR	pInfo;

	if (pConfig == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	list_iterator_start(&pConfig->xInfos);
	while((pInfo = list_iterator_next(&pConfig->xInfos)) != 0)
	{
		free(pInfo);	
	}
	
	list_destroy(&pConfig->xInfos);

	return	FTM_RET_OK;
}

FTM_RET FTDM_initEPClassInfoConfig(FTDM_EP_CLASS_INFO_CONFIG_PTR pConfig)
{
	if (pConfig == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	
	list_init(&pConfig->xInfos);

	return	FTM_RET_OK;
}

FTM_RET FTDM_destroyEPClassInfoConfig(FTDM_EP_CLASS_INFO_CONFIG_PTR pConfig)
{
	FTM_EP_CLASS_INFO_PTR	pInfo;

	if (pConfig == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	list_iterator_start(&pConfig->xInfos);
	while((pInfo = list_iterator_next(&pConfig->xInfos)) != 0)
	{
		free(pInfo);	
	}
	
	list_destroy(&pConfig->xInfos);

	return	FTM_RET_OK;
}

FTM_RET FTDM_showEPClassInfoConfig(FTDM_EP_CLASS_INFO_CONFIG_PTR pConfig)
{
	FTM_EP_CLASS_INFO_PTR pInfo;
	
	if (pConfig == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	TRACE("\n[ END POINT CLASS INFORMATION ]\n");
		TRACE("\t %08s %-16s %-16s %-16s %-16s %-16s %-16s %-16s\n", 
			"CLASS", "ID", "TYPE", "NAME", "SN", "STATE", "VALUE", "TIME");
	list_iterator_start(&pConfig->xInfos);
	while((pInfo = list_iterator_next(&pConfig->xInfos)) != 0)
	{
		TRACE("\t %08lx %-16s %-16s %-16s %-16s %-16s %-16s %-16s\n", 
			pInfo->xClass, 
			pInfo->xOIDs.pID,
			pInfo->xOIDs.pType,
			pInfo->xOIDs.pName,
			pInfo->xOIDs.pSN,
			pInfo->xOIDs.pState,
			pInfo->xOIDs.pValue,
			pInfo->xOIDs.pTime);
	}

	return	FTM_RET_OK;
}
