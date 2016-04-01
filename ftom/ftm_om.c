#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "ftm_om.h"
#include "ftdm_client.h"
#include "ftm_om_node.h"
#include "ftm_om_snmpc.h"
#include "ftm_om_ep.h"
#include "ftm_om_ep_class.h"
#include "ftm_om_service.h"
#include "ftm_om_server.h"
#include "ftm_om_dmc.h"
#include "ftm_om_shell.h"
#include "ftm_mqtt_client.h"
#include "ftm_om_msg.h"
#include "ftm_om_utils.h"
#include "nxjson.h"

FTM_VOID_PTR	FTM_OM_process(FTM_VOID_PTR pData);
FTM_RET			FTM_OM_TASK_startService(FTM_OM_PTR pOM);
FTM_RET			FTM_OM_TASK_sync(FTM_OM_PTR pOM);
FTM_RET			FTM_OM_TASK_startEP(FTM_OM_PTR pOM);
FTM_RET			FTM_OM_TASK_processing(FTM_OM_PTR pOM);
FTM_RET			FTM_OM_TASK_stopService(FTM_OM_PTR pOM);

static FTM_RET	FTM_OM_onTimeSync
(
	FTM_OM_PTR	pOM,
	FTM_OM_MSG_TIME_SYNC_PTR	pMsg
);

static FTM_RET	FTM_OM_onEPCtrl
(
	FTM_OM_PTR		pOM,
	FTM_OM_MSG_EP_CTRL_PTR	pMsg
);

static FTM_RET	FTM_OM_onSendEPData
(
	FTM_OM_PTR	pOM,
	FTM_OM_MSG_SEND_EP_DATA_PTR pMsg
);

static 	FTM_RET	FTM_OM_callback(FTM_OM_SERVICE_ID xID, FTM_OM_MSG_TYPE xMsg, FTM_VOID_PTR pData);
//static	FTM_RET	FTM_OM_SNMPTrapCB(FTM_CHAR_PTR pTrapMsg);
static	FTM_OM_SERVER		xServer;
static	FTM_OM_SNMPC		xSNMPC;
static	FTM_OM_SNMPTRAPD	xSNMPTRAPD;
static	FTM_OM_DMC			xDMC;
static	FTM_OM_SHELL		xShell;
static	FTM_MQTT_CLIENT		xMQTTC;

static 	FTM_OM_SERVICE	pServices[] =
{
	{
		.xType		=	FTM_OM_SERVICE_SERVER,
		.xID		=	FTM_OM_SERVICE_SERVER,
		.pName		=	"Server",
		.fInit		=	(FTM_OM_SERVICE_INIT)FTM_OM_SERVER_init,
		.fFinal		=	(FTM_OM_SERVICE_FINAL)FTM_OM_SERVER_final,
		.fStart 	=	(FTM_OM_SERVICE_START)FTM_OM_SERVER_start,
		.fStop		=	(FTM_OM_SERVICE_STOP)FTM_OM_SERVER_stop,
		.fSetCallback=	(FTM_OM_SERVICE_SET_CALLBACK)FTM_OM_SERVER_setServiceCallback,
		.fCallback	=	FTM_OM_callback,
		.fLoadFromFile=	(FTM_OM_SERVICE_LOAD_FROM_FILE)FTM_OM_SERVER_loadFromFile,
		.fShowConfig=	(FTM_OM_SERVICE_SHOW_CONFIG)FTM_OM_SERVER_showConfig,
		.fNotify	=	(FTM_OM_SERVICE_NOTIFY)FTM_OM_SERVER_notify,
		.pData		= 	(FTM_VOID_PTR)&xServer
	},
	{
		.xType		=	FTM_OM_SERVICE_SNMP_CLIENT,
		.xID		=	FTM_OM_SERVICE_SNMP_CLIENT,
		.pName		=	"SNMP Client",
		.fInit		=	(FTM_OM_SERVICE_INIT)FTM_OM_SNMPC_init,
		.fFinal		=	(FTM_OM_SERVICE_FINAL)FTM_OM_SNMPC_final,
		.fStart 	=	(FTM_OM_SERVICE_START)FTM_OM_SNMPC_start,
		.fStop		=	(FTM_OM_SERVICE_STOP)FTM_OM_SNMPC_stop,
		.fSetCallback=	(FTM_OM_SERVICE_SET_CALLBACK)FTM_OM_SNMPC_setServiceCallback,
		.fCallback	=	FTM_OM_callback,
		.fLoadFromFile=	(FTM_OM_SERVICE_LOAD_FROM_FILE)FTM_OM_SNMPC_loadFromFile,
		.fShowConfig=	(FTM_OM_SERVICE_SHOW_CONFIG)FTM_OM_SNMPC_showConfig,
		.pData		=	(FTM_VOID_PTR)&xSNMPC
	},
	{
		.xType		=	FTM_OM_SERVICE_SNMPTRAPD,
		.xID		=	FTM_OM_SERVICE_SNMPTRAPD,
		.pName		=	"SNMP TrapD",
		.fInit		=	(FTM_OM_SERVICE_INIT)FTM_OM_SNMPTRAPD_init,
		.fFinal		=	(FTM_OM_SERVICE_FINAL)FTM_OM_SNMPTRAPD_final,
		.fStart 	=	(FTM_OM_SERVICE_START)FTM_OM_SNMPTRAPD_start,
		.fStop		=	(FTM_OM_SERVICE_STOP)FTM_OM_SNMPTRAPD_stop,
		.fSetCallback=	(FTM_OM_SERVICE_SET_CALLBACK)FTM_OM_SNMPTRAPD_setServiceCallback,
		.fCallback	=	FTM_OM_callback,
		.fLoadFromFile=	(FTM_OM_SERVICE_LOAD_FROM_FILE)FTM_OM_SNMPTRAPD_loadFromFile,
		.fShowConfig=	(FTM_OM_SERVICE_SHOW_CONFIG)FTM_OM_SNMPTRAPD_showConfig,
		.pData		=	(FTM_VOID_PTR)&xSNMPTRAPD
	},
	{
		.xType		=	FTM_OM_SERVICE_DBM,
		.xID		=	FTM_OM_SERVICE_DBM,
		.pName		=	"DB Client",
		.fInit		=	(FTM_OM_SERVICE_INIT)FTM_OM_DMC_init,
		.fFinal		=	(FTM_OM_SERVICE_FINAL)FTM_OM_DMC_final,
		.fStart 	=	(FTM_OM_SERVICE_START)FTM_OM_DMC_start,
		.fStop		=	(FTM_OM_SERVICE_STOP)FTM_OM_DMC_stop,
		.fSetCallback=	(FTM_OM_SERVICE_SET_CALLBACK)FTM_OM_DMC_setServiceCallback,
		.fCallback	=	FTM_OM_callback,
		.fLoadFromFile=	(FTM_OM_SERVICE_LOAD_FROM_FILE)FTM_OM_DMC_loadFromFile,
		.fShowConfig=	(FTM_OM_SERVICE_SHOW_CONFIG)FTM_OM_DMC_showConfig,
		.fNotify	=	(FTM_OM_SERVICE_NOTIFY)FTM_OM_DMC_notify,
		.pData		=	(FTM_VOID_PTR)&xDMC
	},
	{
		.xType		=	FTM_OM_SERVICE_MQTT_CLIENT,
		.xID		=	FTM_OM_SERVICE_MQTT_CLIENT,
		.pName		=	"MQTT Client",
		.fInit		=	(FTM_OM_SERVICE_INIT)FTM_MQTT_CLIENT_init,
		.fFinal		=	(FTM_OM_SERVICE_FINAL)FTM_MQTT_CLIENT_final,
		.fStart 	=	(FTM_OM_SERVICE_START)FTM_MQTT_CLIENT_start,
		.fStop		=	(FTM_OM_SERVICE_STOP)FTM_MQTT_CLIENT_stop,
		.fSetCallback=	(FTM_OM_SERVICE_SET_CALLBACK)FTM_MQTT_CLIENT_setCallback,
		.fCallback	=	FTM_OM_callback,
		.fLoadFromFile=	(FTM_OM_SERVICE_LOAD_FROM_FILE)FTM_MQTT_CLIENT_loadFromFile,
		.fShowConfig=	(FTM_OM_SERVICE_SHOW_CONFIG)FTM_MQTT_CLIENT_showConfig,
		.fNotify	=	(FTM_OM_SERVICE_NOTIFY)FTM_MQTT_CLIENT_notify,
		.pData		= 	(FTM_VOID_PTR)&xMQTTC
	},
	{
		.xType		=	FTM_OM_SERVICE_SHELL,
		.xID		=	FTM_OM_SERVICE_SHELL,
		.pName		=	"Shell",
		.fInit		=	(FTM_OM_SERVICE_INIT)FTM_OM_SHELL_init,
		.fFinal		=	(FTM_OM_SERVICE_FINAL)FTM_OM_SHELL_final,
		.fStart 	=	(FTM_OM_SERVICE_START)FTM_OM_SHELL_start,
		.fStop		=	(FTM_OM_SERVICE_STOP)FTM_OM_SHELL_stop,
		.fSetCallback=	NULL,
		.fCallback	=	NULL,
		.fLoadFromFile=	(FTM_OM_SERVICE_LOAD_FROM_FILE)FTM_OM_SHELL_loadFromFile,
		.fShowConfig=	(FTM_OM_SERVICE_SHOW_CONFIG)FTM_OM_SHELL_showConfig,
		.fNotify	=	(FTM_OM_SERVICE_NOTIFY)FTM_OM_SHELL_notify,
		.pData		= 	(FTM_VOID_PTR)&xShell
	},
};

FTM_RET	FTM_OM_create(FTM_OM_PTR _PTR_ ppOM)
{
	ASSERT(ppOM != NULL);
	
	FTM_RET				xRet;
	FTM_OM_PTR	pOM;

	pOM = (FTM_OM_PTR)FTM_MEM_malloc(sizeof(FTM_OM));
	if(pOM == NULL)
	{
		ERROR("Not enough memory");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTM_OM_init(pOM);
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

FTM_RET	FTM_OM_destroy(FTM_OM_PTR _PTR_ ppOM)
{
	ASSERT(ppOM != NULL);
	
	if (*ppOM == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	FTM_OM_final(*ppOM);

	*ppOM = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_init(FTM_OM_PTR pOM)
{
	FTM_RET	xRet;

	memset(pOM, 0, sizeof(FTM_OM));

	FTM_OM_getDefaultDeviceID(pOM->xConfig.pDID);
	TRACE("DID : %s\n", pOM->xConfig.pDID);

	xRet = FTM_OM_EPM_create(pOM, &pOM->pEPM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't create EPM\n");
		return	xRet;	
	}

	xRet = FTM_OM_MSGQ_create(&pOM->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't create Message Queue\n");
		FTM_OM_EPM_destroy(&pOM->pEPM);
		return	xRet;	
	}

	FTM_OM_NODE_init();
	FTM_OM_EP_CLASS_init();

	FTM_OM_SERVICE_init(pOM, pServices, sizeof(pServices) / sizeof(FTM_OM_SERVICE));

	FTM_OM_TRIGGERM_create(&pOM->pTriggerM);
	FTM_OM_ACTIONM_create(&pOM->pActionM);
	FTM_OM_RULEM_create(pOM, &pOM->pRuleM);
	FTM_OM_RULEM_setTriggerM(pOM->pRuleM, pOM->pTriggerM);
	FTM_OM_RULEM_setActionM(pOM->pRuleM, pOM->pActionM);

	TRACE("initialization done.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_final(FTM_OM_PTR pOM)
{
	FTM_OM_RULEM_destroy(&pOM->pRuleM);
	FTM_OM_ACTIONM_destroy(&pOM->pActionM);
	FTM_OM_TRIGGERM_destroy(&pOM->pTriggerM);

	FTM_OM_SERVICE_final();

	FTM_OM_EP_CLASS_final();
	FTM_OM_NODE_final();
	FTM_OM_MSGQ_destroy(&pOM->pMsgQ);
	FTM_OM_EPM_destroy(&pOM->pEPM);

	TRACE("finalization done.\n");

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_loadFromFile(FTM_OM_PTR pOM, FTM_CHAR_PTR pFileName)
{
	ASSERT(pOM != NULL);
	ASSERT(pFileName != NULL);

	FTM_OM_SERVICE_loadFromFile(FTM_OM_SERVICE_ALL, pFileName);

	TRACE("loaded configuration.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_showConfig(FTM_OM_PTR pOM)
{
	FTM_OM_SERVICE_showConfig(FTM_OM_SERVICE_ALL);

	return	FTM_RET_OK;
}

FTM_RET FTM_OM_start(FTM_OM_PTR pOM)
{
	if (pthread_create(&pOM->xThread, NULL, FTM_OM_process, (FTM_VOID_PTR)pOM) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET FTM_OM_stop(FTM_OM_PTR pOM)
{
	pOM->bStop = FTM_TRUE;
	pthread_join(pOM->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_RET FTM_OM_waitingForFinished(FTM_OM_PTR pOM)
{
	pthread_join(pOM->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTM_OM_process(FTM_VOID_PTR pData)
{
	ASSERT (pData != NULL);
	
	FTM_OM_PTR	pOM = (FTM_OM_PTR)pData;

	TRACE("started.\n");
	pOM->xState = FTM_OM_STATE_INITIALIZED;
	pOM->bStop	= FTM_FALSE;

	while(!pOM->bStop)
	{
		switch(pOM->xState)
		{
		case	FTM_OM_STATE_INITIALIZED:
			{
				FTM_OM_TASK_startService(pOM);
			}
			break;

		case	FTM_OM_STATE_CONNECTED:
			{
				FTM_OM_TASK_sync(pOM);
			}
			break;

		case	FTM_OM_STATE_SYNCHRONIZED:
			{
				FTM_OM_TASK_startEP(pOM);	
			}
			break;

		case	FTM_OM_STATE_PROCESSING:
			{
				FTM_OM_TASK_processing(pOM);	
			}
			break;

		case	FTM_OM_STATE_STOPED:
			{
				FTM_OM_TASK_stopService(pOM);	
			}
			break;

		case	FTM_OM_STATE_FINISHED:
			{
				pOM->bStop = FTM_TRUE;
			}
		}
	}

	TRACE("finished.\n");
	return	0;
}

FTM_RET	FTM_OM_TASK_startService(FTM_OM_PTR pOM)
{
	ASSERT(pOM != NULL);

	FTM_OM_SERVICE_start(FTM_OM_SERVICE_ALL);

	pOM->xState = FTM_OM_STATE_CONNECTED;

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_TASK_sync(FTM_OM_PTR pOM)
{
	ASSERT(pOM != NULL);

	FTM_RET			xRet;
	FTM_ULONG		ulCount, i;

	xRet = FTDMC_NODE_count(&xDMC.xSession, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}
	
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_NODE	xNodeInfo;
		FTM_OM_NODE_PTR	pNode;

		xRet = FTDMC_NODE_getAt(&xDMC.xSession, i, &xNodeInfo);	
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTDMC_NODE_getAt(%08lx, %d, &xNodeInfo) = %08lx\n",
					xDMC.xSession.hSock, i, xRet);
			continue;	
		}

		xRet = FTM_OM_NODE_create(&xNodeInfo, &pNode);
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTM_OM_NODE_create(xNode, &pNode) = %08lx\n", xRet);
			continue;	
		}

		TRACE("Node[%s] creating success.\n", pNode->xInfo.pDID);
	}

	xRet = FTDMC_EP_CLASS_count(&xDMC.xSession, &ulCount);
	if (xRet != FTM_RET_OK)
	{
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

		xRet = FTM_OM_EP_CLASS_create(&xEPClassInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTM_OM_EP_TYPE_append(&xEPClassInfo) = %08lx\n", xRet);
			continue;	
		}
	}

	xRet = FTDMC_EP_count(&xDMC.xSession, 0, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_OM_NODE_PTR	pNode;
		FTM_EP			xEPInfo;
		FTM_OM_EP_PTR		pEP;

		xRet = FTDMC_EP_getAt(&xDMC.xSession, i, &xEPInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTDMC_EP_getAt(%08lx, %d, &xEPInfo) = %08lx\n",
					xDMC.xSession.hSock, i, xRet);
			continue;
		}

		xRet = FTM_OM_EPM_createEP(pOM->pEPM, &xEPInfo, &pEP);
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTM_OM_EP_create(xEP, &pNode) = %08lx\n", xRet);
			continue;	
		}

		if (FTM_OM_NODE_get(xEPInfo.pDID, &pNode) == FTM_RET_OK)
		{
			FTM_OM_NODE_linkEP(pNode, pEP);
		}
		
		TRACE("EP[%08lx] creating success.\n", pEP->xInfo.xEPID);
	}

	xRet = FTDMC_TRIGGER_count(&xDMC.xSession, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_TRIGGER		xEvent;

		xRet = FTDMC_TRIGGER_getAt(&xDMC.xSession, i, &xEvent);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Event[%d] data load failed.\n", i);	
			continue;
		}

		xRet = FTM_OM_TRIGGERM_add(pOM->pTriggerM, &xEvent);
		if (xRet != FTM_RET_OK)
		{
			ERROR("The new event can not registration!\n") ;
			continue;
		}
	}

	xRet = FTDMC_ACTION_count(&xDMC.xSession, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_ACTION	xAction;

		xRet = FTDMC_ACTION_getAt(&xDMC.xSession, i, &xAction);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Action[%d] data load failed.\n", i);	
			continue;
		}

		xRet = FTM_OM_ACTIONM_add(pOM->pActionM, &xAction);
		if (xRet != FTM_RET_OK)
		{
			ERROR("The new action event can not registration!\n") ;
			continue;
		}
	}

	xRet = FTDMC_RULE_count(&xDMC.xSession, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_RULE	xRule;

		xRet = FTDMC_RULE_getAt(&xDMC.xSession, i, &xRule);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Rule[%d] data load failed.\n", i);	
			continue;
		}

		xRet = FTM_OM_RULEM_add(pOM->pRuleM, &xRule);
		if (xRet != FTM_RET_OK)
		{
			ERROR("The new action event can not registration!\n") ;
			continue;
		}
	}

	pOM->xState = FTM_OM_STATE_SYNCHRONIZED;
	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_TASK_startEP(FTM_OM_PTR pOM)
{
	ASSERT(pOM != NULL);

	FTM_OM_EP_PTR	pEP;
	FTM_ULONG	i, ulCount;
	
	FTM_OM_RULEM_start(pOM->pRuleM);
	FTM_OM_ACTIONM_start(pOM->pActionM);
	FTM_OM_TRIGGERM_start(pOM->pTriggerM);
	FTM_OM_EPM_count(pOM->pEPM, 0, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTM_OM_EPM_getAt(pOM->pEPM, i, &pEP) == FTM_RET_OK)
		{
			FTM_OM_EP_start(pEP);
		}
		else
		{
			ERROR("EP not found at %d\n", i);
		}
	}
	
	pOM->xState = FTM_OM_STATE_PROCESSING;
	return	FTM_RET_OK;
}

FTM_RET			FTM_OM_TASK_processing(FTM_OM_PTR pOM)
{
	ASSERT(pOM != NULL);

	FTM_RET			xRet;
	FTM_BOOL		bStop =  FTM_FALSE;
	FTM_OM_MSG_PTR	pCommonMsg = NULL;

	while(!bStop)
	{
		xRet = FTM_OM_MSGQ_timedPop(pOM->pMsgQ, 1000000, &pCommonMsg);
		if (xRet == FTM_RET_OK)
		{
			switch(pCommonMsg->xType)
			{
			case	FTM_OM_MSG_TYPE_QUIT:
				{
					TRACE("Task stop received.\n");
					bStop = FTM_TRUE;
					pOM->xState = FTM_OM_STATE_STOPED;
				}
				break;

			case	FTM_OM_MSG_TYPE_SAVE_EP_DATA:
				{
					FTM_OM_MSG_SAVE_EP_DATA_PTR	pMsg = (FTM_OM_MSG_SAVE_EP_DATA_PTR)pCommonMsg;

					TRACE("Save EP[%08x] Data.\n", pMsg->xEPID);
					FTM_OM_TRIGGERM_updateEP(pOM->pTriggerM, pMsg->xEPID, &pMsg->xData);
					FTM_OM_DMC_EP_DATA_set(&xDMC, pMsg->xEPID, &pMsg->xData);
				}
				break;

			case	FTM_OM_MSG_TYPE_SEND_EP_DATA:
				{
					xRet = FTM_OM_onSendEPData(pOM, (FTM_OM_MSG_SEND_EP_DATA_PTR)pCommonMsg);

				}
				break;

			case	FTM_OM_MSG_TYPE_TIME_SYNC:
				{
					xRet = FTM_OM_onTimeSync(pOM, (FTM_OM_MSG_TIME_SYNC_PTR)pCommonMsg);
				}
				break;

			case	FTM_OM_MSG_TYPE_EP_CTRL:
				{
					xRet = FTM_OM_onEPCtrl(pOM, (FTM_OM_MSG_EP_CTRL_PTR)pCommonMsg);
				}
				break;

			case	FTM_OM_MSG_TYPE_RULE:
				{
					FTM_OM_MSG_RULE_PTR	pMsg = (FTM_OM_MSG_RULE_PTR)pCommonMsg;

					TRACE("RULE[%d] is %s\n", pMsg->xRuleID, (pMsg->xRuleState == FTM_RULE_STATE_ACTIVATE)?"ACTIVATE":"DEACTIVATE");
				}
				break;

			default:
				{
					ERROR("Message[%08x] not supported.\n", pCommonMsg->xType);
				}
			}

			FTM_MEM_free(pCommonMsg);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_onTimeSync
(
	FTM_OM_PTR	pOM,
	FTM_OM_MSG_TIME_SYNC_PTR	pMsg
)
{
	ASSERT(pOM != NULL);
	ASSERT(pMsg != NULL);

	TRACE("Time Sync - %d\n", pMsg->ulTime);

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_onEPCtrl
(
	FTM_OM_PTR		pOM,
	FTM_OM_MSG_EP_CTRL_PTR	pMsg
)
{
	FTM_RET			xRet;
	FTM_OM_EP_PTR				pEP;
	FTM_EP_DATA					xData;

	TRACE("EP[%08x] Control\n", pMsg->xEPID);

	xRet = FTM_OM_EPM_get(pOM->pEPM, pMsg->xEPID, &pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP[%08x] not found.\n", pMsg->xEPID);
		return	xRet;
	}

	switch(pMsg->xCtrl)
	{
	case FTM_EP_CTRL_OFF:
		{
			xData.xType = FTM_EP_DATA_TYPE_INT;
			xData.xValue.nValue = 0;
		}
		break;

	case FTM_EP_CTRL_ON:
		{
			xData.xType = FTM_EP_DATA_TYPE_INT;
			xData.xValue.nValue = 1;
		}
		break;

	case FTM_EP_CTRL_BLINK:
		{
			xData.xType = FTM_EP_DATA_TYPE_INT;
			xData.xValue.nValue = 2;
		}
		break;
	}

	xRet = FTM_OM_EP_pushData(pEP, &xData);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP[%08x] set failed.\n", pMsg->xEPID);
	}

	return	xRet;
}

FTM_RET	FTM_OM_onSendEPData
(
	FTM_OM_PTR	pOM,
	FTM_OM_MSG_SEND_EP_DATA_PTR pMsg
)
{
	ASSERT(pOM != NULL);
	ASSERT(pMsg != NULL);

	return	FTM_MQTT_CLIENT_publishEPData(&xMQTTC, pMsg->xEPID, pMsg->pData, pMsg->ulCount);
}

FTM_RET	FTM_OM_TASK_stopService(FTM_OM_PTR pOM)
{
	ASSERT(pOM != NULL);

	FTM_OM_EP_PTR	pEP;
	FTM_ULONG	i, ulCount;
	
	FTM_OM_TRIGGERM_stop(pOM->pTriggerM);
	FTM_OM_ACTIONM_stop(pOM->pActionM);
	FTM_OM_RULEM_stop(pOM->pRuleM);
	FTM_OM_EPM_count(pOM->pEPM, 0, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTM_OM_EPM_getAt(pOM->pEPM, i, &pEP) == FTM_RET_OK)
		{
			TRACE("EP[%08x] stop request.\n", pEP->xInfo.xEPID);
			FTM_OM_EP_stop(pEP, FTM_TRUE);
			TRACE("EP[%08x] stop finished.\n", pEP->xInfo.xEPID);
		}
		else
		{
			ERROR("EP not found at %d\n", i);
		}
	}
	
	FTM_OM_SERVICE_stop(FTM_OM_SERVICE_ALL);
	
	pOM->xState = FTM_OM_STATE_FINISHED;

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_getDID(FTM_OM_PTR pOM, FTM_CHAR_PTR pBuff, FTM_ULONG ulBuffLen)
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

FTM_RET	FTM_OM_createEP(FTM_OM_PTR pOM, FTM_EP_PTR pInfo)
{
	ASSERT(pOM != NULL);
	ASSERT(pInfo != NULL);

	return	FTDMC_EP_append(&xDMC.xSession, pInfo);
}

FTM_RET	FTM_OM_destroyEP(FTM_OM_PTR pOM, FTM_EP_ID xEPID)
{
	ASSERT(pOM != NULL);

	return	FTDMC_EP_remove(&xDMC.xSession, xEPID);
}

FTM_RET	FTM_OM_getEPDataList(FTM_OM_PTR pOM, FTM_EP_ID xEPID, FTM_ULONG ulStart, FTM_EP_DATA_PTR pDataList, FTM_ULONG ulMaxCount, FTM_ULONG_PTR pulCount)
{
	ASSERT(pOM != NULL);
	ASSERT(pDataList != NULL);
	ASSERT(pulCount != NULL);

	return	FTDMC_EP_DATA_get(&xDMC.xSession, xEPID, ulStart, pDataList, ulMaxCount, pulCount);
}

FTM_RET	FTM_OM_getEPDataInfo(FTM_OM_PTR pOM, FTM_EP_ID xEPID, FTM_ULONG_PTR pulBeginTime, FTM_ULONG_PTR pulEndTime, FTM_ULONG_PTR pulCount)
{
	return	FTDMC_EP_DATA_info(&xDMC.xSession, xEPID, pulBeginTime, pulEndTime, pulCount);
}

FTM_RET	FTM_OM_getEPDataCount(FTM_OM_PTR pOM, FTM_EP_ID xEPID, FTM_ULONG_PTR pulCount)
{
	return	FTDMC_EP_DATA_count(&xDMC.xSession, xEPID, pulCount);
}

FTM_RET	FTM_OM_NOTIFY_quit(FTM_OM_PTR pOM)
{
	FTM_OM_stop(pOM);

	return	FTM_RET_SHELL_QUIT;
}

FTM_RET	FTM_OM_NOTIFY_rule
(
	FTM_OM_PTR 			pOM,
	FTM_OM_RULE_ID		xRuleID,
	FTM_OM_RULE_STATE	xRuleState
)
{
	ASSERT(pOM != NULL);

	FTM_RET			xRet;
	FTM_OM_MSG_RULE_PTR	pMsg;

	xRet = FTM_OM_MSG_createRule(xRuleID, xRuleState, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message creation failed.\n");
		return	xRet;
	}

	xRet = FTM_OM_MSGQ_push(pOM->pMsgQ, (FTM_OM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message send failed.\n");
		FTM_OM_MSG_destroy((FTM_OM_MSG_PTR _PTR_)&pMsg);		
		return	xRet;
	}

	return	xRet;
}

FTM_RET	FTM_OM_callback(FTM_OM_SERVICE_ID xID, FTM_OM_MSG_TYPE xMsg, FTM_VOID_PTR pData)
{
	switch(xID)
	{
	case	FTM_OM_SERVICE_SERVER:
		{
		}
		break;

	case	FTM_OM_SERVICE_SNMP_CLIENT:
		{
		}
		break;

	case	FTM_OM_SERVICE_SNMPTRAPD:
		{
		}
		break;

	case	FTM_OM_SERVICE_DBM:
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

FTM_RET	FTM_OM_setEPData
(
	FTM_OM_PTR 		pOM, 
	FTM_EP_ID 		xEPID, 
	FTM_EP_DATA_PTR pData
)
{
	ASSERT(pOM != NULL);
	ASSERT(pData != NULL);

	FTM_RET						xRet;
	FTM_OM_MSG_SET_EP_DATA_PTR	pMsg;

	xRet = FTM_OM_MSG_createSetEPData(xEPID, pData, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTM_OM_MSGQ_push(pOM->pMsgQ, (FTM_OM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
		FTM_OM_MSG_destroy((FTM_OM_MSG_PTR _PTR_)&pMsg);
		return	xRet;
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_saveEPData
(
	FTM_OM_PTR 		pOM, 
	FTM_EP_ID 		xEPID, 
	FTM_EP_DATA_PTR pData
)
{
	ASSERT(pOM != NULL);
	ASSERT(pData != NULL);

	FTM_RET						xRet;
	FTM_OM_MSG_SAVE_EP_DATA_PTR	pMsg;

	xRet = FTM_OM_MSG_createSaveEPData(xEPID, pData, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTM_OM_MSGQ_push(pOM->pMsgQ, (FTM_OM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
		FTM_OM_MSG_destroy((FTM_OM_MSG_PTR _PTR_)&pMsg);
		return	xRet;
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_sendEPData
(
	FTM_OM_PTR		pOM,
	FTM_EP_ID 		xEPID, 
	FTM_EP_DATA_PTR	pData,
	FTM_ULONG		ulCount
)
{
	ASSERT(pOM != NULL);
	ASSERT(pData != NULL);

	FTM_RET						xRet;
	FTM_OM_MSG_SEND_EP_DATA_PTR	pMsg;

	xRet = FTM_OM_MSG_createSendEPData(xEPID, pData, ulCount, &pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}


	xRet = FTM_OM_MSGQ_push(pOM->pMsgQ, (FTM_OM_MSG_PTR)pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
		FTM_OM_MSG_destroy((FTM_OM_MSG_PTR _PTR_)&pMsg);
		return	xRet;
	}

	return	FTM_RET_OK;
}

