#include <stdlib.h>
#include <errno.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "libconfig.h"
#include "ftom.h"
#include "ftom_node_snmpc.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"
#include "ftom_ep_management.h"
#include "ftom_snmptrapd.h"


FTM_VOID_PTR	FTOM_SNMPC_process(FTM_VOID_PTR pData);

extern int	active_hosts;

FTM_RET	FTOM_SNMPC_init
(
	FTOM_SNMPC_PTR pClient,
	FTOM_PTR pOM 
)
{
	ASSERT(pClient != NULL);

	init_agent("ftom:snmpc");
	init_snmp("ftom:snmpc");

	memset(pClient, 0, sizeof(FTOM_SNMPC));

	strcpy(pClient->xConfig.pName, FTOM_SNMPC_NAME);
	FTM_LIST_init(&pClient->xConfig.xMIBList);
	pClient->xConfig.ulMaxRetryCount = FTOM_SNMPC_RETRY_COUNT;
	FTM_LOCK_init(&pClient->xLock);
	pClient->pOM = pOM;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SNMPC_final
(
	FTOM_SNMPC_PTR pClient
)
{
	ASSERT(pClient != NULL);

	FTM_ULONG i, ulCount;

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

	FTOM_SNMPC_PTR	pClient = (FTOM_SNMPC_PTR)pData;
	pClient->bStop = FTM_FALSE;

	TRACE("SNMP client started!\n");
	while (!pClient->bStop)
	{	
		//if (active_hosts) 
		{
			FTM_INT	nFDS = 0, nBlock = 0;
			fd_set xFDSet;
			struct timeval xTimeout = {.tv_sec = 1, .tv_usec = 0};

			FD_ZERO(&xFDSet);
			snmp_select_info(&nFDS, &xFDSet, &xTimeout, &nBlock);
			nFDS = select(nFDS, &xFDSet, NULL, NULL, &xTimeout);
			if (nFDS < 0) 
			{
				perror("select failed");
				exit(1);
			}
			if (nFDS)
			{
			//	snmp_read(&fdset);
			}
			else
			{
			//	snmp_timeout();
			}
		}
		usleep(1000);
	}
	
	TRACE("SNMP client stopped!\n");

	return	0;
}

FTM_RET FTOM_SNMPC_loadFromFile
(
	FTOM_SNMPC_PTR 	pClient, 
	FTM_CHAR_PTR 	pFileName
)
{
	ASSERT(pClient != NULL);
	ASSERT(pFileName != NULL);

#if 0
	FTM_RET			xRet;
	FTM_CONFIG_PTR	pConfig;
	FTM_CONFIG_ITEM_PTR	pSection;

	xRet = FTM_CONFIG_create(pFileName, &pConfig);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTM_CONFIG_getItem(pConfig, "snmpc", &pSection);
	if (xRet == FTM_RET_OK)
	{
		FTM_CONFIG_ITEM_PTR	pMIBS;

		xRet = FTM_CONFIG_ITEM_getChildItem(pSection, "mibs", &pMIBS);
		if (xRet == FTM_RET_OK)
		{	
			FTM_ULONG	ulCount;
			xRet = FTM_CONFIG_LIST_getItemCount(pMIBS, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				for(i = 0 ; i < ulCount ; i++)
				{
					FTM_CONFIG_ITEM	xItem;

					xRet = FTM_CONFIG_LIST_getItemAt(pMIBS, i, &xItem);
					if (xRet == FTM_RET_OK)
					{}
				}
			
			}
		}
	}

#else
	config_t			xConfig;
	config_setting_t	*pSection;
	
	config_init(&xConfig);
	if (config_read_file(&xConfig, pFileName) == CONFIG_FALSE)
	{
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	pSection = config_lookup(&xConfig, "snmpc");
	if (pSection != NULL)
	{
		config_setting_t	*pField;

		pField = config_setting_get_member(pSection, "mibs");
		if (pField != NULL)
		{
			FTM_INT	i, nCount = config_setting_length(pField);
			
			for(i = 0 ; i < nCount ; i++)
			{
				const char * pMIBFileName = config_setting_get_string_elem(pField, i);	
				if (pMIBFileName != NULL)
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

		pField = config_setting_get_member(pSection, "retry_count");
		if (pField != NULL)
		{
			pClient->xConfig.ulMaxRetryCount = config_setting_get_int(pField);		
		}
		else
		{
			pClient->xConfig.ulMaxRetryCount = 1;
		}
	}
	config_destroy(&xConfig);
#endif
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
	MESSAGE("%16s : %d\n", "MAX RETRY COUNT", pClient->xConfig.ulMaxRetryCount);
	if (FTM_LIST_count(&pClient->xConfig.xMIBList, &ulCount) == FTM_RET_OK)
	{
		FTM_ULONG i;

		MESSAGE("%16s : %d\n", "MIBs", ulCount);
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
		.pOID = {1,3,6,1,4,1,42251,1,3,0,1,0},
		.ulOIDLen = 12
	};

	xOID.pOID[9] = (xType >> 16);

	return FTOM_SNMPC_getULONG(pClient, pIP, &xOID, pulCount);
}

FTM_RET	FTOM_SNMPC_getEPID
(
	FTOM_SNMPC_PTR	pClient,
	FTM_CHAR_PTR	pIP,
	FTM_EP_TYPE		xType,
	FTM_ULONG		ulIndex,
	FTM_EP_ID_PTR	pEPID
)
{
	ASSERT(pClient != NULL);
	ASSERT(pIP != NULL);
	ASSERT(pEPID != NULL);

	FTM_SNMP_OID	xOID = 
	{ 
		.pOID = {1,3,6,1,4,1,42251,1,3,0,2,1,1},
		.ulOIDLen = 13
	};

	xOID.pOID[9] = (xType >> 16);

	return	FTOM_SNMPC_getULONG(pClient, pIP, &xOID, (FTM_ULONG_PTR)pEPID);
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
		.pOID = {1,3,6,1,4,1,42251,1,3,0,2,1,3},
		.ulOIDLen = 13
	};

	xOID.pOID[9] = (xType >> 16);

	return	FTOM_SNMPC_getString(pClient, pIP, &xOID, pBuff, ulMaxLen);
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

	FTM_RET	xRet = FTM_RET_SNMP_ERROR;
	FTM_INT	nRet;
	struct snmp_session	*pSession = NULL;
	struct snmp_session	xSession;

	sem_wait(&pNode->xLock);
	sem_wait(&pEP->xLock);

	snmp_sess_init(&xSession);			/* initialize session */

	xSession.version 		= pNode->xCommon.xInfo.xOption.xSNMP.ulVersion;
	xSession.peername 		= pNode->xCommon.xInfo.xOption.xSNMP.pURL;
	xSession.community 		= (u_char *)pNode->xCommon.xInfo.xOption.xSNMP.pCommunity;
	xSession.community_len	= strlen(pNode->xCommon.xInfo.xOption.xSNMP.pCommunity);

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
			pReqPDU->time = pNode->xCommon.xInfo.ulTimeout;
			snmp_add_null_var(pReqPDU, pEP->xOption.xSNMP.pOID, pEP->xOption.xSNMP.nOIDLen);
			pNode->xStatistics.ulRequest++;

			nRet = snmp_synch_response(pSession, pReqPDU, &pRespPDU);
			if ((nRet == STAT_SUCCESS) && (pRespPDU->errstat == SNMP_ERR_NOERROR))
			{
				struct variable_list *pVariable = pRespPDU->variables;
				while (pVariable) 
				{
					switch(pVariable->name[pVariable->name_length-2])
					{
					case	6:
						{
							FTM_EP_DATA_TYPE	xDataType;
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

							xRet = FTOM_EP_getDataType(pEP, &xDataType);
							if (xRet != FTM_RET_OK)
							{
								break;
							}

							pData->ulTime = time(NULL);
							pData->xState = FTM_EP_DATA_STATE_VALID;
							pData->xType  = xDataType;
							switch(xDataType)
							{
							case	FTM_EP_DATA_TYPE_BOOL:
								{
									pData->xValue.bValue = (strtol(pBuff, NULL, 10) == 1);
								}
								break;

							case	FTM_EP_DATA_TYPE_INT:
								{
									pData->xValue.nValue = strtol(pBuff, NULL, 10);
								}
								break;

							case	FTM_EP_DATA_TYPE_ULONG:
								{
									pData->xValue.ulValue = strtoul(pBuff, NULL, 10);
								}
								break;

							case	FTM_EP_DATA_TYPE_FLOAT:
								{
									pData->xValue.fValue = strtod(pBuff, NULL);
								}
								break;
							}

							xRet = FTM_RET_OK;
						}
						break;
					};
	
					pVariable= pVariable->next_variable;
				}
		
				pNode->xStatistics.ulResponse++;
			}
			else
			{
				//ERROR("EP(%08x:%s) is occurred synch response error! - %s\n", pEP->xInfo.xEPID, pEP->pNode->xInfo.pDID, snmp_errstring(nRet));
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

	sem_post(&pEP->xLock);
	sem_post(&pNode->xLock);

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

	FTM_RET		xRet = FTM_RET_SNMP_ERROR;
	struct snmp_session	*pSession = NULL;
	struct snmp_session	xSession;

	sem_wait(&pNode->xLock);
	sem_wait(&pEP->xLock);

	snmp_sess_init(&xSession);			/* initialize session */

	xSession.version 		= pNode->xCommon.xInfo.xOption.xSNMP.ulVersion;
	xSession.peername 		= pNode->xCommon.xInfo.xOption.xSNMP.pURL;
	xSession.community 		= (u_char *)pNode->xCommon.xInfo.xOption.xSNMP.pCommunity;
	xSession.community_len	= strlen(pNode->xCommon.xInfo.xOption.xSNMP.pCommunity);

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

			pReqPDU->time = pNode->xCommon.xInfo.ulTimeout;
			FTM_EP_DATA_snprint(pValue, sizeof(pValue), pData);

			snmp_add_var(pReqPDU, pEP->xOption.xSNMP.pOID, pEP->xOption.xSNMP.nOIDLen, 's', pValue);
			pNode->xStatistics.ulRequest++;
		
			int nRet = snmp_synch_response(pSession, pReqPDU, &pRespPDU);
			TRACE("SNMP set request[%08x]\n", nRet);	
			if ((nRet == STAT_SUCCESS) && (pRespPDU->errstat == SNMP_ERR_NOERROR))
			{
				pNode->xStatistics.ulResponse++;
				xRet = FTM_RET_OK;
			}
			else
			{
				ERROR("EP[%08x] - %s\n", pEP->xInfo.xEPID, snmp_errstring(pRespPDU->errstat));
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

	sem_post(&pEP->xLock);
	sem_post(&pNode->xLock);

	return	xRet;
}

FTM_RET	FTOM_SNMPC_getOID
(
	FTM_CHAR_PTR 	pInput, 
	oid 			*pOID, 
	size_t			*pnOIDLen
)
{
	if (read_objid(pInput, pOID, pnOIDLen) == 1)
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

	FTM_RET	xRet = FTM_RET_SNMP_ERROR;
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
			pReqPDU->time = time(NULL);
			snmp_add_null_var(pReqPDU, pOID->pOID, pOID->ulOIDLen);

			nRet = snmp_synch_response(pSession, pReqPDU, &pRespPDU);
			if ((nRet == STAT_SUCCESS) && (pRespPDU->errstat == SNMP_ERR_NOERROR))
			{
				struct variable_list *pVariable = pRespPDU->variables;
				while (pVariable) 
				{
					*pulCount = *pVariable->val.integer;
					TRACE("Count : %d\n", *pVariable->val.integer);
					pVariable= pVariable->next_variable;
				}
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

	FTM_RET	xRet = FTM_RET_SNMP_ERROR;
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
			pReqPDU->time = time(NULL);
			snmp_add_null_var(pReqPDU, pOID->pOID, pOID->ulOIDLen);

			nRet = snmp_synch_response(pSession, pReqPDU, &pRespPDU);
			if ((nRet == STAT_SUCCESS) && (pRespPDU->errstat == SNMP_ERR_NOERROR))
			{
				struct variable_list *pVariable = pRespPDU->variables;
				while (pVariable) 
				{
					if (pVariable->val_len < ulMaxLen)
					{
						memcpy(pBuff, pVariable->val.string, pVariable->val_len);
					}
					else
					{
						memcpy(pBuff, pVariable->val.string, ulMaxLen);
					}

					pVariable= pVariable->next_variable;
				}
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

