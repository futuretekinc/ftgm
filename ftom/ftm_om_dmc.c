#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "libconfig.h"
#include "ftm_om.h"
#include "ftdm_client.h"
#include "ftm_om_node_snmpc.h"
#include "ftm_om_server.h"
#include "ftm_om_dmc.h"

static FTM_VOID_PTR	FTM_OM_DMC_process(FTM_VOID_PTR pData);

FTM_RET FTM_OM_DMC_init
(
	FTM_OM_DMC_PTR 	pDMC,
	FTM_OM_PTR 		pOM 
)
{
	ASSERT(pDMC != NULL);

	memset(pDMC, 0, sizeof(FTM_OM_DMC));

	strcpy(pDMC->xConfig.xNetwork.pServerIP, FTDM_DEFAULT_SERVER_IP);
	pDMC->xConfig.xNetwork.usPort = FTDM_DEFAULT_SERVER_PORT;
	FTM_OM_MSGQ_init(&pDMC->xMsgQ);

	return	FTM_RET_OK;
}

FTM_RET FTM_OM_DMC_final
(
	FTM_OM_DMC_PTR pDMC
)
{
	ASSERT(pDMC != NULL);
	
	FTM_OM_MSGQ_final(&pDMC->xMsgQ);

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_DMC_start
(
	FTM_OM_DMC_PTR pDMC
)
{
	ASSERT(pDMC != NULL);

	pDMC->bStop = FTM_FALSE;

	if (pthread_create(&pDMC->xThread, NULL, FTM_OM_DMC_process, pDMC) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_DMC_stop
(
	FTM_OM_DMC_PTR pDMC
)
{
	ASSERT(pDMC != NULL);

	pDMC->bStop = FTM_TRUE;
	pthread_join(pDMC->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTM_OM_DMC_process
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTM_RET			xRet;
	FTM_OM_DMC_PTR	pDMC = (FTM_OM_DMC_PTR)pData;
		
	while(!pDMC->bStop)
	{
		FTM_BOOL	bConnected;

		FTDMC_isConnected(&pDMC->xSession, &bConnected);
		if (!bConnected)
		{
			xRet = FTDMC_connect(&pDMC->xSession, inet_addr(pDMC->xConfig.xNetwork.pServerIP), pDMC->xConfig.xNetwork.usPort);
			if (xRet != FTM_RET_OK)
			{
				TRACE("DB connection failed.\n");	
				usleep(1000000);
			}
			else
			{
				if (pDMC->fServiceCB != NULL)
				{
					pDMC->fServiceCB(pDMC->xServiceID, FTM_OM_MSG_TYPE_DMC_CONNECTED, NULL);	
				}
			}
		}
		else
		{
			FTM_OM_MSG_PTR	pMsg= NULL;

			while(FTM_OM_MSGQ_timedPop(&pDMC->xMsgQ, 1000000, &pMsg) == FTM_RET_OK)
			{
				switch(pMsg->xType)
				{
				case	FTM_OM_MSG_TYPE_QUIT:
					{
						pDMC->bStop = FTM_TRUE;	
					}
					break;

				default:
					{
						TRACE("Message[%08x] not supported.\n", pMsg->xType);
					}
					
				}

				FTM_OM_MSG_destroy(&pMsg);
			}
		}
	}

	return	0;
}

FTM_RET	FTM_OM_DMC_setServiceCallback
(
	FTM_OM_DMC_PTR pDMC, 
	FTM_OM_SERVICE_ID xServiceID, 
	FTM_OM_SERVICE_CALLBACK pServiceCB
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pServiceCB != NULL);

	pDMC->xServiceID = xServiceID;
	pDMC->fServiceCB = pServiceCB;

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_DMC_notify
(
	FTM_OM_DMC_PTR pDMC, 
	FTM_OM_MSG_PTR pMsg
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pMsg != NULL);

	FTM_RET	xRet;

	switch(pMsg->xType)
	{
	case	FTM_OM_MSG_TYPE_EP_DATA_UPDATED:
		{	
			
			xRet = FTDMC_EP_DATA_append(&pDMC->xSession, pMsg->xParams.xEPDataUpdated.xEPID, &pMsg->xParams.xEPDataUpdated.xData);
		}
		break;

	default:
		xRet = FTM_RET_INVALID_MESSAGE_TYPE;
	}

	FTM_OM_MSG_destroy(&pMsg);

	return	xRet;
}

FTM_RET	FTM_OM_DMC_EP_create
(
	FTM_OM_DMC_PTR 	pDMC, 
	FTM_EP_PTR 		pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pInfo != NULL);

	return	FTDMC_EP_append(&pDMC->xSession, pInfo);
}

FTM_RET	FTM_OM_DMC_EP_destroy
(
	FTM_OM_DMC_PTR 	pDMC, 
	FTM_EP_ID 		xEPID
)
{
	ASSERT(pDMC != NULL);

	return	FTDMC_EP_remove(&pDMC->xSession, xEPID);
}

FTM_RET	FTM_OM_DMC_EP_DATA_set
(
	FTM_OM_DMC_PTR 	pDMC, 
	FTM_EP_ID 		xEPID, 
	FTM_EP_DATA_PTR pData
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pData != NULL);

	return	FTDMC_EP_DATA_append(&pDMC->xSession, xEPID, pData);
}

FTM_RET FTM_OM_DMC_EP_DATA_setINT
(
	FTM_OM_DMC_PTR 	pDMC, 
	FTM_EP_ID 		xEPID, 
	FTM_ULONG 		ulTime, 
	FTM_INT 		nValue
)
{
	ASSERT(pDMC != NULL);

	FTM_EP_DATA	xData;

	xData.ulTime = ulTime;
	xData.xType = FTM_EP_DATA_TYPE_INT;
	xData.xValue.nValue = nValue;

	return	FTDMC_EP_DATA_append(&pDMC->xSession, xEPID, &xData);
}

FTM_RET FTM_OM_DMC_EP_DATA_setULONG
(
	FTM_OM_DMC_PTR 	pDMC, 
	FTM_EP_ID 		xEPID, 
	FTM_ULONG 		ulTime, 
	FTM_ULONG 		ulValue
)
{
	ASSERT(pDMC != NULL);
	
	FTM_EP_DATA	xData;

	xData.ulTime = ulTime;
	xData.xType = FTM_EP_DATA_TYPE_ULONG;
	xData.xValue.ulValue = ulValue;

	return	FTDMC_EP_DATA_append(&pDMC->xSession, xEPID, &xData);
}

FTM_RET FTM_OM_DMC_EP_DATA_setFLOAT
(
	FTM_OM_DMC_PTR 	pDMC, 
	FTM_EP_ID 		xEPID, 
	FTM_ULONG 		ulTime, 
	FTM_DOUBLE 		fValue
)
{
	ASSERT(pDMC != NULL);
	
	FTM_EP_DATA	xData;

	xData.ulTime = ulTime;
	xData.xType = FTM_EP_DATA_TYPE_FLOAT;
	xData.xValue.fValue = fValue;

	return	FTDMC_EP_DATA_append(&pDMC->xSession, xEPID, &xData);
}

FTM_RET	FTM_OM_DMC_EP_DATA_count
(
	FTM_OM_DMC_PTR 	pDMC, 
	FTM_EP_ID 		xEPID, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pulCount != NULL);

	return	FTDMC_EP_DATA_count(&pDMC->xSession, xEPID, pulCount);
}

FTM_RET FTM_OM_DMC_EP_DATA_get
(
	FTM_OM_DMC_PTR	pDMC,
	FTM_EP_ID 		xEPID, 
	FTM_ULONG		ulStartIndex,
	FTM_EP_DATA_PTR	pData,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pData != NULL);
	ASSERT(pulCount != NULL);

	return	FTDMC_EP_DATA_get(&pDMC->xSession, xEPID, ulStartIndex, pData, ulMaxCount, pulCount);
}

FTM_RET FTM_OM_DMC_EP_DATA_info
(
	FTM_OM_DMC_PTR	pDMC,
	FTM_EP_ID 		xEPID, 
	FTM_ULONG_PTR 	pulBeginTime, 
	FTM_ULONG_PTR 	pulEndTime, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pDMC != NULL);

	return	FTDMC_EP_DATA_info(&pDMC->xSession, xEPID, pulBeginTime, pulEndTime, pulCount);
}

FTM_RET FTM_OM_DMC_loadFromFile(FTM_OM_DMC_PTR pDMC, FTM_CHAR_PTR pFileName)
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
			strncpy(pDMC->xConfig.xNetwork.pServerIP, config_setting_get_string(pField), FTDMC_SERVER_IP_LEN);
		}
	
		pField = config_setting_get_member(pSection, "port");
		if (pField != NULL)
		{
			pDMC->xConfig.xNetwork.usPort = (FTM_ULONG)config_setting_get_int(pField);
		}
	}

	config_destroy(&xConfig);

	xRet = FTDMC_init(&pDMC->xConfig);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_RET_OK;
}

FTM_RET FTM_OM_DMC_showConfig(FTM_OM_DMC_PTR pDMC)
{
	ASSERT(pDMC != NULL);

	MESSAGE("\n[ DATA MANAGER CONNECTION CONFIGURATION ]\n");
	MESSAGE("%16s : %s\n", "SERVER", pDMC->xConfig.xNetwork.pServerIP);
	MESSAGE("%16s : %d\n", "PORT", pDMC->xConfig.xNetwork.usPort);

	return	FTM_RET_OK;
}

