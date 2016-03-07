#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "libconfig.h"
#include "ftnm.h"
#include "ftdm_client.h"
#include "ftnm_node_snmpc.h"
#include "ftnm_server.h"
#include "ftnm_dmc.h"

static FTM_VOID_PTR	FTNM_DMC_process(FTM_VOID_PTR pData);

FTM_RET FTNM_DMC_init(FTNM_DMC_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	memset(pCTX, 0, sizeof(FTNM_DMC));

	strcpy(pCTX->xConfig.xNetwork.pServerIP, FTDM_DEFAULT_SERVER_IP);
	pCTX->xConfig.xNetwork.usPort = FTDM_DEFAULT_SERVER_PORT;

	return	FTM_RET_OK;
}

FTM_RET FTNM_DMC_final(FTNM_DMC_PTR pCTX)
{
	ASSERT(pCTX != NULL);
	
	return	FTM_RET_OK;
}

FTM_RET	FTNM_DMC_start(FTNM_DMC_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	if (pthread_create(&pCTX->xThread, NULL, FTNM_DMC_process, pCTX) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_DMC_stop(FTNM_DMC_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTNM_DMC_process(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);

	FTM_RET			xRet;
	FTNM_DMC_PTR	pCTX = (FTNM_DMC_PTR)pData;
		
	xRet = FTDMC_connect(&pCTX->xSession, inet_addr(pCTX->xConfig.xNetwork.pServerIP), pCTX->xConfig.xNetwork.usPort);
	if (xRet != FTM_RET_OK)
	{
		printf("DB connection failed.\n");	
	}

	while(!pCTX->bStop)
	{
		usleep(1000);
	}

	return	0;
}

FTM_RET	FTNM_DMC_EP_DATA_set(FTDMC_SESSION_PTR pSession, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData)
{
	ASSERT(pData != NULL);

	return	FTDMC_EP_DATA_append(pSession, xEPID, pData);
}

FTM_RET FTNM_DMC_EP_DATA_setINT(FTDMC_SESSION_PTR pSession, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_INT nValue)
{
	FTM_EP_DATA	xEPData;

	xEPData.ulTime = ulTime;
	xEPData.xType = FTM_EP_DATA_TYPE_INT;
	xEPData.xValue.nValue = nValue;

	return FTDMC_EP_DATA_append(pSession, xEPID, &xEPData);
}

FTM_RET FTNM_DMC_EP_DATA_setULONG(FTDMC_SESSION_PTR pSession, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_ULONG ulValue)
{
	ASSERT(pSession != NULL);
	
	FTM_EP_DATA	xEPData;

	xEPData.ulTime = ulTime;
	xEPData.xType = FTM_EP_DATA_TYPE_ULONG;
	xEPData.xValue.ulValue = ulValue;

	return FTDMC_EP_DATA_append(pSession, xEPID, &xEPData);
}

FTM_RET FTNM_DMC_EP_DATA_setFLOAT(FTDMC_SESSION_PTR pSession, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_DOUBLE fValue)
{
	ASSERT(pSession != NULL);
	
	FTM_EP_DATA	xEPData;

	xEPData.ulTime = ulTime;
	xEPData.xType = FTM_EP_DATA_TYPE_FLOAT;
	xEPData.xValue.fValue = fValue;

	return FTDMC_EP_DATA_append(pSession, xEPID, &xEPData);
}

FTM_RET	FTNM_DMC_EP_DATA_count(FTDMC_SESSION_PTR pSession, FTM_EP_ID xEPID, FTM_ULONG_PTR pulCount)
{
	ASSERT(pSession != NULL);
	ASSERT(pulCount != NULL);

	return	FTDMC_EP_DATA_count(pSession, xEPID, pulCount);
}

FTM_RET FTNM_DMC_EP_DATA_info
(
	FTDMC_SESSION_PTR pSession, 
	FTM_EP_ID 		xEPID, 
	FTM_ULONG_PTR 	pulBeginTime, 
	FTM_ULONG_PTR 	pulEndTime, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pSession != NULL);

	return	FTDMC_EP_DATA_info(pSession, xEPID, pulBeginTime, pulEndTime, pulCount);
}

FTM_RET FTNM_DMC_loadFromFile(FTNM_DMC_PTR pCTX, FTM_CHAR_PTR pFileName)
{
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
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
			strncpy(pCTX->xConfig.xNetwork.pServerIP, config_setting_get_string(pField), FTDMC_SERVER_IP_LEN);
		}
	
		pField = config_setting_get_member(pSection, "port");
		if (pField != NULL)
		{
			pCTX->xConfig.xNetwork.usPort = (FTM_ULONG)config_setting_get_int(pField);
		}
	}

	config_destroy(&xConfig);

	xRet = FTDMC_init(&pCTX->xConfig);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_RET_OK;
}

FTM_RET FTNM_DMC_showConfig(FTNM_DMC_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	MESSAGE("\n[ DATA MANAGER CONNECTION CONFIGURATION ]\n");
	MESSAGE("%16s : %s\n", "SERVER", pCTX->xConfig.xNetwork.pServerIP);
	MESSAGE("%16s : %d\n", "PORT", pCTX->xConfig.xNetwork.usPort);

	return	FTM_RET_OK;
}

