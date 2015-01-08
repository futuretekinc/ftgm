#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_object.h"
#include "ftdm_server_config.h"

extern char *program_invocation_short_name;

static int	_FTDMS_seekEPOIDInfo(const void *pElement, const void *pKey);

FTM_RET	FTDMS_initConfig(FTDM_SERVER_CONFIG_PTR pConfig)
{
	if (pConfig == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	memset(pConfig, 0, sizeof(FTDM_SERVER_CONFIG));

	list_init(&pConfig->xEP.xInfoList);
	list_init(&pConfig->xEP.xClassInfoList);
	list_attributes_seeker(&pConfig->xEP.xClassInfoList, _FTDMS_seekEPOIDInfo);

	/* set default configuration */
	sprintf(pConfig->xDatabase.pFileName, "%s.db", program_invocation_short_name);
	pConfig->xNetwork.usPort 		= FTDM_SERVER_DEFAULT_PORT;
	pConfig->xNetwork.ulMaxSession	= FTDM_SERVER_DEFAULT_MAX_SESSION;

	pConfig->xDebug.ulPrintOutMode = 2;
	strncpy(pConfig->xDebug.xTrace.pPath, "/var/log/", sizeof(pConfig->xDebug.xTrace.pPath) - 1);
	strncpy(pConfig->xDebug.xTrace.pPrefix, program_invocation_short_name, sizeof(pConfig->xDebug.xTrace.pPrefix) - 1);
	strncpy(pConfig->xDebug.xError.pPath, "/var/log/", sizeof(pConfig->xDebug.xError.pPath) - 1);
	strncpy(pConfig->xDebug.xError.pPrefix, program_invocation_short_name, sizeof(pConfig->xDebug.xError.pPrefix) - 1);

	return	FTM_RET_OK;
}

FTM_RET	FTDMS_destroyConfig(FTDM_SERVER_CONFIG_PTR pConfig)
{
	FTDM_EP_TYPE_INFO_PTR	pTypeInfo;
	FTM_EP_CLASS_INFO_PTR		pOIDInfo;

	list_iterator_start(&pConfig->xEP.xInfoList);
	while((pTypeInfo = list_iterator_next(&pConfig->xEP.xInfoList)) != 0)
	{
		free(pTypeInfo);	
	}
	
	list_destroy(&pConfig->xEP.xInfoList);

	list_iterator_start(&pConfig->xEP.xClassInfoList);
	while((pOIDInfo = list_iterator_next(&pConfig->xEP.xClassInfoList)) != 0)
	{
		free(pOIDInfo);	
	}
	
	list_destroy(&pConfig->xEP.xClassInfoList);

	return	FTM_RET_OK;
}

FTM_RET FTDMS_loadConfig(FTDM_SERVER_CONFIG_PTR pConfig, FTM_CHAR_PTR pFileName)
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
				strcpy(pConfig->xDatabase.pFileName, 
					config_setting_get_string(pDBFileSetting));	
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
		config_setting_t	*pTypeStringSetting;
		config_setting_t	*pClassInfoSetting;

		pTypeStringSetting = config_setting_get_member(pSection, "type_name");
		for( i = 0 ; i < config_setting_length(pTypeStringSetting) ; i++)
		{
			config_setting_t	*pElement;

			pElement = config_setting_get_elem(pTypeStringSetting, i);
			if (pElement != NULL)
			{
				FTDM_EP_TYPE_INFO_PTR	pTypeInfo;

				pTypeInfo = (FTDM_EP_TYPE_INFO_PTR)calloc(1, sizeof(FTDM_EP_TYPE_INFO));
				if (pTypeInfo != 0)
				{
					pTypeInfo->ulType = (FTM_ULONG)config_setting_get_int_elem(pElement, 0);	
					strncpy(pTypeInfo->pName, config_setting_get_string_elem(pElement, 1), FTDM_TYPE_NAME_LEN);	

					list_append(&pConfig->xEP.xInfoList, pTypeInfo);
				}
			}
		}

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


				list_append(&pConfig->xEP.xClassInfoList, pEPClassInfo);

			}

		}

	}

	return	FTM_RET_OK;
}

FTM_RET	FTDMS_showConfig(FTDM_SERVER_CONFIG_PTR pConfig)
{
	//FTDM_EP_TYPE_INFO_PTR	pTypeInfo;
	FTM_EP_CLASS_INFO_PTR		pEPClassInfo;

	TRACE("############ CONFIGURATION #############\n");
	TRACE("[ APPLICATION ]\n");
	TRACE("[ NETWORK ]\n");
	TRACE("\t%-16s : %d\n", "PORT", pConfig->xNetwork.usPort);
	TRACE("\t%-16s : %d\n", "MAX SESSION", pConfig->xNetwork.ulMaxSession);
	TRACE("[ END POINT ]\n");
/*
	list_iterator_start(&pConfig->xEP.xInfoList);
	while((pTypeInfo = list_iterator_next(&pConfig->xEP.xInfoList)) != 0)
	{
		free(pTypeInfo);	
	}
	
	list_destroy(&pConfig->xEP.xInfoList);
*/
	list_iterator_start(&pConfig->xEP.xClassInfoList);
		TRACE("\t %08s %-16s %-16s %-16s %-16s %-16s %-16s %-16s\n", 
			"CLASS", "ID", "TYPE", "NAME", "SN", "STATE", "VALUE", "TIME");
	while((pEPClassInfo = list_iterator_next(&pConfig->xEP.xClassInfoList)) != 0)
	{
		TRACE("\t %08lx %-16s %-16s %-16s %-16s %-16s %-16s %-16s\n", 
			pEPClassInfo->xClass, 
			pEPClassInfo->xOIDs.pID,
			pEPClassInfo->xOIDs.pType,
			pEPClassInfo->xOIDs.pName,
			pEPClassInfo->xOIDs.pSN,
			pEPClassInfo->xOIDs.pState,
			pEPClassInfo->xOIDs.pValue,
			pEPClassInfo->xOIDs.pTime);
	}

	
	return	FTM_RET_OK;
}

FTM_RET	FTDMS_getEPClassInfo
(
	FTDM_SERVER_CONFIG_PTR 		pConfig, 
	FTM_EPID 					xEPID, 
	FTM_EP_CLASS_INFO_PTR _PTR_ 	ppEPClassInfo
)
{
	FTM_EP_CLASS_INFO_PTR	pEPClassInfo;

	pEPClassInfo = (FTM_EP_CLASS_INFO_PTR)list_seek(&pConfig->xEP.xClassInfoList, (FTM_VOID_PTR)&xEPID);

	if (pEPClassInfo == NULL)
	{
		return	FTM_RET_NOT_EXISTS;	
	}

	*ppEPClassInfo = pEPClassInfo;

	return	FTM_RET_OK;
}

int	_FTDMS_seekEPOIDInfo(const void *pElement, const void *pKey)
{
	FTM_EP_CLASS_INFO_PTR 	pEPClassInfo = (FTM_EP_CLASS_INFO_PTR)pElement;
	FTM_EPID_PTR			pEPID = (FTM_EPID_PTR)pKey;

	if (pEPClassInfo->xClass == (FTM_EP_CLASS_MASK & (*pEPID)))
	{
		return	1;	
	}

	return	0;
}
