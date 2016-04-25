#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "ftom.h"
#include "ftdm_client.h"
#include "ftom_node.h"
#include "ftom_node_management.h"
#include "ftom_snmpc.h"
#include "ftom_ep.h"
#include "ftom_ep_management.h"
#include "ftom_service.h"
#include "ftom_server.h"
#include "ftom_dmc.h"
#include "ftom_shell.h"
#include "ftom_mqtt_client.h"
#include "ftom_msg.h"
#include "ftom_utils.h"
#include "ftom_trigger.h"
#include "ftom_action.h"
#include "ftom_rule.h"
#include "ftom_discovery.h"

FTM_VOID_PTR	FTOM_process(FTM_VOID_PTR pData);
FTM_RET			FTOM_TASK_startService(FTOM_PTR pOM);
FTM_RET			FTOM_TASK_stopService(FTOM_PTR pOM);
FTM_RET			FTOM_TASK_sync(FTOM_PTR pOM);

FTM_RET			FTOM_TASK_start(FTOM_PTR pOM);
FTM_RET			FTOM_TASK_stop(FTOM_PTR pOM);
FTM_RET			FTOM_TASK_processing(FTOM_PTR pOM);

static FTM_RET	FTOM_onQuit
(
	FTOM_PTR	pOM,
	FTOM_MSG_TIME_SYNC_PTR	pMsg
);

static FTM_RET	FTOM_onTimeSync
(
	FTOM_PTR	pOM,
	FTOM_MSG_TIME_SYNC_PTR	pMsg
);

static FTM_RET	FTOM_onEPCtrl
(
	FTOM_PTR		pOM,
	FTOM_MSG_EP_CTRL_PTR	pMsg
);

static FTM_RET FTOM_onSaveEPData
(
	FTOM_PTR	pOM,
	FTOM_MSG_SAVE_EP_DATA_PTR	pMsg
);

static FTM_RET	FTOM_onSendEPData
(
	FTOM_PTR	pOM,
	FTOM_MSG_SEND_EP_DATA_PTR pMsg
);

static FTM_RET	FTOM_onRule
(
	FTOM_PTR pOM,
	FTOM_MSG_RULE_PTR pMsg
);

static FTM_RET	FTOM_onAlert
(
	FTOM_PTR pOM,
	FTOM_MSG_ALERT_PTR pMsg
);

static FTM_RET	FTOM_onDiscovery
(
	FTOM_PTR pOM,
	FTOM_MSG_DISCOVERY_PTR pMsg
);

static FTM_RET	FTOM_onDiscoveryInfo
(
	FTOM_PTR pOM,
	FTOM_MSG_DISCOVERY_INFO_PTR pMsg
);

static FTM_RET	FTOM_onDiscoveryDone
(
	FTOM_PTR pOM,
	FTOM_MSG_DISCOVERY_DONE_PTR pMsg
);

static 	FTM_RET	FTOM_callback
(
	FTOM_SERVICE_ID xID, 
	FTOM_MSG_TYPE 	xMsg, 
	FTM_VOID_PTR 	pData
);

//static	FTM_RET	FTOM_SNMPTrapCB(FTM_CHAR_PTR pTrapMsg);
static	FTOM_SERVER		xServer;
static	FTOM_SNMPC		xSNMPC;
static	FTOM_SNMPTRAPD	xSNMPTRAPD;
static	FTOM_DMC		xDMC;
static	FTOM_SHELL		xShell;
static	FTOM_MQTT_CLIENT	xMQTTC;
static 	FTOM_DISCOVERY	xDiscovery;

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
		.pData		= 	(FTM_VOID_PTR)&xServer
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
		.pData		=	(FTM_VOID_PTR)&xSNMPC
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
		.pData		=	(FTM_VOID_PTR)&xSNMPTRAPD
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
		.pData		=	(FTM_VOID_PTR)&xDMC
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
		.pData		= 	(FTM_VOID_PTR)&xMQTTC
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
		.pData		= 	(FTM_VOID_PTR)&xShell
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
		.pData		= 	(FTM_VOID_PTR)&xDiscovery
	},
};

extern char *program_invocation_short_name;

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

FTM_RET	FTOM_create
(
	FTOM_PTR _PTR_ ppOM
)
{
	ASSERT(ppOM != NULL);
	
	FTM_RET				xRet;
	FTOM_PTR	pOM;

	pOM = (FTOM_PTR)FTM_MEM_malloc(sizeof(FTOM));
	if(pOM == NULL)
	{
		ERROR("Not enough memory");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_init(pOM);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pOM);	
	}
	else
	{
		*ppOM = pOM;	
	}

	return	xRet;
}

FTM_RET	FTOM_destroy(FTOM_PTR _PTR_ ppOM)
{
	ASSERT(ppOM != NULL);
	
	FTM_RET	xRet;

	if (*ppOM == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	xRet = FTOM_final(*ppOM);
	if (xRet != FTM_RET_OK)
	{
		WARN("FTOM finalize failed[%08x].\n", xRet);
	}

	FTM_MEM_free(*ppOM);

	*ppOM = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_init(FTOM_PTR pOM)
{
	FTM_RET	xRet;

	memset(pOM, 0, sizeof(FTOM));

	FTOM_getDefaultDeviceID(pOM->xConfig.pDID);
	TRACE("DID : %s\n", pOM->xConfig.pDID);

	pOM->bStop = FTM_TRUE;

	pOM->onMessage[FTOM_MSG_TYPE_QUIT] 			= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onQuit;
	pOM->onMessage[FTOM_MSG_TYPE_SAVE_EP_DATA] 	= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onSaveEPData;
	pOM->onMessage[FTOM_MSG_TYPE_SEND_EP_DATA] 	= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onSendEPData;
	pOM->onMessage[FTOM_MSG_TYPE_TIME_SYNC] 	= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onTimeSync;
	pOM->onMessage[FTOM_MSG_TYPE_EP_CTRL] 		= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onEPCtrl;
	pOM->onMessage[FTOM_MSG_TYPE_RULE] 			= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onRule;
	pOM->onMessage[FTOM_MSG_TYPE_ALERT] 		= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onAlert;
	pOM->onMessage[FTOM_MSG_TYPE_DISCOVERY] 	= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onDiscovery;
	pOM->onMessage[FTOM_MSG_TYPE_DISCOVERY_INFO]= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onDiscoveryInfo;
	pOM->onMessage[FTOM_MSG_TYPE_DISCOVERY_DONE]= (FTOM_ON_MESSAGE_CALLBACK)FTOM_onDiscoveryDone;

	xRet = FTOM_MSGQ_create(&pOM->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message queue creation failed[%08x].\n", xRet);
		goto error;
	}

	xRet = FTOM_NODEM_create(pOM, &pOM->pNodeM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node management creation failed[%08x].\n", xRet);
		goto error;
	}

	xRet = FTOM_EPM_create(pOM, &pOM->pEPM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP management creation failed[%08x].\n", xRet);
		goto error;
	}

	xRet = FTOM_TRIGGERM_create(pOM, &pOM->pTriggerM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Trigger management creation failed[%08x].\n", xRet);
		goto error;
	}

	xRet = FTOM_ACTIONM_create(pOM, &pOM->pActionM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Action management creation failed[%08x].\n", xRet);
		goto error;
	}

	xRet = FTOM_RULEM_create(pOM, &pOM->pRuleM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Rule management creation failed[%08x].\n", xRet);
		goto error;
	}

	FTOM_RULEM_setTriggerM(pOM->pRuleM, pOM->pTriggerM);
	FTOM_RULEM_setActionM(pOM->pRuleM, pOM->pActionM);

	FTOM_SERVICE_init(pOM, pServices, sizeof(pServices) / sizeof(FTOM_SERVICE));

	TRACE("initialization done.\n");

	return	FTM_RET_OK;

error:
	if (pOM->pRuleM != NULL)
	{
		FTOM_RULEM_destroy(&pOM->pRuleM);
	}

	if (pOM->pActionM != NULL)
	{
		FTOM_ACTIONM_destroy(&pOM->pActionM);
	}

	if (pOM->pTriggerM != NULL)
	{
		FTOM_TRIGGERM_destroy(&pOM->pTriggerM);
	}

	if (pOM->pEPM != NULL)
	{
		FTOM_EPM_destroy(&pOM->pEPM);
	}

	if (pOM->pMsgQ!= NULL)
	{
		FTOM_MSGQ_destroy(&pOM->pMsgQ);
	}

	if (pOM->pNodeM != NULL)
	{
		FTOM_NODEM_destroy(&pOM->pNodeM);
	}
	
	return	xRet;
}

FTM_RET	FTOM_final
(
	FTOM_PTR pOM
)
{
	FTM_RET	xRet;

	FTOM_SERVICE_final();

	xRet = FTOM_RULEM_destroy(&pOM->pRuleM);
	if (xRet != FTM_RET_OK)
	{
		WARN("Rule management destruction failed.\n");
	}

	xRet = FTOM_ACTIONM_destroy(&pOM->pActionM);
	if (xRet != FTM_RET_OK)
	{
		WARN("Action management destruction failed.\n");
	}

	xRet = FTOM_TRIGGERM_destroy(&pOM->pTriggerM);
	if (xRet != FTM_RET_OK)
	{
		WARN("Trigger management destruction failed.\n");
	}

	xRet = FTOM_MSGQ_destroy(&pOM->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		WARN("Message queue destruction failed.\n");
	}

	xRet = FTOM_EPM_destroy(&pOM->pEPM);
	if (xRet != FTM_RET_OK)
	{
		WARN("EP management destruction failed.\n");
	}

	xRet = FTOM_NODEM_destroy(&pOM->pNodeM);
	if (xRet != FTM_RET_OK)
	{
		WARN("Node management destruction failed.\n");
	}

	TRACE("Finalize done.\n");

	return	FTM_RET_OK;
}

FTM_RET	FTOM_loadFromFile
(
	FTOM_PTR 		pOM, 
	FTM_CHAR_PTR 	pFileName
)
{
	ASSERT(pOM != NULL);
	ASSERT(pFileName != NULL);

	FTOM_SERVICE_loadFromFile(FTOM_SERVICE_ALL, pFileName);

	TRACE("loaded configuration.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTOM_showConfig
(
	FTOM_PTR 	pOM
)
{
	FTOM_SERVICE_showConfig(FTOM_SERVICE_ALL);

	return	FTM_RET_OK;
}

FTM_RET FTOM_start
(
	FTOM_PTR 	pOM
)
{
	ASSERT(pOM != NULL);

	if (!pOM->bStop)
	{
		return	FTM_RET_ALREADY_STARTED;	
	}

	if (pthread_create(&pOM->xThread, NULL, FTOM_process, (FTM_VOID_PTR)pOM) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET FTOM_stop
(
	FTOM_PTR 	pOM
)
{
	ASSERT(pOM != NULL);

	if (pOM->bStop)
	{
		return	FTM_RET_NOT_START;	
	}

	pOM->bStop = FTM_TRUE;
	pthread_join(pOM->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_RET FTOM_waitingForFinished
(
	FTOM_PTR 	pOM
)
{
	pthread_join(pOM->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTOM_process
(
	FTM_VOID_PTR 	pData
)
{
	ASSERT (pData != NULL);
	
	FTOM_PTR	pOM = (FTOM_PTR)pData;

	pOM->xState = FTOM_STATE_INITIALIZED;
	pOM->bStop	= FTM_FALSE;

	while(!pOM->bStop)
	{
		switch(pOM->xState)
		{
		case	FTOM_STATE_INITIALIZED:
			{
				FTOM_TASK_startService(pOM);
			}
			break;

		case	FTOM_STATE_CONNECTED:
			{
				FTOM_TASK_sync(pOM);
			}
			break;

		case	FTOM_STATE_SYNCHRONIZED:
			{
				FTOM_TASK_start(pOM);	
			}
			break;

		case	FTOM_STATE_PROCESSING:
			{
				FTOM_TASK_processing(pOM);	
			}
			break;
		}
	}

	if (pOM->xState == FTOM_STATE_PROCESSING)
	{
		FTOM_TASK_stop(pOM);	
		FTOM_TASK_stopService(pOM);	
	}

	TRACE("finished.\n");
	return	0;
}

FTM_RET	FTOM_TASK_startService
(
	FTOM_PTR 	pOM
)
{
	ASSERT(pOM != NULL);

	FTOM_SERVICE_start(FTOM_SERVICE_ALL);

	pOM->xState = FTOM_STATE_CONNECTED;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TASK_sync
(
	FTOM_PTR 	pOM
)
{
	ASSERT(pOM != NULL);

	FTM_RET			xRet;
	FTM_ULONG		ulCount, i;

	xRet = FTDMC_NODE_count(&xDMC.xSession, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	TRACE("Load Node Object : %lu\n", ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_NODE	xNodeInfo;
		FTOM_NODE_PTR	pNode;

		xRet = FTDMC_NODE_getAt(&xDMC.xSession, i, &xNodeInfo);	
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

		xRet = FTOM_NODEM_attachNode(pOM->pNodeM, pNode);
		if (xRet != FTM_RET_OK)
		{
			FTOM_NODE_destroy(&pNode);
			ERROR("Can't attach node[%08x].\n", xRet);
			continue;
		}

		TRACE("Node[%s] creating success.\n", pNode->xInfo.pDID);
	}

	xRet = FTDMC_EP_CLASS_count(&xDMC.xSession, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP Class count get failed[%08x].\n", xRet);
		return	xRet;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_EP_CLASS	xEPClassInfo;

		xRet = FTDMC_EP_CLASS_getAt(&xDMC.xSession, i, &xEPClassInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTDMC_EP_CLASS_getAt(%08lx, %d, &xEPInfo) = %08lx\n",
					xDMC.xSession.hSock, i, xRet);
			continue;
		}

		xRet = FTOM_EPM_createClass(pOM->pEPM, &xEPClassInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTOM_EP_TYPE_append(&xEPClassInfo) = %08lx\n", xRet);
			continue;	
		}
	}

	xRet = FTDMC_EP_count(&xDMC.xSession, 0, &ulCount);
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

		xRet = FTDMC_EP_getAt(&xDMC.xSession, i, &xEPInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("EP object get at %d failed[%08x]\n", i, xRet);
			continue;
		}

		xRet = FTOM_EPM_createEP(pOM->pEPM, &xEPInfo, &pEP);
		if (xRet != FTM_RET_OK)
		{
			ERROR("EP[%s] object creation failed[%08x]\n", xEPInfo.pEPID, xRet);
			continue;	
		}

		if (FTOM_NODEM_getNode(pOM->pNodeM, xEPInfo.pDID, &pNode) == FTM_RET_OK)
		{
			FTOM_NODE_linkEP(pNode, pEP);
		}
		
		TRACE("EP[%s] creating success.\n", pEP->xInfo.pEPID);
	}

	xRet = FTDMC_TRIGGER_count(&xDMC.xSession, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Trigger count get failed[%08x].\n", xRet);
		return	xRet;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_TRIGGER		xTriggerInfo;
		FTOM_TRIGGER_PTR	pTrigger = NULL;

		xRet = FTDMC_TRIGGER_getAt(&xDMC.xSession, i, &xTriggerInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Event[%d] data load failed.\n", i);	
			continue;
		}

		xRet = FTOM_TRIGGERM_add(pOM->pTriggerM, &xTriggerInfo, &pTrigger);
		if (xRet != FTM_RET_OK)
		{
			ERROR("The new event can not registration!\n") ;
			continue;
		}
	}

	xRet = FTDMC_ACTION_count(&xDMC.xSession, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Action count get failed[%08x].\n", xRet);
		return	xRet;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_ACTION	xActionInfo;
		FTOM_ACTION_PTR pAction = NULL;

		xRet = FTDMC_ACTION_getAt(&xDMC.xSession, i, &xActionInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Action[%d] data load failed.\n", i);	
			continue;
		}

		xRet = FTOM_ACTIONM_add(pOM->pActionM, &xActionInfo, &pAction);
		if (xRet != FTM_RET_OK)
		{
			ERROR("The new action event can not registration!\n") ;
			continue;
		}
	}

	xRet = FTDMC_RULE_count(&xDMC.xSession, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Rule count get failed[%08x].\n", xRet);
		return	xRet;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_RULE	xRuleInfo;
		FTOM_RULE_PTR	pRule = NULL;

		xRet = FTDMC_RULE_getAt(&xDMC.xSession, i, &xRuleInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Rule[%d] data load failed.\n", i);	
			continue;
		}

		xRet = FTOM_RULEM_add(pOM->pRuleM, &xRuleInfo, &pRule);
		if (xRet != FTM_RET_OK)
		{
			ERROR("The new action event can not registration!\n") ;
			continue;
		}
	}

	pOM->xState = FTOM_STATE_SYNCHRONIZED;
	return	FTM_RET_OK;
}

FTM_RET	FTOM_TASK_start(FTOM_PTR pOM)
{
	ASSERT(pOM != NULL);

	FTM_ULONG	i, ulCount;
	
	TRACE("Start FTOM!\n");
	FTOM_RULEM_start(pOM->pRuleM);
	FTOM_ACTIONM_start(pOM->pActionM);
	FTOM_TRIGGERM_start(pOM->pTriggerM);

	FTOM_NODEM_countNode(pOM->pNodeM, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTOM_NODE_PTR	pNode;

		if (FTOM_NODEM_getNodeAt(pOM->pNodeM, i, &pNode) == FTM_RET_OK)
		{
			FTOM_NODE_start(pNode);
		}
		else
		{
			ERROR("Node not found at %d\n", i);
		}
	}

	pOM->xState = FTOM_STATE_PROCESSING;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TASK_processing
(
	FTOM_PTR pOM
)
{
	ASSERT(pOM != NULL);

	FTM_RET			xRet;
	FTOM_MSG_PTR	pMsg = NULL;
	FTM_TIMER		xLoopTimer;

	FTM_TIMER_init(&xLoopTimer, FTOM_LOOP_INTERVAL);

	while(!pOM->bStop)
	{
		FTM_ULONG	ulRemainTime;
		
		FTM_TIMER_remain(&xLoopTimer, &ulRemainTime);
		xRet = FTOM_MSGQ_timedPop(pOM->pMsgQ, ulRemainTime, &pMsg);
		if (xRet == FTM_RET_OK)
		{
			if ((pMsg->xType < FTOM_MSG_TYPE_MAX) && (pOM->onMessage[pMsg->xType] != NULL))
			{
				xRet = pOM->onMessage[pMsg->xType](pOM, pMsg, pOM->pOnMessageData[pMsg->xType]);
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
	FTOM_PTR 		pOM
)
{
	ASSERT(pOM != NULL);

	FTM_ULONG	i, ulCount;
	
	FTOM_TRIGGERM_stop(pOM->pTriggerM);
	FTOM_ACTIONM_stop(pOM->pActionM);
	FTOM_RULEM_stop(pOM->pRuleM);
	FTOM_NODEM_countNode(pOM->pNodeM, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTOM_NODE_PTR	pNode;

		if (FTOM_NODEM_getNodeAt(pOM->pNodeM, i, &pNode) == FTM_RET_OK)
		{
			FTOM_NODE_stop(pNode);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_TASK_stopService
(
	FTOM_PTR 		pOM
)
{
	ASSERT(pOM != NULL);

	FTOM_SERVICE_stop(FTOM_SERVICE_ALL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_onQuit
(
	FTOM_PTR	pOM,
	FTOM_MSG_TIME_SYNC_PTR	pMsg
)
{
	ASSERT(pOM != NULL);

	pOM->bStop = FTM_TRUE;
	pOM->xState = FTOM_STATE_STOPED;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_onTimeSync
(
	FTOM_PTR	pOM,
	FTOM_MSG_TIME_SYNC_PTR	pMsg
)
{
	ASSERT(pOM != NULL);
	ASSERT(pMsg != NULL);

	TRACE("Time Sync - %d\n", pMsg->ulTime);

	return	FTM_RET_OK;
}

FTM_RET FTOM_onSaveEPData
(
	FTOM_PTR	pOM,
	FTOM_MSG_SAVE_EP_DATA_PTR	pMsg
)
{
	ASSERT(pOM != NULL);
	ASSERT(pMsg != NULL);

	FTM_CHAR	pBuff[64];

	FTM_EP_DATA_snprint(pBuff, sizeof(pBuff), &pMsg->xData);

	FTOM_TRIGGERM_updateEP(pOM->pTriggerM, pMsg->pEPID, &pMsg->xData);
	FTOM_DMC_appendEPData(&xDMC, pMsg->pEPID, &pMsg->xData);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_onEPCtrl
(
	FTOM_PTR		pOM,
	FTOM_MSG_EP_CTRL_PTR	pMsg
)
{
	FTM_RET			xRet;
	FTOM_EP_PTR		pEP;
	FTM_EP_DATA		xData;

	TRACE("EP[%s] Control\n", pMsg->pEPID);

	xRet = FTOM_EPM_getEP(pOM->pEPM, pMsg->pEPID, &pEP);
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
	FTOM_PTR	pOM,
	FTOM_MSG_SEND_EP_DATA_PTR pMsg
)
{
	ASSERT(pOM != NULL);
	ASSERT(pMsg != NULL);

	return	FTOM_MQTT_CLIENT_publishEPData(&xMQTTC, pMsg->pEPID, pMsg->pData, pMsg->ulCount);
}

FTM_RET	FTOM_onRule
(
	FTOM_PTR pOM,
	FTOM_MSG_RULE_PTR pMsg
)
{
	ASSERT(pOM != NULL);
	ASSERT(pMsg != NULL);

	TRACE("RULE[%s] is %s\n", pMsg->pRuleID, (pMsg->xRuleState == FTM_RULE_STATE_ACTIVATE)?"ACTIVATE":"DEACTIVATE");

	return	FTM_RET_OK;
}

FTM_RET	FTOM_onAlert
(
	FTOM_PTR	pOM,
	FTOM_MSG_ALERT_PTR	pMsg
)
{
	ASSERT(pOM != NULL);
	ASSERT(pMsg != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_onDiscovery
(
	FTOM_PTR	pOM,
	FTOM_MSG_DISCOVERY_PTR	pMsg
)
{
	ASSERT(pOM != NULL);
	ASSERT(pMsg != NULL);

	FTOM_DISCOVERY_call(&xDiscovery, pMsg->pNetwork, pMsg->usPort);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_onDiscoveryInfo
(
	FTOM_PTR	pOM,
	FTOM_MSG_DISCOVERY_INFO_PTR	pMsg
)
{
	ASSERT(pOM != NULL);
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
	FTOM_PTR	pOM,
	FTOM_MSG_DISCOVERY_DONE_PTR	pMsg
)
{
	ASSERT(pOM != NULL);
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
	FTOM_PTR					pOM,
	FTOM_MSG_TYPE 				xMsg, 
	FTOM_ON_MESSAGE_CALLBACK	fMessageCB,
	FTM_VOID_PTR				pData,
	FTOM_ON_MESSAGE_CALLBACK _PTR_	pOldCB,
	FTM_VOID_PTR _PTR_	ppOldData

)
{
	ASSERT(pOM != NULL);
	ASSERT(fMessageCB != NULL);

	if (xMsg >= FTOM_MSG_TYPE_MAX)
	{
		ERROR("Message type[%08x] is invalid.\n", xMsg);
		return	FTM_RET_INVALID_MESSAGE_TYPE;
	}

	if (pOldCB != NULL)
	{
		*pOldCB = pOM->onMessage[xMsg];
	}

	if (ppOldData != NULL)
	{
		*ppOldData = pOM->pOnMessageData[xMsg];
	}

	pOM->onMessage[xMsg] = fMessageCB;
	pOM->pOnMessageData[xMsg] = pData;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_getDID
(
	FTOM_PTR 		pOM, 
	FTM_CHAR_PTR 	pBuff, 
	FTM_ULONG 		ulBuffLen
)
{
	ASSERT(pBuff != NULL);

	if (ulBuffLen < strlen(pOM->xConfig.pDID) + 1)
	{
		return	FTM_RET_BUFFER_TOO_SMALL;	
	}

	strcpy(pBuff, pOM->xConfig.pDID);

	return	FTM_RET_OK;
}

/******************************************************************
 * EP management interface
 ******************************************************************/

FTM_RET	FTOM_createEP
(
	FTOM_PTR 	pOM, 
	FTM_EP_PTR 	pInfo
)
{
	ASSERT(pOM != NULL);
	ASSERT(pInfo != NULL);

	return	FTDMC_EP_append(&xDMC.xSession, pInfo);
}

FTM_RET	FTOM_destroyEP
(
	FTOM_PTR 	pOM, 
	FTM_CHAR_PTR	pEPID
)
{
	ASSERT(pOM != NULL);

	return	FTDMC_EP_remove(&xDMC.xSession, pEPID);
}

FTM_RET	FTOM_setEPInfo
(
	FTOM_PTR		pOM,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_FIELD	xFields,
	FTM_EP_PTR		pInfo
)
{
	ASSERT(pOM != NULL);
	ASSERT(pEPID != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTOM_EP_PTR	pEP;

	xRet = FTOM_EPM_getEP(pOM->pEPM, pEPID, &pEP);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}
	
	xRet = FTDMC_EP_set(&xDMC.xSession, pEPID, xFields, pInfo);
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

FTM_RET	FTOM_getEPDataList
(
	FTOM_PTR 		pOM, 
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG 		ulStart, 
	FTM_EP_DATA_PTR pDataList, 
	FTM_ULONG 		ulMaxCount, 
	FTM_ULONG_PTR 	pulCount
)
{
	ASSERT(pOM != NULL);
	ASSERT(pDataList != NULL);
	ASSERT(pulCount != NULL);

	return	FTDMC_EP_DATA_get(&xDMC.xSession, pEPID, ulStart, pDataList, ulMaxCount, pulCount);
}

FTM_RET	FTOM_getEPDataInfo
(
	FTOM_PTR 		pOM, 
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR 	pulBeginTime, 
	FTM_ULONG_PTR 	pulEndTime, 
	FTM_ULONG_PTR 	pulCount
)
{
	return	FTDMC_EP_DATA_info(&xDMC.xSession, pEPID, pulBeginTime, pulEndTime, pulCount);
}

FTM_RET	FTOM_getEPDataCount
(
	FTOM_PTR 		pOM, 
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR 	pulCount
)
{
	return	FTDMC_EP_DATA_count(&xDMC.xSession, pEPID, pulCount);
}


FTM_RET	FTOM_NOTIFY_rule
(
	FTOM_PTR 		pOM,
	FTM_CHAR_PTR	pRuleID,
	FTM_RULE_STATE	xRuleState
)
{
	ASSERT(pOM != NULL);

	FTM_RET			xRet;
	FTOM_MSG_RULE_PTR	pMsg;

	xRet = FTOM_MSG_createRule(pRuleID, xRuleState, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message creation failed.\n");
		return	xRet;
	}

	xRet = FTOM_MSGQ_push(pOM->pMsgQ, (FTOM_MSG_PTR)pMsg);
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

FTM_RET	FTOM_createNode
(
	FTOM_PTR		pOM,
	FTM_NODE_PTR	pInfo,
	FTOM_NODE_PTR _PTR_	ppNode
)
{
	ASSERT(pOM != NULL);
	ASSERT(pInfo != NULL);
	ASSERT(ppNode != NULL);

	FTM_RET	xRet;
	FTOM_NODE_PTR	pNode;

	xRet = FTOM_NODE_create(pInfo, &pNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node creation failed[%08x]\n", xRet);
		return	xRet;	
	}

	xRet = FTOM_NODEM_attachNode(pOM->pNodeM, pNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't attach node[%08x]\n", xRet);
		FTOM_NODE_destroy(&pNode);
		return	xRet;	
	}

	*ppNode = pNode;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_destroyNode
(
	FTOM_PTR		pOM,
	FTOM_NODE_PTR _PTR_	ppNode
)
{
	ASSERT(pOM != NULL);
	ASSERT(ppNode != NULL);

	return	FTOM_NODE_destroy(ppNode);
}

FTM_RET	FTOM_countNode
(
	FTOM_PTR		pOM,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pOM != NULL);
	ASSERT(pulCount != NULL);

	return	FTOM_NODEM_countNode(pOM->pNodeM, pulCount);
}

FTM_RET	FTOM_getNode
(
	FTOM_PTR		pOM,
	FTM_CHAR		pDID[FTM_DID_LEN + 1],
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pOM != NULL);
	ASSERT(pDID != NULL);
	ASSERT(ppNode != NULL);

	return	FTOM_NODEM_getNode(pOM->pNodeM, pDID, ppNode);
}

FTM_RET	FTOM_getNodeAt
(
	FTOM_PTR		pOM,
	FTM_ULONG		ulIndex,
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pOM != NULL);
	ASSERT(ppNode != NULL);

	return	FTOM_NODEM_getNodeAt(pOM->pNodeM, ulIndex, ppNode);
}

FTM_RET	FTOM_setNode
(
	FTOM_PTR		pOM,
	FTM_CHAR_PTR	pDID,
	FTM_NODE_FIELD	xFields,
	FTM_NODE_PTR 	pInfo
)
{
	ASSERT(pOM != NULL);
	ASSERT(pDID != NULL);
	ASSERT(pInfo != NULL);

	return	FTOM_NODEM_setNode(pOM->pNodeM, pDID, xFields, pInfo);
}

FTM_RET	FTOM_setEPData
(
	FTOM_PTR 		pOM, 
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR pData
)
{
	ASSERT(pOM != NULL);
	ASSERT(pData != NULL);

	FTM_RET						xRet;
	FTOM_MSG_SET_EP_DATA_PTR	pMsg;

	xRet = FTOM_MSG_createSetEPData(pEPID, pData, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(pOM->pMsgQ, (FTOM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
		FTOM_MSG_destroy((FTOM_MSG_PTR _PTR_)&pMsg);
		return	xRet;
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTOM_saveEPData
(
	FTOM_PTR 		pOM, 
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR pData
)
{
	ASSERT(pOM != NULL);
	ASSERT(pData != NULL);

	FTM_RET						xRet;
	FTOM_MSG_SAVE_EP_DATA_PTR	pMsg;

	xRet = FTOM_MSG_createSaveEPData(pEPID, pData, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		WARN("Save EP data message creation failed[%08x].\n", xRet);
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(pOM->pMsgQ, (FTOM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
		FTOM_MSG_destroy((FTOM_MSG_PTR _PTR_)&pMsg);
		return	xRet;
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTOM_sendEPData
(
	FTOM_PTR		pOM,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR	pData,
	FTM_ULONG		ulCount
)
{
	ASSERT(pOM != NULL);
	ASSERT(pData != NULL);

	FTM_RET						xRet;
	FTOM_MSG_SEND_EP_DATA_PTR	pMsg;

	xRet = FTOM_MSG_createSendEPData(pEPID, pData, ulCount, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		WARN("Send EP data message creation failed[%08x].\n", xRet);
		return	xRet;	
	}


	xRet = FTOM_MSGQ_push(pOM->pMsgQ, (FTOM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
		FTOM_MSG_destroy((FTOM_MSG_PTR _PTR_)&pMsg);
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_addTrigger
(
	FTOM_PTR			pOM,
	FTM_TRIGGER_PTR		pInfo,
	FTM_CHAR_PTR		pTriggerID
)
{
	ASSERT(pOM != NULL);
	
	FTM_RET		xRet;
	FTOM_TRIGGER_PTR	pTrigger;

	xRet = FTOM_TRIGGERM_add(pOM->pTriggerM, pInfo, &pTrigger);
	if (xRet == FTM_RET_OK)
	{
		strncpy(pTriggerID, pTrigger->xInfo.pID, FTM_ID_LEN);
		xRet = FTDMC_TRIGGER_add(&xDMC.xSession, &pTrigger->xInfo);
		if (xRet != FTM_RET_OK)
		{
			FTOM_TRIGGERM_del(pOM->pTriggerM, pTriggerID);
		}
	}
	return	xRet;
}

FTM_RET	FTOM_delTrigger
(
	FTOM_PTR		pOM,
	FTM_CHAR_PTR	pTriggerID
)
{
	ASSERT(pOM != NULL);

	FTM_RET	xRet;

	xRet =  FTOM_TRIGGERM_del(pOM->pTriggerM, pTriggerID);

	return	xRet;
}

FTM_RET	FTOM_getTriggerCount
(
	FTOM_PTR		pOM,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pOM != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET	xRet;

	xRet = FTOM_TRIGGERM_count(pOM->pTriggerM, pulCount);

	return	xRet;
}

FTM_RET	FTOM_getTriggerInfo
(
	FTOM_PTR		pOM,
	FTM_CHAR_PTR	pTriggerID,
	FTM_TRIGGER_PTR	pInfo
)
{
	ASSERT(pOM != NULL);
	ASSERT(pTriggerID != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTOM_TRIGGER_PTR	pTrigger;

	xRet = FTOM_TRIGGERM_get(pOM->pTriggerM, pTriggerID, &pTrigger);
	if (xRet == FTM_RET_OK)
	{
		memcpy(pInfo, &pTrigger->xInfo, sizeof(FTM_TRIGGER));
	}

	return	xRet;
}

FTM_RET	FTOM_getTriggerInfoAt
(
	FTOM_PTR		pOM,
	FTM_ULONG		ulIndex,
	FTM_TRIGGER_PTR	pInfo
)
{
	ASSERT(pOM != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTOM_TRIGGER_PTR	pTrigger;

	xRet = FTOM_TRIGGERM_getAt(pOM->pTriggerM, ulIndex, &pTrigger);
	if (xRet == FTM_RET_OK)
	{
		memcpy(pInfo, &pTrigger->xInfo, sizeof(FTM_TRIGGER));
	}

	return	xRet;
}

FTM_RET	FTOM_setTriggerInfo
(
	FTOM_PTR			pOM,
	FTM_CHAR_PTR		pTriggerID,
	FTM_TRIGGER_FIELD	xFields,
	FTM_TRIGGER_PTR		pInfo
)
{
	ASSERT(pOM != NULL);
	ASSERT(pTriggerID != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTOM_TRIGGER_PTR	pTrigger;

	xRet = FTOM_TRIGGERM_get(pOM->pTriggerM, pTriggerID, &pTrigger);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Trigger[%s] set failed.\n", pTriggerID);
		return	xRet;
	}

	xRet = FTDMC_TRIGGER_set(&xDMC.xSession, pTriggerID, xFields, pInfo);
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
	FTOM_PTR		pOM,
	FTM_ACTION_PTR	pInfo,
	FTM_CHAR_PTR	pActionID
)
{
	ASSERT(pOM != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTOM_ACTION_PTR	pAction = NULL;

	xRet = FTOM_ACTIONM_add(pOM->pActionM, pInfo, &pAction);
	if (xRet == FTM_RET_OK)
	{
		strncpy(pActionID, pAction->xInfo.pID, FTM_ID_LEN);
		xRet = FTDMC_ACTION_add(&xDMC.xSession, &pAction->xInfo);
		if (xRet != FTM_RET_OK)
		{
			FTOM_ACTIONM_del(pOM->pActionM, pActionID);
		}
	}
	return	xRet;

	return	xRet;
}

FTM_RET	FTOM_delAction
(
	FTOM_PTR		pOM,
	FTM_CHAR_PTR	pActionID
)
{
	ASSERT(pOM != NULL);

	FTM_RET	xRet;

	xRet = FTOM_ACTIONM_del(pOM->pActionM, pActionID);

	return	xRet;
}

FTM_RET	FTOM_getActionCount
(
	FTOM_PTR		pOM,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pOM != NULL);

	FTM_RET	xRet;

	xRet = FTOM_ACTIONM_count(pOM->pActionM, pulCount);

	return	xRet;
}

FTM_RET	FTOM_getActionInfo
(
	FTOM_PTR		pOM,
	FTM_CHAR_PTR	pActionID,
	FTM_ACTION_PTR	pInfo
)
{
	ASSERT(pOM != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTOM_ACTION_PTR	pAction;

	xRet = FTOM_ACTIONM_get(pOM->pActionM, pActionID, &pAction);
	if (xRet == FTM_RET_OK)
	{
		memcpy(pInfo, &pAction->xInfo, sizeof(FTM_ACTION));	
	}

	return	xRet;
}

FTM_RET	FTOM_getActionInfoAt
(
	FTOM_PTR		pOM,
	FTM_ULONG		ulIndex,
	FTM_ACTION_PTR	pInfo
)
{
	ASSERT(pOM != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTOM_ACTION_PTR	pAction;

	xRet = FTOM_ACTIONM_getAt(pOM->pActionM, ulIndex, &pAction);
	if (xRet == FTM_RET_OK)
	{
		memcpy(pInfo, &pAction->xInfo, sizeof(FTM_ACTION));	
	}

	return	xRet;
}

FTM_RET	FTOM_setActionInfo
(
	FTOM_PTR			pOM,
	FTM_CHAR_PTR		pActionID,
	FTM_ACTION_FIELD	xFields,
	FTM_ACTION_PTR		pInfo
)
{
	ASSERT(pOM != NULL);
	ASSERT(pActionID != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTOM_ACTION_PTR	pAction;

	xRet = FTOM_ACTIONM_get(pOM->pActionM, pActionID, &pAction);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Action[%s] get failed[%08x].\n", pActionID, xRet);
		return	xRet;
	}

	xRet = FTDMC_ACTION_set(&xDMC.xSession, pActionID, xFields, pInfo);
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

FTM_RET	FTOM_addRule
(
	FTOM_PTR		pOM,
	FTM_RULE_PTR	pInfo,
	FTM_CHAR_PTR	pRuleID
)
{
	ASSERT(pOM != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTOM_RULE_PTR pRule;

	xRet = FTOM_RULEM_add(pOM->pRuleM, pInfo, &pRule);
	if (xRet == FTM_RET_OK)
	{
		strncpy(pRuleID, pRule->xInfo.pID, FTM_ID_LEN);
		xRet = FTDMC_RULE_add(&xDMC.xSession, &pRule->xInfo);
		if (xRet != FTM_RET_OK)
		{
			FTOM_RULEM_del(pOM->pRuleM, pRuleID);
		}
	}
	return	xRet;
}

FTM_RET	FTOM_delRule
(
	FTOM_PTR	pOM,
	FTM_CHAR_PTR	pRuleID
)
{
	ASSERT(pOM != NULL);

	FTM_RET	xRet;

	xRet = FTOM_RULEM_del(pOM->pRuleM, pRuleID);

	return	xRet;
}

FTM_RET	FTOM_getRuleCount
(
	FTOM_PTR		pOM,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pOM != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET	xRet;

	xRet = FTOM_RULEM_count(pOM->pRuleM, pulCount);

	return	xRet;
}

FTM_RET	FTOM_getRuleInfo
(
	FTOM_PTR		pOM,
	FTM_CHAR_PTR	pRuleID,
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pOM != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTOM_RULE_PTR	pRule;

	xRet = FTOM_RULEM_get(pOM->pRuleM, pRuleID, &pRule);
	if (xRet == FTM_RET_OK)
	{
		memcpy(pInfo, &pRule->xInfo, sizeof(FTM_RULE));	
	}

	return	xRet;
}

FTM_RET	FTOM_getRuleInfoAt
(
	FTOM_PTR		pOM,
	FTM_ULONG		ulIndex,
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pOM != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTOM_RULE_PTR	pRule;

	xRet = FTOM_RULEM_getAt(pOM->pRuleM, ulIndex, &pRule);
	if (xRet == FTM_RET_OK)
	{
		memcpy(pInfo, &pRule->xInfo, sizeof(FTM_RULE));	
	}

	return	xRet;
}

FTM_RET	FTOM_setRuleInfo
(
	FTOM_PTR		pOM,
	FTM_CHAR_PTR	pRuleID,
	FTM_RULE_FIELD	xFields,
	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pOM != NULL);
	ASSERT(pRuleID != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTOM_RULE_PTR	pRule;

	xRet = FTOM_RULEM_get(pOM->pRuleM, pRuleID, &pRule);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Rule[%s] not found!\n", pRuleID);
		return	xRet;
	}

	xRet = FTDMC_RULE_set(&xDMC.xSession, pRuleID, xFields, pInfo);
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
	FTOM_PTR		pOM,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR	pData
)
{
	ASSERT(pOM != NULL);
	ASSERT(pData != NULL);

	FTM_RET				xRet;
	FTOM_MSG_ALERT_PTR	pMsg;

	xRet = FTOM_MSG_createAlert(pEPID, pData, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(pOM->pMsgQ, (FTOM_MSG_PTR)pMsg);
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
	FTOM_PTR		pOM,
	FTM_CHAR_PTR	pName,
	FTM_CHAR_PTR	pDID,
	FTM_CHAR_PTR	pIP,
	FTM_EP_TYPE_PTR	pTypes,
	FTM_ULONG		ulCount
)
{
	ASSERT(pOM != NULL);
	ASSERT(pDID != NULL);
	ASSERT(pTypes != NULL);

	FTM_RET				xRet;
	FTOM_MSG_DISCOVERY_INFO_PTR	pMsg;

	xRet = FTOM_MSG_createDiscoveryInfo(pName, pDID, pIP, pTypes, ulCount, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(pOM->pMsgQ, (FTOM_MSG_PTR)pMsg);
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
	FTOM_PTR		pOM,
	FTM_CHAR_PTR	pNetwork,
	FTM_USHORT		usPort
)
{
	ASSERT(pOM != NULL);
	ASSERT(pNetwork != NULL);

	FTM_RET	xRet;
	FTOM_MSG_DISCOVERY_PTR	pMsg;

	xRet = FTOM_MSG_createDiscovery(pNetwork, usPort, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Discovery message creation failed[%08x].\n", xRet);
		return	xRet;	
	}

	xRet = FTOM_MSGQ_push(pOM->pMsgQ, (FTOM_MSG_PTR)pMsg);
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
	FTOM_PTR		pOM,
	FTM_CHAR_PTR	pIP,
	FTM_EP_TYPE		xType,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pOM != NULL);
	ASSERT(pIP != NULL);
	ASSERT(pulCount != NULL);

	return	FTOM_SNMPC_getEPCount(&xSNMPC, pIP, xType, pulCount);
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
	FTOM_PTR		pOM,
	FTM_CHAR_PTR	pIP,
	FTM_EP_TYPE		xType,
	FTM_ULONG		ulIndex,
	FTM_EP_PTR		pEPInfo
)
{
	ASSERT(pOM != NULL);
	ASSERT(pIP != NULL);
	ASSERT(pEPInfo != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pEPID[FTM_EPID_LEN+1];
	FTM_CHAR	pName[FTM_NAME_LEN + 1];

	xRet = FTOM_SNMPC_getEPID(&xSNMPC, pIP, xType, ulIndex, pEPID);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP not found!\n");
		return	xRet;	
	}

	xRet = FTOM_SNMPC_getEPName(&xSNMPC, pIP, xType, ulIndex, pName, FTM_NAME_LEN);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP not found!\n");
		return	xRet;	
	}

	strncpy(pEPInfo->pEPID, pEPID, FTM_EPID_LEN);
	strcpy(pEPInfo->pName, pName);

	return	FTM_RET_OK;
}
