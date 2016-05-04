#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "libconfig.h"
#include "ftom.h"
#include "ftdm_client.h"
#include "ftom_node_snmpc.h"
#include "ftom_server.h"
#include "ftom_dmc.h"

static FTM_VOID_PTR	FTOM_DMC_process
(
	FTM_VOID_PTR 	pData
);

static FTM_RET	FTOM_DMC_onSaveEPData
(
	FTOM_DMC_PTR	pDMC,
	FTOM_MSG_SAVE_EP_DATA_PTR	pMsg
);

FTM_RET	FTOM_DMC_create
(
	FTOM_DMC_PTR _PTR_ 	ppDMC
)
{
	ASSERT(ppDMC != NULL);

	FTM_RET	xRet;
	FTOM_DMC_PTR	pDMC;

	pDMC = (FTOM_DMC_PTR)FTM_MEM_malloc(sizeof(FTOM_DMC));
	if (pDMC == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_DMC_init(pDMC);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pDMC);
		return	xRet;	
	}

	*ppDMC = pDMC;

	return	FTM_RET_OK;
}
	
FTM_RET FTOM_DMC_init
(
	FTOM_DMC_PTR 	pDMC
)
{
	ASSERT(pDMC != NULL);

	memset(pDMC, 0, sizeof(FTOM_DMC));

	strcpy(pDMC->xConfig.xNetwork.pServerIP, FTDM_DEFAULT_SERVER_IP);
	pDMC->xConfig.xNetwork.usPort = FTDM_DEFAULT_SERVER_PORT;
	FTOM_MSGQ_init(&pDMC->xMsgQ);

	return	FTM_RET_OK;
}

FTM_RET FTOM_DMC_final
(
	FTOM_DMC_PTR pDMC
)
{
	ASSERT(pDMC != NULL);
	
	FTOM_MSGQ_final(&pDMC->xMsgQ);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_DMC_start
(
	FTOM_DMC_PTR pDMC
)
{
	ASSERT(pDMC != NULL);

	if (pDMC->bStop)
	{
		return	FTM_RET_ALREADY_STARTED;	
	}


	if (pthread_create(&pDMC->xThread, NULL, FTOM_DMC_process, pDMC) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_DMC_stop
(
	FTOM_DMC_PTR pDMC
)
{
	ASSERT(pDMC != NULL);

	if (pDMC->bStop)
	{
		return	FTM_RET_NOT_START;	
	}

	pDMC->bStop = FTM_TRUE;
	pthread_join(pDMC->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTOM_DMC_process
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTM_RET			xRet;
	FTOM_DMC_PTR	pDMC = (FTOM_DMC_PTR)pData;

	pDMC->bStop = FTM_FALSE;
		
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
					pDMC->fServiceCB(pDMC->xServiceID, FTOM_MSG_TYPE_CONNECTED, NULL);	
				}
			}
		}
		else
		{
			FTOM_MSG_PTR	pMsg= NULL;

			while(FTOM_MSGQ_timedPop(&pDMC->xMsgQ, 1000000, &pMsg) == FTM_RET_OK)
			{
				switch(pMsg->xType)
				{
				case	FTOM_MSG_TYPE_QUIT:
					{
						pDMC->bStop = FTM_TRUE;	
					}
					break;

				default:
					{
						TRACE("Message[%08x] not supported.\n", pMsg->xType);
					}
					
				}

				FTOM_MSG_destroy(&pMsg);
			}
		}
	}

	return	0;
}

FTM_RET	FTOM_DMC_setServiceCallback
(
	FTOM_DMC_PTR pDMC, 
	FTOM_SERVICE_ID xServiceID, 
	FTOM_SERVICE_CALLBACK pServiceCB
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pServiceCB != NULL);

	pDMC->xServiceID = xServiceID;
	pDMC->fServiceCB = pServiceCB;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_DMC_notify
(
	FTOM_DMC_PTR pDMC, 
	FTOM_MSG_PTR pMsg
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pMsg != NULL);

	FTM_RET	xRet;

	switch(pMsg->xType)
	{
	case	FTOM_MSG_TYPE_SAVE_EP_DATA:
		{	
			xRet = FTOM_DMC_onSaveEPData(pDMC, (FTOM_MSG_SAVE_EP_DATA_PTR)pMsg);
		}
		break;

	default:
		xRet = FTM_RET_INVALID_MESSAGE_TYPE;
	}

	FTOM_MSG_destroy(&pMsg);

	return	xRet;
}

FTM_RET	FTOM_DMC_onSaveEPData
(
	FTOM_DMC_PTR	pDMC,
	FTOM_MSG_SAVE_EP_DATA_PTR	pMsg
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pMsg != NULL);
	
	return	FTDMC_EP_DATA_append(&pDMC->xSession, pMsg->pEPID, &pMsg->xData);
}

FTM_RET	FTOM_DMC_EP_create
(
	FTOM_DMC_PTR 	pDMC, 
	FTM_EP_PTR 		pInfo
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pInfo != NULL);

	return	FTDMC_EP_append(&pDMC->xSession, pInfo);
}

FTM_RET	FTOM_DMC_EP_destroy
(
	FTOM_DMC_PTR 	pDMC, 
	FTM_CHAR_PTR	pEPID
)
{
	ASSERT(pDMC != NULL);

	return	FTDMC_EP_remove(&pDMC->xSession, pEPID);
}

FTM_RET	FTOM_DMC_appendEPData
(
	FTOM_DMC_PTR 	pDMC, 
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR pData
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pData != NULL);

	return	FTDMC_EP_DATA_append(&pDMC->xSession, pEPID, pData);
}

FTM_RET	FTOM_DMC_EP_DATA_count
(
	FTOM_DMC_PTR 	pDMC, 
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pulCount != NULL);

	return	FTDMC_EP_DATA_count(&pDMC->xSession, pEPID, pulCount);
}

FTM_RET FTOM_DMC_EP_DATA_get
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulStartIndex,
	FTM_EP_DATA_PTR	pData,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pEPID != NULL);
	ASSERT(pData != NULL);
	ASSERT(pulCount != NULL);

	return	FTDMC_EP_DATA_get(&pDMC->xSession, pEPID, ulStartIndex, pData, ulMaxCount, pulCount);
}

FTM_RET FTOM_DMC_EP_DATA_del
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pEPID != NULL);
	ASSERT(pulCount != NULL);

	return	FTDMC_EP_DATA_remove(&pDMC->xSession, pEPID, ulIndex, ulCount, pulCount);
}

FTM_RET FTOM_DMC_EP_DATA_delWithTime
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulStart,
	FTM_ULONG		ulEnd,
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pDMC != NULL);
	ASSERT(pEPID != NULL);
	ASSERT(pulCount != NULL);

	return	FTDMC_EP_DATA_removeWithTime(&pDMC->xSession, pEPID, ulStart, ulEnd, pulCount);
}

FTM_RET FTOM_DMC_EP_DATA_info
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR 	pulBeginTime, 
	FTM_ULONG_PTR 	pulEndTime, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pDMC != NULL);

	return	FTDMC_EP_DATA_info(&pDMC->xSession, pEPID, pulBeginTime, pulEndTime, pulCount);
}

FTM_RET FTOM_DMC_loadFromFile(FTOM_DMC_PTR pDMC, FTM_CHAR_PTR pFileName)
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

FTM_RET FTOM_DMC_showConfig(FTOM_DMC_PTR pDMC)
{
	ASSERT(pDMC != NULL);

	MESSAGE("\n[ DATA MANAGER CONNECTION CONFIGURATION ]\n");
	MESSAGE("%16s : %s\n", "SERVER", pDMC->xConfig.xNetwork.pServerIP);
	MESSAGE("%16s : %d\n", "PORT", pDMC->xConfig.xNetwork.usPort);

	return	FTM_RET_OK;
}

