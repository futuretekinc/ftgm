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

FTM_RET FTNM_DMC_init(FTNM_DMC_PTR pDMC)
{
	ASSERT(pDMC != NULL);

	memset(&pDMC->xConfig, 0, sizeof(FTDMC_CFG));

	strcpy(pDMC->xConfig.xNetwork.pServerIP, FTDM_DEFAULT_SERVER_IP);
	pDMC->xConfig.xNetwork.usPort = FTDM_DEFAULT_SERVER_PORT;

	return	FTM_RET_OK;
}

FTM_RET FTNM_DMC_final(FTNM_DMC_PTR pDMC)
{
	ASSERT(pDMC != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_DMC_EP_DATA_set(FTNM_DMC_PTR pDMC, FTM_EPID xEPID, FTM_EP_DATA_PTR pData)
{
	ASSERT(pDMC != NULL);
	ASSERT(pData != NULL);

	return	FTDMC_EP_DATA_append(&pDMC->xSession, xEPID, pData);
}

FTM_RET FTNM_DMC_EP_DATA_setINT(FTNM_DMC_PTR pDMC, FTM_EPID xEPID, FTM_ULONG ulTime, FTM_INT nValue)
{
	ASSERT(pDMC != NULL);

	FTM_EP_DATA	xEPData;

	xEPData.ulTime = ulTime;
	xEPData.xType = FTM_EP_DATA_TYPE_INT;
	xEPData.xValue.nValue = nValue;

	return FTDMC_EP_DATA_append(&pDMC->xSession, xEPID, &xEPData);
}

FTM_RET FTNM_DMC_EP_DATA_setULONG(FTNM_DMC_PTR pDMC, FTM_EPID xEPID, FTM_ULONG ulTime, FTM_ULONG ulValue)
{
	ASSERT(pDMC != NULL);

	FTM_EP_DATA	xEPData;

	xEPData.ulTime = ulTime;
	xEPData.xType = FTM_EP_DATA_TYPE_ULONG;
	xEPData.xValue.ulValue = ulValue;

	return FTDMC_EP_DATA_append(&pDMC->xSession, xEPID, &xEPData);
}

FTM_RET FTNM_DMC_EP_DATA_setFLOAT(FTNM_DMC_PTR pDMC, FTM_EPID xEPID, FTM_ULONG ulTime, FTM_DOUBLE fValue)
{
	ASSERT(pDMC != NULL);

	FTM_EP_DATA	xEPData;

	xEPData.ulTime = ulTime;
	xEPData.xType = FTM_EP_DATA_TYPE_FLOAT;
	xEPData.xValue.fValue = fValue;

	return FTDMC_EP_DATA_append(&pDMC->xSession, xEPID, &xEPData);
}

FTM_RET	FTNM_DMC_EP_DATA_count(FTNM_DMC_PTR pDMC, FTM_EPID xEPID, FTM_ULONG_PTR pulCount)
{
	ASSERT(pDMC != NULL);

	return	FTDMC_EP_DATA_count(&pDMC->xSession, xEPID, pulCount);
}

FTM_RET FTNM_DMC_EP_DATA_info
(
	FTNM_DMC_PTR	pDMC,
	FTM_EPID 		xEPID, 
	FTM_ULONG_PTR 	pulBeginTime, 
	FTM_ULONG_PTR 	pulEndTime, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pDMC != NULL);

	return	FTDMC_EP_DATA_info(&pDMC->xSession, xEPID, pulBeginTime, pulEndTime, pulCount);
}

FTM_RET FTNM_DMC_loadConfig(FTNM_DMC_PTR pDMC, FTM_CHAR_PTR pFileName)
{
	ASSERT(pDMC != NULL);
	ASSERT(pFileName != NULL);

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
			strncpy(pDMC->xConfig.xNetwork.pServerIP, config_setting_get_string(pField), FTDMC_SERVER_IP_LEN);
		}
	
		pField = config_setting_get_member(pSection, "port");
		if (pField != NULL)
		{
			pDMC->xConfig.xNetwork.usPort = (FTM_ULONG)config_setting_get_int(pField);
		}
	}

	config_destroy(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET FTNM_DMC_showConfig(FTNM_DMC_PTR pDMC)
{
	ASSERT(pDMC != NULL);

	MESSAGE("\n[ DATA MANAGER CONNECTION CONFIGURATION ]\n");
	MESSAGE("%16s : %s\n", "SERVER", pDMC->xConfig.xNetwork.pServerIP);
	MESSAGE("%16s : %d\n", "PORT", pDMC->xConfig.xNetwork.usPort);

	return	FTM_RET_OK;
}

