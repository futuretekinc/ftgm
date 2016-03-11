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
	FTNM_MSGQ_init(&pCTX->xMsgQ);

	return	FTM_RET_OK;
}

FTM_RET FTNM_DMC_final(FTNM_DMC_PTR pCTX)
{
	ASSERT(pCTX != NULL);
	
	FTNM_MSGQ_final(&pCTX->xMsgQ);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_DMC_start(FTNM_DMC_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	pCTX->bStop = FTM_FALSE;

	if (pthread_create(&pCTX->xThread, NULL, FTNM_DMC_process, pCTX) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_DMC_stop(FTNM_DMC_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	pCTX->bStop = FTM_TRUE;
	pthread_join(pCTX->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTNM_DMC_process(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);

	FTM_RET			xRet;
	FTNM_DMC_PTR	pCTX = (FTNM_DMC_PTR)pData;
		
	while(!pCTX->bStop)
	{
		FTM_BOOL	bConnected;

		FTDMC_isConnected(&pCTX->xSession, &bConnected);
		if (!bConnected)
		{
			xRet = FTDMC_connect(&pCTX->xSession, inet_addr(pCTX->xConfig.xNetwork.pServerIP), pCTX->xConfig.xNetwork.usPort);
			if (xRet != FTM_RET_OK)
			{
				TRACE("DB connection failed.\n");	
				usleep(1000000);
			}
			else
			{
				if (pCTX->fServiceCB != NULL)
				{
					pCTX->fServiceCB(pCTX->xServiceID, FTNM_MSG_TYPE_DMC_CONNECTED, NULL);	
				}
			}
		}
		else
		{
			FTNM_MSG_PTR	pMsg= NULL;

			while(FTNM_MSGQ_timedPop(&pCTX->xMsgQ, 1000000, &pMsg) == FTM_RET_OK)
			{
				switch(pMsg->xType)
				{
				case	FTNM_MSG_TYPE_QUIT:
					{
						pCTX->bStop = FTM_TRUE;	
					}
					break;

				default:
					{
						TRACE("Message[%08x] not supported.\n", pMsg->xType);
					}
					
				}

				FTNM_MSG_destroy(pMsg);
			}
		}
	}

	return	0;
}

FTM_RET	FTNM_DMC_setServiceCallback(FTNM_DMC_PTR pCTX, FTNM_SERVICE_ID xServiceID, FTNM_SERVICE_CALLBACK pServiceCB)
{
	ASSERT(pCTX != NULL);
	ASSERT(pServiceCB != NULL);

	pCTX->xServiceID = xServiceID;
	pCTX->fServiceCB = pServiceCB;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_DMC_EP_DATA_set(FTNM_DMC_PTR pCTX, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData)
{
	ASSERT(pCTX != NULL);
	ASSERT(pData != NULL);

	return	FTDMC_EP_DATA_append(&pCTX->xSession, xEPID, pData);
}

FTM_RET FTNM_DMC_EP_DATA_setINT(FTNM_DMC_PTR pCTX, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_INT nValue)
{
	ASSERT(pCTX != NULL);

	FTM_EP_DATA	xData;

	xData.ulTime = ulTime;
	xData.xType = FTM_EP_DATA_TYPE_INT;
	xData.xValue.nValue = nValue;

	return	FTDMC_EP_DATA_append(&pCTX->xSession, xEPID, &xData);
}

FTM_RET FTNM_DMC_EP_DATA_setULONG(FTNM_DMC_PTR pCTX, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_ULONG ulValue)
{
	ASSERT(pCTX != NULL);
	
	FTM_EP_DATA	xData;

	xData.ulTime = ulTime;
	xData.xType = FTM_EP_DATA_TYPE_ULONG;
	xData.xValue.ulValue = ulValue;

	return	FTDMC_EP_DATA_append(&pCTX->xSession, xEPID, &xData);
}

FTM_RET FTNM_DMC_EP_DATA_setFLOAT(FTNM_DMC_PTR pCTX, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_DOUBLE fValue)
{
	ASSERT(pCTX != NULL);
	
	FTM_EP_DATA	xData;

	xData.ulTime = ulTime;
	xData.xType = FTM_EP_DATA_TYPE_FLOAT;
	xData.xValue.fValue = fValue;

	return	FTDMC_EP_DATA_append(&pCTX->xSession, xEPID, &xData);
}

FTM_RET	FTNM_DMC_EP_DATA_count(FTNM_DMC_PTR pCTX, FTM_EP_ID xEPID, FTM_ULONG_PTR pulCount)
{
	ASSERT(pCTX != NULL);
	ASSERT(pulCount != NULL);

	return	FTDMC_EP_DATA_count(&pCTX->xSession, xEPID, pulCount);
}

FTM_RET FTNM_DMC_EP_DATA_info
(
	FTNM_DMC_PTR	pCTX,
	FTM_EP_ID 		xEPID, 
	FTM_ULONG_PTR 	pulBeginTime, 
	FTM_ULONG_PTR 	pulEndTime, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pCTX != NULL);

	return	FTDMC_EP_DATA_info(&pCTX->xSession, xEPID, pulBeginTime, pulEndTime, pulCount);
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

