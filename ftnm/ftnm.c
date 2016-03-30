#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "ftnm.h"
#include "ftdm_client.h"
#include "ftnm_node.h"
#include "ftnm_snmpc.h"
#include "ftnm_ep.h"
#include "ftnm_ep_class.h"
#include "ftnm_service.h"
#include "ftnm_server.h"
#include "ftnm_dmc.h"
#include "ftm_mqtt_client.h"
#include "ftnm_msg.h"
#include "ftnm_utils.h"
#include "nxjson.h"

FTM_VOID_PTR	FTNM_process(FTM_VOID_PTR pData);
FTM_RET			FTNM_TASK_startService(FTNM_CONTEXT_PTR pCTX);
FTM_RET			FTNM_TASK_sync(FTNM_CONTEXT_PTR pCTX);
FTM_RET			FTNM_TASK_startEP(FTNM_CONTEXT_PTR pCTX);
FTM_RET			FTNM_TASK_processing(FTNM_CONTEXT_PTR pCTX);
FTM_RET			FTNM_TASK_stopService(FTNM_CONTEXT_PTR pCTX);

static 	FTM_RET	FTNM_callback(FTNM_SERVICE_ID xID, FTNM_MSG_TYPE xMsg, FTM_VOID_PTR pData);
//static	FTM_RET	FTNM_SNMPTrapCB(FTM_CHAR_PTR pTrapMsg);
static	FTNM_SERVER		xServer;
static	FTNM_SNMPC		xSNMPC;
static	FTNM_SNMPTRAPD	xSNMPTRAPD;
static	FTNM_DMC		xDMC;
static	FTM_MQTT_CLIENT	xMQTTC;

static 	FTNM_SERVICE	pServices[] =
{
	{
		.xType		=	FTNM_SERVICE_SERVER,
		.xID		=	FTNM_SERVICE_SERVER,
		.pName		=	"Server",
		.fInit		=	(FTNM_SERVICE_INIT)FTNM_SERVER_init,
		.fFinal		=	(FTNM_SERVICE_FINAL)FTNM_SERVER_final,
		.fStart 	=	(FTNM_SERVICE_START)FTNM_SERVER_start,
		.fStop		=	(FTNM_SERVICE_STOP)FTNM_SERVER_stop,
		.fSetCallback=	(FTNM_SERVICE_SET_CALLBACK)FTNM_SERVER_setServiceCallback,
		.fCallback	=	FTNM_callback,
		.fLoadFromFile=	(FTNM_SERVICE_LOAD_FROM_FILE)FTNM_SERVER_loadFromFile,
		.fShowConfig=	(FTNM_SERVICE_SHOW_CONFIG)FTNM_SERVER_showConfig,
		.fNotify	=	(FTNM_SERVICE_NOTIFY)FTNM_SERVER_notify,
		.pData		= 	(FTM_VOID_PTR)&xServer
	},
	{
		.xType		=	FTNM_SERVICE_SNMP_CLIENT,
		.xID		=	FTNM_SERVICE_SNMP_CLIENT,
		.pName		=	"SNMP Client",
		.fInit		=	(FTNM_SERVICE_INIT)FTNM_SNMPC_init,
		.fFinal		=	(FTNM_SERVICE_FINAL)FTNM_SNMPC_final,
		.fStart 	=	(FTNM_SERVICE_START)FTNM_SNMPC_start,
		.fStop		=	(FTNM_SERVICE_STOP)FTNM_SNMPC_stop,
		.fSetCallback=	(FTNM_SERVICE_SET_CALLBACK)FTNM_SNMPC_setServiceCallback,
		.fCallback	=	FTNM_callback,
		.fLoadFromFile=	(FTNM_SERVICE_LOAD_FROM_FILE)FTNM_SNMPC_loadFromFile,
		.fShowConfig=	(FTNM_SERVICE_SHOW_CONFIG)FTNM_SNMPC_showConfig,
		.pData		=	(FTM_VOID_PTR)&xSNMPC
	},
	{
		.xType		=	FTNM_SERVICE_SNMPTRAPD,
		.xID		=	FTNM_SERVICE_SNMPTRAPD,
		.pName		=	"SNMP TrapD",
		.fInit		=	(FTNM_SERVICE_INIT)FTNM_SNMPTRAPD_init,
		.fFinal		=	(FTNM_SERVICE_FINAL)FTNM_SNMPTRAPD_final,
		.fStart 	=	(FTNM_SERVICE_START)FTNM_SNMPTRAPD_start,
		.fStop		=	(FTNM_SERVICE_STOP)FTNM_SNMPTRAPD_stop,
		.fSetCallback=	(FTNM_SERVICE_SET_CALLBACK)FTNM_SNMPTRAPD_setServiceCallback,
		.fCallback	=	FTNM_callback,
		.fLoadFromFile=	(FTNM_SERVICE_LOAD_FROM_FILE)FTNM_SNMPTRAPD_loadFromFile,
		.fShowConfig=	(FTNM_SERVICE_SHOW_CONFIG)FTNM_SNMPTRAPD_showConfig,
		.pData		=	(FTM_VOID_PTR)&xSNMPTRAPD
	},
	{
		.xType		=	FTNM_SERVICE_DBM,
		.xID		=	FTNM_SERVICE_DBM,
		.pName		=	"DB Client",
		.fInit		=	(FTNM_SERVICE_INIT)FTNM_DMC_init,
		.fFinal		=	(FTNM_SERVICE_FINAL)FTNM_DMC_final,
		.fStart 	=	(FTNM_SERVICE_START)FTNM_DMC_start,
		.fStop		=	(FTNM_SERVICE_STOP)FTNM_DMC_stop,
		.fSetCallback=	(FTNM_SERVICE_SET_CALLBACK)FTNM_DMC_setServiceCallback,
		.fCallback	=	FTNM_callback,
		.fLoadFromFile=	(FTNM_SERVICE_LOAD_FROM_FILE)FTNM_DMC_loadFromFile,
		.fShowConfig=	(FTNM_SERVICE_SHOW_CONFIG)FTNM_DMC_showConfig,
		.fNotify	=	(FTNM_SERVICE_NOTIFY)FTNM_DMC_notify,
		.pData		=	(FTM_VOID_PTR)&xDMC
	},
	{
		.xType		=	FTNM_SERVICE_MQTT_CLIENT,
		.xID		=	FTNM_SERVICE_MQTT_CLIENT,
		.pName		=	"MQTT Client",
		.fInit		=	(FTNM_SERVICE_INIT)FTM_MQTT_CLIENT_init,
		.fFinal		=	(FTNM_SERVICE_FINAL)FTM_MQTT_CLIENT_final,
		.fStart 	=	(FTNM_SERVICE_START)FTM_MQTT_CLIENT_start,
		.fStop		=	(FTNM_SERVICE_STOP)FTM_MQTT_CLIENT_stop,
		.fSetCallback=	(FTNM_SERVICE_SET_CALLBACK)FTM_MQTT_CLIENT_setCallback,
		.fCallback	=	FTNM_callback,
		.fLoadFromFile=	(FTNM_SERVICE_LOAD_FROM_FILE)FTM_MQTT_CLIENT_loadFromFile,
		.fShowConfig=	(FTNM_SERVICE_SHOW_CONFIG)FTM_MQTT_CLIENT_showConfig,
		.fNotify	=	(FTNM_SERVICE_NOTIFY)FTM_MQTT_CLIENT_notify,
		.pData		= 	(FTM_VOID_PTR)&xMQTTC
	},
};

FTM_RET	FTNM_create(FTNM_CONTEXT_PTR _PTR_ ppCTX)
{
	ASSERT(ppCTX != NULL);
	
	FTM_RET				xRet;
	FTNM_CONTEXT_PTR	pCTX;

	pCTX = (FTNM_CONTEXT_PTR)FTM_MEM_malloc(sizeof(FTNM_CONTEXT));
	if(pCTX == NULL)
	{
		ERROR("Not enough memory");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTNM_init(pCTX);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pCTX);	
	}
	else
	{
		*ppCTX = pCTX;	
	}

	return	xRet;
}

FTM_RET	FTNM_destroy(FTNM_CONTEXT_PTR _PTR_ ppCTX)
{
	ASSERT(ppCTX != NULL);
	
	if (*ppCTX == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	FTNM_final(*ppCTX);

	*ppCTX = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_init(FTNM_CONTEXT_PTR pCTX)
{
	FTM_RET	xRet;

	memset(pCTX, 0, sizeof(FTNM_CONTEXT));

	FTNM_getDefaultDeviceID(pCTX->xConfig.pDID);
	TRACE("DID : %s\n", pCTX->xConfig.pDID);

	xRet = FTNM_EPM_create(pCTX, &pCTX->pEPM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't create EPM\n");
		return	xRet;	
	}

	xRet = FTNM_MSGQ_create(&pCTX->pMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't create Message Queue\n");
		FTNM_EPM_destroy(&pCTX->pEPM);
		return	xRet;	
	}

	FTNM_NODE_init();
	FTNM_EP_CLASS_init();

	FTNM_SERVICE_init(pCTX, pServices, sizeof(pServices) / sizeof(FTNM_SERVICE));

	FTNM_TRIGGERM_create(&pCTX->pTriggerM);
	FTNM_ACTIONM_create(&pCTX->pActionM);
	FTNM_RULEM_create(pCTX, &pCTX->pRuleM);
	FTNM_RULEM_setTriggerM(pCTX->pRuleM, pCTX->pTriggerM);
	FTNM_RULEM_setActionM(pCTX->pRuleM, pCTX->pActionM);

	TRACE("FTNM initialization done.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTNM_final(FTNM_CONTEXT_PTR pCTX)
{
	FTNM_RULEM_destroy(&pCTX->pRuleM);
	FTNM_ACTIONM_destroy(&pCTX->pActionM);
	FTNM_TRIGGERM_destroy(&pCTX->pTriggerM);

	FTNM_SERVICE_final();

	FTNM_EP_CLASS_final();
	FTNM_NODE_final();
	FTNM_MSGQ_destroy(&pCTX->pMsgQ);
	FTNM_EPM_destroy(&pCTX->pEPM);

	TRACE("FTNM finalization done.\n");

	return	FTM_RET_OK;
}

FTM_RET	FTNM_loadFromFile(FTNM_CONTEXT_PTR pCTX, FTM_CHAR_PTR pFileName)
{
	ASSERT(pCTX != NULL);
	ASSERT(pFileName != NULL);

	FTNM_SERVICE_loadFromFile(FTNM_SERVICE_ALL, pFileName);

	TRACE("FTNM was loaded configuration.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTNM_showConfig(FTNM_CONTEXT_PTR pCTX)
{
	FTNM_SERVICE_showConfig(FTNM_SERVICE_ALL);

	return	FTM_RET_OK;
}

FTM_RET FTNM_start(FTNM_CONTEXT_PTR pCTX)
{
	if (pthread_create(&pCTX->xThread, NULL, FTNM_process, (FTM_VOID_PTR)pCTX) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET FTNM_stop(FTNM_CONTEXT_PTR pCTX)
{
	FTNM_MSGQ_sendQuit(pCTX->pMsgQ);
	pthread_join(pCTX->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_RET FTNM_waitingForFinished(FTNM_CONTEXT_PTR pCTX)
{
	pthread_join(pCTX->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTNM_process(FTM_VOID_PTR pData)
{
	ASSERT (pData != NULL);
	
	FTNM_CONTEXT_PTR	pCTX = (FTNM_CONTEXT_PTR)pData;

	TRACE("FTNM started.\n");
	pCTX->xState = FTNM_STATE_INITIALIZED;
	pCTX->bStop	= FTM_FALSE;

	while(!pCTX->bStop)
	{
		switch(pCTX->xState)
		{
		case	FTNM_STATE_INITIALIZED:
			{
				FTNM_TASK_startService(pCTX);
			}
			break;

		case	FTNM_STATE_CONNECTED:
			{
				FTNM_TASK_sync(pCTX);
			}
			break;

		case	FTNM_STATE_SYNCHRONIZED:
			{
				FTNM_TASK_startEP(pCTX);	
			}
			break;

		case	FTNM_STATE_PROCESSING:
			{
				FTNM_TASK_processing(pCTX);	
			}
			break;

		case	FTNM_STATE_STOPED:
			{
				FTNM_TASK_stopService(pCTX);	
			}
			break;

		case	FTNM_STATE_FINISHED:
			{
				pCTX->bStop = FTM_TRUE;
			}
		}
	}

	TRACE("FTNM finished.\n");
	return	0;
}

FTM_RET	FTNM_TASK_startService(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTNM_SERVICE_start(FTNM_SERVICE_ALL);

	pCTX->xState = FTNM_STATE_CONNECTED;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_TASK_sync(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

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
		FTNM_NODE_PTR	pNode;

		xRet = FTDMC_NODE_getAt(&xDMC.xSession, i, &xNodeInfo);	
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTDMC_NODE_getAt(%08lx, %d, &xNodeInfo) = %08lx\n",
					xDMC.xSession.hSock, i, xRet);
			continue;	
		}

		xRet = FTNM_NODE_create(&xNodeInfo, &pNode);
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTNM_NODE_create(xNode, &pNode) = %08lx\n", xRet);
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

		xRet = FTNM_EP_CLASS_create(&xEPClassInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTNM_EP_TYPE_append(&xEPClassInfo) = %08lx\n", xRet);
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
		FTNM_NODE_PTR	pNode;
		FTM_EP			xEPInfo;
		FTNM_EP_PTR		pEP;

		xRet = FTDMC_EP_getAt(&xDMC.xSession, i, &xEPInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTDMC_EP_getAt(%08lx, %d, &xEPInfo) = %08lx\n",
					xDMC.xSession.hSock, i, xRet);
			continue;
		}

		xRet = FTNM_EPM_createEP(pCTX->pEPM, &xEPInfo, &pEP);
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTNM_EP_create(xEP, &pNode) = %08lx\n", xRet);
			continue;	
		}

		if (FTNM_NODE_get(xEPInfo.pDID, &pNode) == FTM_RET_OK)
		{
			FTNM_NODE_linkEP(pNode, pEP);
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

		xRet = FTNM_TRIGGERM_add(pCTX->pTriggerM, &xEvent);
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

		xRet = FTNM_ACTIONM_add(pCTX->pActionM, &xAction);
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

		xRet = FTNM_RULEM_add(pCTX->pRuleM, &xRule);
		if (xRet != FTM_RET_OK)
		{
			ERROR("The new action event can not registration!\n") ;
			continue;
		}
	}

	pCTX->xState = FTNM_STATE_SYNCHRONIZED;
	return	FTM_RET_OK;
}

FTM_RET	FTNM_TASK_startEP(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTNM_EP_PTR	pEP;
	FTM_ULONG	i, ulCount;
	
	FTNM_RULEM_start(pCTX->pRuleM);
	FTNM_ACTIONM_start(pCTX->pActionM);
	FTNM_TRIGGERM_start(pCTX->pTriggerM);
	FTNM_EPM_count(pCTX->pEPM, 0, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTNM_EPM_getAt(pCTX->pEPM, i, &pEP) == FTM_RET_OK)
		{
			FTNM_EP_start(pEP);
		}
		else
		{
			ERROR("EP not found at %d\n", i);
		}
	}
	
	pCTX->xState = FTNM_STATE_PROCESSING;
	return	FTM_RET_OK;
}

FTM_RET			FTNM_TASK_processing(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTM_RET			xRet;
	FTM_BOOL		bStop =  FTM_FALSE;
	FTNM_MSG_PTR	pMsg = NULL;

	while(!bStop)
	{
		xRet = FTNM_MSGQ_timedPop(pCTX->pMsgQ, 1000000, &pMsg);
		if (xRet == FTM_RET_OK)
		{
			switch(pMsg->xType)
			{
			case	FTNM_MSG_TYPE_QUIT:
				{
					TRACE("Task stop received.\n");
					bStop = FTM_TRUE;
					pCTX->xState = FTNM_STATE_STOPED;
				}
				break;

			case	FTNM_MSG_TYPE_SNMPTRAP:
				{
					FTM_EP_ID		xEPID = 0;
					FTNM_EP_PTR		pEP = NULL;
					FTM_EP_DATA		xData;

					TRACE("TRAP : %s\n", pMsg->xParams.xSNMPTrap.pString);
					const nx_json *pRoot, *pItem;

					pRoot = nx_json_parse_utf8(pMsg->xParams.xSNMPTrap.pString);
					if (pRoot == NULL)
					{
						ERROR("Invalid trap message[%s]\n", pMsg->xParams.xSNMPTrap.pString);
						break;	
					}

					pItem = nx_json_get(pRoot, "id");
					if (pItem != NULL)
					{
						xEPID = strtoul(pItem->text_value, 0, 16);

						xRet = FTNM_EPM_get(pCTX->pEPM, xEPID, &pEP);
						if (xRet == FTM_RET_OK)
						{
							FTNM_EP_getData(pEP, &xData);

							pItem = nx_json_get(pRoot, "value");
							if (pItem != NULL)
							{
								switch(pItem->type)
								{
								case	NX_JSON_STRING:
									{
										switch(xData.xType)
										{
										case	FTM_EP_DATA_TYPE_INT:
											{
												xData.xValue.nValue = strtol(pItem->text_value, NULL, 10);
											}
											break;

										case	FTM_EP_DATA_TYPE_ULONG:
											{
												xData.xValue.ulValue = strtoul(pItem->text_value, NULL, 10);
											}
											break;

										case	FTM_EP_DATA_TYPE_FLOAT:
											{
												xData.xValue.fValue = atof(pItem->text_value);
											}
											break;
										}
										
									}
									break;

								case 	NX_JSON_INTEGER:
								case	NX_JSON_BOOL:
									{
										xData.xValue.nValue = pItem->int_value;
									}
									break;

								case	NX_JSON_DOUBLE:
									{

										switch(xData.xType)
										{
										case	FTM_EP_DATA_TYPE_INT:
											{
												xData.xValue.nValue = (FTM_INT)pItem->dbl_value;
											}
											break;
	
										case	FTM_EP_DATA_TYPE_ULONG:
											{
												xData.xValue.ulValue = (FTM_ULONG)pItem->dbl_value;
											}
											break;

										case	FTM_EP_DATA_TYPE_FLOAT:
											{
												xData.xValue.fValue = pItem->dbl_value;
											}
											break;
										}
									}
									break;
					
								default:
									{
										ERROR("Invalid value type[%d].\n", pItem->type);
									}
									break;
								}

								pItem = nx_json_get(pRoot, "time");
								if (pItem != NULL)
								{
									xData.ulTime = (FTM_ULONG)pItem->int_value;
								}

								pItem = nx_json_get(pRoot, "state");
								if (pItem != NULL)
								{
									if (strcasecmp(pItem->text_value, "enable") == 0)
									{
										xData.xState = FTM_EP_STATE_RUN;
									}
									else if (strcasecmp(pItem->text_value, "disable") == 0)
									{
										xData.xState = FTM_EP_STATE_STOP;
									}
									else if (strcasecmp(pItem->text_value, "error") == 0)
									{
										xData.xState = FTM_EP_STATE_ERROR;
									}
								}
							}
							else
							{
								ERROR("TRAP : Value is not exist.\n");
							}
						}
						else
						{
								ERROR("Can't found EP[%08x]\n", xEPID);	
						}
					}
					else
					{
						ERROR("TRAP : ID is not exist.\n");
					}

					nx_json_free(pRoot);


					FTNM_EP_trap(pEP, &xData);
				}	
				break;

			case	FTNM_MSG_TYPE_MQTT_REQ:
				{
				}
				break;

			case	FTNM_MSG_TYPE_MQTT_REQ_TIME_SYNC:
				{
					FTNM_EP_PTR	pEP;
					FTM_EP_DATA	xData;

					TRACE("MQTT REQ CONTROL - EP[%08x]\n", pMsg->xParams.xMQTTReqControl.xEPID);

					xRet = FTNM_EPM_get(pCTX->pEPM, pMsg->xParams.xMQTTReqControl.xEPID, &pEP);
					if (xRet != FTM_RET_OK)
					{
						ERROR("EP[%08x] not found.\n", pMsg->xParams.xMQTTReqControl.xEPID);
						break;
					}

					switch(pMsg->xParams.xMQTTReqControl.xCmd)
					{
					case FTNM_MSG_MQTT_REQ_CONTROL_CMD_OFF:
						{
							xData.xType = FTM_EP_DATA_TYPE_INT;
							xData.xValue.nValue = 0;
						}
						break;

					case FTNM_MSG_MQTT_REQ_CONTROL_CMD_ON:
						{
							xData.xType = FTM_EP_DATA_TYPE_INT;
							xData.xValue.nValue = 1;
						}
						break;

					case FTNM_MSG_MQTT_REQ_CONTROL_CMD_BLINK:
						{
							xData.xType = FTM_EP_DATA_TYPE_INT;
							xData.xValue.nValue = 2;
						}
						break;
					}

					xRet = FTNM_EP_setData(pEP, &xData);
					if (xRet != FTM_RET_OK)
					{
						ERROR("EP[%08x] set failed.\n", pMsg->xParams.xMQTTReqControl.xEPID);
						break;	
					}

				}
				break;

			case	FTNM_MSG_TYPE_MQTT_REQ_CONTROL:
				{
					FTNM_EP_PTR	pEP;
					FTM_EP_DATA	xData;

					TRACE("MQTT REQ CONTROL - EP[%08x]\n", pMsg->xParams.xMQTTReqControl.xEPID);

					xRet = FTNM_EPM_get(pCTX->pEPM, pMsg->xParams.xMQTTReqControl.xEPID, &pEP);
					if (xRet != FTM_RET_OK)
					{
						ERROR("EP[%08x] not found.\n", pMsg->xParams.xMQTTReqControl.xEPID);
						break;
					}

					switch(pMsg->xParams.xMQTTReqControl.xCmd)
					{
					case FTNM_MSG_MQTT_REQ_CONTROL_CMD_OFF:
						{
							xData.xType = FTM_EP_DATA_TYPE_INT;
							xData.xValue.nValue = 0;
						}
						break;

					case FTNM_MSG_MQTT_REQ_CONTROL_CMD_ON:
						{
							xData.xType = FTM_EP_DATA_TYPE_INT;
							xData.xValue.nValue = 1;
						}
						break;

					case FTNM_MSG_MQTT_REQ_CONTROL_CMD_BLINK:
						{
							xData.xType = FTM_EP_DATA_TYPE_INT;
							xData.xValue.nValue = 2;
						}
						break;
					}

					xRet = FTNM_EP_setData(pEP, &xData);
					if (xRet != FTM_RET_OK)
					{
						ERROR("EP[%08x] set failed.\n", pMsg->xParams.xMQTTReqControl.xEPID);
						break;	
					}

				}
				break;

			case	FTNM_MSG_TYPE_EP_CHANGED:
				{
					FTNM_EP_PTR pEP;

					TRACE("EP[%08x] changed.\n", pMsg->xParams.xEPChanged.xEPID);
					xRet = FTNM_EPM_get(pCTX->pEPM, pMsg->xParams.xEPChanged.xEPID, &pEP);
					if (xRet != FTM_RET_OK)
					{
						ERROR("EP[%08x] not found.\n", pMsg->xParams.xEPChanged.xEPID);
					}
					else
					{
						FTNM_EP_updateData(pEP, &pMsg->xParams.xEPChanged.xData);
					}
				}
				break;

			case	FTNM_MSG_TYPE_EP_DATA_SAVE_TO_DB:
				{
					TRACE("DATA SAVE TO DB : %08x\n", pMsg->xParams.xEPDataUpdated.xEPID);
					FTNM_TRIGGERM_updateEP(pCTX->pTriggerM, pMsg->xParams.xEPDataUpdated.xEPID, &pMsg->xParams.xEPDataUpdated.xData);
					FTNM_DMC_EP_DATA_set(&xDMC, pMsg->xParams.xEPDataUpdated.xEPID, &pMsg->xParams.xEPDataUpdated.xData);
				}
				break;

			case	FTNM_MSG_TYPE_EP_DATA_UPDATED:
				{
					TRACE("DATA UPDATE : %08x.\n", pMsg->xParams.xEPDataUpdated.xEPID);
					FTNM_TRIGGERM_updateEP(pCTX->pTriggerM, pMsg->xParams.xEPDataUpdated.xEPID, &pMsg->xParams.xEPDataUpdated.xData);
					FTNM_SERVICE_notify(FTNM_SERVICE_ALL, pMsg);
				}
				break;

			case	FTNM_MSG_TYPE_EP_DATA_TRANS:
				{
					TRACE("DATA TRANSFER TO SERVER : %08x\n", pMsg->xParams.xEPDataTrans.xEPID);
					FTNM_SERVICE_notify(FTNM_SERVICE_MQTT_CLIENT, pMsg);
				}
				break;

			case	FTNM_MSG_TYPE_RULE:
				{
					TRACE("RULE[%d] is %s\n", pMsg->xParams.xRule.xRuleID, 
						(pMsg->xParams.xRule.xRuleState == FTM_RULE_STATE_ACTIVATE)?"ACTIVATE":"DEACTIVATE");
				}
				break;

			default:
				{
					ERROR("Message[%08x] not supported.\n", pMsg->xType);
				}
			}

			FTM_MEM_free(pMsg);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_TASK_stopService(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTNM_EP_PTR	pEP;
	FTM_ULONG	i, ulCount;
	
	FTNM_TRIGGERM_stop(pCTX->pTriggerM);
	FTNM_ACTIONM_stop(pCTX->pActionM);
	FTNM_RULEM_stop(pCTX->pRuleM);
	FTNM_EPM_count(pCTX->pEPM, 0, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTNM_EPM_getAt(pCTX->pEPM, i, &pEP) == FTM_RET_OK)
		{
			TRACE("EP[%08x] stop request.\n", pEP->xInfo.xEPID);
			FTNM_EP_stop(pEP, FTM_TRUE);
			TRACE("EP[%08x] stop finished.\n", pEP->xInfo.xEPID);
		}
		else
		{
			ERROR("EP not found at %d\n", i);
		}
	}
	
	FTNM_SERVICE_stop(FTNM_SERVICE_ALL);
	
	pCTX->xState = FTNM_STATE_FINISHED;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_getDID(FTNM_CONTEXT_PTR pCTX, FTM_CHAR_PTR pBuff, FTM_ULONG ulBuffLen)
{
	ASSERT(pBuff != NULL);

	if (ulBuffLen < strlen(pCTX->xConfig.pDID) + 1)
	{
		return	FTM_RET_BUFFER_TOO_SMALL;	
	}

	strcpy(pBuff, pCTX->xConfig.pDID);

	return	FTM_RET_OK;
}

/******************************************************************
 * EP management interface
 ******************************************************************/

FTM_RET	FTNM_createEP(FTNM_CONTEXT_PTR pCTX, FTM_EP_PTR pInfo)
{
	ASSERT(pCTX != NULL);
	ASSERT(pInfo != NULL);

	return	FTDMC_EP_append(&xDMC.xSession, pInfo);
}

FTM_RET	FTNM_destroyEP(FTNM_CONTEXT_PTR pCTX, FTM_EP_ID xEPID)
{
	ASSERT(pCTX != NULL);

	return	FTDMC_EP_remove(&xDMC.xSession, xEPID);
}

FTM_RET	FTNM_getEPDataList(FTNM_CONTEXT_PTR pCTX, FTM_EP_ID xEPID, FTM_ULONG ulStart, FTM_EP_DATA_PTR pDataList, FTM_ULONG ulMaxCount, FTM_ULONG_PTR pulCount)
{
	ASSERT(pCTX != NULL);
	ASSERT(pDataList != NULL);
	ASSERT(pulCount != NULL);

	return	FTDMC_EP_DATA_get(&xDMC.xSession, xEPID, ulStart, pDataList, ulMaxCount, pulCount);
}

FTM_RET	FTNM_getEPDataInfo(FTNM_CONTEXT_PTR pCTX, FTM_EP_ID xEPID, FTM_ULONG_PTR pulBeginTime, FTM_ULONG_PTR pulEndTime, FTM_ULONG_PTR pulCount)
{
	return	FTDMC_EP_DATA_info(&xDMC.xSession, xEPID, pulBeginTime, pulEndTime, pulCount);
}

FTM_RET	FTNM_getEPDataCount(FTNM_CONTEXT_PTR pCTX, FTM_EP_ID xEPID, FTM_ULONG_PTR pulCount)
{
	return	FTDMC_EP_DATA_count(&xDMC.xSession, xEPID, pulCount);
}

FTM_RET	FTNM_NOTIFY_quit(FTNM_CONTEXT_PTR pCTX)
{
	FTNM_stop(pCTX);

	return	FTM_RET_SHELL_QUIT;
}

FTM_RET	FTNM_callback(FTNM_SERVICE_ID xID, FTNM_MSG_TYPE xMsg, FTM_VOID_PTR pData)
{
	switch(xID)
	{
	case	FTNM_SERVICE_SERVER:
		{
		}
		break;

	case	FTNM_SERVICE_SNMP_CLIENT:
		{
		}
		break;

	case	FTNM_SERVICE_SNMPTRAPD:
		{
		}
		break;

	case	FTNM_SERVICE_DBM:
		{
			switch(xMsg)
			{
			case	FTNM_MSG_TYPE_DMC_CONNECTED:
				{
					TRACE("DMC connected!\n");
				}
				break;

			case	FTNM_MSG_TYPE_DMC_DISCONNECTED:
				{
				}
				break;

			default:
				{
					ERROR("Invalid service callback parameter!\n");	
				}
			}
		}
		break;

	default:
		{
			ERROR("Invalid service ID[%08x] received.\n");
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NOTIFY_SNMPTrap(FTNM_CONTEXT_PTR pCTX, FTM_CHAR_PTR pTrapMsg)
{
	ASSERT(pCTX != NULL);
	ASSERT(pTrapMsg != NULL);

	FTM_RET			xRet;
	FTNM_MSG_PTR 	pMsg;

	xRet = FTNM_MSG_create(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message creation failed.\n");
		return	xRet;
	}
	
	pMsg->xType = FTNM_MSG_TYPE_SNMPTRAP;
	strncpy(pMsg->xParams.xSNMPTrap.pString, pTrapMsg, sizeof(pMsg->xParams.xSNMPTrap.pString) - 1);

	xRet = FTNM_MSGQ_push(pCTX->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push failed.\n");
		FTNM_MSG_destroy(&pMsg);
	}

	return	xRet;
}

FTM_RET FTNM_NOTIFY_EPChanged(FTNM_CONTEXT_PTR pCTX, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData)
{
	ASSERT(pCTX != NULL);
	ASSERT(pData != NULL);

	FTM_RET			xRet;
	FTNM_MSG_PTR 	pMsg;

	xRet = FTNM_MSG_create(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message creation failed.\n");
		return	xRet;
	}
	
	pMsg->xType = FTNM_MSG_TYPE_EP_CHANGED;
	pMsg->xParams.xEPChanged.xEPID = xEPID;
	memcpy(&pMsg->xParams.xEPChanged.xData, pData, sizeof(FTM_EP_DATA));

	xRet = FTNM_MSGQ_push(pCTX->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push failed.\n");
		FTNM_MSG_destroy(&pMsg);
	}

	return	xRet;
}

FTM_RET	FTNM_NOTIFY_EPUpdated(FTNM_CONTEXT_PTR pCTX, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData)
{
	ASSERT(pCTX != NULL);
	ASSERT(pData != NULL);

	FTM_RET			xRet;
	FTNM_MSG_PTR	pMsg;

	xRet = FTNM_MSG_create(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pMsg->xType = FTNM_MSG_TYPE_EP_DATA_UPDATED;
	pMsg->xParams.xEPDataUpdated.xEPID = xEPID;
	memcpy(&pMsg->xParams.xEPDataUpdated.xData, pData, sizeof(FTM_EP_DATA));

	xRet = FTNM_MSGQ_push(pCTX->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
		FTNM_MSG_destroy(&pMsg);
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NOTIFY_EPDataSaveToDB(FTNM_CONTEXT_PTR pCTX, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData)
{
	ASSERT(pCTX != NULL);
	ASSERT(pData != NULL);

	FTM_RET			xRet;
	FTNM_MSG_PTR	pMsg;

	xRet = FTNM_MSG_create(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pMsg->xType = FTNM_MSG_TYPE_EP_DATA_SAVE_TO_DB;
	pMsg->xParams.xEPDataUpdated.xEPID = xEPID;
	memcpy(&pMsg->xParams.xEPDataUpdated.xData, pData, sizeof(FTM_EP_DATA));

	xRet = FTNM_MSGQ_push(pCTX->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
		FTNM_MSG_destroy(&pMsg);
		return	xRet;
	}
	
	return	FTM_RET_OK;
}

FTM_RET		FTNM_NOTIFY_EPDataTransINT
(
	FTNM_CONTEXT_PTR	pCTX,
	FTM_EP_ID 			xEPID, 
	FTM_INT				nValue,
	FTM_INT 			nAverage, 
	FTM_INT 			nCount, 
	FTM_INT 			nMax, 
	FTM_INT 			nMin
)
{
	ASSERT(pCTX != NULL);

	FTM_RET			xRet;
	FTNM_MSG_PTR	pMsg;

	xRet = FTNM_MSG_create(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pMsg->xType = FTNM_MSG_TYPE_EP_DATA_TRANS;
	pMsg->xParams.xEPDataTrans.xEPID = xEPID;
	pMsg->xParams.xEPDataTrans.nType = FTM_EP_DATA_TYPE_INT;
	pMsg->xParams.xEPDataTrans.xValue.xINT.nValue	= nValue;
	pMsg->xParams.xEPDataTrans.xValue.xINT.nAverage = nAverage;
	pMsg->xParams.xEPDataTrans.xValue.xINT.nCount 	= nCount;
	pMsg->xParams.xEPDataTrans.xValue.xINT.nMax 	= nMax;
	pMsg->xParams.xEPDataTrans.xValue.xINT.nMin 	= nMin;

	xRet = FTNM_MSGQ_push(pCTX->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
		FTNM_MSG_destroy(&pMsg);
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET		FTNM_NOTIFY_EPDataTransULONG
(
	FTNM_CONTEXT_PTR	pCTX,
	FTM_EP_ID 			xEPID, 
	FTM_ULONG 			ulValue, 
	FTM_ULONG 			ulAverage, 
	FTM_INT 			nCount, 
	FTM_ULONG 			ulMax, 
	FTM_ULONG 			ulMin
)
{
	ASSERT(pCTX != NULL);

	FTM_RET			xRet;
	FTNM_MSG_PTR	pMsg;

	xRet = FTNM_MSG_create(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pMsg->xType = FTNM_MSG_TYPE_EP_DATA_TRANS;
	pMsg->xParams.xEPDataTrans.xEPID = xEPID;
	pMsg->xParams.xEPDataTrans.nType = FTM_EP_DATA_TYPE_ULONG;
	pMsg->xParams.xEPDataTrans.xValue.xULONG.ulValue	= ulValue;
	pMsg->xParams.xEPDataTrans.xValue.xULONG.ulAverage 	= ulAverage;
	pMsg->xParams.xEPDataTrans.xValue.xULONG.nCount 	= nCount;
	pMsg->xParams.xEPDataTrans.xValue.xULONG.ulMax 		= ulMax;
	pMsg->xParams.xEPDataTrans.xValue.xULONG.ulMin 		= ulMin;

	xRet = FTNM_MSGQ_push(pCTX->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
		FTNM_MSG_destroy(&pMsg);
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET		FTNM_NOTIFY_EPDataTransFLOAT
(
	FTNM_CONTEXT_PTR	pCTX,
	FTM_EP_ID 			xEPID, 
	FTM_FLOAT 			fValue, 
	FTM_FLOAT 			fAverage, 
	FTM_INT 			nCount, 
	FTM_FLOAT 			fMax, 
	FTM_FLOAT 			fMin
)
{
	ASSERT(pCTX != NULL);

	FTM_RET			xRet;
	FTNM_MSG_PTR	pMsg;

	xRet = FTNM_MSG_create(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pMsg->xType = FTNM_MSG_TYPE_EP_DATA_TRANS;
	pMsg->xParams.xEPDataTrans.xEPID = xEPID;
	pMsg->xParams.xEPDataTrans.nType = FTM_EP_DATA_TYPE_FLOAT;
	pMsg->xParams.xEPDataTrans.xValue.xFLOAT.fValue		= fValue;
	pMsg->xParams.xEPDataTrans.xValue.xFLOAT.fAverage 	= fAverage;
	pMsg->xParams.xEPDataTrans.xValue.xFLOAT.nCount 	= nCount;
	pMsg->xParams.xEPDataTrans.xValue.xFLOAT.fMax 		= fMax;
	pMsg->xParams.xEPDataTrans.xValue.xFLOAT.fMin 		= fMin;

	xRet = FTNM_MSGQ_push(pCTX->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
		FTNM_MSG_destroy(&pMsg);
		return	xRet;
	}

	return	FTM_RET_OK;
}

FTM_RET		FTNM_NOTIFY_EPDataTransBOOL
(
	FTNM_CONTEXT_PTR	pCTX,
	FTNM_EPM_PTR 		pEPM, 
	FTM_EP_ID 			xEPID, 
	FTM_BOOL 			bValue
)
{
	ASSERT(pCTX != NULL);

	FTM_RET			xRet;
	FTNM_MSG_PTR	pMsg;

	xRet = FTNM_MSG_create(&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pMsg->xType = FTNM_MSG_TYPE_EP_DATA_TRANS;
	pMsg->xParams.xEPDataTrans.xEPID = xEPID;
	pMsg->xParams.xEPDataTrans.nType = FTM_EP_DATA_TYPE_BOOL;
	pMsg->xParams.xEPDataTrans.xValue.xBOOL.bValue	= bValue;

	xRet = FTNM_MSGQ_push(pCTX->pMsgQ, pMsg);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Message push error![%08x]\n", xRet);
		FTNM_MSG_destroy(&pMsg);
		return	xRet;
	}

	return	FTM_RET_OK;
}

