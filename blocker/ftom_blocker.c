#include "ftom_blocker.h"
#include "ftom_blocker_cb.h"
#include "ftom_blocker_cloud_client_wrapper.h"
#include "ftom_blocker_server_client_wrapper.h"

#undef	__MODULE__
#define __MODULE__ FTOM_TRACE_MODULE_CLIENT

static
FTM_VOID_PTR	FTOM_BLOCKER_threadMain
(
	FTM_VOID_PTR	pData
);

static
FTM_RET	FTOM_BLOCKER_notifyCB
(
	FTOM_MSG_PTR	pMsg,
	FTM_VOID_PTR	pData
);

static
FTOM_BLOCKER_CONFIG	_xDefaultConfig =
{
	.xServerSync=
	{
		.bEnabled 	= FTM_FALSE,
	},
	.xAutoStatusPublish =
	{
		.bEnabled 	= FTM_FALSE,
		.ulInterval = 60000
	}
};

FTM_RET	FTOM_BLOCKER_create
(
	FTM_CHAR_PTR	pName,
	FTM_CONFIG_PTR	pConfig,
	FTOM_BLOCKER_PTR _PTR_ ppBlocker
)
{
	ASSERT(ppBlocker != NULL);
	FTM_RET	xRet;	
	FTOM_BLOCKER_PTR	pBlocker = NULL;
	FTM_CHAR	pModuleName[FTM_NAME_LEN + 1];

	pBlocker = (FTOM_BLOCKER_PTR)FTM_MEM_malloc(sizeof(FTOM_BLOCKER));
	if (pBlocker == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;	
		ERROR2(xRet, "Failed to create blocker!\n");
		return	xRet;	
	}

	xRet = FTM_EVENT_TIMER_MANAGER_create(&pBlocker->pETM);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create event timer!\n");	
		goto error;
	}

	memcpy(&pBlocker->xConfig, &_xDefaultConfig, sizeof(FTOM_BLOCKER_CONFIG));

	strncpy(pBlocker->xConfig.pName, pName, sizeof(pBlocker->xConfig.pName) - 1);
	pBlocker->bStop = FTM_TRUE;

	memset(pModuleName, 0, sizeof(pModuleName));

	if (pConfig != NULL)
	{
		FTM_CONFIG_ITEM	xSection;

		xRet = FTM_CONFIG_getItem(pConfig, pBlocker->xConfig.pName, &xSection);
		if (xRet != FTM_RET_OK)
		{
			TRACE("Failed to get blocker section!\n");
			return	xRet;	
		}
		xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "server", pModuleName, sizeof(pModuleName) - 1);
		if (xRet == FTM_RET_OK)
		{
			strncpy(pBlocker->xConfig.xServerClient.pName, pModuleName, sizeof(pBlocker->xConfig.xServerClient.pName) - 1);
	
			xRet = FTOM_SERVER_CLIENT_getModule(pBlocker->xConfig.xServerClient.pName, &pBlocker->pServerClientModule);
			if (xRet != FTM_RET_OK)
			{
				TRACE("Failed to get module[%s]!\n", pBlocker->xConfig.xServerClient.pName);
				return	xRet;
			}
	
			xRet = FTOM_BLOCKER_SERVER_CLIENT_create(pBlocker, &pBlocker->pServerClient);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to create server client!\n");
				return	xRet;
			}
		}
		else
		{
			TRACE("Failed to get server type!\n");
			return	xRet;	
		}
	
		xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "cloud", pModuleName, sizeof(pModuleName) - 1);
		if (xRet == FTM_RET_OK)
		{
			strncpy(pBlocker->xConfig.xCloudClient.pName, pModuleName, sizeof(pBlocker->xConfig.xCloudClient.pName) - 1);
	
			xRet = FTOM_CLOUD_CLIENT_getModule(pBlocker->xConfig.xCloudClient.pName, &pBlocker->pCloudClientModule);
			if (xRet != FTM_RET_OK)
			{
				TRACE("Failed to get module[%s]!\n", pBlocker->xConfig.xCloudClient.pName);
				return	xRet;
			}
	
			xRet = FTOM_BLOCKER_CLOUD_CLIENT_create(pBlocker, &pBlocker->pCloudClient);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to create cloud client!\n");
				return	xRet;
			}
		}
		else
		{
			TRACE("Failed to get could type!\n");
			return	xRet;	
		}

		FTOM_BLOCKER_CONFIG_load(pBlocker, pConfig);
	}

	*ppBlocker = pBlocker;

	return	FTM_RET_OK;

error:
	if (pBlocker != NULL)
	{
		if (pBlocker->pETM != NULL)
		{
			FTM_EVENT_TIMER_MANAGER_destroy(&pBlocker->pETM);	
		}

		FTM_MEM_free(pBlocker);
	
	}
	return	xRet;
}

FTM_RET	FTOM_BLOCKER_destroy
(
	FTOM_BLOCKER_PTR _PTR_ ppBlocker
)
{
	ASSERT(ppBlocker != NULL);

	FTOM_BLOCKER_final(*ppBlocker);

	FTM_EVENT_TIMER_MANAGER_destroy(&(*ppBlocker)->pETM);

	FTM_MEM_free(*ppBlocker);

	*ppBlocker = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_BLOCKER_init
(
	FTOM_BLOCKER_PTR pBlocker
)
{
	ASSERT(pBlocker != NULL);
	FTM_RET	xRet;	

	xRet = FTOM_MSGQ_create(&pBlocker->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create message queue!\n");
		goto error;
	}

	return	FTM_RET_OK;

error:
	if (pBlocker->pMsgQ != NULL)
	{
		FTOM_MSGQ_destroy(&pBlocker->pMsgQ);	
	}

	return	xRet;
}

FTM_RET	FTOM_BLOCKER_final
(
	FTOM_BLOCKER_PTR pBlocker
)
{
	ASSERT(pBlocker != NULL);
	FTM_RET	xRet;

	if (pBlocker->pMsgQ != NULL)
	{
		xRet = FTOM_MSGQ_destroy(&pBlocker->pMsgQ);	
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to destroy message queue!\n");	
		}
	}

	if (pBlocker->pServerClient != NULL)
	{
		xRet = FTOM_BLOCKER_SERVER_CLIENT_destroy(pBlocker, &pBlocker->pServerClient);	
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to destroy ftom client!\n");	
		}
	}

	if (pBlocker->pCloudClient != NULL)
	{
		
		xRet = FTOM_BLOCKER_CLOUD_CLIENT_destroy(pBlocker, &pBlocker->pCloudClient);	
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to destroy cloud client!\n");	
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_BLOCKER_start
(
	FTOM_BLOCKER_PTR pBlocker
)
{
	ASSERT(pBlocker != NULL);
	ASSERT(pBlocker->pServerClient != NULL);
	ASSERT(pBlocker->pCloudClient != NULL);
	FTM_RET	xRet;

    if (pthread_create(&pBlocker->xThreadMain, NULL, FTOM_BLOCKER_threadMain, pBlocker) < 0)
	{
		xRet = FTM_RET_THREAD_CREATION_ERROR;
		ERROR2(xRet, "The blocker main thread creation failed!\n");
		return  xRet;
    }

	return	FTM_RET_OK;	
}

FTM_RET	FTOM_BLOCKER_stop
(
	FTOM_BLOCKER_PTR pBlocker
)
{
	ASSERT(pBlocker != NULL);
	ASSERT(pBlocker->pServerClient != NULL);
	ASSERT(pBlocker->pCloudClient != NULL);

	pBlocker->bStop = FTM_TRUE;
	
	pthread_join(pBlocker->xThreadMain, NULL);
	pBlocker->xThreadMain = 0;
	
	return	FTM_RET_OK;
}

FTM_RET FTOM_BLOCKER_waitingForFinished
(	
	FTOM_BLOCKER_PTR pBlocker
)
{
	ASSERT(pBlocker != NULL);
	FTM_RET	xRet;

	xRet = FTOM_BLOCKER_SERVER_CLIENT_waitingForFinished(pBlocker, pBlocker->pServerClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Exit wait was processed abnormally.\n");
	}

	xRet = FTOM_BLOCKER_CLOUD_CLIENT_waitingForFinished(pBlocker, pBlocker->pCloudClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Exit wait was processed abnormally.\n");
	}

	pthread_join(pBlocker->xThreadMain, NULL);
	pBlocker->xThreadMain = 0;

	return	xRet;
}

FTM_RET	FTOM_BLOCKER_CONFIG_load
(
	FTOM_BLOCKER_PTR	pBlocker,
	FTM_CONFIG_PTR		pConfig
)
{
	ASSERT(pBlocker != NULL);
	ASSERT(pConfig != NULL);
	FTM_RET	xRet;
	FTM_CONFIG_ITEM	xSection;
	FTM_CONFIG_ITEM	xServices;

	xRet = FTM_CONFIG_getItem(pConfig, pBlocker->xConfig.pName, &xSection);
	if (xRet != FTM_RET_OK)
	{
		TRACE("Failed to get blocker section!\n");
		return	xRet;	
	}

	xRet = FTM_CONFIG_ITEM_getChildItem(&xSection, "services", &xServices);
	if (xRet == FTM_RET_OK)
	{
		FTM_ULONG	i, ulServiceCount = 0;

		xRet = FTM_CONFIG_LIST_getItemCount(&xServices, &ulServiceCount);	
		if (xRet == FTM_RET_OK)
		{
			for(i = 0 ; i < ulServiceCount ; i++)
			{
			
				FTM_CONFIG_ITEM	xService;

				xRet = FTM_CONFIG_LIST_getItemAt(&xServices, i, &xService);	
				if (xRet == FTM_RET_OK)
				{
					FTM_CHAR	pTypeName[FTM_NAME_LEN+1];
					FTM_BOOL	bActivation = FTM_FALSE;

					FTM_CONFIG_ITEM_getItemString(&xService, "type", pTypeName, FTM_NAME_LEN);	
					if (strcasecmp(pTypeName, "auto_server_sync") == 0)
					{
						xRet = FTM_CONFIG_ITEM_getItemBOOL(&xService, "activation", &bActivation);	
						if (xRet == FTM_RET_OK)
						{
							pBlocker->xConfig.xServerSync.bEnabled = bActivation;
						}
					}
					else if (strcasecmp(pTypeName, "auto_status_publish") == 0)
					{
						FTM_ULONG	ulInterval;

						xRet = FTM_CONFIG_ITEM_getItemBOOL(&xService, "activation", &bActivation);	
						if (xRet == FTM_RET_OK)
						{
							xRet = FTM_CONFIG_ITEM_getItemULONG(&xService, "interval", &ulInterval);	
							if (xRet == FTM_RET_OK)
							{
								pBlocker->xConfig.xAutoStatusPublish.bEnabled = bActivation;
								pBlocker->xConfig.xAutoStatusPublish.ulInterval = ulInterval;
							}
							else
							{
								WARN2(xRet, "The time interval setting is missing!\n");	
							}
						}
						else
						{
							WARN2(xRet, "The activation setting is missing!\n");	
						}
					}
					else
					{
						WARN2(xRet, "Not supported service[%s]!\n", pTypeName);	
					
					}
						
				}
				else
				{
					ERROR2(xRet, "Failed to get service item!\n");	
				}
			}
		
		}
	}

	if (pBlocker->pServerClient != NULL)
	{
		xRet = FTOM_BLOCKER_SERVER_CLIENT_CONFIG_load(pBlocker, pBlocker->pServerClient, pConfig);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to load sever client configuration.\n");
			return	xRet;
		}
	}

	if (pBlocker->pCloudClient != NULL)
	{
		xRet = FTOM_BLOCKER_CLOUD_CLIENT_CONFIG_load(pBlocker, pBlocker->pCloudClient, pConfig);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to load cloud client configuration.\n");
			return	xRet;
		}
	}

	return	xRet;
}

FTM_RET	FTOM_BLOCKER_CONFIG_save
(
	FTOM_BLOCKER_PTR	pBlocker,
	FTM_CONFIG_PTR			pConfig
)
{
	ASSERT(pBlocker != NULL);
	ASSERT(pConfig != NULL);
	FTM_RET	xRet;

	xRet = FTOM_BLOCKER_CLOUD_CLIENT_CONFIG_save(pBlocker, pBlocker->pCloudClient, pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load cloud client configuration.\n");
		return	xRet;
	}

	xRet = FTOM_BLOCKER_SERVER_CLIENT_CONFIG_save(pBlocker, pBlocker->pServerClient, pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load serverftom client configuration.\n");
		return	xRet;
	}

	return	xRet;
}

FTM_RET	FTOM_BLOCKER_CONFIG_show
(
	FTOM_BLOCKER_PTR	pBlocker
)
{
	ASSERT(pBlocker != NULL);

	MESSAGE("[ Blocker Configuration ]\n");
	MESSAGE("# Auto Server Sync\n");
	MESSAGE("%16s : %s\n", "Activation", (pBlocker->xConfig.xServerSync.bEnabled)?"on":"off");
	MESSAGE("# Auto Status Publish \n");
	MESSAGE("%16s : %s\n", "Activation", (pBlocker->xConfig.xAutoStatusPublish.bEnabled)?"on":"off");
	MESSAGE("%16s : %lu ms\n", "Interval" ,pBlocker->xConfig.xAutoStatusPublish.ulInterval);

	FTOM_BLOCKER_SERVER_CLIENT_CONFIG_show(pBlocker, pBlocker->pServerClient);
	FTOM_BLOCKER_CLOUD_CLIENT_CONFIG_show(pBlocker, pBlocker->pCloudClient);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTOM_BLOCKER_threadMain
(
	FTM_VOID_PTR	pData
)
{
	FTOM_BLOCKER_PTR	pBlocker = (FTOM_BLOCKER_PTR)pData;
	FTM_RET     xRet;
	FTM_TIMER   xLoopTimer;
	FTM_ULONG   ulLoopInterval;
	FTM_EVENT_TIMER_PTR	pEvent;

	FTM_TIMER_initMS(&xLoopTimer, 1000);

	TRACE("The blocker was started!\n");

	FTOM_BLOCKER_SERVER_CLIENT_setNotifyCB(pBlocker, pBlocker->pServerClient, FTOM_BLOCKER_notifyCB, pBlocker);

	xRet = FTOM_BLOCKER_SERVER_CLIENT_start(pBlocker, pBlocker->pServerClient);	
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to start server client!\n");
	}

	xRet = FTOM_BLOCKER_CLOUD_CLIENT_setNotifyCB(pBlocker, pBlocker->pCloudClient, FTOM_BLOCKER_notifyCB, pBlocker);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set notify callback!\n");	
	}

	xRet = FTOM_BLOCKER_CLOUD_CLIENT_start(pBlocker, pBlocker->pCloudClient);	
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to start cloud client!\n");
	}

	xRet = FTM_EVENT_TIMER_MANAGER_start(pBlocker->pETM);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to start event timer!\n");	
	}

	if (pBlocker->xConfig.xAutoStatusPublish.bEnabled)
	{
		xRet = FTM_EVENT_TIMER_MANAGER_add(pBlocker->pETM, FTM_EVENT_TIMER_TYPE_REPEAT, pBlocker->xConfig.xAutoStatusPublish.ulInterval, FTOM_BLOCKER_EVENT_publishStatusCB, pBlocker, &pEvent);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to add event timer!\n");	
		}
	}

	pBlocker->bStop = FTM_FALSE;

	while(!pBlocker->bStop)
	{    
		FTOM_MSG_PTR    pBaseMsg;

		FTM_TIMER_remainMS(&xLoopTimer, &ulLoopInterval);

		while (!pBlocker->bStop)
		{    
			xRet = FTOM_MSGQ_timedPop(pBlocker->pMsgQ, ulLoopInterval, &pBaseMsg);
			if (xRet != FTM_RET_OK)
			{    
				break;  
			}    

			xRet = FTOM_BLOCKER_MESSAGE_process(pBlocker, pBaseMsg);
			if (xRet != FTM_RET_OK)
			{
				FTOM_MSG_destroy(&pBaseMsg);
			}
		}    

		FTM_TIMER_addMS(&xLoopTimer, 100);
	}    

	FTM_EVENT_TIMER_MANAGER_stop(pBlocker->pETM);

	xRet = FTOM_BLOCKER_CLOUD_CLIENT_stop(pBlocker, pBlocker->pCloudClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to stop cloud client!\n");	
	}

	xRet = FTOM_BLOCKER_CLOUD_CLIENT_setNotifyCB(pBlocker, pBlocker->pCloudClient, NULL, NULL);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set notify callback!\n");	
	}

	xRet = FTOM_BLOCKER_SERVER_CLIENT_stop(pBlocker, pBlocker->pServerClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to stop server client!\n");	
	}

	FTOM_BLOCKER_SERVER_CLIENT_setNotifyCB(pBlocker, pBlocker->pServerClient, NULL, NULL);

	TRACE("The blocker was finished !\n");

	return  0;
}

FTM_RET	FTOM_BLOCKER_MESSAGE_send
(
	FTOM_BLOCKER_PTR	pBlocker,
	FTOM_MSG_PTR		pMsg
)
{
	ASSERT(pBlocker != NULL);
	ASSERT(pMsg != NULL);

	return	FTOM_MSGQ_push(pBlocker->pMsgQ, pMsg);
}

FTM_RET	FTOM_BLOCKER_MESSAGE_process
(
	FTOM_BLOCKER_PTR	pBlocker,
	FTOM_MSG_PTR			pBaseMsg
)
{
	ASSERT(pBlocker != NULL);
	ASSERT(pBaseMsg != NULL);

	FTM_RET	xRet;

	TRACE("MESSAGE : %s\n", FTOM_MSG_printType(pBaseMsg));
	switch(pBaseMsg->xType)
	{
	case	FTOM_MSG_TYPE_CONNECTED:
		{
			if (pBaseMsg->xSenderID == pBlocker->pCloudClient)
			{
				if (pBlocker->xConfig.xServerSync.bEnabled)
				{
					xRet = FTOM_BLOCKER_SERVER_sync(pBlocker);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet, "Failed  to server sync!\n");	
					}
				}
			}
		}
		break;

	case	FTOM_MSG_TYPE_REPORT:
		{
			if (pBlocker->xConfig.xAutoStatusPublish.bEnabled)
			{
				xRet = FTOM_BLOCKER_SERVER_updateStatus(pBlocker);
				if (xRet != FTM_RET_OK)
				{
					ERROR2(xRet, "Failed to update status!\n");	
				}
			}
		}
		break;

	default:
		return	FTOM_BLOCKER_CLOUD_CLIENT_MESSAGE_send(pBlocker, pBlocker->pCloudClient, pBaseMsg);
	}

	FTOM_MSG_destroy(&pBaseMsg);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_BLOCKER_notifyCB
(
	FTOM_MSG_PTR	pMsg,
	FTM_VOID_PTR	pData
)
{
	ASSERT(pMsg != NULL);
	ASSERT(pData != NULL);

	FTOM_BLOCKER_PTR	pBlocker = (FTOM_BLOCKER_PTR)pData;

	return	FTOM_MSGQ_push(pBlocker->pMsgQ, pMsg);

}

/*******************************************************
 *	For cloud connector
 *******************************************************/
FTM_RET	FTOM_BLOCKER_MODULE_load
(
	FTOM_BLOCKER_PTR	pBlocker,
	FTM_CHAR_PTR		pModule
)
{
	ASSERT(pBlocker != NULL);
	ASSERT(pModule != NULL);


	return	FTM_RET_OK;
}


FTM_RET	FTOM_BLOCKER_SERVER_sync
(
	FTOM_BLOCKER_PTR	pBlocker
)
{
	FTM_RET		xRet;
	FTM_ULONG	ulNodeCount = 0;
	FTM_ULONG	ulEPCount = 0;
	FTM_ULONG	ulRegisteredDeviceCount = 0;
	FTM_ULONG	ulRegisteredSensorCount = 0;
	FTM_TIME	xTime;
	FTM_ULONG	ulCurrentTime;
	FTM_INT		i;

	FTM_TIME_getCurrent(&xTime);
	FTM_TIME_toSecs(&xTime, &ulCurrentTime);


	xRet = FTOM_BLOCKER_CLOUD_CLIENT_NODE_getCount(pBlocker, pBlocker->pCloudClient, &ulRegisteredDeviceCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get node count in cloud!\n");	
		return	xRet;
	}

	xRet = FTOM_BLOCKER_CLOUD_CLIENT_EP_getCount(pBlocker, pBlocker->pCloudClient, &ulRegisteredSensorCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get ep count in cloud!\n");	
		return	xRet;
	}

	xRet = FTOM_CLIENT_NODE_count((FTOM_CLIENT_PTR)pBlocker->pServerClient, &ulNodeCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get Node count!\n");
		return	xRet;
	}

	for(i = 0 ; i < ulNodeCount ; i++)
	{
		FTM_NODE	xNode;
		
		xRet = FTOM_CLIENT_NODE_getAt((FTOM_CLIENT_PTR)pBlocker->pServerClient, i, &xNode);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to get node information at %d\n", i);
			continue;	
		}

	
		xRet = FTOM_BLOCKER_CLOUD_CLIENT_NODE_isExist(pBlocker, pBlocker->pCloudClient, xNode.pDID);
		if (xRet != FTM_RET_OK)
		{
			xRet = FTOM_BLOCKER_CLOUD_CLIENT_NODE_register(pBlocker, pBlocker->pCloudClient, &xNode);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to register node!\n");
			}
		}

		xRet = FTOM_CLIENT_EP_count((FTOM_CLIENT_PTR)pBlocker->pServerClient, 0, xNode.pDID, &ulEPCount);
		if (xRet == FTM_RET_OK)
		{
			FTM_INT		j;

			for(j = 0 ; j < ulEPCount ;j++)
			{
				FTM_EP	xEP;
	
	
				xRet = FTOM_CLIENT_EP_getAt((FTOM_CLIENT_PTR)pBlocker->pServerClient, j, &xEP);
				if (xRet != FTM_RET_OK)
				{
					ERROR2(xRet, "Failed to get EP information at %d\n", i);
					continue;	
				}
		
				xRet = FTOM_BLOCKER_CLOUD_CLIENT_EP_isExist(pBlocker, pBlocker->pCloudClient, xEP.pEPID);
				if (xRet != FTM_RET_OK)
				{
					xRet = FTOM_BLOCKER_CLOUD_CLIENT_EP_register(pBlocker, pBlocker->pCloudClient, &xEP);	
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet, "Failed to register EP!\n");
					}
				}
			}
		}
	}

	return	FTOM_BLOCKER_SERVER_updateStatus(pBlocker);
}

FTM_RET	FTOM_BLOCKER_SERVER_updateStatus
(
	FTOM_BLOCKER_PTR	pBlocker
)
{
	ASSERT(pBlocker != NULL);

	FTM_RET		xRet;
	FTM_TIME	xTime;
	FTM_ULONG	ulEPCount;

	FTOM_BLOCKER_CLOUD_CLIENT_GATEWAY_updateStatus(pBlocker, pBlocker->pCloudClient, FTM_TRUE);

	FTM_TIME_getCurrent(&xTime);

	xRet = FTOM_BLOCKER_CLOUD_CLIENT_EP_getCount(pBlocker, pBlocker->pCloudClient, &ulEPCount);
	if (xRet == FTM_RET_OK)
	{
		FTM_ULONG			i;
		FTM_ULONG			ulCurrentTime = 0;
		FTM_EP				xEPInfo;
		FTM_ULONG			ulMaxDataCount = 100;
		FTM_EP_DATA_PTR		pData;

		pData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA) * ulMaxDataCount);
		if (pData == NULL)
		{
			ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]\n", sizeof(FTM_EP_DATA) * ulMaxDataCount);	
		}

		for(i = 0 ; i < ulEPCount ; i++)
		{
			FTM_CHAR	pEPID[FTM_ID_LEN+1];

			xRet = FTOM_BLOCKER_CLOUD_CLIENT_EP_getEPIDAt(pBlocker, pBlocker->pCloudClient, i, pEPID, FTM_ID_LEN);
			if (xRet == FTM_RET_OK)
			{
				xRet = FTOM_CLIENT_EP_get((FTOM_CLIENT_PTR)pBlocker->pServerClient, pEPID, &xEPInfo);
				if (xRet == FTM_RET_OK)
				{
					FTM_BOOL	bRun = FTM_FALSE;

					xRet = FTOM_CLIENT_EP_isRun((FTOM_CLIENT_PTR)pBlocker->pServerClient, pEPID, &bRun);
					if (xRet == FTM_RET_OK)
					{
						FTOM_BLOCKER_CLOUD_CLIENT_EP_updateStatus(pBlocker, pBlocker->pCloudClient, pEPID, bRun);
					}
				}

				if (xRet == FTM_RET_OK)
				{
					FTM_TIME_toSecs(&xTime, &ulCurrentTime);
				}

				if (pData != NULL)
				{
					FTM_ULONG	ulUnsyncDataCount = 0;
					FTM_ULONG	ulSendDataCount = 0;
					FTM_ULONG	ulStartTime;

					xRet = FTOM_BLOCKER_CLOUD_CLIENT_EP_DATA_getLastTime(pBlocker, pBlocker->pCloudClient, pEPID, &ulStartTime);
					if (xRet == FTM_RET_OK)
					{
						xRet = FTOM_CLIENT_EP_DATA_countWithTime((FTOM_CLIENT_PTR)pBlocker->pServerClient, pEPID, ulStartTime + 1, ulCurrentTime, &ulUnsyncDataCount);
						if (xRet == FTM_RET_OK)
						{
							while(ulSendDataCount < ulUnsyncDataCount)
							{
								FTM_ULONG	ulDataCount = 0;
	
								xRet = FTOM_CLIENT_EP_DATA_getListWithTime((FTOM_CLIENT_PTR)pBlocker->pServerClient, pEPID, ulStartTime + 1, ulCurrentTime, pData, ulMaxDataCount, &ulDataCount);
								if (xRet != FTM_RET_OK)
								{
									ERROR2(xRet, "Failed to get ep data with time!\n");
									break;
								}
	
								if (ulDataCount == 0)
								{
									break;
								}

								FTOM_BLOCKER_CLOUD_CLIENT_EP_DATA_send(pBlocker, pBlocker->pCloudClient, pEPID, pData, ulDataCount);
								if (xRet != FTM_RET_OK)
								{
									ERROR2(xRet, "Failed to send EP data!\n");	
									break;
								}
	
								ulSendDataCount += ulDataCount;
								ulStartTime = pData[ulDataCount - 1].ulTime;
							}
						}
					}
				}

			}
		}

		if (pData != NULL)
		{
			FTM_MEM_free(pData);
		}
	}

	return	FTM_RET_OK;
}
