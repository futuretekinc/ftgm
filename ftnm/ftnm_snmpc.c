#include <stdlib.h>
#include "libconfig.h"

#include "ftnm.h"
#include "ftnm_node_snmpc.h"
#include "ftnm_dmc.h"
#include "ftnm_ep.h"
#include "ftnm_ep_class.h"

FTM_VOID_PTR	FTNM_SNMPC_asyncResponseManager(FTM_VOID_PTR pData);

extern int	active_hosts;

FTM_RET	FTNM_SNMPC_init(FTNM_SNMPC_PTR pSNMPC)
{
	ASSERT(pSNMPC != NULL);
	
	FTNM_SNMPC_initConfig(pSNMPC);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_SNMPC_final(FTNM_SNMPC_PTR pSNMPC)
{
	ASSERT(pSNMPC != NULL);

	FTNM_SNMPC_finalConfig(pSNMPC);

	return	FTM_RET_OK;
}

FTM_RET FTNM_SNMPC_run(FTNM_SNMPC_PTR pSNMPC)
{
	ASSERT(pSNMPC != NULL);


	FTM_ULONG	ulCount;
	int	nRet;

	init_snmp(pSNMPC->xConfig.pName);

	if (FTM_LIST_count(&pSNMPC->xConfig.xMIBList, &ulCount) == FTM_RET_OK)
	{
		FTM_ULONG	i;

		for(i = 0 ; i < ulCount ; i++)
		{
			FTM_VOID_PTR	pValue;

			if (FTM_LIST_getAt(&pSNMPC->xConfig.xMIBList, i, &pValue) == FTM_RET_OK)
			{
				TRACE("Load MIB : %s\n", (FTM_CHAR_PTR)pValue);

				read_mib((FTM_CHAR_PTR)pValue);
			}
		}
	}

	nRet = pthread_create(&pSNMPC->xPThread, NULL, FTNM_SNMPC_asyncResponseManager, 0);
	if (nRet != 0)
	{
		return	FTM_RET_ERROR;		
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTNM_SNMPC_asyncResponseManager(FTM_VOID_PTR pData)
{
	while (FTM_TRUE)
	{	
		if (active_hosts) 
		{
			FTM_INT	fds = 0, block = 1;
			fd_set fdset;
			struct timeval timeout;

			FD_ZERO(&fdset);
			snmp_select_info(&fds, &fdset, &timeout, &block);
			fds = select(fds, &fdset, NULL, NULL, block ? NULL : &timeout);
			if (fds < 0) 
			{
				perror("select failed");
				exit(1);
			}
			if (fds)
			{
				snmp_read(&fdset);
			}
			else
			{
				snmp_timeout();
			}
		}
		usleep(1000);
	}

	return	0;
}

FTM_RET	FTNM_SNMPC_initConfig(FTNM_SNMPC_PTR pSNMPC)
{
	ASSERT(pSNMPC != NULL);

	memset(&pSNMPC->xConfig, 0, sizeof(FTNM_SNMPC_CONFIG));

	strcpy(pSNMPC->xConfig.pName, "ftnm");
	FTM_LIST_init(&pSNMPC->xConfig.xMIBList);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_SNMPC_finalConfig(FTNM_SNMPC_PTR pSNMPC)
{
	FTM_ULONG i, ulCount;

	FTM_LIST_count(&pSNMPC->xConfig.xMIBList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_VOID_PTR pValue;

		if (FTM_LIST_getAt(&pSNMPC->xConfig.xMIBList, i, &pValue) == FTM_RET_OK)
		{
			FTM_MEM_free(pValue);
		}
	}

	FTM_LIST_final(&pSNMPC->xConfig.xMIBList);

	return	FTM_RET_OK;
}

FTM_RET FTNM_SNMPC_loadConfig(FTNM_SNMPC_PTR pSNMPC, FTM_CHAR_PTR pFileName)
{
	ASSERT(pSNMPC != NULL);
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
					FTM_CHAR_PTR pBuff = (FTM_CHAR_PTR)FTM_MEM_malloc(strlen(pMIBFileName)+1);
					if (pBuff != NULL)
					{
						strcpy(pBuff, pMIBFileName);
						FTM_LIST_append(&pSNMPC->xConfig.xMIBList, pBuff);
					}
				}
			
			}
		
		}
	}
	config_destroy(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET FTNM_SNMPC_showConfig(FTNM_SNMPC_PTR pSNMPC)
{
	ASSERT(pSNMPC != NULL);

	FTM_ULONG	ulCount;

	if (FTM_LIST_count(&pSNMPC->xConfig.xMIBList, &ulCount) == FTM_RET_OK)
	{
		FTM_ULONG i;

		MESSAGE("\n[ MIBs ]\n");
		for(i = 0 ; i < ulCount ; i++)
		{
			FTM_VOID_PTR	pValue;

			if (FTM_LIST_getAt(&pSNMPC->xConfig.xMIBList, i, &pValue) == FTM_RET_OK)
			{
				MESSAGE("%16d : %s\n", i+1, (FTM_CHAR_PTR)pValue);
			}
		}
	}

	return	FTM_RET_OK;
}