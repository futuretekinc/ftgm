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
#include "ftnm_server.h"
#include "ftnm_dmc.h"
#include "ftnm_msg.h"
#include "nxjson.h"

FTM_VOID_PTR	FTNM_task(FTM_VOID_PTR pData);
FTM_RET			FTNM_taskInit(FTNM_CONTEXT_PTR pCTX);
FTM_RET			FTNM_taskConnect(FTNM_CONTEXT_PTR pCTX);
FTM_RET			FTNM_taskSync(FTNM_CONTEXT_PTR pCTX);
FTM_RET			FTNM_taskRunChild(FTNM_CONTEXT_PTR pCTX);
FTM_RET			FTNM_taskWait(FTNM_CONTEXT_PTR pCTX);

static	FTM_RET	FTNM_SNMPTRAPCB(FTM_CHAR_PTR pTrapMsg);

FTNM_CONTEXT	xCTX;

FTM_RET	FTNM_init(void)
{
	FTNM_EP_init();
	FTNM_NODE_init();
	FTNM_EP_CLASS_INFO_init();
	FTNM_MSG_init();

	FTNM_DMC_init();

	FTNM_SRV_init();
	FTNM_SRV_create(&xCTX.pServer);

	FTNM_SNMPC_init();
	FTNM_SNMPC_create(&xCTX.pSNMPC);

	FTNM_SNMPTRAPD_init();
	FTNM_SNMPTRAPD_create(&xCTX.pSNMPTrapd);
	FTNM_SNMPTRAPD_setTrapCB(xCTX.pSNMPTrapd, FTNM_SNMPTRAPCB);

	TRACE("FTNM initialization done.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTNM_final(void)
{
	FTNM_SNMPTRAPD_destroy(xCTX.pSNMPTrapd);
	FTNM_SNMPTRAPD_final();

	FTNM_SNMPC_destroy(xCTX.pSNMPC);
	FTNM_SNMPC_final();

	FTNM_SRV_destroy(xCTX.pServer);
	FTNM_SRV_final();

	FTNM_DMC_final();

	FTNM_MSG_final();
	FTNM_EP_CLASS_INFO_final();
	FTNM_NODE_final();
	FTNM_EP_final();

	return	FTM_RET_OK;
}

FTM_RET	FTNM_loadConfig(FTM_CHAR_PTR pFileName)
{
	ASSERT(pFileName != NULL);

	FTNM_DMC_loadConfig(pFileName);
	FTNM_SRV_loadConfig(xCTX.pServer, pFileName);
	FTNM_SNMPC_loadConfig(xCTX.pSNMPC, pFileName);
	FTNM_SNMPTRAPD_loadConfig(xCTX.pSNMPTrapd, pFileName);

	TRACE("FTNM was loaded configuration.\n");
	return	FTM_RET_OK;
}

FTM_RET	FTNM_showConfig(void)
{
	FTNM_DMC_showConfig();
	FTNM_SRV_showConfig(xCTX.pServer);
	FTNM_SNMPC_showConfig(xCTX.pSNMPC);
	FTNM_SNMPTRAPD_showConfig(xCTX.pSNMPTrapd);

	return	FTM_RET_OK;
}

FTM_RET FTNM_run(void)
{
	if (pthread_create(&xCTX.xPThread, NULL, FTNM_task, (FTM_VOID_PTR)&xCTX) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET FTNM_waitingForFinished(void)
{
	pthread_join(xCTX.xPThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTNM_task(FTM_VOID_PTR pData)
{
	ASSERT (pData != NULL);
	
	FTNM_CONTEXT_PTR	pCTX = (FTNM_CONTEXT_PTR)pData;

	pCTX->xState = FTNM_STATE_CREATED;

	while(FTM_TRUE)
	{
		switch(pCTX->xState)
		{
		case	FTNM_STATE_CREATED:
			{
				FTNM_taskInit(pCTX);
			}
			break;

		case	FTNM_STATE_INITIALIZED:
			{
				FTNM_SRV_start(pCTX->pServer);
				FTNM_SNMPC_start(pCTX->pSNMPC);
				FTNM_SNMPTRAPD_start(pCTX->pSNMPTrapd);

				FTNM_taskConnect(pCTX);
			}
			break;

		case	FTNM_STATE_CONNECTED:
			{
				FTNM_taskSync(pCTX);
			}
			break;

		case	FTNM_STATE_SYNCHRONIZED:
			{
				FTNM_taskRunChild(pCTX);	
			}
			break;

		case	FTNM_STATE_PROCESS_FINISHED:
			{
				FTNM_taskWait(pCTX);	
			}
			break;
		}
	}

	return	0;
}

FTM_RET	FTNM_taskInit(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	xCTX.xState = FTNM_STATE_INITIALIZED;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_taskConnect(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTM_RET			xRet;

	xRet = FTNM_DMC_connect(&pCTX->xDMCSession);
	if (xRet != FTM_RET_OK)
	{
		usleep(1000000);
		return	xRet;	
	}

	xCTX.xState = FTNM_STATE_CONNECTED;

	return	xRet;
}

FTM_RET	FTNM_taskSync(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTM_RET			xRet;
	FTM_ULONG		ulCount, i;

	xRet = FTDMC_NODE_INFO_count(&pCTX->xDMCSession, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_NODE_INFO	xNodeInfo;
		FTNM_NODE_PTR	pNode;

		xRet = FTDMC_NODE_INFO_getAt(&pCTX->xDMCSession, i, &xNodeInfo);	
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTDMC_NODE_INFO_getAt(%08lx, %d, &xNodeInfo) = %08lx\n",
					pCTX->xDMCSession.hSock, i, xRet);
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

	xRet = FTDMC_EP_CLASS_INFO_count(&pCTX->xDMCSession, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_EP_CLASS_INFO	xEPClassInfo;

		xRet = FTDMC_EP_CLASS_INFO_getAt(&pCTX->xDMCSession, i, &xEPClassInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTDMC_EP_CLASS_INFO_getAt(%08lx, %d, &xEPInfo) = %08lx\n",
					pCTX->xDMCSession.hSock, i, xRet);
			continue;
		}

		xRet = FTNM_EP_CLASS_INFO_create(&xEPClassInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTNM_EP_CLASS_append(&xEPClassInfo) = %08lx\n", xRet);
			continue;	
		}
	}

	xRet = FTDMC_EP_INFO_count(&pCTX->xDMCSession, 0, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTNM_NODE_PTR	pNode;
		FTM_EP_INFO	xEPInfo;
		FTNM_EP_PTR	pEP;

		xRet = FTDMC_EP_INFO_getAt(&pCTX->xDMCSession, i, &xEPInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("FTDMC_EP_INFO_getAt(%08lx, %d, &xEPInfo) = %08lx\n",
					pCTX->xDMCSession.hSock, i, xRet);
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

	xCTX.xState = FTNM_STATE_SYNCHRONIZED;
	return	FTM_RET_OK;
}

FTM_RET	FTNM_taskRunChild(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTNM_EP_PTR	pEP;
	FTM_ULONG	i, ulCount;

	FTNM_EP_count(0, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		if (FTNM_EP_getAt(i, &pEP) == FTM_RET_OK)
		{
			FTNM_EP_start(pEP);
		}
	}
	
	xCTX.xState = FTNM_STATE_PROCESS_FINISHED;
	return	FTM_RET_OK;
}

FTM_RET			FTNM_taskWait(FTNM_CONTEXT_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTM_RET			xRet;
	FTNM_MSG_PTR	pMsg = NULL;

	while(1)
	{
		xRet = FTNM_MSG_pop(&pMsg);
		if (xRet == FTM_RET_OK)
		{
			switch(pMsg->xType)
			{
			case	FTNM_MSG_TYPE_SNMPTRAP:
				{
					FTM_EPID		xEPID = 0;
					FTNM_EP_PTR		pEP = NULL;
					FTM_EP_DATA		xData;

					TRACE("TRAP : %s\n", pMsg->xParams.pString);
					const nx_json *pRoot, *pItem;

					pRoot = nx_json_parse_utf8(pMsg->xParams.pString);
					if (pRoot == NULL)
					{
						ERROR("Invalid trap message[%s]\n", pMsg->xParams.pString);
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
			}

			FTM_MEM_free(pMsg);
		}
		usleep(1000);
	}

	return	FTM_RET_OK;
}


FTM_RET	FTNM_setEPData(FTM_EPID xEPID, FTM_EP_DATA_PTR pData)
{
	return	FTNM_DMC_EP_DATA_set(&xCTX.xDMCSession, xEPID, pData);
}

FTM_RET	FTNM_getEPDataInfo(FTM_EPID xEPID, FTM_ULONG_PTR pulBeginTime, FTM_ULONG_PTR pulEndTime, FTM_ULONG_PTR pulCount)
{
	return	FTNM_DMC_EP_DATA_info(&xCTX.xDMCSession, xEPID, pulBeginTime, pulEndTime, pulCount);
}

FTM_RET	FTNM_getEPDataCount(FTM_EPID xEPID, FTM_ULONG_PTR pulCount)
{
	return	FTNM_DMC_EP_DATA_count(&xCTX.xDMCSession, xEPID, pulCount);
}

FTM_RET	FTNM_SNMPTRAPCB(FTM_CHAR_PTR pTrapMsg)
{
	return	FTNM_MSG_pushSNMPTRAP(pTrapMsg);
}
