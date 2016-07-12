#include <stdlib.h>
#include <errno.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "ftom.h"
#include "ftom_node_snmp_client.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"
#include "ftom_snmptrapd.h"


FTM_VOID_PTR	FTOM_SNMPC_process(FTM_VOID_PTR pData);
static
FTM_RET	FTOM_SNMPC_get
(
	FTM_ULONG			ulVersion,
	FTM_CHAR_PTR		pURL,
	FTM_CHAR_PTR		pCommunity,
	FTM_SNMP_OID_PTR	pOID,
	FTM_ULONG			ulTimeout,
	FTM_EP_DATA_PTR 	pData
);

static
FTM_RET	FTOM_SNMPC_set
(
	FTM_ULONG			ulVersion,
	FTM_CHAR_PTR		pURL,
	FTM_CHAR_PTR		pCommunity,
	FTM_SNMP_OID_PTR	pOID,
	FTM_ULONG			ulTimeout,
	FTM_EP_DATA_PTR 	pData
);

extern int	active_hosts;

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

	init_agent("ftom:snmpc");
	init_snmp("ftom:snmpc");

	memset(pClient, 0, sizeof(FTOM_SNMPC));

	xRet = FTOM_MSGQ_init(&pClient->xMsgQ);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to init message queue[%08x].\n", xRet);
		return	xRet;	
	}

	xRet = FTM_LOCK_init(&pClient->xLock);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to init lock[%08x].\n", xRet);
		return	xRet;
	}

	xRet = FTM_LIST_init(&pClient->xConfig.xMIBList);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to init list[%08x].\n", xRet);
		return	xRet;
	}

	strcpy(pClient->xConfig.pName, FTOM_SNMPC_NAME);
	pClient->xConfig.ulLoopInterval	= FTOM_SNMPC_LOOP_INTERVAL;
	pClient->xConfig.ulMaxRetryCount= FTOM_SNMPC_RETRY_COUNT;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SNMPC_final
(
	FTOM_SNMPC_PTR pClient
)
{
	ASSERT(pClient != NULL);
	FTM_RET		xRet;
	FTM_ULONG 	i, ulCount;

	FTM_LOCK_final(&pClient->xLock);
	FTM_LIST_count(&pClient->xConfig.xMIBList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_VOID_PTR pValue;

		if (FTM_LIST_getAt(&pClient->xConfig.xMIBList, i, &pValue) == FTM_RET_OK)
		{
			FTM_MEM_free(pValue);
		}
	}

	FTM_LIST_final(&pClient->xConfig.xMIBList);

	xRet = FTOM_MSGQ_final(&pClient->xMsgQ);
	if (xRet != FTM_RET_OK)
	{
		WARN("Failed to release message queue SNMP Client[%08x]!\n", xRet);
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
					FTM_EP_DATA	xData;
					FTOM_MSG_SNMPC_GET_EP_DATA_PTR	pMsg = (FTOM_MSG_SNMPC_GET_EP_DATA_PTR)pBaseMsg;

					xData.xType = pMsg->xDataType;
					xRet = FTOM_SNMPC_get( pMsg->ulVersion, pMsg->pURL, pMsg->pCommunity, &pMsg->xOID, pMsg->ulTimeout, &xData);
					if (xRet != FTM_RET_OK)
					{
						ERROR("Failed to snmp get[%s:%s:%s:%s]!\n", pMsg->pDID, pMsg->pEPID, pMsg->pURL, FTM_SNMP_OID_toStr(&pMsg->xOID));
						break;
					}

					xRet = FTOM_EP_get(pMsg->pEPID, &pEP); 
					if (xRet == FTM_RET_OK) 
					{
						FTOM_MSG_PTR	pNewMsg; 

						xRet = FTOM_MSG_EP_createInsertData(&xData, 1, &pNewMsg);
						if (xRet != FTM_RET_OK) 
						{ 
							ERROR("Failed to create message[%08x]!\n", xRet);	
						}
						else
						{
							xRet = FTOM_EP_sendMessage(pEP, pNewMsg);	
							if (xRet != FTM_RET_OK)
							{
								ERROR("Failed to send message[%08x]!\n", xRet);	
								FTOM_MSG_destroy(&pNewMsg);
							}	
						}
					}
				}
				break;

			case	FTOM_MSG_TYPE_SNMPC_SET_EP_DATA:
				{
					FTOM_EP_PTR	pEP;
					FTOM_MSG_SNMPC_SET_EP_DATA_PTR	pMsg = (FTOM_MSG_SNMPC_SET_EP_DATA_PTR)pBaseMsg;

					xRet = FTOM_SNMPC_set( pMsg->ulVersion, pMsg->pURL, pMsg->pCommunity, &pMsg->xOID, pMsg->ulTimeout, &pMsg->xData);
					if (xRet != FTM_RET_OK)
					{
						ERROR("Failed to snmp get!\n");	
						break;
					}

					xRet = FTOM_EP_get(pMsg->pEPID, &pEP); 
					if (xRet == FTM_RET_OK) 
					{
						FTOM_MSG_PTR	pNewMsg; 

						xRet = FTOM_MSG_EP_createInsertData(&pMsg->xData, 1, &pNewMsg);
						if (xRet != FTM_RET_OK) 
						{ 
							ERROR("Failed to create message[%08x]!\n", xRet);	
						}
						else
						{
							xRet = FTOM_EP_sendMessage(pEP, pNewMsg);	
							if (xRet != FTM_RET_OK)
							{
								ERROR("Failed to send message[%08x]!\n", xRet);	
								FTOM_MSG_destroy(&pNewMsg);
							}	
						}
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
		//if (active_hosts) 
		{
			FTM_INT	nFDS = 0, nBlock = 0;
			fd_set xFDSet;
			struct timeval xTimeout = {.tv_sec = 0, .tv_usec = 100000};

			FD_ZERO(&xFDSet);
			snmp_select_info(&nFDS, &xFDSet, &xTimeout, &nBlock);
			nFDS = select(nFDS, &xFDSet, NULL, NULL, &xTimeout);
			if (nFDS < 0) 
			{
				perror("select failed");
				exit(1);
			}
			else if (nFDS > 0)
			{
			//	snmp_read(&fdset);
			}
			else
			{
			//	snmp_timeout();
			}
		}
#endif
		FTM_TIMER_addMS(&xLoopTimer, pClient->xConfig.ulLoopInterval);
	}
	
	TRACE("SNMP client stopped!\n");

	return	0;
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
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	xRet = FTOM_SNMPC_loadConfig(pClient, pConfig);

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
		ERROR("Can't save retry count[%08x] !\n", xRet);
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
	FTOM_SNMPC_PTR 			pClient, 
	FTOM_SERVICE_ID 		xServiceID, 
	FTOM_SERVICE_CALLBACK 	fServiceCB
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

	FTM_SNMP_OID	xOID = 
	{ 
		.pIDs = {1,3,6,1,4,1,42251,1,3,0,1,0},
		.nLen = 12
	};

	xOID.pIDs[9] = (xType >> 16);

	return FTOM_SNMPC_getULONG(pClient, pIP, &xOID, pulCount);
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

FTM_RET	FTOM_SNMPC_getEPData
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);
	ASSERT(pData != NULL);

	FTM_RET				xRet;
	FTM_EP_DATA			xData;

	xRet = FTOM_EP_getDataType(pEP, &xData.xType);
	if (xRet != FTM_RET_OK)
	{
		TRACE("Failed to get EP data type!\n");
		return	xRet;
	}

	FTM_LOCK_set(pNode->pLock);
	FTM_LOCK_set(pEP->pLock);

	xRet = FTOM_SNMPC_get( 
				pNode->xCommon.xInfo.xOption.xSNMP.ulVersion,
				pNode->xCommon.xInfo.xOption.xSNMP.pURL,
				pNode->xCommon.xInfo.xOption.xSNMP.pCommunity,
				&pEP->xOption.xSNMP.xOID,
				pNode->xCommon.xInfo.ulTimeout,
				&xData);
	if (xRet == FTM_RET_OK)
	{
		memcpy(pData, &xData, sizeof(FTM_EP_DATA));	
	}

	FTM_LOCK_reset(pEP->pLock);
	FTM_LOCK_reset(pNode->pLock);

	return	xRet;
}

FTM_RET	FTOM_SNMPC_setEPData
(
	FTOM_NODE_SNMPC_PTR pNode, 
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);
	ASSERT(pData != NULL);

	FTM_RET	xRet;

	FTM_LOCK_set(pNode->pLock);
	FTM_LOCK_set(pEP->pLock);

	xRet = FTOM_SNMPC_set(
				pNode->xCommon.xInfo.xOption.xSNMP.ulVersion,
				pNode->xCommon.xInfo.xOption.xSNMP.pURL,
				pNode->xCommon.xInfo.xOption.xSNMP.pCommunity,
				&pEP->xOption.xSNMP.xOID,
				pNode->xCommon.xInfo.ulTimeout,
				pData);
			

	FTM_LOCK_reset(pEP->pLock);
	FTM_LOCK_reset(pNode->pLock);

	return	xRet;
}

FTM_RET	FTOM_SNMPC_getEPDataAsync
(
	FTOM_SNMPC_PTR 		pClient, 
	FTOM_NODE_SNMPC_PTR pNode, 
	FTOM_EP_PTR 		pEP 
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);
	
	FTM_RET				xRet;
	FTOM_MSG_PTR		pMsg = NULL;
	FTM_EP_DATA_TYPE	xDataType;

	xRet = FTOM_EP_getDataType(pEP, &xDataType);
	if (xRet != FTM_RET_OK)
	{
		TRACE("Error! invalid data type\n");
		return	xRet;
	}

	xRet = FTOM_MSG_SNMPC_createGetEPData(
				pNode->xCommon.xInfo.pDID, 
				pEP->xInfo.pEPID,
				pNode->xCommon.xInfo.xOption.xSNMP.ulVersion,
				pNode->xCommon.xInfo.xOption.xSNMP.pURL,
				pNode->xCommon.xInfo.xOption.xSNMP.pCommunity,
				&pEP->xOption.xSNMP.xOID,
				pNode->xCommon.xInfo.ulTimeout,
				xDataType,
				&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	xRet = FTOM_MSGQ_push(&pClient->xMsgQ, pMsg);

	return	xRet;
}

FTM_RET	FTOM_SNMPC_setEPDataAsync
(
	FTOM_SNMPC_PTR 		pClient, 
	FTOM_NODE_SNMPC_PTR pNode, 
	FTOM_EP_PTR 		pEP, 
	FTM_EP_DATA_PTR 	pData
)
{
	ASSERT(pNode != NULL);
	ASSERT(pEP != NULL);
	ASSERT(pData != NULL);

	FTM_RET			xRet;
	FTOM_MSG_PTR	pMsg = NULL;

	xRet = FTOM_MSG_SNMPC_createSetEPData(
				pNode->xCommon.xInfo.pDID, 
				pEP->xInfo.pEPID,
				pNode->xCommon.xInfo.xOption.xSNMP.ulVersion,
				pNode->xCommon.xInfo.xOption.xSNMP.pURL,
				pNode->xCommon.xInfo.xOption.xSNMP.pCommunity,
				&pEP->xOption.xSNMP.xOID,
				pNode->xCommon.xInfo.ulTimeout,
				pData,
				&pMsg);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	xRet = FTOM_MSGQ_push(&pClient->xMsgQ, pMsg);

	return	xRet;
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
	ASSERT(pClient != NULL);
	ASSERT(pIP != NULL);
	ASSERT(pOID != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_INT	nRet;
	struct snmp_session	*pSession = NULL;
	struct snmp_session	xSession;

	snmp_sess_init(&xSession);			/* initialize session */

	xSession.version 		= FTM_SNMP_VERSION_2;
	xSession.peername 		= pIP;
	xSession.community 		= (FTM_UINT8_PTR)"public";
	xSession.community_len	= 6;

	pSession = snmp_open(&xSession);
	if (pSession != NULL)
	{
		netsnmp_pdu 	*pReqPDU = NULL;
		netsnmp_pdu		*pRespPDU = NULL; 

		pReqPDU = snmp_pdu_create(SNMP_MSG_GET);	/* send the first GET */
		if (pReqPDU == NULL)
		{
			ERROR("SNMP PDU creation error - %s\n", snmp_errstring(snmp_errno));
			xRet = FTM_RET_SNMP_ERROR;
		}
		else
		{
			pReqPDU->time = 1;
			snmp_add_null_var(pReqPDU, pOID->pIDs, pOID->nLen);

			nRet = snmp_synch_response(pSession, pReqPDU, &pRespPDU);
			if ((nRet == STAT_SUCCESS) && (pRespPDU->errstat == SNMP_ERR_NOERROR))
			{
				struct variable_list *pVariable = pRespPDU->variables;
				if (pVariable != NULL)
				{
					switch (pVariable->val_len)
					{
					case	1: 	*pulCount = *(FTM_UINT8_PTR)pVariable->val.integer; break;
					case	2:	*pulCount = *(FTM_UINT16_PTR)pVariable->val.integer; break;
					case	4:	*pulCount = *(FTM_UINT32_PTR)pVariable->val.integer; break;
					default: 	*pulCount = 0;
					}
				}
				else
				{
					ERROR("SNMP synch respose error - %s\n", snmp_errstring(snmp_errno));
					xRet = FTM_RET_SNMP_ERROR;
				}
			}
			else
			{
				ERROR("SNMP synch respose error - %s\n", snmp_errstring(snmp_errno));
				xRet = FTM_RET_SNMP_ERROR;
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
		ERROR("SNMP open error - %s\n", snmp_errstring(snmp_errno));
		xRet = FTM_RET_SNMP_CANT_OPEN_SESSION;
	}

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
	ASSERT(pClient != NULL);
	ASSERT(pIP != NULL);
	ASSERT(pOID != NULL);
	ASSERT(pBuff != NULL);

	FTM_RET	xRet = FTM_RET_OK;
	FTM_INT	nRet;
	struct snmp_session	*pSession = NULL;
	struct snmp_session	xSession;

	snmp_sess_init(&xSession);			/* initialize session */

	xSession.version 		= FTM_SNMP_VERSION_2;
	xSession.peername 		= pIP;
	xSession.community 		= (FTM_UINT8_PTR)"public";
	xSession.community_len	= 6;

	pSession = snmp_open(&xSession);
	if (pSession != NULL)
	{
		netsnmp_pdu 	*pReqPDU = NULL;
		netsnmp_pdu		*pRespPDU = NULL; 

		pReqPDU = snmp_pdu_create(SNMP_MSG_GET);	/* send the first GET */
		if (pReqPDU == NULL)
		{
			ERROR("SNMP PDU creation error - %s\n", snmp_errstring(snmp_errno));
			xRet = FTM_RET_SNMP_ERROR;
		}
		else
		{
			pReqPDU->time = 1;
			snmp_add_null_var(pReqPDU, pOID->pIDs, pOID->nLen);

			nRet = snmp_synch_response(pSession, pReqPDU, &pRespPDU);
			if ((nRet == STAT_SUCCESS) && (pRespPDU->errstat == SNMP_ERR_NOERROR))
			{
				struct variable_list *pVariable = pRespPDU->variables;
				if (pVariable != NULL)
				{
					if (pVariable->val_len < ulMaxLen)
					{
						memcpy(pBuff, pVariable->val.string, pVariable->val_len);
					}
					else
					{
						memcpy(pBuff, pVariable->val.string, ulMaxLen);
					}
				}
				else
				{
					ERROR("SNMP synch respose error - %s\n", snmp_errstring(snmp_errno));
					xRet = FTM_RET_SNMP_ERROR;
				}
			}
			else
			{
				ERROR("SNMP synch respose error - %s\n", snmp_errstring(snmp_errno));
				xRet = FTM_RET_SNMP_ERROR;
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
		ERROR("SNMP open error - %s\n", snmp_errstring(snmp_errno));
		xRet = FTM_RET_SNMP_CANT_OPEN_SESSION;
	}

	return	xRet;
}

FTM_RET	FTOM_SNMPC_get
(
	FTM_ULONG			ulVersion,
	FTM_CHAR_PTR		pURL,
	FTM_CHAR_PTR		pCommunity,
	FTM_SNMP_OID_PTR	pOID,
	FTM_ULONG			ulTimeout,
	FTM_EP_DATA_PTR 	pData
)
{
	ASSERT(pURL != NULL);
	ASSERT(pCommunity != NULL);
	ASSERT(pOID != NULL);
	ASSERT(pData != NULL);

	FTM_RET	xRet = FTM_RET_SNMP_ERROR;
	FTM_INT	nRet;
	struct snmp_session	*pSession = NULL;
	struct snmp_session	xSession;
	netsnmp_pdu 	*pReqPDU = NULL;
	netsnmp_pdu		*pRespPDU = NULL; 

	snmp_sess_init(&xSession);			/* initialize session */

	xSession.version 		= ulVersion;
	xSession.peername 		= pURL;
	xSession.community 		= (FTM_UINT8_PTR)pCommunity;
	xSession.community_len	= strlen(pCommunity);

	pSession = snmp_open(&xSession);
	if (pSession == NULL)
	{
		ERROR("SNMP open error - %s\n", snmp_errstring(snmp_errno));
		xRet = FTM_RET_SNMP_CANT_OPEN_SESSION;
	}

	pReqPDU = snmp_pdu_create(SNMP_MSG_GET);	/* send the first GET */
	if (pReqPDU == NULL)
	{
		ERROR("SNMP PDU creation error - %s\n", snmp_errstring(snmp_errno));
		xRet = FTM_RET_SNMP_ERROR;

		goto finish;
	}

	pReqPDU->time = ulTimeout;
	snmp_add_null_var(pReqPDU, pOID->pIDs, pOID->nLen);

	nRet = snmp_synch_response(pSession, pReqPDU, &pRespPDU);
	if ((nRet != STAT_SUCCESS) || (pRespPDU->errstat != SNMP_ERR_NOERROR))
	{
		xRet = FTM_RET_SNMP_ERROR;
		goto finish;
	}

	struct variable_list *pVariable = pRespPDU->variables;
	if (pVariable != NULL) 
	{
		switch(pVariable->name[pVariable->name_length-2])
		{
		case	6:
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

				xRet = FTM_EP_DATA_setValueFromString(pData, pBuff);
			}
			break;
		}
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

	return	xRet;
}

FTM_RET	FTOM_SNMPC_set
(
	FTM_ULONG			ulVersion,
	FTM_CHAR_PTR		pURL,
	FTM_CHAR_PTR		pCommunity,
	FTM_SNMP_OID_PTR	pOID,
	FTM_ULONG			ulTimeout,
	FTM_EP_DATA_PTR 	pData
)
{
	ASSERT(pURL != NULL);
	ASSERT(pCommunity != NULL);
	ASSERT(pOID != NULL);
	ASSERT(pData != NULL);

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
			ERROR("SNMP PDU creation error - %s\n", snmp_errstring(snmp_errno));
			xRet = FTM_RET_SNMP_ERROR;
		}
		else
		{
			FTM_CHAR	pValue[32];

			pReqPDU->time = ulTimeout;
			FTM_EP_DATA_snprint(pValue, sizeof(pValue), pData);

			snmp_add_var(pReqPDU, pOID->pIDs, pOID->nLen, 's', pValue);
		
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
		ERROR("SNMP open error - %s\n", snmp_errstring(snmp_errno));
		xRet = FTM_RET_SNMP_CANT_OPEN_SESSION;
	}

	return	xRet;
}
