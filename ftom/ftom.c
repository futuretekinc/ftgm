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
#include "ftom_trigger.h"
#include "ftom_action.h"
#include "ftom_rule.h"
#include "ftom_discovery.h"
#include "ftom_utils.h"

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
	FTOM_MSG_TIME_SYNC_PTR	pMsg
);

static 
FTM_RET	FTOM_onTimeSync
(
	FTOM_MSG_TIME_SYNC_PTR	pMsg
);

static 
FTM_RET	FTOM_onEPCtrl
(
	FTOM_MSG_EP_CTRL_PTR	pMsg
);

static 
FTM_RET FTOM_onSaveEPData
(
	FTOM_MSG_SAVE_EP_DATA_PTR	pMsg
);

static 
FTM_RET	FTOM_onSendEPData
(
	FTOM_MSG_SEND_EP_DATA_PTR pMsg
);

static 
FTM_RET	FTOM_onRule
(
	FTOM_MSG_RULE_PTR pMsg
);

static 
FTM_RET	FTOM_onAlert
(
	FTOM_MSG_ALERT_PTR pMsg
);

static 
FTM_RET	FTOM_onDiscovery
(
	FTOM_MSG_DISCOVERY_PTR pMsg
);

static 
FTM_RET	FTOM_onDiscoveryInfo
(
	FTOM_MSG_DISCOVERY_INFO_PTR pMsg
);

static 
FTM_RET	FTOM_onDiscoveryDone
(
	FTOM_MSG_DISCOVERY_DONE_PTR pMsg
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
		.fInit		=	(FTOM_SERVICE_INIT)FTOM_SERVER_init,
		.fFinal		=	(FTOM_SERVICE_FINAL)FTOM_SERVER_final,
		.fStart 	=	(FTOM_SERVICE_START)FTOM_SERVER_start,
		.fStop		=	(FTOM_SERVICE_STOP)FTOM_SERVER_stop,
		.fSetCallback=	(FTOM_SERVICE_SET_CALLBACK)FTOM_SERVER_setServiceCallback,
		.fCallback	=	FTOM_callback,
		.fLoadFromFile=	(FTOM_SERVICE_LOAD_FROM_FILE)FTOM_SERVER_loadFromFile,
		.fShowConfig=	(FTOM_SERVICE_SHOW_CONFIG)FTOM_SERVER_showConfig,
		.fNotify	=	(FTOM_SERVICE_NOTIFY)FTOM_SERVER_notify,
		.pData		= 	NULL
	},
	{
		.xType		=	FTOM_SERVICE_SNMP_CLIENT,
		.xID		=	FTOM_SERVICE_SNMP_CLIENT,
		.pName		=	"SNMP Client",
		.fInit		=	(FTOM_SERVICE_INIT)FTOM_SNMPC_init,
		.fFinal		=	(FTOM_SERVICE_FINAL)FTOM_SNMPC_final,
		.fStart 	=	(FTOM_SERVICE_START)FTOM_SNMPC_start,
		.fStop		=	(FTOM_SERVICE_STOP)FTOM_SNMPC_stop,
		.fSetCallback=	(FTOM_SERVICE_SET_CALLBACK)FTOM_SNMPC_setServiceCallback,
		.fCallback	=	FTOM_callback,
		.fLoadFromFile=	(FTOM_SERVICE_LOAD_FROM_FILE)FTOM_SNMPC_loadFromFile,
		.fShowConfig=	(FTOM_SERVICE_SHOW_CONFIG)FTOM_SNMPC_showConfig,
		.pData		= 	NULL
	},
	{
		.xType		=	FTOM_SERVICE_SNMPTRAPD,
		.xID		=	FTOM_SERVICE_SNMPTRAPD,
		.pName		=	"SNMP TrapD",
		.fInit		=	(FTOM_SERVICE_INIT)FTOM_SNMPTRAPD_init,
		.fFinal		=	(FTOM_SERVICE_FINAL)FTOM_SNMPTRAPD_final,
		.fStart 	=	(FTOM_SERVICE_START)FTOM_SNMPTRAPD_start,
		.fStop		=	(FTOM_SERVICE_STOP)FTOM_SNMPTRAPD_stop,
		.fSetCallback=	(FTOM_SERVICE_SET_CALLBACK)FTOM_SNMPTRAPD_setServiceCallback,
		.fCallback	=	FTOM_callback,
		.fLoadFromFile=	(FTOM_SERVICE_LOAD_FROM_FILE)FTOM_SNMPTRAPD_loadFromFile,
		.fShowConfig=	(FTOM_SERVICE_SHOW_CONFIG)FTOM_SNMPTRAPD_showConfig,
		.pData		= 	NULL
	},
	{
		.xType		=	FTOM_SERVICE_DBM,
		.xID		=	FTOM_SERVICE_DBM,
		.pName		=	"DB Client",
		.fInit		=	(FTOM_SERVICE_INIT)FTOM_DMC_init,
		.fFinal		=	(FTOM_SERVICE_FINAL)FTOM_DMC_final,
		.fStart 	=	(FTOM_SERVICE_START)FTOM_DMC_start,
		.fStop		=	(FTOM_SERVICE_STOP)FTOM_DMC_stop,
		.fSetCallback=	(FTOM_SERVICE_SET_CALLBACK)FTOM_DMC_setServiceCallback,
		.fCallback	=	FTOM_callback,
		.fLoadFromFile=	(FTOM_SERVICE_LOAD_FROM_FILE)FTOM_DMC_loadFromFile,
		.fShowConfig=	(FTOM_SERVICE_SHOW_CONFIG)FTOM_DMC_showConfig,
		.fNotify	=	(FTOM_SERVICE_NOTIFY)FTOM_DMC_notify,
		.pData		= 	NULL
	},
	{
		.xType		=	FTOM_SERVICE_MQTT_CLIENT,
		.xID		=	FTOM_SERVICE_MQTT_CLIENT,
		.pName		=	"MQTT Client",
		.fInit		=	(FTOM_SERVICE_INIT)FTOM_MQTT_CLIENT_init,
		.fFinal		=	(FTOM_SERVICE_FINAL)FTOM_MQTT_CLIENT_final,
		.fStart 	=	(FTOM_SERVICE_START)FTOM_MQTT_CLIENT_start,
		.fStop		=	(FTOM_SERVICE_STOP)FTOM_MQTT_CLIENT_stop,
		.fSetCallback=	(FTOM_SERVICE_SET_CALLBACK)FTOM_MQTT_CLIENT_setCallback,
		.fCallback	=	FTOM_callback,
		.fLoadFromFile=	(FTOM_SERVICE_LOAD_FROM_FILE)FTOM_MQTT_CLIENT_loadFromFile,
		.fShowConfig=	(FTOM_SERVICE_SHOW_CONFIG)FTOM_MQTT_CLIENT_showConfig,
		.fNotify	=	(FTOM_SERVICE_NOTIFY)FTOM_MQTT_CLIENT_notify,
		.pData		= 	NULL
	},
	{
		.xType		=	FTOM_SERVICE_SHELL,
		.xID		=	FTOM_SERVICE_SHELL,
		.pName		=	"Shell",
		.fInit		=	(FTOM_SERVICE_INIT)FTOM_SHELL_init,
		.fFinal		=	(FTOM_SERVICE_FINAL)FTOM_SHELL_final,
		.fStart 	=	(FTOM_SERVICE_START)FTOM_SHELL_start,
		.fStop		=	(FTOM_SERVICE_STOP)FTOM_SHELL_stop,
		.fSetCallback=	NULL,
		.fCallback	=	NULL,
		.fLoadFromFile=	(FTOM_SERVICE_LOAD_FROM_FILE)FTOM_SHELL_loadFromFile,
		.fShowConfig=	(FTOM_SERVICE_SHOW_CONFIG)FTOM_SHELL_showConfig,
		.fNotify	=	(FTOM_SERVICE_NOTIFY)FTOM_SHELL_notify,
		.pData		= 	NULL
	},
	{
		.xType		=	FTOM_SERVICE_DISCOVERY,
		.xID		=	FTOM_SERVICE_DISCOVERY,
		.pName		=	"Discovery",
		.fInit		=	(FTOM_SERVICE_INIT)FTOM_DISCOVERY_init,
		.fFinal		=	(FTOM_SERVICE_FINAL)FTOM_DISCOVERY_final,
		.fStart 	=	(FTOM_SERVICE_START)FTOM_DISCOVERY_start,
		.fStop		=	(FTOM_SERVICE_STOP)FTOM_DISCOVERY_stop,
		.fSetCallback=	NULL,
		.fCallback	=	NULL,
		.fLoadFromFile=	NULL,
		.fShowConfig=	NULL,
		.fNotify	=	NULL,
		.pData		= 	NULL
	},
};

extern char *program_invocation_short_name;
FTOM_PTR	pOM;
FTOM_CONFIG			xConfig;

FTOM_STATE			xState;
pthread_t			xThread;

FTM_BOOL			bStop;

FTOM_MSG_QUEUE_PTR	pMsgQ;
FTM_SHELL			xShell;
FTOM_SERVER_PTR		pServer;
FTOM_SNMPC_PTR		pSNMPC;
FTOM_SNMPTRAPD_PTR	pSNMPTRAPD;
FTOM_DMC_PTR		pDMC;
FTOM_MQTT_CLIENT_PTR	pMQTTC;
FTOM_SHELL_PTR		pShell;
FTOM_DISCOVERY_PTR	pDiscovery;

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
	FTM_INT	i;

	FTOM_getDefaultDeviceID(xConfig.pDID);
	TRACE("DID : %s\n", xConfig.pDID);

	bStop = FTM_TRUE;

	onMessage[FTOM_MSG_TYPE_QUIT] 			= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onQuit;
	onMessage[FTOM_MSG_TYPE_SAVE_EP_DATA] 	= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onSaveEPData;
	onMessage[FTOM_MSG_TYPE_SEND_EP_DATA] 	= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onSendEPData;
	onMessage[FTOM_MSG_TYPE_TIME_SYNC] 		= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onTimeSync;
	onMessage[FTOM_MSG_TYPE_EP_CTRL] 		= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onEPCtrl;
	onMessage[FTOM_MSG_TYPE_RULE] 			= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onRule;
	onMessage[FTOM_MSG_TYPE_ALERT] 			= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onAlert;
	onMessage[FTOM_MSG_TYPE_DISCOVERY] 		= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onDiscovery;
	onMessage[FTOM_MSG_TYPE_DISCOVERY_INFO]	= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onDiscoveryInfo;
	onMessage[FTOM_MSG_TYPE_DISCOVERY_DONE]	= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onDiscoveryDone;

	xRet = FTOM_MSGQ_create(&pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message queue creation failed[%08x].\n", xRet);
		goto error;
	}

	xRet = FTOM_NODE_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node management creation failed[%08x].\n", xRet);
		goto error;
	}

	xRet = FTOM_EP_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP management creation failed[%08x].\n", xRet);
		goto error;
	}

	xRet = FTOM_TRIGGER_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Trigger management creation failed[%08x].\n", xRet);
		goto error;
	}

	xRet = FTOM_ACTION_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Action management creation failed[%08x].\n", xRet);
		goto error;
	}

	xRet = FTOM_RULE_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Rule management creation failed[%08x].\n", xRet);
		goto error;
	}

	FTOM_SERVER_create(&pServer);
	FTOM_SNMPC_create(&pSNMPC);
	FTOM_SNMPTRAPD_create(&pSNMPTRAPD);
	FTOM_DMC_create(&pDMC);
	FTOM_MQTT_CLIENT_create(&pMQTTC);
	FTOM_SHELL_create(&pShell);
	FTOM_DISCOVERY_create(&pDiscovery);

	for(i = 0 ; i < sizeof(pServices) / sizeof(FTOM_SERVICE) ; i++)
	{
		switch(pServices[i].xType)
		{
		case	FTOM_SERVICE_SERVER: pServices[i].pData = (FTM_VOID_PTR)pServer; break;
		case	FTOM_SERVICE_SNMP_CLIENT: pServices[i].pData = (FTM_VOID_PTR)pSNMPC; break;
		case	FTOM_SERVICE_SNMPTRAPD: pServices[i].pData = (FTM_VOID_PTR)pSNMPTRAPD; break;
		case	FTOM_SERVICE_DBM: pServices[i].pData =(FTM_VOID_PTR)pDMC; break;
		case	FTOM_SERVICE_MQTT_CLIENT: pServices[i].pData =(FTM_VOID_PTR)pMQTTC; break;
		case	FTOM_SERVICE_SHELL: pServices[i].pData =(FTM_VOID_PTR)pShell; break;
		case	FTOM_SERVICE_DISCOVERY: pServices[i].pData = (FTM_VOID_PTR)pDiscovery; break;
		default: break;
		}
	}

	FTOM_SERVICE_init(pServices, sizeof(pServices) / sizeof(FTOM_SERVICE));

	TRACE("initialization done.\n");

	return	FTM_RET_OK;

error:
	if (pOM != NULL)
	{
		FTOM_RULE_final();
		FTOM_ACTION_final();
		FTOM_TRIGGER_final();
	
		FTOM_EP_final();
	
		if (pMsgQ!= NULL)
		{
			FTOM_MSGQ_destroy(&pMsgQ);
		}
	
		FTOM_NODE_final();
	
		pOM = NULL;
	}

	return	xRet;
}

FTM_RET	FTOM_final
(
	FTM_VOID
)
{
	FTM_RET	xRet;

	if (pOM == NULL)
	{
		return	FTM_RET_OK;	
	}

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

	FTM_MEM_free(pOM);

	pOM = NULL;

	TRACE("Finalize done.\n");

	return	FTM_RET_OK;
}

FTM_RET	FTOM_loadFromFile
(
	FTM_CHAR_PTR 	pFileName
)
{
	ASSERT(pFileName != NULL);

	FTOM_SERVICE_loadFromFile(FTOM_SERVICE_ALL, pFileName);

	TRACE("loaded configuration.\n");
	return	FTM_RET_OK;
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
	ASSERT (pData != NULL);
	
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
		FTOM_TASK_stop();	
		FTOM_TASK_stopService();	
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
	FTM_ULONG		ulCount, i;

	xRet = FTDMC_NODE_count(&pDMC->xSession, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	TRACE("Load Node Object : %lu\n", ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_NODE	xNodeInfo;
		FTOM_NODE_PTR	pNode;

		xRet = FTDMC_NODE_getAt(&pDMC->xSession, i, &xNodeInfo);	
		if (xRet != FTM_RET_OK)
		{
			ERROR("Can't get node info from DMC!\n");
			exit(1);
			continue;	
		}

		xRet = FTOM_NODE_create(&xNodeInfo, &pNode);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Node creation failed[%08lx].\n", xRet);
			continue;	
		}

		TRACE("Node[%s] creating success.\n", pNode->xInfo.pDID);
	}

	xRet = FTDMC_EP_CLASS_count(&pDMC->xSession, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP Class count get failed[%08x].\n", xRet);
		return	xRet;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_EP_CLASS	xEPClassInfo;
		FTOM_EP_CLASS_PTR	pEPClass;

		xRet = FTDMC_EP_CLASS_getAt(&pDMC->xSession, i, &xEPClassInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTDMC_EP_CLASS_getAt(%08lx, %d, &xEPInfo) = %08lx\n",
					pDMC->xSession.hSock, i, xRet);
			continue;
		}

		xRet = FTOM_EP_CLASS_create(&xEPClassInfo, &pEPClass);
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTOM_EP_TYPE_append(&xEPClassInfo) = %08lx\n", xRet);
			continue;	
		}
	}

	xRet = FTDMC_EP_count(&pDMC->xSession, 0, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP count get failed[%08x].\n", xRet);
		return	xRet;	
	}
	TRACE("Load EP Object : %lu\n", ulCount);

	for(i = 0 ; i < ulCount ; i++)
	{
		FTOM_NODE_PTR	pNode;
		FTM_EP			xEPInfo;
		FTOM_EP_PTR		pEP;

		xRet = FTDMC_EP_getAt(&pDMC->xSession, i, &xEPInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("EP object get at %d failed[%08x]\n", i, xRet);
			continue;
		}

		xRet = FTOM_EP_create(&xEPInfo, &pEP, FTM_FALSE);
		if (xRet != FTM_RET_OK)
		{
			ERROR("EP[%s] object creation failed[%08x]\n", xEPInfo.pEPID, xRet);
			continue;	
		}

		if (FTOM_NODE_get(xEPInfo.pDID, &pNode) == FTM_RET_OK)
		{
			FTOM_NODE_linkEP(pNode, pEP);
		}
		
		TRACE("EP[%s] creating success.\n", pEP->xInfo.pEPID);
	}

	xRet = FTDMC_TRIGGER_count(&pDMC->xSession, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Trigger count get failed[%08x].\n", xRet);
		return	xRet;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_TRIGGER		xTriggerInfo;
		FTOM_TRIGGER_PTR	pTrigger = NULL;

		xRet = FTDMC_TRIGGER_getAt(&pDMC->xSession, i, &xTriggerInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Event[%d] data load failed.\n", i);	
			continue;
		}

		xRet = FTOM_TRIGGER_create(&xTriggerInfo, &pTrigger);
		if (xRet != FTM_RET_OK)
		{
			ERROR("The new event can not registration!\n") ;
			continue;
		}
	}

	xRet = FTDMC_ACTION_count(&pDMC->xSession, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Action count get failed[%08x].\n", xRet);
		return	xRet;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_ACTION	xActionInfo;
		FTOM_ACTION_PTR pAction = NULL;

		xRet = FTDMC_ACTION_getAt(&pDMC->xSession, i, &xActionInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Action[%d] data load failed.\n", i);	
			continue;
		}

		xRet = FTOM_ACTION_create(&xActionInfo, &pAction);
		if (xRet != FTM_RET_OK)
		{
			ERROR("The new action event can not registration!\n") ;
			continue;
		}
	}

	xRet = FTDMC_RULE_count(&pDMC->xSession, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Rule count get failed[%08x].\n", xRet);
		return	xRet;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_RULE	xRuleInfo;
		FTOM_RULE_PTR	pRule = NULL;

		xRet = FTDMC_RULE_getAt(&pDMC->xSession, i, &xRuleInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Rule[%d] data load failed.\n", i);	
			continue;
		}

		xRet = FTOM_RULE_create(&xRuleInfo, &pRule, FTM_FALSE);
		if (xRet != FTM_RET_OK)
		{
			ERROR("The new action event can not registration!\n") ;
			continue;
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
	
	TRACE("Start FTOM!\n");
	FTOM_RULE_start();
	FTOM_ACTION_start();
	FTOM_TRIGGER_start();

	FTOM_NODE_count(&ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTOM_NODE_PTR	pNode;

		if (FTOM_NODE_getAt(i, &pNode) == FTM_RET_OK)
		{
			FTOM_NODE_start(pNode);
		}
		else
		{
			ERROR("Node not found at %d\n", i);
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

	FTM_TIMER_init(&xLoopTimer, FTOM_LOOP_INTERVAL);

	while(!bStop)
	{
		FTM_ULONG	ulRemainTime;
		
		FTM_TIMER_remain(&xLoopTimer, &ulRemainTime);
		xRet = FTOM_MSGQ_timedPop(pMsgQ, ulRemainTime, &pMsg);
		if (xRet == FTM_RET_OK)
		{
			if ((pMsg->xType < FTOM_MSG_TYPE_MAX) && (onMessage[pMsg->xType] != NULL))
			{
				xRet = onMessage[pMsg->xType](pMsg, pOnMessageData[pMsg->xType]);
			}
			else
			{
				ERROR("Message[%08x] not supported.\n", pMsg->xType);
			}

			FTM_MEM_free(pMsg);
		}

		if (FTM_TIMER_isExpired(&xLoopTimer))
		{
			FTM_TIMER_add(&xLoopTimer, FTOM_LOOP_INTERVAL);
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
	
	FTOM_TRIGGER_stop();
	FTOM_ACTION_stop();
	FTOM_RULE_stop();
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
	FTOM_MSG_TIME_SYNC_PTR	pMsg
)
{
	bStop = FTM_TRUE;
	xState = FTOM_STATE_STOPED;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_onTimeSync
(
	FTOM_MSG_TIME_SYNC_PTR	pMsg
)
{
	ASSERT(pMsg != NULL);

	TRACE("Time Sync - %d\n", pMsg->ulTime);

	return	FTM_RET_OK;
}

FTM_RET FTOM_onSaveEPData
(
	FTOM_MSG_SAVE_EP_DATA_PTR	pMsg
)
{
	ASSERT(pMsg != NULL);

	FTM_CHAR	pBuff[64];

	FTM_EP_DATA_snprint(pBuff, sizeof(pBuff), &pMsg->xData);

	FTOM_TRIGGER_updateEP(pMsg->pEPID, &pMsg->xData);
	FTOM_DMC_appendEPData(pDMC, pMsg->pEPID, &pMsg->xData);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_onEPCtrl
(
	FTOM_MSG_EP_CTRL_PTR	pMsg
)
{
	FTM_RET			xRet;
	FTOM_EP_PTR		pEP;
	FTM_EP_DATA		xData;

	TRACE("EP[%s] Control\n", pMsg->pEPID);

	xRet = FTOM_EP_get(pMsg->pEPID, &pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP[%s] not found.\n", pMsg->pEPID);
		return	xRet;
	}

	FTM_EP_DATA_initINT(&xData, pMsg->xCtrl);

	xRet = FTOM_EP_pushData(pEP, &xData);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP[%s] set failed.\n", pMsg->pEPID);
	}

	return	xRet;
}

FTM_RET	FTOM_onSendEPData
(
	FTOM_MSG_SEND_EP_DATA_PTR pMsg
)
{
	ASSERT(pMsg != NULL);

	return	FTOM_MQTT_CLIENT_publishEPData(pMQTTC, pMsg->pEPID, pMsg->pData, pMsg->ulCount);
}

FTM_RET	FTOM_onRule
(
	FTOM_MSG_RULE_PTR pMsg
)
{
	ASSERT(pMsg != NULL);

	TRACE("RULE[%s] is %s\n", pMsg->pRuleID, (pMsg->xRuleState == FTM_RULE_STATE_ACTIVATE)?"ACTIVATE":"DEACTIVATE");

	return	FTM_RET_OK;
}

FTM_RET	FTOM_onAlert
(
	FTOM_MSG_ALERT_PTR	pMsg
)
{
	ASSERT(pMsg != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_onDiscovery
(
	FTOM_MSG_DISCOVERY_PTR	pMsg
)
{
	ASSERT(pMsg != NULL);

	FTOM_DISCOVERY_call(pDiscovery, pMsg->pNetwork, pMsg->usPort);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_onDiscoveryInfo
(
	FTOM_MSG_DISCOVERY_INFO_PTR	pMsg
)
{
	ASSERT(pMsg != NULL);
	
	FTM_INT	i;

	TRACE("MSG : DISCOVERY\n");
	TRACE("DID - %s\n", pMsg->pDID);
	for(i = 0 ; i < pMsg->ulCount; i++)
	{
		TRACE("TYPE[%d] - %s[%08x]\n", i, FTM_EP_typeString(pMsg->pTypes[i]), pMsg->pTypes[i]);	
	}
	return	FTM_RET_OK;
}

FTM_RET	FTOM_onDiscoveryDone
(
	FTOM_MSG_DISCOVERY_DONE_PTR	pMsg
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
		ERROR("Message type[%08x] is invalid.\n", xMsg);
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
 * EP management interface
 ******************************************************************/

FTM_RET	FTOM_SYS_EP_addToDB
(
	FTM_EP_PTR 	pInfo
)
{
	ASSERT(pInfo != NULL);

	return	FTDMC_EP_append(&pDMC->xSession, pInfo);
}

FTM_RET	FTOM_SYS_EP_removeFromDB
(
	FTM_CHAR_PTR	pEPID
)
{
	return	FTDMC_EP_remove(&pDMC->xSession, pEPID);
}

FTM_RET	FTOM_setEPInfo
(
	FTM_CHAR_PTR	pEPID,
	FTM_EP_FIELD	xFields,
	FTM_EP_PTR		pInfo
)
{
	ASSERT(pEPID != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTOM_EP_PTR	pEP;

	xRet = FTOM_EP_get(pEPID, &pEP);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}
	
	xRet = FTDMC_EP_set(&pDMC->xSession, pEPID, xFields, pInfo);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (xFields & FTM_EP_FIELD_FLAGS)
	{
		pEP->xInfo.xFlags = pInfo->xFlags;
	}

	if (xFields & FTM_EP_FIELD_NAME)
	{
		strcpy(pEP->xInfo.pName, pInfo->pName);
	}

	if (xFields & FTM_EP_FIELD_UNIT)
	{
		strcpy(pEP->xInfo.pUnit, pInfo->pUnit);
	}

	if (xFields & FTM_EP_FIELD_ENABLE)
	{
		pEP->xInfo.bEnable = pInfo->bEnable;
	}

	if (xFields & FTM_EP_FIELD_TIMEOUT)
	{
		pEP->xInfo.ulTimeout = pInfo->ulTimeout;
	}

	if (xFields & FTM_EP_FIELD_INTERVAL)
	{
		pEP->xInfo.ulInterval = pInfo->ulInterval;
	}

	if (xFields & FTM_EP_FIELD_DID)
	{
		strcpy(pEP->xInfo.pDID, pInfo->pDID);
	}

	if (xFields & FTM_EP_FIELD_LIMIT)
	{
		if (pEP->xInfo.xLimit.xType != pInfo->xLimit.xType)
		{
			memset(&pEP->xInfo.xLimit, 0, sizeof(FTM_EP_LIMIT));
		}
		pEP->xInfo.xLimit.xType = pInfo->xLimit.xType;

		switch(pEP->xInfo.xLimit.xType)
		{
		case	FTM_EP_LIMIT_TYPE_COUNT:	
			{
				pEP->xInfo.xLimit.xParams.ulCount = pInfo->xLimit.xParams.ulCount;
			}
			break;
	
		case	FTM_EP_LIMIT_TYPE_TIME:	
			{
				pEP->xInfo.xLimit.xParams.xTime.ulStart = pInfo->xLimit.xParams.xTime.ulStart;
				pEP->xInfo.xLimit.xParams.xTime.ulEnd = pInfo->xLimit.xParams.xTime.ulEnd;
			}
			break;
	
		case	FTM_EP_LIMIT_TYPE_HOURS:	
			{
				pEP->xInfo.xLimit.xParams.ulHours= pInfo->xLimit.xParams.ulCount;
			}
			break;
	
		case	FTM_EP_LIMIT_TYPE_DAYS:	
			{
				pEP->xInfo.xLimit.xParams.ulDays = pInfo->xLimit.xParams.ulCount;
			}
			break;
	
		case	FTM_EP_LIMIT_TYPE_MONTHS:	
			{
				pEP->xInfo.xLimit.xParams.ulMonths = pInfo->xLimit.xParams.ulCount;
			}
			break;
		}
	}

	return	FTM_RET_OK;
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

	return	FTDMC_EP_DATA_get(&pDMC->xSession, pEPID, ulStart, pDataList, ulMaxCount, pulCount);
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

	return	FTDMC_EP_DATA_info(&pDMC->xSession, pEPID, pulBeginTime, pulEndTime, pulCount);
}

FTM_RET	FTOM_DB_EP_getDataCount
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR 	pulCount
)
{
	return	FTDMC_EP_DATA_count(&pDMC->xSession, pEPID, pulCount);
}


FTM_RET	FTOM_NOTIFY_rule
(
	FTM_CHAR_PTR	pRuleID,
	FTM_RULE_STATE	xRuleState
)
{
	FTM_RET			xRet;
	FTOM_MSG_RULE_PTR	pMsg;

	xRet = FTOM_MSG_createRule(pRuleID, xRuleState, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message creation failed.\n");
		return	xRet;
	}

	xRet = FTOM_MSGQ_push(pMsgQ, (FTOM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message send failed.\n");
		FTOM_MSG_destroy((FTOM_MSG_PTR _PTR_)&pMsg);		
		return	xRet;
	}

	return	xRet;
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

	case	FTOM_SERVICE_DBM:
		{
		}
		break;

	default:
		{
			ERROR("Invalid service ID[%08x] received.\n");
		}
	}

	return	FTM_RET_OK;
}

/************************************************************
 *	Node management 
 ************************************************************/


FTM_RET	FTOM_setEPData
(
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR pData
)
{
	ASSERT(pData != NULL);

	FTM_RET						xRet;
	FTOM_MSG_SET_EP_DATA_PTR	pMsg;

	xRet = FTOM_MSG_createSetEPData(pEPID, pData, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(pMsgQ, (FTOM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
		FTOM_MSG_destroy((FTOM_MSG_PTR _PTR_)&pMsg);
		return	xRet;
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTOM_SYS_EP_storeData
(
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR pData
)
{
	ASSERT(pData != NULL);

	FTM_RET						xRet;
	FTOM_MSG_SAVE_EP_DATA_PTR	pMsg;

	xRet = FTOM_MSG_createSaveEPData(pEPID, pData, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		WARN("Save EP data message creation failed[%08x].\n", xRet);
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(pMsgQ, (FTOM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
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
		WARN("Send EP data message creation failed[%08x].\n", xRet);
		return	xRet;	
	}


	xRet = FTOM_MSGQ_push(pMsgQ, (FTOM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
		FTOM_MSG_destroy((FTOM_MSG_PTR _PTR_)&pMsg);
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_delEPData
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulDeletedCount
)
{
	ASSERT(pulDeletedCount != NULL);

	return	FTOM_DMC_EP_DATA_del(pDMC, pEPID, ulIndex, ulCount, pulDeletedCount);
}

FTM_RET	FTOM_delEPDataWithTime
(
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulBegin,
	FTM_ULONG		ulEnd,
	FTM_ULONG_PTR	pulDeletedCount
)
{
	ASSERT(pulDeletedCount != NULL);

	return	FTOM_DMC_EP_DATA_delWithTime(pDMC, pEPID, ulBegin, ulEnd, pulDeletedCount);
}

FTM_RET	FTOM_addTrigger
(
	FTM_TRIGGER_PTR		pInfo,
	FTM_CHAR_PTR		pTriggerID
)
{
	
	FTM_RET		xRet;
	FTOM_TRIGGER_PTR	pTrigger;

	xRet = FTOM_TRIGGER_create(pInfo, &pTrigger);
	if (xRet == FTM_RET_OK)
	{
		strncpy(pTriggerID, pTrigger->xInfo.pID, FTM_ID_LEN);
		xRet = FTDMC_TRIGGER_add(&pDMC->xSession, &pTrigger->xInfo);
		if (xRet != FTM_RET_OK)
		{
			FTOM_TRIGGER_destroy(&pTrigger);
		}
	}
	return	xRet;
}

FTM_RET	FTOM_delTrigger
(
	FTM_CHAR_PTR	pTriggerID
)
{

	FTM_RET	xRet;
	FTOM_TRIGGER_PTR	pTrigger = NULL;

	xRet =	FTOM_TRIGGER_get(pTriggerID, &pTrigger);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet =  FTOM_TRIGGER_destroy(&pTrigger);

	return	xRet;
}

FTM_RET	FTOM_getTriggerInfo
(
	FTM_CHAR_PTR	pTriggerID,
	FTM_TRIGGER_PTR	pInfo
)
{
	ASSERT(pTriggerID != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTOM_TRIGGER_PTR	pTrigger;

	xRet = FTOM_TRIGGER_get(pTriggerID, &pTrigger);
	if (xRet == FTM_RET_OK)
	{
		memcpy(pInfo, &pTrigger->xInfo, sizeof(FTM_TRIGGER));
	}

	return	xRet;
}

FTM_RET	FTOM_getTriggerInfoAt
(
	FTM_ULONG		ulIndex,
	FTM_TRIGGER_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTOM_TRIGGER_PTR	pTrigger;

	xRet = FTOM_TRIGGER_getAt(ulIndex, &pTrigger);
	if (xRet == FTM_RET_OK)
	{
		memcpy(pInfo, &pTrigger->xInfo, sizeof(FTM_TRIGGER));
	}

	return	xRet;
}

FTM_RET	FTOM_setTriggerInfo
(
	FTM_CHAR_PTR		pTriggerID,
	FTM_TRIGGER_FIELD	xFields,
	FTM_TRIGGER_PTR		pInfo
)
{
	ASSERT(pTriggerID != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTOM_TRIGGER_PTR	pTrigger;

	xRet = FTOM_TRIGGER_get(pTriggerID, &pTrigger);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Trigger[%s] set failed.\n", pTriggerID);
		return	xRet;
	}

	xRet = FTDMC_TRIGGER_set(&pDMC->xSession, pTriggerID, xFields, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Trigger[%s] DB update failed.\n", pTriggerID);	
		return	xRet;
	}

	if (xFields & FTM_TRIGGER_FIELD_NAME)
	{
		strcpy(pTrigger->xInfo.pName, pInfo->pName);
	}

	if (xFields & FTM_TRIGGER_FIELD_EPID)
	{
		strcpy(pTrigger->xInfo.pEPID, pInfo->pEPID);
	}

	if (xFields & FTM_TRIGGER_FIELD_DETECT_TIME)
	{
		pTrigger->xInfo.xParams.xCommon.ulDetectionTime = pInfo->xParams.xCommon.ulDetectionTime;
	}

	if (xFields & FTM_TRIGGER_FIELD_HOLD_TIME)
	{
		pTrigger->xInfo.xParams.xCommon.ulHoldingTime = pInfo->xParams.xCommon.ulHoldingTime;
	}

	if (xFields & FTM_TRIGGER_FIELD_VALUE)
	{
		memcpy(&pTrigger->xInfo.xParams.xAbove.xValue, &pInfo->xParams.xAbove.xValue, sizeof(FTM_VALUE));
	}

	if (xFields & FTM_TRIGGER_FIELD_LOWER)
	{
		memcpy(&pTrigger->xInfo.xParams.xInclude.xLower, &pInfo->xParams.xInclude.xLower, sizeof(FTM_VALUE));
	}

	if (xFields & FTM_TRIGGER_FIELD_UPPER)
	{
		memcpy(&pTrigger->xInfo.xParams.xInclude.xUpper, &pInfo->xParams.xInclude.xUpper, sizeof(FTM_VALUE));
	}

	return	xRet;
}

FTM_RET	FTOM_addAction
(
	FTM_ACTION_PTR	pInfo,
	FTM_CHAR_PTR	pActionID
)
{
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTOM_ACTION_PTR	pAction = NULL;

	xRet = FTOM_ACTION_create(pInfo, &pAction);
	if (xRet == FTM_RET_OK)
	{
		strncpy(pActionID, pAction->xInfo.pID, FTM_ID_LEN);
		xRet = FTDMC_ACTION_add(&pDMC->xSession, &pAction->xInfo);
		if (xRet != FTM_RET_OK)
		{
			FTOM_ACTION_destroy(&pAction);
		}
	}
	return	xRet;
}

FTM_RET	FTOM_delAction
(
	FTM_CHAR_PTR	pActionID
)
{
	FTM_RET	xRet;
	FTOM_ACTION_PTR	pAction = NULL;

	xRet = FTOM_ACTION_get(pActionID, &pAction);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTOM_ACTION_destroy(&pAction);
}

FTM_RET	FTOM_getActionInfo
(
	FTM_CHAR_PTR	pActionID,
	FTM_ACTION_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTOM_ACTION_PTR	pAction;

	xRet = FTOM_ACTION_get(pActionID, &pAction);
	if (xRet == FTM_RET_OK)
	{
		memcpy(pInfo, &pAction->xInfo, sizeof(FTM_ACTION));	
	}
	else
	{
		ERROR("Action[%s] get failed.[%08x]\n", pActionID, xRet);	
	}

	return	xRet;
}

FTM_RET	FTOM_getActionInfoAt
(
	FTM_ULONG		ulIndex,
	FTM_ACTION_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTOM_ACTION_PTR	pAction;

	xRet = FTOM_ACTION_getAt(ulIndex, &pAction);
	if (xRet == FTM_RET_OK)
	{
		memcpy(pInfo, &pAction->xInfo, sizeof(FTM_ACTION));	
	}

	return	xRet;
}

FTM_RET	FTOM_setActionInfo
(
	FTM_CHAR_PTR		pActionID,
	FTM_ACTION_FIELD	xFields,
	FTM_ACTION_PTR		pInfo
)
{
	ASSERT(pActionID != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTOM_ACTION_PTR	pAction;

	xRet = FTOM_ACTION_get(pActionID, &pAction);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Action[%s] get failed[%08x].\n", pActionID, xRet);
		return	xRet;
	}

	xRet = FTDMC_ACTION_set(&pDMC->xSession, pActionID, xFields, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Action[%s] DB update failed.\n", pActionID);	
		return	xRet;
	}

	switch(pAction->xInfo.xType)
	{
	case	FTM_ACTION_TYPE_SET:
		{
			if (xFields & FTM_ACTION_FIELD_NAME)
			{
				strcpy(pAction->xInfo.pName, pInfo->pName);
			}

			if (xFields & FTM_ACTION_FIELD_EPID)
			{
				strcpy(pAction->xInfo.xParams.xSet.pEPID, pInfo->xParams.xSet.pEPID);
			}
			
			if (xFields & FTM_ACTION_FIELD_VALUE)
			{
				memcpy(&pAction->xInfo.xParams.xSet.xValue, &pInfo->xParams.xSet.xValue, sizeof(FTM_VALUE));
			}
		}
		break;

	default:
		{
			return	FTM_RET_ERROR;	
		}
	}

	return	xRet;
}

FTM_RET	FTOM_DB_RULE_add
(
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);

	return	FTDMC_RULE_add(&pDMC->xSession, pInfo);
}

FTM_RET	FTOM_DB_RULE_remove
(
	FTM_CHAR_PTR	pRuleID
)
{
	return	FTDMC_RULE_del(&pDMC->xSession, pRuleID);
}

FTM_RET	FTOM_DB_RULE_getInfo
(
	FTM_CHAR_PTR	pRuleID,
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pRuleID != NULL);
	ASSERT(pInfo != NULL);

	return	FTDMC_RULE_get(&pDMC->xSession, pRuleID, pInfo);
}

FTM_RET	FTOM_DB_RULE_getInfoAt
(
	FTM_ULONG		ulIndex,
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);

	return	FTDMC_RULE_getAt(&pDMC->xSession, ulIndex, pInfo);
}

FTM_RET	FTOM_setRuleInfo
(
	FTM_CHAR_PTR	pRuleID,
	FTM_RULE_FIELD	xFields,
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pRuleID != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTOM_RULE_PTR	pRule;

	xRet = FTOM_RULE_get(pRuleID, &pRule);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Rule[%s] not found!\n", pRuleID);
		return	xRet;
	}

	xRet = FTDMC_RULE_set(&pDMC->xSession, pRuleID, xFields, pInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Rule[%s] DB update failed.\n", pRuleID);	
		return	xRet;
	}

	if (xFields & FTM_RULE_FIELD_NAME)
	{
		strcpy(pRule->xInfo.pName, pInfo->pName);
	}

	if (xFields & FTM_RULE_FIELD_STATE)
	{
		pRule->xInfo.xState = pInfo->xState;
	}
	
	if (xFields & FTM_RULE_FIELD_TRIGGERS)
	{
		pRule->xInfo.xParams.ulTriggers = pInfo->xParams.ulTriggers;
		memcpy(pRule->xInfo.xParams.pTriggers, pInfo->xParams.pTriggers, sizeof(pInfo->xParams.pTriggers));
	}

	if (xFields & FTM_RULE_FIELD_ACTIONS)
	{
		pRule->xInfo.xParams.ulActions = pInfo->xParams.ulActions;
		memcpy(pRule->xInfo.xParams.pActions, pInfo->xParams.pActions, sizeof(pInfo->xParams.pActions));
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
		ERROR("Message push error![%08x]\n", xRet);
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
		ERROR("Message push error![%08x]\n", xRet);
		FTOM_MSG_destroy((FTOM_MSG_PTR _PTR_)&pMsg);
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_discovery
(
	FTM_CHAR_PTR	pNetwork,
	FTM_USHORT		usPort
)
{
	ASSERT(pNetwork != NULL);

	FTM_RET	xRet;
	FTOM_MSG_DISCOVERY_PTR	pMsg;

	xRet = FTOM_MSG_createDiscovery(pNetwork, usPort, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Discovery message creation failed[%08x].\n", xRet);
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(pMsgQ, (FTOM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
		FTOM_MSG_destroy((FTOM_MSG_PTR _PTR_)&pMsg);
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_discoveryEPCount
(
	FTM_CHAR_PTR	pIP,
	FTM_EP_TYPE		xType,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pIP != NULL);
	ASSERT(pulCount != NULL);

	return	FTOM_SNMPC_getEPCount(pSNMPC, pIP, xType, pulCount);
}

FTM_RET	FTOM_getNodeInfo
(
	FTM_CHAR_PTR	pDID,
	FTM_NODE_PTR	pNodeInfo
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTOM_discoveryEP
(
	FTM_CHAR_PTR	pIP,
	FTM_EP_TYPE		xType,
	FTM_ULONG		ulIndex,
	FTM_EP_PTR		pEPInfo
)
{
	ASSERT(pIP != NULL);
	ASSERT(pEPInfo != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pEPID[FTM_EPID_LEN+1];
	FTM_CHAR	pName[FTM_NAME_LEN + 1];

	xRet = FTOM_SNMPC_getEPID(pSNMPC, pIP, xType, ulIndex, pEPID);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP not found!\n");
		return	xRet;	
	}

	xRet = FTOM_SNMPC_getEPName(pSNMPC, pIP, xType, ulIndex, pName, FTM_NAME_LEN);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP not found!\n");
		return	xRet;	
	}

	strncpy(pEPInfo->pEPID, pEPID, FTM_EPID_LEN);
	strcpy(pEPInfo->pName, pName);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_sendMessage
(
	FTOM_MSG_PTR	pMsg
)
{
	return	FTOM_MSGQ_push(pMsgQ, pMsg);

}
