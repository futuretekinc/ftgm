#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "ftom.h"
#include "ftdm_client.h"
#include "ftom_db.h"
#include "ftom_node.h"
#include "ftom_snmpc.h"
#include "ftom_ep.h"
#include "ftom_service.h"
#include "ftom_server.h"
#include "ftom_dmc.h"
#include "ftom_shell.h"
#include "ftom_msg.h"
#include "ftom_message_queue.h"
#include "ftom_trigger.h"
#include "ftom_action.h"
#include "ftom_rule.h"
#include "ftom_discovery.h"
#include "ftom_utils.h"
#include "ftom_logger.h"

#undef	__MODULE__
#define	__MODULE__	FTOM_TRACE_MODULE_FTOM

typedef	struct	
{
	FTM_CHAR	pIP[64];
	FTM_USHORT	usPort;
}	FTOM_SUBNET, _PTR_ FTOM_SUBNET_PTR;
	
FTM_VOID_PTR	FTOM_process
(
	FTM_VOID_PTR pData
);

FTM_RET			FTOM_TASK_startService
(
	FTM_VOID
);

FTM_RET			FTOM_TASK_stopService
(
	FTM_VOID
);

FTM_RET			FTOM_TASK_sync
(
	FTM_VOID
);

FTM_RET			FTOM_TASK_start
(
	FTM_VOID
);

FTM_RET			FTOM_TASK_stop
(
	FTM_VOID
);

FTM_RET			FTOM_TASK_processing
(
	FTM_VOID
);

static 
FTM_RET	FTOM_onQuit
(
	FTOM_MSG_TIME_SYNC_PTR	pMsg,
	FTM_VOID_PTR		pData
);

static 
FTM_RET	FTOM_onGWStatus
(
	FTOM_MSG_GW_STATUS_PTR pMsg,
	FTM_VOID_PTR		pData
);

static 
FTM_RET	FTOM_onTimeSync
(
	FTOM_MSG_TIME_SYNC_PTR	pMsg,
	FTM_VOID_PTR		pData
);

static 
FTM_RET	FTOM_onEPCtrl
(
	FTOM_MSG_EP_CTRL_PTR	pMsg,
	FTM_VOID_PTR		pData
);

static 
FTM_RET	FTOM_onEPStatus
(
	FTOM_MSG_EP_STATUS_PTR pMsg,
	FTM_VOID_PTR		pData
);

static 
FTM_RET	FTOM_onEPData
(
	FTOM_MSG_EP_DATA_PTR pMsg,
	FTM_VOID_PTR		pData
);

static 
FTM_RET	FTOM_onRule
(
	FTOM_MSG_RULE_PTR pMsg,
	FTM_VOID_PTR		pData
);

static 
FTM_RET	FTOM_onAlert
(
	FTOM_MSG_ALERT_PTR pMsg,
	FTM_VOID_PTR		pData
);

static 
FTM_RET	FTOM_onDiscovery
(
	FTOM_MSG_DISCOVERY_PTR pMsg,
	FTM_VOID_PTR		pData
);

static 
FTM_RET	FTOM_onDiscoveryInfo
(
	FTOM_MSG_DISCOVERY_INFO_PTR pMsg,
	FTM_VOID_PTR		pData
);

static 
FTM_RET	FTOM_onDiscoveryDone
(
	FTOM_MSG_DISCOVERY_DONE_PTR pMsg,
	FTM_VOID_PTR		pData
);

static 	
FTM_RET	FTOM_callback
(
	FTOM_SERVICE_ID xID, 
	FTOM_MSG_TYPE 	xMsg, 
	FTM_VOID_PTR 	pData
);

static 	FTOM_SERVICE	pServices[] =
{
	{
		.xType		=	FTOM_SERVICE_SERVER,
		.xID		=	FTOM_SERVICE_SERVER,
		.pName		=	"Server",
		.fCreate	=	(FTOM_SERVICE_CREATE)FTOM_SERVER_create,
		.fDestroy	=	(FTOM_SERVICE_DESTROY)FTOM_SERVER_destroy,
		.fInit		=	(FTOM_SERVICE_INIT)FTOM_SERVER_init,
		.fFinal		=	(FTOM_SERVICE_FINAL)FTOM_SERVER_final,
		.fStart 	=	(FTOM_SERVICE_START)FTOM_SERVER_start,
		.fStop		=	(FTOM_SERVICE_STOP)FTOM_SERVER_stop,
		.fIsRun		=	(FTOM_SERVICE_IS_RUN)FTOM_SERVER_isRun,
		.fSetCallback=	(FTOM_SERVICE_SET_CB)FTOM_SERVER_setServiceCallback,
		.fCallback	=	FTOM_callback,
		.fLoadConfig=	(FTOM_SERVICE_LOAD_CONFIG)FTOM_SERVER_loadConfig,
		.fSaveConfig=	(FTOM_SERVICE_SAVE_CONFIG)FTOM_SERVER_saveConfig,
		.fShowConfig=	(FTOM_SERVICE_SHOW_CONFIG)FTOM_SERVER_showConfig,
		.fSendMessage	=	(FTOM_SERVICE_SEND_MESSAGE)FTOM_SERVER_sendMessage,
		.pData		= 	NULL
	},
	{
		.xType		=	FTOM_SERVICE_SNMP_CLIENT,
		.xID		=	FTOM_SERVICE_SNMP_CLIENT,
		.pName		=	"SNMP Client",
		.fCreate	=	(FTOM_SERVICE_CREATE)FTOM_SNMPC_create,
		.fDestroy	=	(FTOM_SERVICE_DESTROY)FTOM_SNMPC_destroy,
		.fInit		=	(FTOM_SERVICE_INIT)FTOM_SNMPC_init,
		.fFinal		=	(FTOM_SERVICE_FINAL)FTOM_SNMPC_final,
		.fStart 	=	(FTOM_SERVICE_START)FTOM_SNMPC_start,
		.fStop		=	(FTOM_SERVICE_STOP)FTOM_SNMPC_stop,
		.fIsRun		=	NULL,
		.fSetCallback=	(FTOM_SERVICE_SET_CB)FTOM_SNMPC_setServiceCallback,
		.fCallback	=	FTOM_callback,
		.fLoadConfig=	(FTOM_SERVICE_LOAD_CONFIG)FTOM_SNMPC_loadConfig,
		.fSaveConfig=	(FTOM_SERVICE_SAVE_CONFIG)FTOM_SNMPC_saveConfig,
		.fShowConfig=	(FTOM_SERVICE_SHOW_CONFIG)FTOM_SNMPC_showConfig,
		.fSendMessage=	(FTOM_SERVICE_SEND_MESSAGE)FTOM_SNMPC_sendMessage,
		.pData		= 	NULL
	},
	{
		.xType		=	FTOM_SERVICE_DMC,
		.xID		=	FTOM_SERVICE_DMC,
		.pName		=	"DB Client",
		.fCreate	=	(FTOM_SERVICE_CREATE)FTOM_DMC_create,
		.fDestroy	=	(FTOM_SERVICE_DESTROY)FTOM_DMC_destroy,
		.fInit		=	(FTOM_SERVICE_INIT)FTOM_DMC_init,
		.fFinal		=	(FTOM_SERVICE_FINAL)FTOM_DMC_final,
		.fStart 	=	(FTOM_SERVICE_START)FTOM_DMC_start,
		.fStop		=	(FTOM_SERVICE_STOP)FTOM_DMC_stop,
		.fIsRun		=	NULL,
		.fSetCallback=	(FTOM_SERVICE_SET_CB)FTOM_DMC_setServiceCallback,
		.fCallback	=	FTOM_callback,
		.fLoadConfig=	(FTOM_SERVICE_LOAD_CONFIG)FTOM_DMC_loadConfig,
		.fSaveConfig=	(FTOM_SERVICE_SAVE_CONFIG)FTOM_DMC_saveConfig,
		.fShowConfig=	(FTOM_SERVICE_SHOW_CONFIG)FTOM_DMC_showConfig,
		.fSendMessage=	(FTOM_SERVICE_SEND_MESSAGE)FTOM_DMC_sendMessage,
		.pData		= 	NULL
	},
#if 1
	{
		.xType		=	FTOM_SERVICE_DISCOVERY,
		.xID		=	FTOM_SERVICE_DISCOVERY,
		.pName		=	"Discovery",
		.fCreate	=	(FTOM_SERVICE_CREATE)FTOM_DISCOVERY_create,
		.fDestroy	=	(FTOM_SERVICE_DESTROY)FTOM_DISCOVERY_destroy,
		.fInit		=	(FTOM_SERVICE_INIT)FTOM_DISCOVERY_init,
		.fFinal		=	(FTOM_SERVICE_FINAL)FTOM_DISCOVERY_final,
		.fStart 	=	(FTOM_SERVICE_START)FTOM_DISCOVERY_start,
		.fStop		=	(FTOM_SERVICE_STOP)FTOM_DISCOVERY_stop,
		.fIsRun		=	NULL,
		.fSetCallback=	NULL,
		.fCallback	=	NULL,
		.fShowConfig=	NULL,
		.fSendMessage=	NULL,
		.pData		= 	NULL
	},
#endif
};

extern char *program_invocation_short_name;
FTOM_CONFIG			xConfig;

FTOM_STATE			xState;
pthread_t			xThread;

FTM_BOOL			bStop;

FTOM_MSG_QUEUE_PTR	pMsgQ;
FTM_SHELL			xShell;

FTM_LIST			xSubnetList;

FTOM_ON_MESSAGE_CALLBACK	onMessage[FTOM_MSG_TYPE_MAX];
FTM_VOID_PTR				pOnMessageData[FTOM_MSG_TYPE_MAX];

FTOM_LOG_MANAGER	xLogManager;

FTM_CHAR_PTR	FTOM_getProgramName
(
	FTM_VOID
)
{
	return	program_invocation_short_name;
}

pid_t	FTOM_getPID
(
	FTM_VOID
)
{
	return	getpid();
}

FTM_RET	FTOM_init
(
	FTM_VOID
)
{
	FTM_RET	xRet;

	FTOM_getDefaultDeviceID(xConfig.pDID);
	TRACE("DID : %s\n", xConfig.pDID);

	bStop = FTM_TRUE;

	onMessage[FTOM_MSG_TYPE_QUIT] 			= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onQuit;
	onMessage[FTOM_MSG_TYPE_GW_STATUS]		= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onGWStatus;
	onMessage[FTOM_MSG_TYPE_EP_STATUS] 		= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onEPStatus;
	onMessage[FTOM_MSG_TYPE_EP_DATA] 		= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onEPData;
	onMessage[FTOM_MSG_TYPE_TIME_SYNC] 		= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onTimeSync;
	onMessage[FTOM_MSG_TYPE_EP_CTRL] 		= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onEPCtrl;
	onMessage[FTOM_MSG_TYPE_RULE] 			= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onRule;
	onMessage[FTOM_MSG_TYPE_ALERT] 			= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onAlert;
	onMessage[FTOM_MSG_TYPE_DISCOVERY] 		= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onDiscovery;
	onMessage[FTOM_MSG_TYPE_DISCOVERY_INFO]	= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onDiscoveryInfo;
	onMessage[FTOM_MSG_TYPE_DISCOVERY_DONE]	= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onDiscoveryDone;

	xRet = FTM_LIST_init(&xSubnetList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Failed to initialize list!\n");	
	}

	xRet = FTOM_LOGGER_init(&xLogManager);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Failed to initialize logger!\n");	
	}

	xRet = FTOM_MSGQ_create(&pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Message queue creation failed.\n");
		goto error;
	}

	xRet = FTOM_NODE_MODULE_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Node management creation failed.\n");
		goto error;
	}

	xRet = FTOM_EP_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"EP management creation failed.\n");
		goto error;
	}

	xRet = FTOM_TRIGGER_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Trigger management creation failed.\n");
		goto error;
	}

	xRet = FTOM_ACTION_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Action management creation failed.\n");
		goto error;
	}

	xRet = FTOM_RULE_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Rule management creation failed.\n");
		goto error;
	}

	FTOM_SERVICE_init(pServices, sizeof(pServices) / sizeof(FTOM_SERVICE));
	TRACE("initialization done.\n");

	return	FTM_RET_OK;

error:
		FTOM_RULE_final();
		FTOM_ACTION_final();
		FTOM_TRIGGER_final();
		FTOM_EP_final();
		FTOM_NODE_MODULE_final();
	
		if (pMsgQ!= NULL)
		{
			FTOM_MSGQ_destroy(&pMsgQ);
		}
	

	return	xRet;
}

FTM_RET	FTOM_final
(
	FTM_VOID
)
{
	FTM_RET	xRet;

	xRet = FTOM_SERVICE_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to finalize service!\n");
	}

	xRet = FTOM_RULE_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to finalize rule.\n");
	}

	xRet = FTOM_MSGQ_destroy(&pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to destroy Message queue.\n");
	}

	xRet = FTOM_ACTION_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to finalize action!\n");	
	}

	xRet = FTOM_TRIGGER_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to finalize trigger!\n");	
	}

	xRet = FTOM_EP_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to finalize EP!\n");	
	}

	xRet = FTOM_NODE_MODULE_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to finalize Node!\n");	
	}

	xRet = FTOM_LOGGER_final(&xLogManager);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Failed to finalize logger!\n");	
	}

	xRet =FTM_LIST_iteratorStart(&xSubnetList);
	if (xRet == FTM_RET_OK)
	{
		FTOM_SUBNET_PTR	pSubnet;
	
		while(FTM_LIST_iteratorNext(&xSubnetList, (FTM_VOID_PTR _PTR_)&pSubnet) == FTM_RET_OK)
		{
			FTM_LIST_remove(&xSubnetList, pSubnet);	
			FTM_MEM_free(pSubnet);
		}
	}

	FTM_LIST_final(&xSubnetList);

	TRACE("Finalize done.\n");

	return	FTM_RET_OK;
}

FTM_RET	FTOM_loadConfig
(
	FTM_CONFIG_PTR	pConfig
)
{
	ASSERT(pConfig != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_ITEM		xSection;	

	xRet = FTM_CONFIG_getItem(pConfig, "network", &xSection);
	if (xRet == FTM_RET_OK)
	{
		FTM_ULONG	ulCount;
		xRet = FTM_CONFIG_LIST_getItemCount(&xSection, &ulCount);
		if (xRet == FTM_RET_OK)
		{
			FTM_INT	i;

			for(i = 0  ; i < ulCount ; i++)
			{
				FTM_CONFIG_ITEM	xItem;
				FTM_CHAR		pNet[64];
				FTM_USHORT		usPort;

				xRet = FTM_CONFIG_LIST_getItemAt(&xSection, i, &xItem);
				if (xRet != FTM_RET_OK)
				{
					continue;	
				}
				
				memset(pNet, 0, sizeof(pNet));
				xRet = FTM_CONFIG_ITEM_getItemString(&xItem, "net", pNet, sizeof(pNet) - 1);
				if (xRet != FTM_RET_OK)
				{
					continue;	
				}
				
				xRet = FTM_CONFIG_ITEM_getItemUSHORT(&xItem, "port", &usPort);
				if (xRet != FTM_RET_OK)
				{
					continue;	
				}
			

				FTOM_SUBNET_PTR	pSubnet = (FTOM_SUBNET_PTR)FTM_MEM_malloc(sizeof(FTOM_SUBNET));
				if (pSubnet != NULL)
				{
					strncpy(pSubnet->pIP, pNet, sizeof(pNet) - 1);
					pSubnet->usPort = usPort;

					xRet = FTM_LIST_append(&xSubnetList, pSubnet);
					if (xRet != FTM_RET_OK)
					{
						FTM_MEM_free(pSubnet);	
					}
				}
			}
		}

	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_loadConfigFromFile
(
	FTM_CHAR_PTR 	pFileName
)
{
	ASSERT(pFileName != NULL);
	FTM_RET	xRet;
	FTM_CONFIG_PTR		pConfig;

	xRet =FTM_CONFIG_create(pFileName, &pConfig, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "SERVER configuration file[%s] load failed\n", pFileName);
		return	xRet;	
	}

	xRet = FTOM_loadConfig(pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load configuration[%s]\n", pFileName);
	}

	xRet = FTOM_SERVICE_loadConfig(FTOM_SERVICE_ALL, pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load configuration from file[%s]\n", pFileName);
	}

	xRet = FTM_TRACE_loadConfig(pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load configuration from file[%s]\n", pFileName);
	}

	FTM_CONFIG_destroy(&pConfig);

	TRACE("Configuraion loaded.\n");

	return	FTM_RET_OK;
}

FTM_RET	FTOM_saveConfigToFile
(
	FTM_CHAR_PTR	pFileName
)
{
	ASSERT(pFileName != NULL);
	FTM_RET	xRet;

	xRet =FTOM_SERVICE_saveConfigToFile(FTOM_SERVICE_ALL, pFileName);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to save configuration to file[%s]\n", pFileName);
	}

	TRACE("Configuration saved.\n");

	return	xRet;
}

FTM_RET	FTOM_showConfig
(
	FTM_VOID
)
{
	FTOM_SERVICE_showConfig(FTOM_SERVICE_ALL);

	return	FTM_RET_OK;
}

FTM_RET FTOM_start
(
	FTM_VOID
)
{
	if (!bStop)
	{
		return	FTM_RET_ALREADY_STARTED;	
	}

	if (pthread_create(&xThread, NULL, FTOM_process, NULL) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET FTOM_stop
(
	FTM_VOID
)
{
	if (bStop)
	{
		WARN("Not started\n");
		return	FTM_RET_NOT_START;	
	}

	bStop = FTM_TRUE;
	pthread_join(xThread, NULL);

	return	FTM_RET_OK;
}

FTM_RET FTOM_waitingForFinished
(
	FTM_VOID
)
{
	pthread_join(xThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTOM_process
(
	FTM_VOID_PTR 	pData
)
{
	xState = FTOM_STATE_INITIALIZED;
	bStop	= FTM_FALSE;

	while(!bStop)
	{
		switch(xState)
		{
		case	FTOM_STATE_INITIALIZED:
			{
				FTOM_TASK_startService();
			}
			break;

		case	FTOM_STATE_CONNECTED:
			{
				FTOM_TASK_sync();
			}
			break;

		case	FTOM_STATE_SYNCHRONIZED:
			{
				FTOM_TASK_start();	
			}
			break;

		case	FTOM_STATE_PROCESSING:
			{
				FTOM_TASK_processing();	
			}
			break;
		}
	}

	if (xState == FTOM_STATE_PROCESSING)
	{
		FTOM_TASK_stopService();	
		FTOM_TASK_stop();	
	}

	TRACE("finished.\n");
	return	0;
}

FTM_RET	FTOM_TASK_startService
(
	FTM_VOID
)
{
	FTOM_SERVICE_start(FTOM_SERVICE_ALL);

	xState = FTOM_STATE_CONNECTED;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TASK_sync
(
	FTM_VOID
)
{
	FTM_RET			xRet;
	FTM_BOOL		bConnected = FTM_FALSE;
	FTM_ULONG		ulCount = 0, i;
	FTOM_SERVICE_PTR	pService;

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_DMC_isConnected(pService->pData, &bConnected);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	else if (!bConnected)
	{
		return	FTM_RET_NOT_CONNECTED;
	}

	xRet = FTOM_DMC_NODE_count(pService->pData, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Node count failed to get from DB\n");
		return	xRet;	
	}

	if (ulCount != 0)
	{
		FTM_DID_PTR		pDIDs = NULL;
		pDIDs = (FTM_DID_PTR)FTM_MEM_malloc(sizeof(FTM_DID) * ulCount);
		if (pDIDs != NULL)
		{
			xRet = FTOM_DB_NODE_getDIDList(pDIDs, 0, ulCount,  &ulCount);
			if (xRet == FTM_RET_OK)
			{
				for(i = 0 ; i  < ulCount; i++)	
				{
					FTM_NODE		xInfo;
					FTOM_NODE_PTR	pNode;

					xRet = FTOM_DB_NODE_getInfo(pDIDs[i], &xInfo);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet,"Failed to get Node[%s] information!\n", pDIDs[i]);
						continue;
					}

	
					xRet = FTOM_NODE_create(&xInfo, FTM_FALSE, &pNode);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet,"Node creation failed[%08lx].\n", xRet);
						continue;	
					}
	
					TRACE("Node[%s] creating success.\n", pNode->xInfo.pDID);
				}
			}
	
			FTM_MEM_free(pDIDs);
		}
		else
		{
			ERROR2(xRet,"Not enough memory[size = %d]\n", sizeof(FTM_DID) * ulCount);
		}
	}

	xRet = FTOM_DMC_EP_CLASS_count(pService->pData, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"EP Class count get failed.\n");
		return	xRet;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_EP_CLASS	xEPClassInfo;
		FTOM_EP_CLASS_PTR	pEPClass;

		xRet = FTOM_DMC_EP_CLASS_getAt(pService->pData, i, &xEPClassInfo);
		if (xRet != FTM_RET_OK)
		{
			continue;
		}

		xRet = FTOM_EP_CLASS_create(&xEPClassInfo, &pEPClass);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet,"FTOM_EP_TYPE_append(&xEPClassInfo) = %08lx\n", xRet);
			continue;	
		}
	}

	xRet = FTOM_DMC_EP_count(pService->pData, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"EP count get failed.\n");
		return	xRet;	
	}
	TRACE("Load EP Object : %lu\n", ulCount);

	if (ulCount != 0)
	{
		FTM_EPID_PTR	pEPIDs;

		pEPIDs = (FTM_EPID_PTR)FTM_MEM_malloc(sizeof(FTM_EPID) * ulCount);
		if (pEPIDs != NULL)
		{
			xRet = FTOM_DB_EP_getEPIDList(pEPIDs, 0, ulCount,  &ulCount);
			if (xRet == FTM_RET_OK)
			{
				for(i = 0 ; i < ulCount ; i++)
				{
					FTOM_NODE_PTR	pNode;
					FTM_EP			xInfo;
					FTOM_EP_PTR		pEP;
			
					xRet = FTOM_DB_EP_getInfo(pEPIDs[i], &xInfo);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet,"Failed to get EP[%s] information!\n", pEPIDs[i]);
						continue;
					}

					xRet = FTOM_EP_create(&xInfo, FTM_FALSE, &pEP);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet,"Failed to create EP[%s]!\n", pEPIDs[i]);
						continue;	
					}
			
					if (FTOM_NODE_get(pEP->xInfo.pDID, &pNode) == FTM_RET_OK)
					{
						FTOM_NODE_linkEP(pNode, pEP);
					}
					
					TRACE("EP[%s] creating success.\n", pEPIDs[i]);
				}
			}
			FTM_MEM_free(pEPIDs);
		}
		else
		{
			ERROR2(xRet,"Not enough memory[size = %d]\n", sizeof(FTM_EPID) * ulCount);
		}
	}

	xRet = FTOM_DMC_TRIGGER_count(pService->pData, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Trigger count get failed.\n");
		return	xRet;
	}

	if (ulCount != 0)
	{
		FTM_ID_PTR	pIDs;

		pIDs = (FTM_ID_PTR)FTM_MEM_malloc(sizeof(FTM_ID) * ulCount);
		if (pIDs != NULL)
		{
			xRet = FTOM_DB_TRIGGER_getIDList(pIDs, 0, ulCount, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				for(i = 0 ; i < ulCount ; i++)
				{
					FTM_TRIGGER			xInfo;
					FTOM_TRIGGER_PTR	pTrigger = NULL;

					xRet = FTOM_DB_TRIGGER_getInfo(pIDs[i], &xInfo);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet,"Failed to get Trigger[%s] information!\n", pIDs[i]);
						continue;
					}

					xRet = FTOM_TRIGGER_create(&xInfo, FTM_FALSE, &pTrigger);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet,"The new event can not registration!\n") ;
						continue;
					}
				}
			}

			FTM_MEM_free(pIDs);
		}
		else
		{
			ERROR2(xRet,"Not enough memory[size = %d]\n", sizeof(FTM_ID) * ulCount);	
		}
	}

	xRet = FTOM_DMC_ACTION_count(pService->pData, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Action count get failed.\n");
		return	xRet;
	}

	if (ulCount != 0)
	{
		FTM_ID_PTR	pIDs;

		pIDs = (FTM_ID_PTR)FTM_MEM_malloc(sizeof(FTM_ID) * ulCount);
		if (pIDs != NULL)
		{
			xRet = FTOM_DB_ACTION_getIDList(pIDs, 0, ulCount, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				for(i = 0 ; i < ulCount ; i++)
				{
					FTM_ACTION		xInfo;
					FTOM_ACTION_PTR pAction = NULL;

					xRet = FTOM_DB_ACTION_getInfo(pIDs[i], &xInfo);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet,"Failed to get Action[%s] information!\n", pIDs[i]);
						continue;
					}

					xRet = FTOM_ACTION_create(&xInfo, FTM_FALSE, &pAction);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet,"The new action event can not registration!\n") ;
						continue;
					}
				}
			}

			FTM_MEM_free(pIDs);
		}
		else
		{
			ERROR2(xRet,"Not enough memory[size = %d]\n", sizeof(FTM_ID)*ulCount);	
		}
	}
	
	xRet = FTOM_DMC_RULE_count(pService->pData, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Rule count get failed.\n");
		return	xRet;
	}

	TRACE("DMC Rule Count : %d\n", ulCount);
	if (ulCount != 0)
	{
		FTM_ID_PTR	pIDs;

		pIDs = (FTM_ID_PTR)FTM_MEM_malloc(sizeof(FTM_ID) * ulCount);
		if (pIDs != NULL)
		{
			xRet = FTOM_DB_RULE_getIDList(pIDs, 0, ulCount, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				for(i = 0 ; i < ulCount ; i++)
				{
					FTM_RULE		xInfo;
					FTOM_RULE_PTR	pRule = NULL;

					xRet = FTOM_DB_RULE_getInfo(pIDs[i], &xInfo);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet,"Failed to get Rule[%s] information!\n", pIDs[i]);
						continue;
					}
		
					xRet = FTOM_RULE_create(&xInfo, FTM_FALSE, &pRule);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet,"The new rule can not registration!\n") ;
						continue;
					}
				}
			}
			FTM_MEM_free(pIDs);
		}
		else
		{
			ERROR2(xRet,"Not enough memory[size = %d]\n", sizeof(FTM_ID)*ulCount);	
		}
	}

	xState = FTOM_STATE_SYNCHRONIZED;
	return	FTM_RET_OK;
}

FTM_RET	FTOM_TASK_start
(
	FTM_VOID
)
{
	FTM_ULONG	i, ulCount;
	
	FTOM_RULE_start(NULL);
	FTOM_ACTION_start(NULL);
	FTOM_TRIGGER_start(NULL);

	FTOM_NODE_count(&ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTOM_NODE_PTR	pNode;

		if (FTOM_NODE_getAt(i, &pNode) == FTM_RET_OK)
		{
			FTOM_NODE_start(pNode);
		}
	}

	xState = FTOM_STATE_PROCESSING;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TASK_processing
(
	FTM_VOID
)
{
	FTM_RET			xRet;
	FTOM_MSG_PTR	pMsg = NULL;
	FTM_TIMER		xLoopTimer;

	FTM_TIMER_initMS(&xLoopTimer, FTOM_LOOP_INTERVAL);

	xRet = FTOM_MSG_createInitializeDone(NULL, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Failed to initialize message!\n");
		return	0;	
	}

	xRet = FTOM_SERVICE_sendMessage(FTOM_SERVICE_ALL, pMsg);
	if (xRet != FTM_RET_OK)
	{
		FTOM_MSG_destroy(&pMsg);	
	}

	while(!bStop)
	{
		FTM_ULONG	ulRemainTime;
		
		FTM_TIMER_remainMS(&xLoopTimer, &ulRemainTime);
		xRet = FTOM_MSGQ_timedPop(pMsgQ, ulRemainTime, &pMsg);
		if (xRet == FTM_RET_OK)
		{
			if ((pMsg->xType < FTOM_MSG_TYPE_MAX) && (onMessage[pMsg->xType] != NULL))
			{
				xRet = onMessage[pMsg->xType](pMsg, pOnMessageData[pMsg->xType]);
			}
			else
			{
				ERROR2(xRet,"Message[%08x] not supported.\n", pMsg->xType);
			}

			FTOM_MSG_destroy(&pMsg);
		}

		if (FTM_TIMER_isExpired(&xLoopTimer))
		{
			FTM_TIMER_addMS(&xLoopTimer, FTOM_LOOP_INTERVAL);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TASK_stop
(
	FTM_VOID
)
{
	FTM_ULONG	i, ulCount;
	
	FTOM_TRIGGER_stop(NULL);
	FTOM_ACTION_stop(NULL);
	FTOM_RULE_stop(NULL);
	FTOM_EP_count(0, NULL, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTOM_EP_PTR	pEP;

		if (FTOM_EP_getAt(i, &pEP) == FTM_RET_OK)
		{
			FTOM_EP_stop(pEP, FTM_TRUE);	
		}
	}

	FTOM_NODE_count(&ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTOM_NODE_PTR	pNode;

		if (FTOM_NODE_getAt(i, &pNode) == FTM_RET_OK)
		{
			FTOM_NODE_stop(pNode);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TASK_stopService
(
	FTM_VOID
)
{
	FTOM_SERVICE_stop(FTOM_SERVICE_ALL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_onQuit
(
	FTOM_MSG_TIME_SYNC_PTR	pMsg,
	FTM_VOID_PTR		pData
)
{
	bStop = FTM_TRUE;
	xState = FTOM_STATE_STOPED;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_onTimeSync
(
	FTOM_MSG_TIME_SYNC_PTR	pMsg,
	FTM_VOID_PTR		pData
)
{
	ASSERT(pMsg != NULL);

	TRACE("Time Sync - %d\n", pMsg->ulTime);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_onEPCtrl
(
	FTOM_MSG_EP_CTRL_PTR	pMsg,
	FTM_VOID_PTR		pData
)
{
	FTM_RET			xRet;
	FTOM_EP_PTR		pEP;
	FTM_EP_DATA		xData;

	TRACE("EP[%s] Control\n", pMsg->pEPID);

	xRet = FTOM_EP_get(pMsg->pEPID, &pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"EP[%s] not found.\n", pMsg->pEPID);
		return	xRet;
	}

	FTM_EP_DATA_initINT(&xData, pMsg->xCtrl);

	xRet = FTOM_EP_remoteSetData(pEP, &xData);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"EP[%s] set failed.\n", pMsg->pEPID);
	}

	return	xRet;
}
/**********************************************************************
 * Gateway functions
 **********************************************************************/
FTM_RET	FTOM_onGWStatus
(
	FTOM_MSG_GW_STATUS_PTR pMsg,
	FTM_VOID_PTR		pData
)
{
	ASSERT(pMsg != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	FTOM_MSG_PTR	pNewMsg;
	FTM_BOOL		bRun = FTM_FALSE;

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_SERVER, &pService);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Service[SERVER] not found\n");
		return	xRet;	
	}

	if ((pService->fIsRun == NULL) || (pService->fSendMessage == NULL))
	{
		ERROR2(xRet,"Service[SERVER] not found\n");
		return	FTM_RET_OK;			
	}

	xRet = pService->fIsRun(pService->pData, &bRun);
	if ((xRet != FTM_RET_OK) || (!bRun))
	{
		return	FTM_RET_OK;	
	}

	xRet = FTOM_MSG_copy((FTOM_MSG_PTR)pMsg, &pNewMsg);	
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	pService->fSendMessage(pService->pData, pNewMsg);
}

FTM_RET	FTOM_onEPStatus
(
	FTOM_MSG_EP_STATUS_PTR pMsg,
	FTM_VOID_PTR		pData
)
{
	ASSERT(pMsg != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	FTOM_MSG_PTR	pNewMsg;
	FTM_BOOL		bRun = FTM_FALSE;

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_SERVER, &pService);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Service[SERVER] not found\n");
		return	xRet;	
	}

	if ((pService->fIsRun == NULL) || (pService->fSendMessage == NULL))
	{
		ERROR2(xRet, "Service[SERVER] not found\n");
		return	FTM_RET_OK;			
	}

	xRet = pService->fIsRun(pService->pData, &bRun);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Service[SERVER] status unknown.\n");
		return	FTM_RET_OK;	
	}

	if (!bRun)
	{
		ERROR2(xRet, "Service[SERVER] stopped.\n");
		return	FTM_RET_OK;	
	}

	xRet = FTOM_MSG_copy((FTOM_MSG_PTR)pMsg, &pNewMsg);	
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to copy message.\n");
		return	xRet;	
	}

	xRet = pService->fSendMessage(pService->pData, pNewMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to send message.\n");
		FTOM_MSG_destroy(&pNewMsg);	
	}

	return	xRet;
}

FTM_RET	FTOM_onEPData
(
	FTOM_MSG_EP_DATA_PTR pMsg,
	FTM_VOID_PTR		pData
)
{
	ASSERT(pMsg != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	FTOM_MSG_PTR	pNewMsg;
	FTM_BOOL		bRun = FTM_FALSE;

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_SERVER, &pService);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Service[SERVER] not found\n");
		return	xRet;	
	}

	if ((pService->fIsRun == NULL) || (pService->fSendMessage == NULL))
	{
		ERROR2(xRet,"Service[SERVER] not found\n");
		return	FTM_RET_OK;			
	}

	xRet = pService->fIsRun(pService->pData, &bRun);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Service[SERVER] status is unknwon.\n");
		return	FTM_RET_OK;	
	}

	if (!bRun)
	{
		ERROR2(xRet, "Service[SERVER] stopped.\n");
		return	FTM_RET_OK;	
	}

	xRet = FTOM_MSG_copy((FTOM_MSG_PTR)pMsg, &pNewMsg);	
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to copy message.\n");
		return	xRet;	
	}

	xRet = pService->fSendMessage(pService->pData, pNewMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to send message.\n");
		FTOM_MSG_destroy(&pNewMsg);	
	}

	return	xRet;
}

FTM_RET	FTOM_onRule
(
	FTOM_MSG_RULE_PTR pMsg,
	FTM_VOID_PTR		pData
)
{
	ASSERT(pMsg != NULL);

	TRACE("RULE[%s] is %s\n", pMsg->pRuleID, (pMsg->xRuleState == FTM_RULE_STATE_ACTIVATE)?"ACTIVATE":"DEACTIVATE");

	return	FTM_RET_OK;
}

FTM_RET	FTOM_onAlert
(
	FTOM_MSG_ALERT_PTR	pMsg,
	FTM_VOID_PTR		pData
)
{
	ASSERT(pMsg != NULL);
	FTM_RET		xRet;
	FTOM_EP_PTR	pEP;

	xRet = FTOM_EP_get(pMsg->pEPID, &pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get EP[%s].\n",	pMsg->pEPID);
		return	xRet;
	}

	xRet = FTOM_EP_setData(pEP, &pMsg->xData);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to set EP[%s] data.\n", pMsg->pEPID);	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_onDiscovery
(
	FTOM_MSG_DISCOVERY_PTR	pMsg,
	FTM_VOID_PTR		pData
)
{
	ASSERT(pMsg != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DISCOVERY, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTOM_DISCOVERY_startSearch(pService->pData, pMsg->pNetwork, pMsg->usPort, pMsg->ulRetryCount);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_onDiscoveryInfo
(
	FTOM_MSG_DISCOVERY_INFO_PTR	pMsg,
	FTM_VOID_PTR		pData
)
{
	ASSERT(pMsg != NULL);
	
	FTM_INT	i;

	TRACE("MSG : DISCOVERY INFO\n");
	TRACE("DID - %s\n", pMsg->pDID);
	for(i = 0 ; i < pMsg->ulCount; i++)
	{
		TRACE("TYPE[%d] - %s[%08x]\n", i, FTM_EP_typeString(pMsg->pTypes[i]), pMsg->pTypes[i]);	
	}
	return	FTM_RET_OK;
}

FTM_RET	FTOM_onDiscoveryDone
(
	FTOM_MSG_DISCOVERY_DONE_PTR	pMsg,
	FTM_VOID_PTR		pData
)
{
	ASSERT(pMsg != NULL);
	
	TRACE("Discovery Done!\n");
	FTM_INT	i;

	for(i = 0 ; i < pMsg->ulNodeCount ; i++)
	{
		TRACE("NODE[%d] : %s\n", pMsg->pNodeInfos[i].pDID);	
	}

	for(i = 0 ; i < pMsg->ulEPCount ; i++)
	{
		TRACE("EP[%s] : %08x\n", pMsg->pEPInfos[i].pEPID);	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_setMessageCallback
(
	FTOM_MSG_TYPE 				xMsg, 
	FTOM_ON_MESSAGE_CALLBACK	fMessageCB,
	FTM_VOID_PTR				pData,
	FTOM_ON_MESSAGE_CALLBACK _PTR_	pOldCB,
	FTM_VOID_PTR _PTR_	ppOldData

)
{
	ASSERT(fMessageCB != NULL);

	if (xMsg >= FTOM_MSG_TYPE_MAX)
	{
		ERROR2(FTM_RET_INVALID_MESSAGE_TYPE, "Message type[%08x] is invalid.\n", xMsg);
		return	FTM_RET_INVALID_MESSAGE_TYPE;
	}

	if (pOldCB != NULL)
	{
		*pOldCB = onMessage[xMsg];
	}

	if (ppOldData != NULL)
	{
		*ppOldData = pOnMessageData[xMsg];
	}

	onMessage[xMsg] = fMessageCB;
	pOnMessageData[xMsg] = pData;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_getDID
(
	FTM_CHAR_PTR 	pBuff, 
	FTM_ULONG 		ulBuffLen
)
{
	ASSERT(pBuff != NULL);

	if (ulBuffLen < strlen(xConfig.pDID) + 1)
	{
		return	FTM_RET_BUFFER_TOO_SMALL;	
	}

	strcpy(pBuff, xConfig.pDID);

	return	FTM_RET_OK;
}


FTM_RET	FTOM_callback
(
	FTOM_SERVICE_ID 	xID, 
	FTOM_MSG_TYPE 		xMsg, 
	FTM_VOID_PTR 		pData
)
{
	switch(xID)
	{
	case	FTOM_SERVICE_SERVER:
		{
		}
		break;

	case	FTOM_SERVICE_SNMP_CLIENT:
		{
		}
		break;

	case	FTOM_SERVICE_SNMPTRAPD:
		{
		}
		break;

	case	FTOM_SERVICE_DMC:
		{
		}
		break;

	default:
		{
			ERROR2(FTM_RET_INVALID_TYPE,"Invalid service ID[%08x] received.\n", xID);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SYS_EP_publishStatus
(
	FTM_CHAR_PTR	pEPID,
	FTM_BOOL		bStatus,
	FTM_ULONG		ulTimeout
)
{
	FTM_RET			xRet;
	FTOM_MSG_PTR	pMsg;

	xRet = FTOM_MSG_createEPStatus(NULL, pEPID, bStatus, ulTimeout, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		WARN("Send EP data message creation failed.\n");
		return	xRet;	
	}


	xRet = FTOM_MSGQ_push(pMsgQ, (FTOM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Message push error!\n");
		FTOM_MSG_destroy((FTOM_MSG_PTR _PTR_)&pMsg);
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SYS_EP_publishData
(
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR	pData,
	FTM_ULONG		ulCount
)
{
	ASSERT(pData != NULL);

	FTM_RET			xRet;
	FTOM_MSG_PTR	pMsg;

	xRet = FTOM_MSG_createEPData(NULL, pEPID, pData, ulCount, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		WARN("Send EP data message creation failed.\n");
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(pMsgQ, (FTOM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Message push error!\n");
		FTOM_MSG_destroy((FTOM_MSG_PTR _PTR_)&pMsg);
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_sendAlert
(
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR	pData
)
{
	ASSERT(pData != NULL);

	FTM_RET				xRet;
	FTOM_MSG_ALERT_PTR	pMsg;

	xRet = FTOM_MSG_createAlert(NULL, pEPID, pData, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(pMsgQ, (FTOM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Message push error!\n");
		FTOM_MSG_destroy((FTOM_MSG_PTR _PTR_)&pMsg);
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_receivedDiscovery
(
	FTM_CHAR_PTR	pName,
	FTM_CHAR_PTR	pDID,
	FTM_CHAR_PTR	pIP,
	FTM_EP_TYPE_PTR	pTypes,
	FTM_ULONG		ulCount
)
{
	ASSERT(pDID != NULL);
	ASSERT(pTypes != NULL);

	FTM_RET				xRet;
	FTOM_MSG_DISCOVERY_INFO_PTR	pMsg;

	xRet = FTOM_MSG_createDiscoveryInfo(NULL, pName, pDID, pIP, pTypes, ulCount, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create message!\n");
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(pMsgQ, (FTOM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Message push error!\n");
		FTOM_MSG_destroy((FTOM_MSG_PTR _PTR_)&pMsg);
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_discoveryStart
(
	FTM_CHAR_PTR	pNetwork,
	FTM_USHORT		usPort,
	FTM_ULONG		ulRetryCount
)
{
	ASSERT(pNetwork != NULL);

	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DISCOVERY, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTOM_DISCOVERY_startSearch(pService->pData, pNetwork, usPort, ulRetryCount);
}

FTM_RET	FTOM_discoveryIsFinished
(
	FTM_BOOL_PTR	pbFinished
)
{
	ASSERT(pbFinished != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DISCOVERY, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTOM_DISCOVERY_isFinished(pService->pData, pbFinished);
}

FTM_RET	FTOM_discoveryNodeCount
(
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pulCount != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DISCOVERY, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTOM_DISCOVERY_getNodeInfoCount(pService->pData, pulCount);
}

FTM_RET	FTOM_discoveryEPCount
(
	FTOM_NODE_PTR	pNode,
	FTM_EP_TYPE		xType,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pNode != NULL);
	ASSERT(pulCount != NULL);
	FTM_RET	xRet;

	xRet = FTOM_NODE_getEPCount(pNode, xType, pulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"SNMP client get EP count failed.\n");	
	}

	return	xRet;
}

FTM_RET	FTOM_discoveryEP
(
	FTOM_NODE_PTR	pNode,
	FTM_EP_TYPE		xType,
	FTM_ULONG		ulIndex,
	FTM_EP_PTR		pEPInfo
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEPInfo != NULL);

	FTM_RET		xRet;

	FTM_EP_setDefault(pEPInfo);

	pEPInfo->xType = xType;

	xRet = FTOM_NODE_getEPID(pNode, xType, ulIndex, pEPInfo->pEPID, FTM_EPID_LEN);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"SNMP get EPID failed!\n");
		return	xRet;	
	}

	xRet = FTOM_NODE_getEPName(pNode, xType, ulIndex, pEPInfo->pName, FTM_NAME_LEN);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"SNMP get EP Name failed!\n");
		return	xRet;	
	}

	xRet = FTOM_NODE_getEPState(pNode, xType, ulIndex, &pEPInfo->bEnable);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"SNMP get EP state failed!\n");
		return	xRet;	
	}

	if ((xType != FTM_EP_TYPE_DO) && (xType < FTM_EP_TYPE_DEVICE))
	{
		xRet = FTOM_NODE_getEPUpdateInterval(pNode, xType, ulIndex, &pEPInfo->ulUpdateInterval);
		if (xRet != FTM_RET_OK)
		{
			if (xType != FTM_EP_TYPE_DI)
			{
				ERROR2(xRet,"SNMP get EP[%s] interval failed!\n", pEPInfo->pEPID);
				return	xRet;	
			}
		}
	}

	FTM_EP_getDefaultUnit(xType, pEPInfo->pUnit, FTM_UNIT_LEN);

	strcpy(pEPInfo->pDID, pNode->xInfo.pDID);

	if (strlen(pEPInfo->pEPID) == 8)
	{
		FTM_INT		nLen;
		FTM_CHAR	pBuff[FTM_EPID_LEN + FTM_DID_LEN+1];

		sprintf(pBuff, "%s%s", pEPInfo->pDID, pEPInfo->pEPID);

		nLen = strlen(pBuff);
		if (nLen > 14)
		{
			strncpy(pEPInfo->pEPID,  &pBuff[nLen - 14], FTM_EPID_LEN); 
		}
		else
		{
			strcpy(pEPInfo->pEPID, pBuff);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_sendMessage
(
	FTOM_SERVICE_TYPE	xService,
	FTOM_MSG_PTR		pMsg
)
{
	FTM_RET	xRet;
	FTOM_SERVICE_PTR	pService;

	xRet = FTOM_SERVICE_get(xService, &pService);
	if ((xRet != FTM_RET_OK) || (pService->fSendMessage == NULL))
	{
		ERROR2(xRet, "Send message not supported on service[%d]\n", xService);
		xRet = FTOM_MSGQ_push(pMsgQ, pMsg);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to send message to main!\n");	
		}
	}
	else
	{
		xRet = pService->fSendMessage(pService->pData, pMsg);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to send message to service[%d].\n", xService);
		}
	}

	return	xRet;
}

FTM_RET	FTOM_serverSync
(
	FTM_BOOL			bAutoRegister
)
{
	FTM_RET		xRet;
	FTOM_SERVICE_PTR	pService;
	FTOM_MSG_SERVER_SYNC_PTR	pMsg;

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_SERVER, &pService);
	if (xRet != FTM_RET_OK)
	{
		WARN("Server not supported!\n");
		return	xRet;	
	}

	if (pService->fSendMessage == NULL)
	{
		xRet = FTM_RET_FUNCTION_NOT_SUPPORTED;
		ERROR2(xRet, "Not supported send message on server.\n");
		return	xRet;
	}

	xRet = FTOM_MSG_createServerSync(NULL, bAutoRegister, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Message creation failed!\n");
		return	xRet;	
	}

	xRet = pService->fSendMessage(pService->pData, (FTOM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to send message to server.\n");
	}

	return	xRet;

}

FTM_RET	FTOM_getDefaultUpdateInterval
(
	FTM_ULONG_PTR	pulUpdateInterval
)
{
	ASSERT(pulUpdateInterval != NULL);

	*pulUpdateInterval = FTOM_DEFAULT_UPDATE_INTERVAL;

	return	FTM_RET_OK;
}


/*********************************************************************
 * Log messages
 *********************************************************************/
FTM_RET	FTOM_addNodeCreationLog
(
	FTOM_NODE_PTR	pNode
)
{
	ASSERT(pNode != NULL);
	
	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_CREATE_NODE;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pNode->xInfo.pDID, FTM_DID_LEN);

	xRet = FTOM_LOGGER_add(&xLogManager, pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}

FTM_RET	FTOM_addNodeRemovalLog
(
	FTM_CHAR_PTR	pDID
)
{
	ASSERT(pDID != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_DESTROY_NODE;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pDID, FTM_DID_LEN);

	xRet = FTOM_LOGGER_add(&xLogManager, pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}

FTM_RET	FTOM_addEPCreationLog
(
	FTOM_EP_PTR		pEP
)
{
	ASSERT(pEP != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_CREATE_EP;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pEP->xInfo.pEPID, FTM_EPID_LEN);

	xRet = FTOM_LOGGER_add(&xLogManager, pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}

FTM_RET	FTOM_addEPRemovalLog
(
	FTM_CHAR_PTR	pEPID
)
{
	ASSERT(pEPID != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_DESTROY_EP;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pEPID, FTM_DID_LEN);

	xRet = FTOM_LOGGER_add(&xLogManager, pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}

FTM_RET	FTOM_addTriggerCreationLog
(
	FTOM_TRIGGER_PTR	pTrigger
)
{
	ASSERT(pTrigger != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_CREATE_TRIGGER;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pTrigger->xInfo.pID, FTM_ID_LEN);

	xRet = FTOM_LOGGER_add(&xLogManager, pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}

FTM_RET	FTOM_addTriggerRemovalLog
(
	FTM_CHAR_PTR	pID
)
{
	ASSERT(pID != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_DESTROY_TRIGGER;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pID, FTM_DID_LEN);

	xRet = FTOM_LOGGER_add(&xLogManager, pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}

FTM_RET	FTOM_addActionCreationLog
(
	FTOM_ACTION_PTR	pAction
)
{
	ASSERT(pAction != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_CREATE_ACTION;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pAction->xInfo.pID, FTM_ID_LEN);

	xRet = FTOM_LOGGER_add(&xLogManager, pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}

FTM_RET	FTOM_addActionRemovalLog
(
	FTM_CHAR_PTR	pID
)
{
	ASSERT(pID != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_DESTROY_ACTION;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pID, FTM_DID_LEN);

	xRet = FTOM_LOGGER_add(&xLogManager, pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}

FTM_RET	FTOM_addRuleCreationLog
(
	FTOM_RULE_PTR	pRule
)
{
	ASSERT(pRule != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_CREATE_RULE;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pRule->xInfo.pID, FTM_ID_LEN);

	xRet = FTOM_LOGGER_add(&xLogManager, pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}

FTM_RET	FTOM_getLogCount
(
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pulCount != NULL);

	return	FTM_LOGGER_count(&xLogManager.xLogger, pulCount);
}

FTM_RET	FTOM_addRuleRemovalLog
(
	FTM_CHAR_PTR	pID
)
{
	ASSERT(pID != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType = FTM_LOG_TYPE_DESTROY_RULE;
	pLog->ulTime = time(NULL);
	strncpy(pLog->xParams.xCreateObject.pObjectID, pID, FTM_DID_LEN);

	xRet = FTOM_LOGGER_add(&xLogManager, pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}

FTM_RET	FTOM_addEventCreationLog
(
	FTOM_RULE_PTR	pRule
)
{
	ASSERT(pRule != NULL);

	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_create(&pLog);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pLog->xType 	= FTM_LOG_TYPE_EVENT;
	pLog->ulTime 	= time(NULL);
	strncpy(pLog->xParams.xEvent.pRuleID, pRule->xInfo.pID, FTM_ID_LEN);
	pLog->xParams.xEvent.bOccurred = pRule->bActive;

	xRet = FTOM_LOGGER_add(&xLogManager, pLog);
	if (xRet != FTM_RET_OK)
	{
		FTM_LOG_destroy(&pLog);
	}

	return	xRet;
}

FTM_RET	FTOM_getLogsAt
(
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_LOG_PTR		pLogs,
	FTM_ULONG_PTR	pulCount
)
{
	return	FTOM_LOGGER_getAt(&xLogManager, ulIndex, ulCount, pLogs, pulCount);
}

FTM_RET	FTOM_removeLogsFrom
(
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulRemovedCount
)
{
	return	FTOM_LOGGER_remove(&xLogManager, ulIndex, ulCount, pulRemovedCount);
}

