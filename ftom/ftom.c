#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "ftom.h"
#include "ftdm_client.h"
#include "ftom_node.h"
#include "ftom_snmpc.h"
#include "ftom_ep.h"
#include "ftom_service.h"
#include "ftom_server.h"
#include "ftom_dmc.h"
#include "ftom_shell.h"
#include "ftom_mqtt_client.h"
#include "ftom_msg.h"
#include "ftom_message_queue.h"
#include "ftom_trigger.h"
#include "ftom_action.h"
#include "ftom_rule.h"
#include "ftom_discovery.h"
#include "ftom_utils.h"
#include "ftom_logger.h"
#include "ftom_node_snmp_client.h"

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
FTM_RET	FTOM_onReportGWStatus
(
	FTOM_MSG_REPORT_GW_STATUS_PTR pMsg,
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
FTM_RET FTOM_onAddEPData
(
	FTOM_MSG_ADD_EP_DATA_PTR	pMsg,
	FTM_VOID_PTR		pData
);

static 
FTM_RET	FTOM_onSendEPStatus
(
	FTOM_MSG_SEND_EP_STATUS_PTR pMsg,
	FTM_VOID_PTR		pData
);

static 
FTM_RET	FTOM_onSendEPData
(
	FTOM_MSG_SEND_EP_DATA_PTR pMsg,
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
};

extern char *program_invocation_short_name;
FTOM_CONFIG			xConfig;

FTOM_STATE			xState;
pthread_t			xThread;

FTM_BOOL			bStop;

FTOM_MSG_QUEUE_PTR	pMsgQ;
FTM_SHELL			xShell;

FTOM_ON_MESSAGE_CALLBACK	onMessage[FTOM_MSG_TYPE_MAX];
FTM_VOID_PTR				pOnMessageData[FTOM_MSG_TYPE_MAX];

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
	onMessage[FTOM_MSG_TYPE_REPORT_GW_STATUS]=(FTOM_ON_MESSAGE_CALLBACK)FTOM_onReportGWStatus;
	onMessage[FTOM_MSG_TYPE_ADD_EP_DATA] 	= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onAddEPData;
	onMessage[FTOM_MSG_TYPE_SEND_EP_STATUS] = (FTOM_ON_MESSAGE_CALLBACK)FTOM_onSendEPStatus;
	onMessage[FTOM_MSG_TYPE_SEND_EP_DATA] 	= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onSendEPData;
	onMessage[FTOM_MSG_TYPE_TIME_SYNC] 		= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onTimeSync;
	onMessage[FTOM_MSG_TYPE_EP_CTRL] 		= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onEPCtrl;
	onMessage[FTOM_MSG_TYPE_RULE] 			= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onRule;
	onMessage[FTOM_MSG_TYPE_ALERT] 			= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onAlert;
	onMessage[FTOM_MSG_TYPE_DISCOVERY] 		= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onDiscovery;
	onMessage[FTOM_MSG_TYPE_DISCOVERY_INFO]	= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onDiscoveryInfo;
	onMessage[FTOM_MSG_TYPE_DISCOVERY_DONE]	= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onDiscoveryDone;

	xRet = FTOM_LOGGER_init();
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

	xRet = FTOM_NODE_init();
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
		FTOM_NODE_final();
	
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

	FTOM_SERVICE_final();

	xRet = FTOM_RULE_final();
	if (xRet != FTM_RET_OK)
	{
		WARN("Rule management destruction failed.\n");
	}

	xRet = FTOM_MSGQ_destroy(&pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		WARN("Message queue destruction failed.\n");
	}

	FTOM_ACTION_final();
	FTOM_TRIGGER_final();

	FTOM_EP_final();
	FTOM_NODE_final();

	xRet = FTOM_LOGGER_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Failed to finalize logger!\n");	
	}

	TRACE("Finalize done.\n");

	return	FTM_RET_OK;
}

FTM_RET	FTOM_loadConfigFromFile
(
	FTM_CHAR_PTR 	pFileName
)
{
	ASSERT(pFileName != NULL);
	FTM_RET	xRet;

	xRet = FTOM_SERVICE_loadConfigFromFile(FTOM_SERVICE_ALL, pFileName);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load configuration from file[%s]\n", pFileName);
	}

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
			xRet = FTOM_DMC_NODE_getDIDList(pService->pData, pDIDs, 0, ulCount,  &ulCount);
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

	
					xRet = FTOM_NODE_create(&xInfo, &pNode);
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
			xRet = FTOM_DMC_EP_getEPIDList(pService->pData, pEPIDs, 0, ulCount,  &ulCount);
			if (xRet == FTM_RET_OK)
			{
				for(i = 0 ; i < ulCount ; i++)
				{
					FTOM_NODE_PTR	pNode;
					FTOM_EP_PTR		pEP;
			
					xRet = FTOM_EP_createFromDB(pEPIDs[i], &pEP);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet,"EP[%s] object creation failed\n", pEPIDs[i]);
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
			xRet = FTOM_DMC_TRIGGER_getIDList(pService->pData, pIDs, 0, ulCount, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				for(i = 0 ; i < ulCount ; i++)
				{
					FTOM_TRIGGER_PTR	pTrigger = NULL;
			
					xRet = FTOM_TRIGGER_createFromDB(pIDs[i], &pTrigger);
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
			xRet = FTOM_DMC_ACTION_getIDList(pService->pData, pIDs, 0, ulCount, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				for(i = 0 ; i < ulCount ; i++)
				{
					FTOM_ACTION_PTR pAction = NULL;
			
					xRet = FTOM_ACTION_createFromDB(pIDs[i], &pAction);
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
			xRet = FTOM_DMC_RULE_getIDList(pService->pData, pIDs, 0, ulCount, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				for(i = 0 ; i < ulCount ; i++)
				{
					FTOM_RULE_PTR	pRule = NULL;
		
					xRet = FTOM_RULE_createFromDB(pIDs[i], &pRule);
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

	xRet = FTOM_MSG_createInitializeDone(&pMsg);
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
			TRACE("Reqiest stop EP[%s].\n", pEP->xInfo.pEPID);
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

FTM_RET FTOM_onAddEPData
(
	FTOM_MSG_ADD_EP_DATA_PTR	pMsg,
	FTM_VOID_PTR		pData
)
{
	ASSERT(pMsg != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pBuff[64];
	FTOM_SERVICE_PTR	pService;

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	FTM_EP_DATA_snprint(pBuff, sizeof(pBuff), &pMsg->xData);

	FTOM_TRIGGER_updateEP(pMsg->pEPID, &pMsg->xData);
	FTOM_DMC_EP_DATA_add(pService->pData, pMsg->pEPID, &pMsg->xData);

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
FTM_RET	FTOM_onReportGWStatus
(
	FTOM_MSG_REPORT_GW_STATUS_PTR pMsg,
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

FTM_RET	FTOM_onSendEPStatus
(
	FTOM_MSG_SEND_EP_STATUS_PTR pMsg,
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

FTM_RET	FTOM_onSendEPData
(
	FTOM_MSG_SEND_EP_DATA_PTR pMsg,
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

	TRACE("MSG : ON ALERT\n");
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
	
	TRACE("MSG : ON DISCOVERY\n");
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DISCOVERY, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTOM_DISCOVERY_call(pService->pData, pMsg->pNetwork, pMsg->usPort, pMsg->ulRetryCount);

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

	TRACE("Set Message CB : %08x, %08x\n", xMsg, fMessageCB);
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

/******************************************************************
 * Node management interface
 ******************************************************************/
FTM_RET	FTOM_DB_NODE_add
(
	FTM_NODE_PTR 	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}
	xRet = FTOM_DMC_NODE_add(pService->pData, pInfo);
	if (xRet != FTM_RET_OK)
	{
		WARN("Failed to add Node[%s] to DB.\n", pInfo->pDID);	
	}

	return	xRet;
}

FTM_RET	FTOM_DB_NODE_remove
(
	FTM_CHAR_PTR	pDID
)
{
	ASSERT(pDID != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}
	xRet = FTOM_DMC_NODE_remove(pService->pData, pDID);
	if (xRet != FTM_RET_OK)
	{
		INFO("Failed to remove Node[%s] from DB.\n", pDID);	
	}

	return	xRet;
}

FTM_RET	FTOM_DB_NDOE_count
(
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pulCount != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}
	xRet = FTOM_DMC_NODE_count(pService->pData, pulCount);
	if (xRet != FTM_RET_OK)
	{
		INFO("Failed to get node count from DB.\n");
	}

	return	xRet;
}

FTM_RET	FTOM_DB_NODE_getInfo
(
	FTM_CHAR_PTR	pDID,
	FTM_NODE_PTR	pInfo
)
{
	ASSERT(pDID != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}
	xRet = FTOM_DMC_NODE_get(pService->pData, pDID, pInfo);
	if (xRet != FTM_RET_OK)
	{
		INFO("Node[%s] is not found!\n", pDID);
	}

	return	xRet;
}

FTM_RET	FTOM_DB_NODE_getInfoAt
(
	FTM_ULONG		ulIndex,
	FTM_NODE_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}
	xRet = FTOM_DMC_NODE_getAt(pService->pData, ulIndex, pInfo);
	if (xRet != FTM_RET_OK)
	{
		INFO("Node[%d] is not found!\n", ulIndex);
	}

	return	xRet;
}

FTM_RET	FTOM_DB_NODE_set
(
	FTM_CHAR_PTR	pDID,
	FTM_ULONG		xFields,
	FTM_NODE_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}
	xRet = FTOM_DMC_NODE_set(pService->pData, pDID, xFields, pInfo);
	if (xRet != FTM_RET_OK)
	{
		INFO("Node[%s] is not found!\n", pDID);
	}

	return	xRet;
}

/******************************************************************
 * EP management interface
 ******************************************************************/

FTM_RET	FTOM_DB_EP_add
(
	FTM_EP_PTR 	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_DMC_EP_add(pService->pData, pInfo);
	if (xRet != FTM_RET_OK)
	{
		WARN("Failed to add EP[%s] to DB!\n", pInfo->pEPID);
	}

	return	xRet;
}

FTM_RET	FTOM_DB_EP_remove
(
	FTM_CHAR_PTR	pEPID
)
{
	ASSERT(pEPID != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_DMC_EP_remove(pService->pData, pEPID);
	if (xRet != FTM_RET_OK)
	{
		WARN("Failed to remove EP[%s] from DB!\n", pEPID);
	}

	return	xRet;
}

FTM_RET	FTOM_DB_EP_getInfo
(
	FTM_CHAR_PTR	pEPID,
	FTM_EP_PTR		pInfo
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_DMC_EP_get(pService->pData, pEPID, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"EP[%s] failed to get info from DB.\n", pEPID);
	}

	return	xRet;
}

FTM_RET	FTOM_DB_EP_setInfo
(
	FTM_CHAR_PTR	pEPID,
	FTM_EP_FIELD	xFields,
	FTM_EP_PTR		pInfo
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_DMC_EP_set(pService->pData, pEPID, xFields, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"EP[%s] failed to set info to DB.\n", pEPID);
	}

	return	xRet;
}

FTM_RET	FTOM_DB_EP_getDataList
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG 		ulStart, 
	FTM_EP_DATA_PTR pDataList, 
	FTM_ULONG 		ulMaxCount, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pDataList != NULL);
	ASSERT(pulCount != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTOM_DMC_EP_DATA_get(pService->pData, pEPID, ulStart, pDataList, ulMaxCount, pulCount);
}

FTM_RET	FTOM_DB_EP_getDataInfo
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR 	pulBeginTime, 
	FTM_ULONG_PTR 	pulEndTime, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pEPID != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTOM_DMC_EP_DATA_info(pService->pData, pEPID, pulBeginTime, pulEndTime, pulCount);
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

/****************************************************************
 * Gateway management
 ****************************************************************/
FTM_RET	FTOM_SYS_GW_reportStatus
(
	FTM_CHAR_PTR	pGatewayID,
	FTM_BOOL		bStatus,
	FTM_ULONG		ulTimeout
)
{
	FTM_RET							xRet;
	FTOM_MSG_REPORT_GW_STATUS_PTR	pMsg;

	xRet = FTOM_MSG_createReportGWStatus(pGatewayID, bStatus, ulTimeout, &pMsg);
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

/************************************************************
 *	Node management 
 ************************************************************/

FTM_RET	FTOM_DB_EP_addData
(
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR pData
)
{
	ASSERT(pData != NULL);

	FTM_RET						xRet;
	FTOM_MSG_ADD_EP_DATA_PTR	pMsg;

	xRet = FTOM_MSG_createAddEPData(pEPID, pData, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		WARN("Save EP data message creation failed.\n");
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

FTM_RET	FTOM_SYS_EP_publishStatus
(
	FTM_CHAR_PTR	pEPID,
	FTM_BOOL		bStatus,
	FTM_ULONG		ulTimeout
)
{
	FTM_RET						xRet;
	FTOM_MSG_SEND_EP_STATUS_PTR	pMsg;

	xRet = FTOM_MSG_createSendEPStatus(pEPID, bStatus, ulTimeout, &pMsg);
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

	FTM_RET						xRet;
	FTOM_MSG_SEND_EP_DATA_PTR	pMsg;

	xRet = FTOM_MSG_createSendEPData(pEPID, pData, ulCount, &pMsg);
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

FTM_RET	FTOM_DB_EP_getDataCount
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pulCount != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Service DMC  not found!\n");
		return	xRet;
	}

	xRet = FTOM_DMC_EP_DATA_count(pService->pData, pEPID, pulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Failed to EP[%s] data count from DMC!\n", pEPID);
	}

	return	xRet;
}

FTM_RET	FTOM_DB_EP_removeData
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulDeletedCount
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pulDeletedCount != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTOM_DMC_EP_DATA_remove(pService->pData, pEPID, ulIndex, ulCount, pulDeletedCount);
}

FTM_RET	FTOM_DB_EP_removeDataWithTime
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulBegin,
	FTM_ULONG		ulEnd,
	FTM_ULONG_PTR	pulDeletedCount
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pulDeletedCount != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTOM_DMC_EP_DATA_removeWithTime(pService->pData, pEPID, ulBegin, ulEnd, pulDeletedCount);
}

FTM_RET	FTOM_DB_TRIGGER_add
(
	FTM_TRIGGER_PTR		pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_DMC_TRIGGER_add(pService->pData, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Trigger[%s] failed to add to DB.\n", pInfo->pID);
	}

	return	xRet;
}

FTM_RET	FTOM_DB_TRIGGER_remove
(
	FTM_CHAR_PTR	pTriggerID
)
{
	ASSERT(pTriggerID != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_DMC_TRIGGER_remove(pService->pData, pTriggerID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Trigger[%s] failed to remove from DB.\n", pTriggerID);
	}

	return	xRet;
}

FTM_RET	FTOM_DB_TRIGGER_getInfo
(
	FTM_CHAR_PTR	pTriggerID,
	FTM_TRIGGER_PTR	pInfo
)
{
	ASSERT(pTriggerID != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_DMC_TRIGGER_get(pService->pData, pTriggerID, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Trigger[%s] failed to get information.\n", pTriggerID);	
	}

	return	xRet;
}

FTM_RET	FTOM_DB_TRIgger_getInfoAt
(
	FTM_ULONG		ulIndex,
	FTM_TRIGGER_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_DMC_TRIGGER_getAt(pService->pData, ulIndex, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Trigger[%d] failed to get information.\n", ulIndex);	
	}

	return	xRet;
}

FTM_RET	FTOM_DB_TRIGGER_setInfo
(
	FTM_CHAR_PTR		pTriggerID,
	FTM_TRIGGER_FIELD	xFields,
	FTM_TRIGGER_PTR		pInfo
)
{
	ASSERT(pTriggerID != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_DMC_TRIGGER_set(pService->pData, pTriggerID, xFields, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Trigger[%s] DB update failed.\n", pTriggerID);	
	}

	return	xRet;
}

FTM_RET	FTOM_DB_ACTION_add
(
	FTM_ACTION_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_DMC_ACTION_add(pService->pData, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Action[%s] failed to add to DB.\n", pInfo->pID);	
	}

	return	xRet;
}

FTM_RET	FTOM_DB_ACTION_remove
(
	FTM_CHAR_PTR	pActionID
)
{
	ASSERT(pActionID != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_DMC_ACTION_remove(pService->pData, pActionID);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Action[%s] failed to remove from DB.\n", pActionID);
	}

	return	xRet;	
}

FTM_RET	FTOM_DB_ACTION_getInfo
(
	FTM_CHAR_PTR	pActionID,
	FTM_ACTION_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_DMC_ACTION_get(pService->pData, pActionID, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Action[%s] info failed to get from DB.\n", pActionID);	
	}

	return	xRet;
}

FTM_RET	FTOM_DB_ACTION_getInfoAt
(
	FTM_ULONG		ulIndex,
	FTM_ACTION_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}
	
	xRet = FTOM_DMC_ACTION_getAt(pService->pData, ulIndex, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Action[%lu] info failed to get from DB.", ulIndex);
	}

	return	xRet;	
}

FTM_RET	FTOM_DB_ACTION_setInfo
(
	FTM_CHAR_PTR		pActionID,
	FTM_ACTION_FIELD	xFields,
	FTM_ACTION_PTR		pInfo
)
{
	ASSERT(pActionID != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_DMC_ACTION_set(pService->pData, pActionID, xFields, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Action[%s] DB update failed.\n", pActionID);	
	}

	return	xRet;
}

FTM_RET	FTOM_DB_RULE_add
(
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTOM_DMC_RULE_add(pService->pData, pInfo);
}

FTM_RET	FTOM_DB_RULE_remove
(
	FTM_CHAR_PTR	pRuleID
)
{
	ASSERT(pRuleID != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTOM_DMC_RULE_remove(pService->pData, pRuleID);
}

FTM_RET	FTOM_DB_RULE_getInfo
(
	FTM_CHAR_PTR	pRuleID,
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pRuleID != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTOM_DMC_RULE_get(pService->pData, pRuleID, pInfo);
}

FTM_RET	FTOM_DB_RULE_getInfoAt
(
	FTM_ULONG		ulIndex,
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTOM_DMC_RULE_getAt(pService->pData, ulIndex, pInfo);
}

FTM_RET	FTOM_DB_RULE_setInfo
(
	FTM_CHAR_PTR	pRuleID,
	FTM_RULE_FIELD	xFields,
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pRuleID != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTOM_SERVICE_PTR pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DMC, &pService);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_DMC_RULE_set(pService->pData, pRuleID, xFields, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Rule[%s] DB update failed.\n", pRuleID);	
	}

	return	xRet;
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

	xRet = FTOM_MSG_createAlert(pEPID, pData, &pMsg);
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

	xRet = FTOM_MSG_createDiscoveryInfo(pName, pDID, pIP, pTypes, ulCount, &pMsg);
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

FTM_RET	FTOM_discoveryStart
(
	FTM_CHAR_PTR	pNetwork,
	FTM_USHORT		usPort,
	FTM_ULONG		ulRetryCount
)
{
	ASSERT(pNetwork != NULL);

	FTM_RET	xRet;
	FTOM_MSG_DISCOVERY_PTR	pMsg;

	xRet = FTOM_MSG_createDiscovery(pNetwork, usPort, ulRetryCount, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Discovery message creation failed.\n");
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

	memset(pEPInfo, 0, sizeof(FTM_EP));

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

	xRet = FTOM_NODE_getEPUpdateInterval(pNode, xType, ulIndex, &pEPInfo->ulUpdateInterval);
	if (xRet != FTM_RET_OK)
	{
		if (xType != FTM_EP_TYPE_DI)
		{
			ERROR2(xRet,"SNMP get EP[%s] interval failed!\n", pEPInfo->pEPID);
			return	xRet;	
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
		return	FTOM_MSGQ_push(pMsgQ, pMsg);
	}

	return	pService->fSendMessage(pService->pData, pMsg);
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
		return	FTM_RET_FUNCTION_NOT_SUPPORTED;
	}

	xRet = FTOM_MSG_createServerSync(bAutoRegister, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet,"Message creation failed!\n");
		return	xRet;	
	}

	return	pService->fSendMessage(pService->pData, (FTOM_MSG_PTR)pMsg);

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

