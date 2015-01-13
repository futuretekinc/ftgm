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
	FTM_RET		nRet;
	FTM_ULONG	ulEPCount;

	ASSERT((pClient != NULL) && (pNode != NULL));

	pNode->xSNMPC.nStatus = FTNM_SNMP_STATUS_COMPLETED;
	FTM_LIST_init(&pNode->xSNMPC.xEPList);

	snmp_sess_init(&pNode->xSNMPC.xSession);			/* initialize session */
	pNode->xSNMPC.xSession.version 		= SNMP_VERSION_2c;// pNode->xCommon.xInfo.xOption.xSNMP.nVersion;
	pNode->xSNMPC.xSession.peername 	= FTM_MEM_calloc(1, strlen(pNode->xCommon.xInfo.xOption.xSNMP.pURL) + 1);
	strcpy(pNode->xSNMPC.xSession.peername, pNode->xCommon.xInfo.xOption.xSNMP.pURL);
	pNode->xSNMPC.xSession.community_len= strlen(pNode->xCommon.xInfo.xOption.xSNMP.pCommunity);
	pNode->xSNMPC.xSession.community 	= FTM_MEM_calloc(1, pNode->xSNMPC.xSession.community_len + 1);
	memcpy(	pNode->xSNMPC.xSession.community, 
			pNode->xCommon.xInfo.xOption.xSNMP.pCommunity, 
			pNode->xSNMPC.xSession.community_len);
	pNode->xSNMPC.xSession.callback 		= FTNM_NODE_SNMPC_asyncResponse;
	pNode->xSNMPC.xSession.callback_magic= pNode;

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
			FTNM_EP_PTR				pEP;

			FTNM_NODE_EP_getAt((FTNM_NODE_PTR)pNode, i, &pEP);
			FTM_LIST_append(&pNode->xSNMPC.xEPList, pNode);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_SNMPC_final(FTNM_NODE_SNMPC_PTR pNode)
{
	ASSERT(pClient != NULL);

	FTM_MEM_free(pNode->xSNMPC.xSession.peername);
	FTM_MEM_free(pNode->xSNMPC.xSession.community);
	FTM_LIST_destroy(&pNode->xSNMPC.xEPList);

	return	FTM_RET_OK;
}

FTM_BOOL	FTNM_NODE_SNMPC_isRunning(FTNM_NODE_SNMPC_PTR pNode)
{
	ASSERT(pClient != NULL);

	if (pNode->xSNMPC.nStatus & FTNM_SNMP_STATUS_RUNNING)
	{
		return	FTM_BOOL_TRUE;	
	}

	return	FTM_BOOL_FALSE;
}

FTM_BOOL	FTNM_NODE_SNMPC_isCompleted(FTNM_NODE_SNMPC_PTR pNode)
{
	ASSERT(pClient != NULL);

	if (pNode->xSNMPC.nStatus & FTNM_SNMP_STATUS_COMPLETED)
	{
		return	FTM_BOOL_TRUE;	
	}

	return	FTM_BOOL_FALSE;
}

FTM_RET FTNM_NODE_SNMPC_async(FTNM_NODE_SNMPC_PTR pNode)
{
	/* startup all hosts */
	struct snmp_pdu *pPDU;
	oid				pOID[MAX_OID_LEN];
	size_t			nOIDLen = MAX_OID_LEN;

	ASSERT(pClient != NULL);

	FTM_LIST_iteratorStart(&pNode->xSNMPC.xEPList);
	FTM_LIST_iteratorNext(&pNode->xSNMPC.xEPList, (FTM_VOID_PTR _PTR_)&pNode->xSNMPC.pCurrentEP);

	pNode->xSNMPC.pSession = snmp_open(&pNode->xSNMPC.xSession);
	if (pNode->xSNMPC.pSession == 0)
	{
		snmp_perror("snmp_open");
		return	FTM_RET_COMM_ERROR;
	}

	if (read_objid("FTM50S-MIB::tempValue", pOID, &nOIDLen) == 0)
	{
		snmp_perror("read_objid");	
		pNode->xSNMPC.nStatus = FTNM_SNMP_STATUS_COMPLETED;
		return	FTM_RET_ERROR;
	} 

	pPDU = snmp_pdu_create(SNMP_MSG_GET);	/* send the first GET */
	if (pPDU == NULL)
	{
		snmp_close(pNode->xSNMPC.pSession);
		pNode->xSNMPC.pSession = NULL;

		snmp_perror("snmp_pdu_create");
		return	FTM_RET_COMM_ERROR;
	}

	pNode->xSNMPC.nStatus = 0;

	snmp_add_null_var(pPDU, pOID, nOIDLen);
	if (snmp_send(pNode->xSNMPC.pSession, pPDU) == 0)
	{
		snmp_perror("snmp_send");
		snmp_free_pdu(pPDU);
		return	FTM_RET_COMM_ERROR;
	}

	pNode->xSNMPC.nStatus = FTNM_SNMP_STATUS_RUNNING;


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

	switch(nOperation)
	{
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
			}
			else
			{
				ERROR("SNMPC response error	[%08lx]\n", pRespPDU->errstat);
			}

			nRet = FTM_LIST_iteratorNext(&pNode->xSNMPC.xEPList,(FTM_VOID_PTR _PTR_)&pNode->xSNMPC.pCurrentEP);
			if (nRet == FTM_RET_OK)
			{
				oid					pOID[MAX_OID_LEN];
				size_t				nOIDLen = MAX_OID_LEN;

				if (read_objid("FTM50S-MIB::tempValue", pOID, &nOIDLen) == 0)
				{
					snmp_perror("read_objid");	
					pNode->xSNMPC.nStatus = FTNM_SNMP_STATUS_COMPLETED;
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
						pNode->xSNMPC.nStatus = FTNM_SNMP_STATUS_COMPLETED;
					}
				}
				
				return 1;
			}
			else
			{
				pNode->xSNMPC.nStatus = FTNM_SNMP_STATUS_COMPLETED;
			}
		}
		break;

	default:
		{	
		}
	}

	return 1;

}

