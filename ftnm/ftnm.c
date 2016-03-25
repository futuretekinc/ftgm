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
#include "ftnm_msg.h"
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
static  FTNM_MSG_QUEUE	xMsgQ;

		FTNM_TRIGGERM	xTriggerM;
		FTNM_ACTIONM	xActionM;
		FTNM_RULEM_PTR	pRuleM = NULL;
static 	FTNM_CONTEXT	xCTX;

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
		.pData		=	(FTM_VOID_PTR)&xDMC
	}
};

FTM_RET	FTNM_init(FTM_VOID)
{
	FTNM_EP_init();
	FTNM_NODE_init();
	FTNM_EP_CLASS_init();
	FTNM_MSGQ_init(&xMsgQ);

	FTNM_SERVICE_init(pServices, sizeof(pServices) / sizeof(FTNM_SERVICE));

	FTNM_TRIGGERM_init(&xTriggerM);
	FTNM_ACTIONM_init(&xActionM);
	FTNM_RULEM_create(&pRuleM);
	FTNM_RULEM_setTriggerM(pRuleM, &xTriggerM);
	FTNM_RULEM_setActionM(pRuleM, &xActionM);

	TRACE("FTNM initialization done.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTNM_final(FTM_VOID)
{
	FTNM_RULEM_destroy(&pRuleM);
	FTNM_ACTIONM_final(&xActionM);
	FTNM_TRIGGERM_final(&xTriggerM);

	FTNM_SERVICE_final();

	FTNM_MSGQ_final(&xMsgQ);
	FTNM_EP_CLASS_final();
	FTNM_NODE_final();
	FTNM_EP_final();

	TRACE("FTNM finalization done.\n");

	return	FTM_RET_OK;
}

FTM_RET	FTNM_loadFromFile(FTM_CHAR_PTR pFileName)
{
	ASSERT(pFileName != NULL);

	FTNM_SERVICE_loadFromFile(FTNM_SERVICE_ALL, pFileName);

	TRACE("FTNM was loaded configuration.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTNM_showConfig(FTM_VOID)
{
	FTNM_SERVICE_showConfig(FTNM_SERVICE_ALL);

	return	FTM_RET_OK;
}

FTM_RET FTNM_start(FTM_VOID)
{
	if (pthread_create(&xCTX.xThread, NULL, FTNM_process, (FTM_VOID_PTR)&xCTX) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET FTNM_stop(FTM_VOID)
{
	FTNM_MSGQ_sendQuit(&xMsgQ);
	pthread_join(xCTX.xThread, NULL);

	return	FTM_RET_OK;
}

FTM_RET FTNM_waitingForFinished(FTM_VOID)
{
	pthread_join(xCTX.xThread, NULL);

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

	xCTX.xState = FTNM_STATE_CONNECTED;

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

		xRet = FTNM_EP_create(&xEPInfo, &pEP);
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

		xRet = FTNM_TRIGGERM_add(&xTriggerM, &xEvent);
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

		xRet = FTNM_ACTIONM_add(&xActionM, &xAction);
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

		xRet = FTNM_RULEM_append(pRuleM, &xRule);
		if (xRet != FTM_RET_OK)
		{
			ERROR("The new action event can not registration!\n") ;
			continue;
		}
	}

	xCTX.xState = FTNM_STATE_SYNCHRONIZED;
	return	FTM_RET_OK;
}

FTM_RET	FTNM_TASK_startEP(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTNM_EP_PTR	pEP;
	FTM_ULONG	i, ulCount;
	
	FTNM_RULEM_start(pRuleM);
	FTNM_ACTIONM_start(&xActionM);
	FTNM_TRIGGERM_start(&xTriggerM);
	FTNM_EP_count(0, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTNM_EP_getAt(i, &pEP) == FTM_RET_OK)
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
		xRet = FTNM_MSGQ_timedPop(&xMsgQ, 1000000, &pMsg);
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

						xRet = FTNM_EP_get(xEPID, &pEP);
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

			case	FTNM_MSG_TYPE_EP_CHANGED:
				{
					xRet = FTNM_setEPData(pMsg->xParams.xEPChanged.xEPID, &pMsg->xParams.xEPChanged.xData);
					if (xRet != FTM_RET_OK)
					{
						ERROR("EP[%08x] data save failed.\n", pMsg->xParams.xEPChanged.xEPID);
					}
		
					FTNM_TRIGGERM_updateEP(&xTriggerM, pMsg->xParams.xEPChanged.xEPID, &pMsg->xParams.xEPChanged.xData);
					FTNM_SERVICE_notify(FTNM_SERVICE_SERVER, pMsg);

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
	
	FTNM_TRIGGERM_stop(&xTriggerM);
	FTNM_ACTIONM_stop(&xActionM);
	FTNM_RULEM_stop(pRuleM);
	FTNM_EP_count(0, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTNM_EP_getAt(i, &pEP) == FTM_RET_OK)
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

FTM_RET	FTNM_getDMC(FTNM_DMC_PTR _PTR_ ppDMC)
{
	ASSERT(ppDMC != NULL);

	*ppDMC = &xDMC;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_setEPData(FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData)
{
	FTNM_TRIGGERM_updateEP(&xTriggerM, xEPID, pData);
	FTNM_DMC_EP_DATA_set(&xDMC, xEPID, pData);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_getEPDataInfo(FTM_EP_ID xEPID, FTM_ULONG_PTR pulBeginTime, FTM_ULONG_PTR pulEndTime, FTM_ULONG_PTR pulCount)
{
	return	FTNM_DMC_EP_DATA_info(&xDMC, xEPID, pulBeginTime, pulEndTime, pulCount);
}

FTM_RET	FTNM_getEPDataCount(FTM_EP_ID xEPID, FTM_ULONG_PTR pulCount)
{
	return	FTNM_DMC_EP_DATA_count(&xDMC, xEPID, pulCount);
}

#if 0
FTM_RET	FTNM_SNMPTrapCB(FTM_CHAR_PTR pTrapMsg)
{
	return	FTNM_MSGQ_sendSNMPTrap(&xMsgQ, pTrapMsg);
}
#endif

FTM_RET	FTNM_NOTIFY_quit(FTM_VOID)
{
	FTNM_stop();

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
			switch(xMsg)
			{
			case	FTNM_MSG_TYPE_EP_CHANGED:
				{
					if (pData != NULL)
					{
						FTNM_MSG_EP_CHANGED_PARAMS_PTR pParam = (FTNM_MSG_EP_CHANGED_PARAMS_PTR)pData;

						FTNM_TRIGGERM_updateEP(&xTriggerM, pParam->xEPID, &pParam->xData);
						FTNM_DMC_EP_DATA_set(&xDMC, pParam->xEPID, &pParam->xData);
					}
					else
					{
						ERROR("Invalid service callback parameter!\n");	
					}
				}
				break;

			default:
				{
					ERROR("Invalid service callback parameter!\n");	
				}
			}
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
