#include <stdlib.h>
#include "ftnm.h"
#include "ftnm_node_snmpc.h"

FTM_VOID_PTR	FTNM_SNMPC_asyncResponseManager(FTM_VOID_PTR pData);
FTM_CHAR_PTR	FTNM_NODE_SNMPC_getStateString(FTNM_SNMPC_STATE xState);

static FTM_INT	FTNM_NODE_SNMPC_asyncResponse
(
	FTM_INT				operation, 
	struct snmp_session *sp, 
	FTM_INT				reqid,
	struct snmp_pdu 	*pdu, 
	FTM_VOID_PTR		magic
);

pthread_t	xSNMPManager;
int	active_hosts = 0;

FTM_RET	FTNM_SNMPC_init(FTM_CHAR_PTR pAppName, FTNM_CFG_SNMPC_PTR pConfig)
{
	FTM_ULONG	i, ulCount;

	init_snmp("ftnm");

	if (FTM_LIST_count(&pConfig->xMIBList, &ulCount) == FTM_RET_OK)
	{
		for(i = 0 ; i < ulCount ; i++)
		{
			FTM_VOID_PTR	pValue;

			if (FTM_LIST_getAt(&pConfig->xMIBList, i, &pValue) == FTM_RET_OK)
			{
				TRACE("Load MIB : %s\n", (FTM_CHAR_PTR)pValue);

				read_mib((FTM_CHAR_PTR)pValue);
			}
		}
	}

	FTM_initEPOIDInfo();

	pthread_create(&xSNMPManager, NULL, FTNM_SNMPC_asyncResponseManager, 0);
	return	FTM_RET_OK;
}

FTM_RET	FTNM_SNMPC_final(FTM_VOID)
{
	FTM_finalEPOIDInfo();

	return	FTM_RET_OK;
}


FTM_VOID_PTR	FTNM_SNMPC_asyncResponseManager(FTM_VOID_PTR pData)
{
	while (1)
	{	
		if (active_hosts) 
	{
		FTM_INT	fds = 0, block = 1;
	    fd_set fdset;
		struct timeval timeout;

		FD_ZERO(&fdset);
		snmp_select_info(&fds, &fdset, &timeout, &block);
		fds = select(fds, &fdset, NULL, NULL, block ? NULL : &timeout);
		if (fds < 0) 
		{
			perror("select failed");
			exit(1);
		}
		if (fds)
		{
			snmp_read(&fdset);
		}
		else
		{
			snmp_timeout();
		}
	}
		usleep(1000);
	}

	return	0;
}

FTM_RET	FTNM_NODE_SNMPC_init(FTNM_NODE_SNMPC_PTR pNode)
{
	FTM_RET				nRet;
	FTM_ULONG			ulEPCount;

	ASSERT(pNode != NULL);

	FTM_LIST_init(&pNode->xSNMPC.xEPList);

	nRet = FTNM_NODE_EP_count((FTNM_NODE_PTR)pNode, &ulEPCount);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	TRACE("NODE[%s] has %d EPs\n", pNode->xCommon.xInfo.pDID, ulEPCount);
	if (ulEPCount != 0)
	{
		FTM_ULONG	i;

		for(i = 0 ; i < ulEPCount ; i++)
		{
			FTNM_EP_PTR				pEP;
			FTM_EP_CLASS_INFO_PTR	pEPClassInfo;
			FTM_CHAR				pOIDName[1024];

			if (FTNM_NODE_EP_getAt((FTNM_NODE_PTR)pNode, i, (FTNM_EP_PTR _PTR_)&pEP) != FTM_RET_OK)
			{
				TRACE("EP[%d] information not found\n", i);
				continue;
			}

			if (FTNM_EP_CLASS_INFO_get((pEP->xInfo.xEPID & FTM_EP_CLASS_MASK), &pEPClassInfo) != FTM_RET_OK)
			{
				TRACE("EP CLASS[%08lx] information not found\n", pEP->xInfo.xEPID);
				continue;
			}

			snprintf(pOIDName, sizeof(pOIDName) - 1, "%s::%s", 
				pNode->xCommon.xInfo.xOption.xSNMP.pMIB, 
				pEPClassInfo->xOIDs.pValue);
			pEP->xOption.xSNMP.nOIDLen = MAX_OID_LEN;
			if (read_objid(pOIDName, pEP->xOption.xSNMP.pOID, &pEP->xOption.xSNMP.nOIDLen) == 0)
			{
				TRACE("Can't find MIB\n");
				continue;
			}
			pEP->xOption.xSNMP.pOID[pEP->xOption.xSNMP.nOIDLen++] = pEP->xInfo.xDEPID & 0xFF;
			FTM_LIST_append(&pNode->xSNMPC.xEPList, pEP);
		}
	}
	pNode->xSNMPC.nState = FTNM_SNMPC_STATE_INITIALIZED;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_SNMPC_final(FTNM_NODE_SNMPC_PTR pNode)
{
	ASSERT(pNode != NULL);

	FTM_LIST_destroy(&pNode->xSNMPC.xEPList);

	return	FTM_RET_OK;
}

FTM_BOOL	FTNM_NODE_SNMPC_isRunning(FTNM_NODE_SNMPC_PTR pNode)
{
	ASSERT(pNode != NULL);

	if (pNode->xSNMPC.nState == FTNM_SNMPC_STATE_RUNNING)
	{
		return	FTM_BOOL_TRUE;	
	}

	return	FTM_BOOL_FALSE;
}

FTM_BOOL	FTNM_NODE_SNMPC_isCompleted(FTNM_NODE_SNMPC_PTR pNode)
{
	ASSERT(pNode != NULL);

	if (pNode->xSNMPC.nState != FTNM_SNMPC_STATE_RUNNING)
	{
		return	FTM_BOOL_TRUE;	
	}

	return	FTM_BOOL_FALSE;
}

FTM_RET FTNM_NODE_SNMPC_startAsync(FTNM_NODE_SNMPC_PTR pNode)
{
	/* startup all hosts */
	//struct snmp_pdu 	*pPDU;
	netsnmp_pdu 		*pPDU;
	struct snmp_session	xSession;
	FTNM_EP_PTR			pEP;

	ASSERT(pNode != NULL);

	snmp_sess_init(&xSession);			/* initialize session */

	xSession.version 		= pNode->xCommon.xInfo.xOption.xSNMP.ulVersion;
	xSession.peername 		= pNode->xCommon.xInfo.xOption.xSNMP.pURL;
	xSession.community 		= (u_char *)pNode->xCommon.xInfo.xOption.xSNMP.pCommunity;
	xSession.community_len	= strlen(pNode->xCommon.xInfo.xOption.xSNMP.pCommunity);
	xSession.callback 		= FTNM_NODE_SNMPC_asyncResponse;
	xSession.callback_magic	= pNode;

	FTM_LIST_iteratorStart(&pNode->xSNMPC.xEPList);
	if (FTM_LIST_iteratorNext(&pNode->xSNMPC.xEPList, (FTM_VOID_PTR _PTR_)&pEP) != FTM_RET_OK)
	{
		pNode->xSNMPC.nState = FTNM_SNMPC_STATE_COMPLETED;
		return	FTM_RET_OK;
	}

	pNode->xSNMPC.pSession = snmp_open(&xSession);
	if (pNode->xSNMPC.pSession == NULL)
	{
		ERROR("snmp_open: %s\n", snmp_errstring(snmp_errno));
		pNode->xSNMPC.nState = FTNM_SNMPC_STATE_ERROR;
		return	FTM_RET_COMM_ERROR;
	}

	pPDU = snmp_pdu_create(SNMP_MSG_GET);	/* send the first GET */
	if (pPDU == NULL)
	{
		ERROR("snmp_pdu_create: %s\n", snmp_errstring(snmp_errno));
		snmp_close(pNode->xSNMPC.pSession);
		pNode->xSNMPC.pSession = NULL;

		return	FTM_RET_COMM_ERROR;
	}

	pPDU->time = pNode->xCommon.xInfo.ulTimeout;
	snmp_add_null_var(pPDU, pEP->xOption.xSNMP.pOID, pEP->xOption.xSNMP.nOIDLen);
	if (snmp_send(pNode->xSNMPC.pSession, pPDU) == 0)
	{
		ERROR("snmp_send: %s\n", snmp_errstring(snmp_errno));
		pNode->xSNMPC.nState = FTNM_SNMPC_STATE_ERROR;
		snmp_free_pdu(pPDU);
		return	FTM_RET_COMM_ERROR;
	}

	pNode->xSNMPC.pCurrentEP = pEP;
	pNode->xSNMPC.xStatistics.ulRequest++;
	pNode->xSNMPC.nState = FTNM_SNMPC_STATE_RUNNING;

	active_hosts++;

	return	FTM_RET_OK;
}

FTM_INT	FTNM_NODE_SNMPC_asyncResponse
(
	FTM_INT				nOperation, 
	struct snmp_session *pSession, 
	FTM_INT				nReqID,
	netsnmp_pdu			*pRespPDU, 
	FTM_VOID_PTR		pParams
)
{
	FTM_RET					nRet;
	FTNM_NODE_SNMPC_PTR		pNode = (FTNM_NODE_SNMPC_PTR)pParams;

	active_hosts--;

	switch(nOperation)
	{
	case	NETSNMP_CALLBACK_OP_TIMED_OUT:
		{
			if (pNode->xSNMPC.pSession != NULL)
			{
				pNode->xSNMPC.nState = FTNM_SNMPC_STATE_TIMEOUT;
				TRACE("NODE : %s, SNMP STATE : %s\n", 
					pNode->xCommon.xInfo.pDID, 
					FTNM_NODE_SNMPC_getStateString(pNode->xSNMPC.nState));
			}
		}
		break;

	case	NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE: 
		{
			FTNM_EP_PTR			pEP;
			if (pRespPDU->errstat == SNMP_ERR_NOERROR) 
			{
				struct variable_list *pVariable;
				pEP = pNode->xSNMPC.pCurrentEP;

				pVariable= pRespPDU->variables;
				while (pVariable) 
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

							pEP->xData.ulTime = time(NULL);
							pEP->xData.xType  = FTM_EP_DATA_TYPE_FLOAT;
							pEP->xData.xValue.fValue = strtod(pBuff, NULL);

							FTNM_DMC_EP_DATA_set(pEP);
						}
						break;
					};

					pVariable= pVariable->next_variable;
				}

				pNode->xSNMPC.xStatistics.ulResponse++;
			}
			else
			{
				ERROR("SNMPC response error	[%08lx]\n", pRespPDU->errstat);
			}

			nRet = FTM_LIST_iteratorNext(&pNode->xSNMPC.xEPList,(FTM_VOID_PTR _PTR_)&pEP);
			if (nRet == FTM_RET_OK)
			{
				netsnmp_pdu	*pPDU;

				pPDU = snmp_pdu_create(SNMP_MSG_GET);
				if (pPDU == NULL)
				{
					ERROR("snmp_pdu_create: %s\n", snmp_errstring(snmp_errno));
					snmp_close(pNode->xSNMPC.pSession);
					pNode->xSNMPC.pSession = NULL;

					return	FTM_RET_COMM_ERROR;
				}
				pPDU->time = pNode->xCommon.xInfo.ulTimeout;
		
				snmp_add_null_var(pPDU, pEP->xOption.xSNMP.pOID, pEP->xOption.xSNMP.nOIDLen);
				if (snmp_send(pNode->xSNMPC.pSession, pPDU) == 0)
				{
					snmp_perror("snmp_send");
					snmp_free_pdu(pPDU);
					pNode->xSNMPC.pCurrentEP = NULL;
					pNode->xSNMPC.nState = FTNM_SNMPC_STATE_ERROR;
				}
				else
				{
					pNode->xSNMPC.pCurrentEP = pEP;
					pNode->xSNMPC.xStatistics.ulRequest++;
					pNode->xSNMPC.nState = FTNM_SNMPC_STATE_RUNNING;
					active_hosts++;
				}
			}
			else
			{
				pNode->xSNMPC.nState = FTNM_SNMPC_STATE_COMPLETED;
				TRACE("NODE : %s, SNMP STATE : %s\n", 
					pNode->xCommon.xInfo.pDID, 
					FTNM_NODE_SNMPC_getStateString(pNode->xSNMPC.nState));
			}
		}
		break;

	default:
		{	
		}
	}

	return 1;

}

FTM_RET	FTNM_NODE_SNMPC_stop(FTNM_NODE_SNMPC_PTR pNode)
{
	if (pNode->xSNMPC.pSession != NULL)
	{
		snmp_close(pNode->xSNMPC.pSession);
		pNode->xSNMPC.pSession = NULL;
	}

	return	FTM_RET_OK;
}

FTM_CHAR_PTR	FTNM_NODE_SNMPC_getStateString(FTNM_SNMPC_STATE xState)
{
	switch(xState)
	{
	case	FTNM_SNMPC_STATE_INITIALIZED:	return	"INITIALIZED";
	case	FTNM_SNMPC_STATE_RUNNING:		return	"RUNNING";
	case	FTNM_SNMPC_STATE_TIMEOUT:		return	"TIMEOUT";
	case	FTNM_SNMPC_STATE_ERROR:			return	"ERROR";
	case	FTNM_SNMPC_STATE_COMPLETED:		return	"COMPLETED";
	default:
		return	"UNKNOWN";
	}
}
