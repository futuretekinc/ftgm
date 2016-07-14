#include <string.h>
#include "ftm.h"
#include "ftm_config.h"
#include "ftdm_client_config.h"

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

FTM_RET	FTDMC_CFG_load
(
	FTDMC_CFG_PTR pConfig, 
	FTM_CHAR_PTR pFileName
)
{
	FTM_RET				xRet;
	FTM_CONFIG_PTR		pRoot;
	FTM_CONFIG_ITEM		xSection;
	FTM_CONFIG_ITEM		xTypeString;
	FTM_CONFIG_ITEM		xEPTest;
	FTM_INT				i;

	if ((pConfig == NULL) || (pFileName == NULL))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	
	xRet = FTM_CONFIG_create(pFileName, &pRoot, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Configuration loading failed.[FILE = %s]\n", pFileName);
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	xRet = FTM_CONFIG_getItem(pRoot, "type_string", &xTypeString);
	if (xRet == FTM_RET_OK)
	{
		FTM_ULONG	ulCount = 0;

		xRet = FTM_CONFIG_LIST_getItemCount(&xTypeString, &ulCount);
		if (xRet == FTM_RET_OK)
		{
			for( i = 0 ; i < ulCount ; i++)
			{
				FTM_CONFIG_ITEM	xElement;

				xRet = FTM_CONFIG_LIST_getItemAt(&xTypeString, i, &xElement);
				if (xRet == FTM_RET_OK)
				{
					FTM_INT		 	nType;
					FTM_CHAR		pName[256];
				
					memset(pName, 0, sizeof(pName));
					xRet = FTM_CONFIG_ITEM_getItemINT(&xElement, "type", &nType);
					xRet |= FTM_CONFIG_ITEM_getItemString(&xElement, "name", pName, sizeof(pName) - 1);
	
					if (xRet == FTM_RET_OK)
					{
						FTM_appendEPTypeString(nType, pName);	
					}
				}
			}
		}
	}

	xRet = FTM_CONFIG_getItem(pRoot, "default", &xSection);
	if (xRet == FTM_RET_OK)
	{
		FTM_CONFIG_ITEM	xServerSetting;

		xRet = FTM_CONFIG_ITEM_getChildItem(&xSection, "server", &xServerSetting);
		if (xRet == FTM_RET_OK)
		{
			FTM_CONFIG_ITEM_getItemString(&xServerSetting, "ip", pConfig->xNetwork.pServerIP, sizeof(pConfig->xNetwork.pServerIP) - 1);
			FTM_CONFIG_ITEM_getItemUSHORT(&xServerSetting, "port", &pConfig->xNetwork.usPort );
			FTM_CONFIG_ITEM_getItemBOOL(&xServerSetting, "auto_connect", &pConfig->xNetwork.bAutoConnect);
		}
	}

	xRet = FTM_CONFIG_getItem(pRoot, "ep_test", &xEPTest);
	if (xRet == FTM_RET_OK)
	{
		FTM_CONFIG_ITEM	xEPIDList;
		
		xRet = FTM_CONFIG_ITEM_getChildItem(&xEPTest, "epid", &xEPIDList);
		if (xRet == FTM_RET_OK)
		{
			FTM_ULONG	ulCount = 0;

			FTM_CONFIG_LIST_getItemCount(&xEPIDList, &ulCount);
			for( i = 0 ; i < ulCount ; i++)
			{
				FTM_CONFIG_ITEM	xElement;
				
				xRet = FTM_CONFIG_LIST_getItemAt(&xEPIDList, i, &xElement);
				if (xRet == FTM_RET_OK)
				{
					FTM_INT	nEPID;
					
					xRet = FTM_CONFIG_ITEM_getINT(&xElement, &nEPID);
					if (xRet == FTM_RET_OK)
					{
						FTM_LIST_append(&pConfig->xDiagnostic.xEPList, (void *)nEPID);
					}
				}
			}
		}

		FTM_CHAR	pTimeString[128];

		xRet = FTM_CONFIG_ITEM_getItemString(&xEPTest, "start_time", pTimeString, sizeof(pTimeString) - 1);
		if (xRet == FTM_RET_OK)
		{
		}
	
		xRet = FTM_CONFIG_ITEM_getItemString(&xEPTest, "end_time", pTimeString, sizeof(pTimeString) - 1);
		if (xRet == FTM_RET_OK)
		{
		}
	}
	

	FTM_CONFIG_destroy(&pRoot);

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
