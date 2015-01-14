#include "ftnm.h"
#include "ftnm_node_snmpc.h"

static FTM_INT	FTNM_NODE_SNMPC_asyncResponse
(
	FTM_INT				operation, 
	struct snmp_session *sp, 
	FTM_INT				reqid,
	struct snmp_pdu 	*pdu, 
	FTM_VOID_PTR		magic
);

FTM_RET	FTNM_SNMPC_init(FTM_VOID)
{
	init_snmp("ftnm");
	read_mib("FTM50S-MIB.txt");
	read_mib("FTE-E.txt");
	FTM_initEPOIDInfo();

	return	FTM_RET_OK;
}

FTM_RET	FTNM_SNMPC_final(FTM_VOID)
{
	FTM_finalEPOIDInfo();

	return	FTM_RET_OK;
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

	if (ulEPCount != 0)
	{
		FTM_ULONG	i;

		for(i = 0 ; i < ulEPCount ; i++)
		{
			FTNM_EP_SNMP_PTR		pEP;
			FTM_EP_CLASS_INFO_PTR	pEPClassInfo;
			FTM_CHAR				pOIDName[1024];

			if (FTNM_NODE_EP_getAt((FTNM_NODE_PTR)pNode, i, (FTNM_EP_PTR _PTR_)&pEP) != FTM_RET_OK)
			{
				continue;
			}

			if (FTNM_EP_CLASS_INFO_get((pEP->xCommon.xInfo.xEPID & FTM_EP_CLASS_MASK), &pEPClassInfo) != FTM_RET_OK)
			{
				TRACE("EP CLASS[%08lx] information not found\n", pEP->xCommon.xInfo.xEPID);
				continue;	
			}

			snprintf(pOIDName, sizeof(pOIDName) - 1, "%s::%s", pNode->xCommon.xInfo.xOption.xSNMP.pMIB, pEPClassInfo->xOIDs.pValue);
			TRACE("OBJID : %s\n", pOIDName);
			pEP->nOIDLen = MAX_OID_LEN;
			if (read_objid(pOIDName, pEP->pOID, &pEP->nOIDLen) != 0)
			{
				TRACE("~~~~~~~~~~~~~~~~~~~~~\n");
				continue;			
			}

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
	struct snmp_pdu 	*pPDU;
	struct snmp_session	xSession;
	oid					pOID[MAX_OID_LEN];
	size_t				nOIDLen = MAX_OID_LEN;

	ASSERT(pNode != NULL);

	snmp_sess_init(&xSession);			/* initialize session */

	xSession.version 		= pNode->xCommon.xInfo.xOption.xSNMP.nVersion;
	xSession.peername 		= pNode->xCommon.xInfo.xOption.xSNMP.pURL;
	xSession.community 		= pNode->xCommon.xInfo.xOption.xSNMP.pCommunity;
	xSession.community_len	= strlen(pNode->xCommon.xInfo.xOption.xSNMP.pCommunity);
	xSession.callback 		= FTNM_NODE_SNMPC_asyncResponse;
	xSession.callback_magic	= pNode;

	FTM_LIST_iteratorStart(&pNode->xSNMPC.xEPList);
	if (FTM_LIST_iteratorNext(&pNode->xSNMPC.xEPList, (FTM_VOID_PTR _PTR_)&pNode->xSNMPC.pCurrentEP) != FTM_RET_OK)
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

	if (read_objid("FTM50S-MIB::tempValue", pOID, &nOIDLen) == 0)
	{
		ERROR("read_objid: %s\n", snmp_errstring(snmp_errno));
		return	FTM_RET_ERROR;
	} 

	pPDU = snmp_pdu_create(SNMP_MSG_GET);	/* send the first GET */
	if (pPDU == NULL)
	{
		ERROR("snmp_pdu_create: %s\n", snmp_errstring(snmp_errno));
		snmp_close(pNode->xSNMPC.pSession);
		pNode->xSNMPC.pSession = NULL;

		return	FTM_RET_COMM_ERROR;
	}

	snmp_add_null_var(pPDU, pOID, nOIDLen);
	if (snmp_send(pNode->xSNMPC.pSession, pPDU) == 0)
	{
		ERROR("snmp_send: %s\n", snmp_errstring(snmp_errno));
		pNode->xSNMPC.nState = FTNM_SNMPC_STATE_ERROR;
		snmp_free_pdu(pPDU);
		return	FTM_RET_COMM_ERROR;
	}

	pNode->xSNMPC.xStatistics.ulRequest++;
	pNode->xSNMPC.nState = FTNM_SNMPC_STATE_RUNNING;
	pNode->xSNMPC.xTimeout = time(NULL) + 5;
	
	return	FTM_RET_OK;
}

FTM_INT	FTNM_NODE_SNMPC_asyncResponse
(
	FTM_INT				nOperation, 
	struct snmp_session *pSession, 
	FTM_INT				nReqID,
	struct snmp_pdu 	*pRespPDU, 
	FTM_VOID_PTR		pParams
)
{
	FTM_RET					nRet;
	FTNM_NODE_SNMPC_PTR		pNode = (FTNM_NODE_SNMPC_PTR)pParams;

	TRACE("Accept SNMP Response[nOperation = %d]\n", nOperation);
	switch(nOperation)
	{
	case	NETSNMP_CALLBACK_OP_TIMED_OUT:
		{
			if (pNode->xSNMPC.pSession != NULL)
			{
				pNode->xSNMPC.nState = FTNM_SNMPC_STATE_TIMEOUT;
			}
		}
		break;

	case	NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE: 
		{
			if (pRespPDU->errstat == SNMP_ERR_NOERROR) 
			{
				struct variable_list *pVariable;

				pVariable= pRespPDU->variables;
				while (pVariable) 
				{
					print_variable(pVariable->name, pVariable->name_length, pVariable);
					pVariable= pVariable->next_variable;
				}

				pNode->xSNMPC.xStatistics.ulResponse++;
			}
			else
			{
				ERROR("SNMPC response error	[%08lx]\n", pRespPDU->errstat);
			}

			nRet = FTM_LIST_iteratorNext(&pNode->xSNMPC.xEPList,(FTM_VOID_PTR _PTR_)&pNode->xSNMPC.pCurrentEP);
			if (nRet == FTM_RET_OK)
			{
				oid		pOID[MAX_OID_LEN];
				size_t	nOIDLen = MAX_OID_LEN;

				if (read_objid("FTM50S-MIB::tempValue", pOID, &nOIDLen) == 0)
				{
					snmp_perror("read_objid");	
					pNode->xSNMPC.nState = FTNM_SNMPC_STATE_ERROR;
				} 
				else
				{
					struct snmp_pdu 	*pPDU;

					pPDU = snmp_pdu_create(SNMP_MSG_GET);
			
					snmp_add_null_var(pPDU, pOID, nOIDLen);
					if (snmp_send(pNode->xSNMPC.pSession, pPDU) == 0)
					{
						snmp_perror("snmp_send");
						snmp_free_pdu(pPDU);
						pNode->xSNMPC.nState = FTNM_SNMPC_STATE_ERROR;
					}
					pNode->xSNMPC.xStatistics.ulRequest++;
				}
			}
			else
			{
				pNode->xSNMPC.nState = FTNM_SNMPC_STATE_COMPLETED;
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
