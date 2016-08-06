#include <stdlib.h>
#include <errno.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "ftom.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"
#include "ftom_snmptrap.h"

#undef	__MODULE__
#define	__MODULE__	FTOM_TRACE_MODULE_SNMPC

FTM_VOID_PTR	FTOM_SNMPC_process(FTM_VOID_PTR pData);
static
FTM_RET	FTOM_SNMPC_dumpPDU
(
	netsnmp_pdu 	*pPDU
) ;

FTM_RET	FTOM_SNMPC_create
(
	FTOM_SNMPC_PTR _PTR_ 	ppSNMPC
)
{
	ASSERT(ppSNMPC != NULL);

	FTM_RET	xRet;
	FTOM_SNMPC_PTR	pSNMPC;

	pSNMPC = (FTOM_SNMPC_PTR)FTM_MEM_malloc(sizeof(FTOM_SNMPC));
	if (pSNMPC == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_SNMPC_init(pSNMPC);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pSNMPC);
		return	xRet;	
	}

	*ppSNMPC = pSNMPC;

	return	FTM_RET_OK;
}
	
FTM_RET	FTOM_SNMPC_destroy
(
	FTOM_SNMPC_PTR _PTR_ 	ppSNMPC
)
{
	ASSERT(ppSNMPC != NULL);

	FTOM_SNMPC_final(*ppSNMPC);

	FTM_MEM_free(*ppSNMPC);

	*ppSNMPC = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SNMPC_init
(
	FTOM_SNMPC_PTR pClient
)
{
	ASSERT(pClient != NULL);
	FTM_RET	xRet;
	
	static FTM_BOOL	bInit = FTM_FALSE;

	if (!bInit)
	{
		init_agent("ftom:snmpc");
		init_snmp("ftom:snmpc");

		bInit = FTM_TRUE;
	}

	memset(pClient, 0, sizeof(FTOM_SNMPC));

	xRet = FTOM_MSGQ_init(&pClient->xMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to init message queue.\n");
		goto finish;	
	}

	xRet = FTM_LOCK_init(&pClient->xLock);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to init lock.\n");
		goto finish;
	}

	xRet = FTM_LIST_init(&pClient->xConfig.xMIBList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to init list.\n");
		goto finish;
	}

	xRet = FTOM_SNMPTRAP_create(&pClient->pTrap);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create SNMP trap!\n");
		goto finish;
	}

	strcpy(pClient->xConfig.pName, FTOM_SNMPC_NAME);
	pClient->xConfig.ulLoopInterval	= FTOM_SNMPC_LOOP_INTERVAL;
	pClient->xConfig.ulMaxRetryCount= FTOM_SNMPC_RETRY_COUNT;

finish:
	return	xRet;
}

FTM_RET	FTOM_SNMPC_final
(
	FTOM_SNMPC_PTR pClient
)
{
	ASSERT(pClient != NULL);
	FTM_RET		xRet;
	FTM_ULONG 	i, ulCount;

	xRet = FTOM_SNMPTRAP_destroy(&pClient->pTrap);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to destroy SNMP trap!\n");
	}

	xRet = FTM_LOCK_final(&pClient->xLock);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to finalize lock!\n");	
	}

	FTM_LIST_count(&pClient->xConfig.xMIBList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_VOID_PTR pValue;

		if (FTM_LIST_getAt(&pClient->xConfig.xMIBList, i, &pValue) == FTM_RET_OK)
		{
			FTM_MEM_free(pValue);
		}
	}

	xRet = FTM_LIST_final(&pClient->xConfig.xMIBList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to finalize list!\n");
	}

	xRet = FTOM_MSGQ_final(&pClient->xMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to release message queue SNMP Client!\n");
	}

	return	FTM_RET_OK;
}

FTM_RET FTOM_SNMPC_start
(
	FTOM_SNMPC_PTR pClient
)
{
	ASSERT(pClient != NULL);

	FTM_INT	nRet;

	if (pClient->bStop)
	{
		return	FTM_RET_ALREADY_STARTED;	
	}

	if (pClient->pTrap != NULL)
	{
		FTOM_SNMPTRAP_start(pClient->pTrap);	
	}

	nRet = pthread_create(&pClient->xPThread, NULL, FTOM_SNMPC_process, pClient);
	if (nRet != 0)
	{
		switch(nRet)
		{
		case	EAGAIN: 
			{
				MESSAGE(" Insufficient resources to create another thread, or a system-imposed limit on the number of threads was encountered.\n"); 
			}
			break;

		case	EINVAL:	
			{
				MESSAGE("Invalid settings in attr.\n");
			}
 			break;

		case	EPERM:	
			{
				MESSAGE("No permission to set the scheduling policy and parameters specified in attr.\n"); 
			}
			break;
		}

		return	FTM_RET_THREAD_CREATION_ERROR;
	}

	return	FTM_RET_OK;
}


FTM_RET	FTOM_SNMPC_stop
(
	FTOM_SNMPC_PTR pClient
)
{
	ASSERT(pClient != NULL);
	
	FTM_VOID_PTR 	pRet = NULL;

	if (!pClient->bStop)
	{
		return	FTM_RET_NOT_START;	
	}

	if (pClient->pTrap != NULL)
	{
		FTOM_SNMPTRAP_stop(pClient->pTrap);	
	}

	pClient->bStop = FTM_TRUE;
	pthread_join(pClient->xPThread, &pRet);
	TRACE("SNMP client finished.\n");

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTOM_SNMPC_process
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTM_RET			xRet;
	FTOM_SNMPC_PTR	pClient = (FTOM_SNMPC_PTR)pData;
	FTOM_MSG_PTR	pBaseMsg;
	FTM_TIMER		xLoopTimer;

//	init_agent(pClient->xConfig.pName);
//	init_snmp(pClient->xConfig.pName);

	FTM_TIMER_initMS(&xLoopTimer, pClient->xConfig.ulLoopInterval);

	pClient->bStop = FTM_FALSE;

	TRACE("SNMP client started!\n");
	while (!pClient->bStop)
	{	
		FTM_ULONG	ulLoopInterval = 0;

		FTM_TIMER_remainMS(&xLoopTimer, &ulLoopInterval);

		while (!pClient->bStop && (FTOM_MSGQ_timedPop(&pClient->xMsgQ, ulLoopInterval, &pBaseMsg) == FTM_RET_OK))
		{
			switch(pBaseMsg->xType)
			{
			case	FTOM_MSG_TYPE_SNMPC_GET_EP_DATA:
				{
					FTOM_EP_PTR	pEP;
					FTM_VALUE	xValue;
					FTM_BOOL	bValid = FTM_TRUE;
					FTOM_MSG_SNMPC_GET_EP_DATA_PTR	pMsg = (FTOM_MSG_SNMPC_GET_EP_DATA_PTR)pBaseMsg;

					FTM_VALUE_init(&xValue, pMsg->xDataType);

					xRet = FTOM_SNMPC_get( pClient, pMsg->ulVersion, pMsg->pURL, pMsg->pCommunity, &pMsg->xOID, pMsg->ulTimeout, &xValue, &bValid);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet, "Failed to snmp get[%s:%s:%s:%s]!\n", pMsg->pDID, pMsg->pEPID, pMsg->pURL, FTM_SNMP_OID_print(&pMsg->xOID));
					}
					else
					{
						xRet = FTOM_EP_get(pMsg->pEPID, &pEP); 
						if (xRet == FTM_RET_OK) 
						{
							FTM_EP_DATA		xData;
							FTOM_MSG_PTR	pNewMsg; 

							xRet = FTM_EP_DATA_initVALUE(&xData, &xValue);	
							if (xRet != FTM_RET_OK)
							{
								ERROR2(xRet, "Failed to create EP data.\n");	
							}
							else
							{
								if (!bValid)
								{ 
									xData.xState = FTM_EP_DATA_STATE_INVALID;
								}

								xRet = FTOM_MSG_createEPData(pMsg->pEPID, &xData, 1, &pNewMsg);
								if (xRet != FTM_RET_OK) 
								{ 
									ERROR2(xRet, "Failed to create message!\n");	
								}
								else
								{
									xRet = FTOM_EP_sendMessage(pEP, pNewMsg);	
									if (xRet != FTM_RET_OK)
									{
										ERROR2(xRet, "Failed to send message!\n");	
										FTOM_MSG_destroy(&pNewMsg);
									}	
								}
							}

							FTM_EP_DATA_final(&xData);
						}
					}

					FTM_VALUE_final(&xValue);
				}
				break;

			case	FTOM_MSG_TYPE_SNMPC_SET_EP_DATA:
				{
					FTOM_MSG_SNMPC_SET_EP_DATA_PTR	pMsg = (FTOM_MSG_SNMPC_SET_EP_DATA_PTR)pBaseMsg;
					FTOM_MSG_PTR	pNewMsg; 

					xRet = FTOM_SNMPC_set( pClient, pMsg->ulVersion, pMsg->pURL, pMsg->pCommunity, &pMsg->xOID, pMsg->ulTimeout, &pMsg->xValue);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet, "Failed to snmp get!\n");	
						break;
					}

					xRet = FTOM_MSG_SNMPC_createGetEPData(
								pMsg->pDID, 
								pMsg->pEPID,
								pMsg->ulVersion,
								pMsg->pURL,
								pMsg->pCommunity,
								&pMsg->xOID,
								pMsg->ulTimeout,
								pMsg->xValue.xType,
								&pNewMsg);
					if (xRet != FTM_RET_OK)
					{
						WARN("Failed to get EP Data.\n");
						break;
					}

					xRet = FTOM_MSGQ_push(&pClient->xMsgQ, pNewMsg);
					if (xRet != FTM_RET_OK)
					{
						FTOM_MSG_destroy(&pNewMsg);	
						WARN("Failed to send message.\n");
					}
				}
				break;

			case	FTOM_MSG_TYPE_QUIT:
				{	
					pClient->bStop = FTM_TRUE;
				}
				break;

			default:
				{
					WARN("Invalid message[%08x]\n", pBaseMsg->xType);	
				}
			}

			FTOM_MSG_destroy(&pBaseMsg);

			FTM_TIMER_remainMS(&xLoopTimer, &ulLoopInterval);
		}

#if 1
		{
			FTM_INT	nFDS = 0, nBlock = 0;
			fd_set xReadFD;
			fd_set xWriteFD;
			fd_set xExceptFD;
			struct timeval xTimeout = {.tv_sec = 0, .tv_usec = 100000};

			FD_ZERO(&xReadFD);
			FD_ZERO(&xWriteFD);
			FD_ZERO(&xExceptFD);
			snmp_select_info(&nFDS, &xReadFD, &xTimeout, &nBlock);
			nFDS = select(nFDS, &xReadFD, &xWriteFD, &xExceptFD, &xTimeout);
			if (nFDS < 0) 
			{
				WARN("Failed to select file descripters!\n");
			}
			else if (nFDS > 0)
			{
				snmp_read(&xReadFD);
			}
			else
			{
				run_alarms();
			}
		}
#endif
		FTM_TIMER_addMS(&xLoopTimer, pClient->xConfig.ulLoopInterval);
	}
	
	TRACE("SNMP client stopped!\n");

	return	0;
}

FTM_RET	FTOM_SNMPC_setConfig
(
	FTOM_SNMPC_PTR 	pClient, 
	FTOM_SNMPC_CONFIG_PTR	pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	FTM_RET		xRet;
	FTM_ULONG	i, ulCount = 0;

	strcpy(pClient->xConfig.pName, pConfig->pName);

	FTM_LIST_count(&pClient->xConfig.xMIBList, &ulCount);
	FTM_LIST_init(&pConfig->xMIBList);

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_CHAR_PTR	pMIBFileName;
		FTM_CHAR_PTR	pBuff;

		xRet = FTM_LIST_getAt(&pConfig->xMIBList, i, (FTM_VOID_PTR _PTR_)&pMIBFileName);
		if (xRet != FTM_RET_OK)
		{
			continue;		
		}

		pBuff = (FTM_CHAR_PTR)FTM_MEM_malloc(strlen(pMIBFileName) + 1);
		if (pBuff == NULL)
		{
			continue;	
		}

		strcpy(pBuff, pMIBFileName);

		xRet = FTM_LIST_append(&pClient->xConfig.xMIBList, pBuff);
		if (xRet != FTM_RET_OK)
		{
			FTM_MEM_free(pBuff);
		}
	}

	pClient->xConfig.ulLoopInterval = pConfig->ulLoopInterval;
	pClient->xConfig.ulMaxRetryCount = pConfig->ulMaxRetryCount;

	return	FTM_RET_OK;
}

FTM_RET FTOM_SNMPC_loadConfig
(
	FTOM_SNMPC_PTR 	pClient, 
	FTM_CONFIG_PTR		pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_ITEM		xSection;	
	FTM_ULONG			ulValue;

	xRet = FTM_CONFIG_getItem(pConfig, "snmpc", &xSection);
	if (xRet == FTM_RET_OK)
	{
		FTM_CONFIG_ITEM	xArray;

		xRet = FTM_CONFIG_ITEM_getChildItem(&xSection, "mibs", &xArray);
		if (xRet == FTM_RET_OK)
		{
			FTM_ULONG	i, ulCount = 0;

			xRet = FTM_CONFIG_LIST_getItemCount(&xArray, &ulCount);
			for(i = 0 ; i < ulCount ; i++)
			{
				FTM_CONFIG_ITEM	xItem;
				
				xRet = FTM_CONFIG_LIST_getItemAt(&xArray, i, &xItem);
				if (xRet == FTM_RET_OK)
				{
					FTM_CHAR	pMIBFileName[1024];

					xRet = FTM_CONFIG_ITEM_getString(&xItem, pMIBFileName, sizeof(pMIBFileName) - 1);
					if (xRet == FTM_RET_OK)
					{
						FTM_CHAR_PTR pBuff = (FTM_CHAR_PTR)FTM_MEM_malloc(strlen(pMIBFileName)+1);
						if (pBuff != NULL)
						{
							strcpy(pBuff, pMIBFileName);
							FTM_LIST_append(&pClient->xConfig.xMIBList, pBuff);
							read_mib(pBuff);
						}
					}
				}
			}
		}

		xRet = FTM_CONFIG_ITEM_getItemULONG(&xSection, "retry_count", &ulValue);
		if (xRet == FTM_RET_OK)
		{
			pClient->xConfig.ulMaxRetryCount = ulValue;
		}

		xRet = FTM_CONFIG_ITEM_getItemULONG(&xSection, "loop_interval", &ulValue);
		if (xRet == FTM_RET_OK)
		{
			pClient->xConfig.ulLoopInterval = ulValue;
		}
	}

	if (pClient->pTrap != NULL)
	{
		xRet = FTOM_SNMPTRAP_loadConfig(pClient->pTrap, pConfig);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to load SNMP client configuration!\n");	
		}
	}

	return	FTM_RET_OK;
}

FTM_RET FTOM_SNMPC_loadConfigFromFile
(
	FTOM_SNMPC_PTR 	pClient, 
	FTM_CHAR_PTR 	pFileName
)
{
	ASSERT(pClient != NULL);
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_PTR		pConfig;

	xRet = FTM_CONFIG_create(pFileName, &pConfig, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load SNMP client configuration from file!\n");	
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	xRet = FTOM_SNMPC_loadConfig(pClient, pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load SNMP client configuration!\n");	
	}


	FTM_CONFIG_destroy(&pConfig);

	return	xRet;
}

FTM_RET FTOM_SNMPC_saveConfig
(
	FTOM_SNMPC_PTR 	pClient, 
	FTM_CONFIG_PTR	pConfig
)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_ITEM		xSection;	

	xRet = FTM_CONFIG_getItem(pConfig, "snmpc", &xSection);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_CONFIG_addItem(pConfig, "snmpc", &xSection);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}
	}

	FTM_CONFIG_ITEM	xArray;

	xRet = FTM_CONFIG_ITEM_getChildItem(&xSection, "mibs", &xArray);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_createChildList(&xSection, "mibs", &xArray);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;
		}
	}

	FTM_ULONG	i, ulCount = 0;

	xRet = FTM_CONFIG_LIST_getItemCount(&xArray, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_CONFIG_LIST_deleteItemAt(&xArray, i);
	}

	FTM_LIST_count(&pClient->xConfig.xMIBList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_CHAR_PTR	pMIBFileName = NULL;

		xRet = FTM_LIST_getAt(&pClient->xConfig.xMIBList, i, (FTM_VOID_PTR _PTR_)&pMIBFileName);
		if (xRet == FTM_RET_OK)
		{
			FTM_CONFIG_ITEM	xElement;
			FTM_CONFIG_LIST_addItemString(&xArray, 	pMIBFileName, &xElement);
		}
	}

	xRet = FTM_CONFIG_ITEM_setItemULONG(&xSection, "retry_count", pClient->xConfig.ulMaxRetryCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can't save retry count!\n");
	}

	return	FTM_RET_OK;
}

FTM_RET FTOM_SNMPC_showConfig
(
	FTOM_SNMPC_PTR pClient
)
{
	ASSERT(pClient != NULL);

	FTM_ULONG	ulCount;

	MESSAGE("\n[ SNMP CONFIGURATION ]\n");
	MESSAGE("%16s : %lu\n", "MAX RETRY COUNT", pClient->xConfig.ulMaxRetryCount);
	if (FTM_LIST_count(&pClient->xConfig.xMIBList, &ulCount) == FTM_RET_OK)
	{
		FTM_INT	i;

		MESSAGE("%16s : %lu\n", "MIBs", ulCount);
		for(i = 0 ; i < ulCount ; i++)
		{
			FTM_VOID_PTR	pValue;

			if (FTM_LIST_getAt(&pClient->xConfig.xMIBList, i, &pValue) == FTM_RET_OK)
			{
				MESSAGE("%16d - %s\n", i+1, (FTM_CHAR_PTR)pValue);
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SNMPC_setServiceCallback
(
	FTOM_SNMPC_PTR 	pClient, 
	FTOM_SERVICE_ID	xServiceID, 
	FTOM_SERVICE_CB	fServiceCB
)
{
	ASSERT(pClient != NULL);
	ASSERT(fServiceCB != NULL);

	pClient->xServiceID = xServiceID;
	pClient->fServiceCB = fServiceCB;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SNMPC_getEPCount
(
	FTOM_SNMPC_PTR 	pClient, 
	FTM_CHAR_PTR	pIP,	
	FTM_EP_TYPE		xType,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pIP != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET	xRet;
	FTM_ULONG	ulCount = 0;
	FTM_SNMP_OID	xOID = 
	{ 
		.pIDs = {1,3,6,1,4,1,42251,1,3,0,1,0},
		.nLen = 12
	};

	xOID.pIDs[9] = (xType >> 16);

	xRet = FTOM_SNMPC_getULONG(pClient, pIP, &xOID, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		*pulCount = ulCount;	
	}

	return	xRet;
}

FTM_RET	FTOM_SNMPC_getEPID
(
	FTOM_SNMPC_PTR	pClient,
	FTM_CHAR_PTR	pIP,
	FTM_EP_TYPE		xType,
	FTM_ULONG		ulIndex,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulMaxLen
)
{
	ASSERT(pClient != NULL);
	ASSERT(pIP != NULL);
	ASSERT(pEPID != NULL);

	FTM_SNMP_OID	xOID = 
	{ 
		.pIDs = {1,3,6,1,4,1,42251,1,3,0,2,1,1,0},
		.nLen = 14
	};

	xOID.pIDs[9] = (xType >> 16);
	xOID.pIDs[13] = ulIndex + 1;

	return	FTOM_SNMPC_getString(pClient, pIP, &xOID, pEPID, ulMaxLen);
}

FTM_RET	FTOM_SNMPC_getEPType
(
	FTOM_SNMPC_PTR	pClient,
	FTM_CHAR_PTR	pIP,
	FTM_EP_TYPE		xType,
	FTM_ULONG		ulIndex,
	FTM_EP_TYPE_PTR	pType,
	FTM_ULONG		ulMaxLen
)
{
	ASSERT(pClient != NULL);
	ASSERT(pIP != NULL);
	ASSERT(pType != NULL);
	
	FTM_RET		xRet;
	FTM_CHAR	pTypeString[64];

	FTM_SNMP_OID	xOID = 
	{ 
		.pIDs = {1,3,6,1,4,1,42251,1,3,0,2,1,2,0},
		.nLen = 14
	};

	xOID.pIDs[9] = (xType >> 16);
	xOID.pIDs[13] = ulIndex + 1;

	xRet = FTOM_SNMPC_getString(pClient, pIP, &xOID, pTypeString, sizeof(pTypeString));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTM_EP_strToType(pTypeString, pType);

	return	xRet;
}

FTM_RET	FTOM_SNMPC_getEPName
(
	FTOM_SNMPC_PTR	pClient,
	FTM_CHAR_PTR	pIP,
	FTM_EP_TYPE		xType,
	FTM_ULONG		ulIndex,
	FTM_CHAR_PTR	pBuff,
	FTM_ULONG		ulMaxLen
)
{
	ASSERT(pClient != NULL);
	ASSERT(pIP != NULL);
	ASSERT(pBuff != NULL);

	FTM_SNMP_OID	xOID = 
	{ 
		.pIDs = {1,3,6,1,4,1,42251,1,3,0,2,1,3,0},
		.nLen = 14
	};

	xOID.pIDs[9] = (xType >> 16);
	xOID.pIDs[13] = ulIndex + 1;

	return	FTOM_SNMPC_getString(pClient, pIP, &xOID, pBuff, ulMaxLen);
}

FTM_RET	FTOM_SNMPC_getEPUnit
(
	FTOM_SNMPC_PTR	pClient,
	FTM_CHAR_PTR	pIP,
	FTM_EP_TYPE		xType,
	FTM_ULONG		ulIndex,
	FTM_CHAR_PTR	pBuff,
	FTM_ULONG		ulMaxLen
)
{
	ASSERT(pClient != NULL);
	ASSERT(pIP != NULL);
	ASSERT(pBuff != NULL);

	FTM_SNMP_OID	xOID = 
	{ 
		.pIDs = {1,3,6,1,4,1,42251,1,3,0,2,1,4,0},
		.nLen = 14
	};

	xOID.pIDs[9] = (xType >> 16);
	xOID.pIDs[13] = ulIndex + 1;

	return	FTOM_SNMPC_getString(pClient, pIP, &xOID, pBuff, ulMaxLen);
}

FTM_RET	FTOM_SNMPC_getEPState
(
	FTOM_SNMPC_PTR 	pClient, 
	FTM_CHAR_PTR	pIP,	
	FTM_EP_TYPE		xType,
	FTM_ULONG		ulIndex,
	FTM_BOOL_PTR	pbEnable
)
{
	ASSERT(pClient != NULL);
	ASSERT(pIP != NULL);
	ASSERT(pbEnable != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pBuff[64];

	FTM_SNMP_OID	xOID = 
	{ 
		.pIDs = {1,3,6,1,4,1,42251,1,3,0,2,1, 5, 0},
		.nLen = 14
	};

	xOID.pIDs[9] = (xType >> 16);
	xOID.pIDs[13] = ulIndex + 1;

	xRet = FTOM_SNMPC_getString(pClient, pIP, &xOID, pBuff, sizeof(pBuff));
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (strcasecmp(pBuff, "run") == 0)
	{
		*pbEnable = FTM_TRUE;	
	}
	else if (strcasecmp(pBuff, "stop") == 0)
	{
		*pbEnable = FTM_FALSE;	
	}
	else
	{
		return	FTM_RET_INVALID_DATA;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SNMPC_getEPInterval
(
	FTOM_SNMPC_PTR	pClient,
	FTM_CHAR_PTR	pIP,
	FTM_EP_TYPE		xType,
	FTM_ULONG		ulIndex,
	FTM_ULONG_PTR	pulInterval
)
{
	ASSERT(pClient != NULL);
	ASSERT(pIP != NULL);
	ASSERT(pulInterval != NULL);

	FTM_SNMP_OID	xOID = 
	{ 
		.pIDs = {1,3,6,1,4,1,42251,1,3,0,2,1,9,0},
		.nLen = 14
	};

	xOID.pIDs[9] = (xType >> 16);
	xOID.pIDs[13] = ulIndex + 1;

	return	FTOM_SNMPC_getULONG(pClient, pIP, &xOID, pulInterval);
}

FTM_RET	FTOM_SNMPC_getOID
(
	FTM_CHAR_PTR 		pInput, 
	FTM_SNMP_OID_PTR	pOID
)
{
	pOID->nLen = FTM_SNMP_OID_LENGTH;
	if (read_objid(pInput, pOID->pIDs, &pOID->nLen) == 1)
	{
		return	FTM_RET_OK;
	}

	return	FTM_RET_SNMP_CANT_FIND_OID;

}

FTM_RET	FTOM_SNMPC_getULONG
(
	FTOM_SNMPC_PTR 		pClient, 
	FTM_CHAR_PTR		pIP,	
	FTM_SNMP_OID_PTR	pOID,
	FTM_ULONG_PTR		pulCount
)
{
	FTM_RET		xRet;
	FTM_VALUE	xValue;
	FTM_BOOL	bValid = FTM_TRUE;

	FTM_VALUE_init(&xValue, FTM_VALUE_TYPE_ULONG);

	xRet = FTOM_SNMPC_get( pClient, FTM_SNMP_VERSION_2, pIP, "public", pOID, 10, &xValue, &bValid);
	if (xRet == FTM_RET_OK)
	{
		FTM_VALUE_getULONG(&xValue, pulCount);
	}

	FTM_VALUE_final(&xValue);

	return	xRet;
}

FTM_RET	FTOM_SNMPC_getString
(
	FTOM_SNMPC_PTR 		pClient, 
	FTM_CHAR_PTR		pIP,	
	FTM_SNMP_OID_PTR	pOID,
	FTM_CHAR_PTR		pBuff,
	FTM_ULONG			ulMaxLen
)
{
	FTM_RET		xRet;
	FTM_VALUE	xValue;
	FTM_BOOL	bValid = FTM_TRUE;

	FTM_VALUE_init(&xValue, FTM_VALUE_TYPE_STRING);

	xRet = FTOM_SNMPC_get( pClient, FTM_SNMP_VERSION_2, pIP, "public", pOID, 1, &xValue, &bValid);
	if (xRet == FTM_RET_OK)
	{
		FTM_VALUE_getSTRING(&xValue, pBuff, ulMaxLen);
	}

	FTM_VALUE_final(&xValue);

	return	xRet;
}

FTM_RET	FTOM_SNMPC_sendMessage
(
	FTOM_SNMPC_PTR	pClient,
	FTOM_MSG_PTR	pMsg
)
{
	ASSERT(pClient != NULL);
	ASSERT(pMsg != NULL);

	FTOM_MSGQ_push(&pClient->xMsgQ, pMsg);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SNMPC_get
(
	FTOM_SNMPC_PTR		pClient,
	FTM_ULONG			ulVersion,
	FTM_CHAR_PTR		pURL,
	FTM_CHAR_PTR		pCommunity,
	FTM_SNMP_OID_PTR	pOID,
	FTM_ULONG			ulTimeout,
	FTM_VALUE_PTR		pValue,
	FTM_BOOL_PTR		pbValid
)
{
	ASSERT(pURL != NULL);
	ASSERT(pCommunity != NULL);
	ASSERT(pOID != NULL);
	ASSERT(pValue != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_INT	nRet;
	struct snmp_session	*pSession = NULL;
	struct snmp_session	xSession;
	netsnmp_pdu 	*pReqPDU = NULL;
	netsnmp_pdu		*pRespPDU = NULL; 

	FTM_LOCK_set(&pClient->xLock);

	snmp_sess_init(&xSession);			/* initialize session */

	xSession.version 		= ulVersion;
	xSession.peername 		= pURL;
	xSession.community 		= (FTM_UINT8_PTR)pCommunity;
	xSession.community_len	= strlen(pCommunity);


	pSession = snmp_open(&xSession);
	if (pSession == NULL)
	{
		xRet = FTM_RET_SNMP_CANT_OPEN_SESSION;
		ERROR2(xRet, "SNMP open error - %s\n", snmp_errstring(snmp_errno));
	}

	pReqPDU = snmp_pdu_create(SNMP_MSG_GET);	/* send the first GET */
	if (pReqPDU == NULL)
	{
		xRet = FTM_RET_SNMP_ERROR;
		ERROR2(xRet, "SNMP PDU creation error - %s\n", snmp_errstring(snmp_errno));

		goto finish;
	}

	pReqPDU->time = ulTimeout;
	snmp_add_null_var(pReqPDU, pOID->pIDs, pOID->nLen);

	nRet = snmp_synch_response(pSession, pReqPDU, &pRespPDU);
	if ((nRet != STAT_SUCCESS) || (pRespPDU->errstat != SNMP_ERR_NOERROR))
	{
		xRet = FTM_RET_SNMP_ERROR;
		ERROR2(xRet, "SNMP reponse error!\n");
		goto finish;
	}

	struct variable_list *pVariable = pRespPDU->variables;
	if (pVariable != NULL) 
	{
		switch(pVariable->type)
		{
		case	ASN_INTEGER:
			{
				FTM_INT	nValue;

				switch (pVariable->val_len)
				{
				case	1: 	nValue = *(FTM_UINT8_PTR)pVariable->val.integer; break;
				case	2:	nValue = *(FTM_UINT16_PTR)pVariable->val.integer; break;
				case	4:	nValue = *(FTM_UINT32_PTR)pVariable->val.integer; break;
				default: 	nValue = 0;
				}

				switch(pValue->xType)
				{
				case	FTM_VALUE_TYPE_UNKNOWN:	FTM_VALUE_initINT(pValue, nValue);	break;
				case	FTM_VALUE_TYPE_INT: 	FTM_VALUE_setINT(pValue, nValue);	break;
				case	FTM_VALUE_TYPE_ULONG: 	FTM_VALUE_setULONG(pValue, (FTM_ULONG)nValue);	break;
				case	FTM_VALUE_TYPE_BOOL: 	FTM_VALUE_setBOOL(pValue, (nValue != 0));	break;
				case	FTM_VALUE_TYPE_FLOAT: 	FTM_VALUE_setFLOAT(pValue, (FTM_FLOAT)nValue);	break;
				case	FTM_VALUE_TYPE_STRING: 	
					{
						FTM_CHAR	pBuff[32];
					
						memset(pBuff, 0, sizeof(pBuff));
						snprintf(pBuff, sizeof(pBuff) - 1, "%ld", *pVariable->val.integer);
						FTM_VALUE_setSTRING(pValue, pBuff);	
					}
					break;

				default:
					{
						WARN("Invalid value type[%d]!\n", pValue->xType);
						xRet = FTM_RET_INVALID_TYPE;
						*pbValid = FTM_FALSE;
					}
				}
			}
			break;

		case	ASN_OCTET_STR:
			{
				FTM_CHAR	pBuff[1024];

				if (pVariable->val_len < 1024)
				{
					memcpy(pBuff, pVariable->val.string, pVariable->val_len);
					pBuff[pVariable->val_len] = 0;
				}
				else
				{
					memcpy(pBuff, pVariable->val.string, 1023);
					pBuff[1023] = 0;
				}

				if (strcasecmp(pBuff, "N/A") != 0)
				{
					if (pValue->xType == FTM_VALUE_TYPE_UNKNOWN)
					{
						xRet = FTM_VALUE_initSTRING(pValue, pBuff);
					}
					else
					{
						xRet = FTM_VALUE_setFromString(pValue, pBuff);
					}

					*pbValid = (xRet == FTM_RET_OK);
				}
				else
				{
					*pbValid = FTM_FALSE;
				}
			}
			break;
		default:
			{

				ERROR2(FTM_RET_INVALID_TYPE, "Invalid data type[%d]!\n", pVariable->type);
				FTOM_SNMPC_dumpPDU(pRespPDU);
				xRet = FTM_RET_INVALID_TYPE;
				*pbValid = FTM_FALSE;
			}
		}
		
		pVariable = pVariable->next_variable;
	}

finish:
	if (pRespPDU != NULL)
	{
		snmp_free_pdu(pRespPDU);
	}

	if (pSession != NULL)
	{
		snmp_close(pSession);
	}

	FTM_LOCK_reset(&pClient->xLock);

	return	xRet;
}

FTM_RET	FTOM_SNMPC_set
(
	FTOM_SNMPC_PTR		pClient,
	FTM_ULONG			ulVersion,
	FTM_CHAR_PTR		pURL,
	FTM_CHAR_PTR		pCommunity,
	FTM_SNMP_OID_PTR	pOID,
	FTM_ULONG			ulTimeout,
	FTM_VALUE_PTR		pValue
)
{
	ASSERT(pURL != NULL);
	ASSERT(pCommunity != NULL);
	ASSERT(pOID != NULL);
	ASSERT(pValue != NULL);

	FTM_RET		xRet = FTM_RET_SNMP_ERROR;
	struct snmp_session	*pSession = NULL;
	struct snmp_session	xSession;

	snmp_sess_init(&xSession);			/* initialize session */

	xSession.version 		= ulVersion;
	xSession.peername 		= pURL;
	xSession.community 		= (FTM_UINT8_PTR)pCommunity;
	xSession.community_len	= strlen(pCommunity);

	pSession = snmp_open(&xSession);
	if (pSession != NULL)
	{
		netsnmp_pdu 	*pReqPDU = NULL;
		netsnmp_pdu		*pRespPDU = NULL; 

		pReqPDU = snmp_pdu_create(SNMP_MSG_SET);	/* send the first GET */
		if (pReqPDU == NULL)
		{
			xRet = FTM_RET_NOT_ENOUGH_MEMORY;
			ERROR2(xRet, "SNMP PDU creation error - %s\n", snmp_errstring(snmp_errno));
		}
		else
		{
			FTM_CHAR	pBuff[32];

			pReqPDU->time = ulTimeout;
			FTM_VALUE_snprint(pBuff, sizeof(pBuff), pValue);

			snmp_add_var(pReqPDU, pOID->pIDs, pOID->nLen, 's', pBuff);
		
			int nRet = snmp_synch_response(pSession, pReqPDU, &pRespPDU);
			TRACE("SNMP set request[%08x]\n", nRet);	
			if ((nRet == STAT_SUCCESS) && (pRespPDU->errstat == SNMP_ERR_NOERROR))
			{
				xRet = FTM_RET_OK;
			}
			else
			{
				xRet = FTM_RET_SNMP_NOT_SUPPORT_WRITE;
			}
		}

		if (pRespPDU != NULL)
		{
			snmp_free_pdu(pRespPDU);
		}

		snmp_close(pSession);

	}
	else
	{
		xRet = FTM_RET_SNMP_CANT_OPEN_SESSION;
		ERROR2(xRet, "Failed to open SNMP - %s\n", snmp_errstring(snmp_errno));
	}

	return	xRet;
}

FTM_RET	FTOM_SNMPC_dumpPDU
(
	netsnmp_pdu 	*pPDU
) 
{
    netsnmp_variable_list *vars;

	for(vars = pPDU->variables; vars ; vars = vars->next_variable)
	{
		int	i;
		FTM_CHAR	pBuff[1024];
		FTM_ULONG	ulLen = 0;

		memset(pBuff, 0, sizeof(pBuff));
		for(i = 0 ; i < vars->name_length ; i++)
		{
			ulLen += snprintf(&pBuff[ulLen], sizeof(pBuff) - ulLen, ".%lu", vars->name[i]);
		}
		TRACE("%6s : %s\n", "NAME", pBuff);

		switch(vars->type)
		{
		case	ASN_OBJECT_ID:
			{
				memset(pBuff, 0, sizeof(pBuff));
				snprint_objid(pBuff, sizeof(pBuff) - 1, vars->val.objid, vars->val_len / sizeof(oid));
			}
			break;

		case	ASN_OCTET_STR:
			{
				memcpy(pBuff, vars->val.string, vars->val_len);
				pBuff[vars->val_len] = 0;
			}
			break;
		}
		TRACE("%6s : %s\n", "VALUE", pBuff);
	}

	return	FTM_RET_OK;
}

