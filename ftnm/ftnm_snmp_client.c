#include <string.h>
#include "ftnm.h"
#include "ftnm_snmp_client.h"

static FTM_INT	FTNM_snmpClientAsyncResponse
(
	FTM_INT				operation, 
	struct snmp_session *sp, 
	FTM_INT				reqid,
	struct snmp_pdu 	*pdu, 
	FTM_VOID_PTR		magic
);

FTM_RET	FTNM_snmpClientInit(void)
{
	struct tree *pMIBTree;

	init_snmp("FTNM");
	pMIBTree = read_mib("FTE-E.txt");
	if (pMIBTree == NULL)
	{
		printf("MIB loading failed\n");
		return	FTM_RET_SNMP_INIT_FAILED;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_snmpClientFinal(void)
{
	return	FTM_RET_OK;
}

FTM_BOOL	FTNM_snmpIsRunning(FTNM_NODE_PTR pNode)
{
	FTNM_SNMP_CONTEXT_PTR	pSNMP = (FTNM_SNMP_CONTEXT_PTR)pNode->pData;

	if (pSNMP->nStatus & FTNM_SNMP_STATUS_RUNNING)
	{
		return	FTM_BOOL_TRUE;	
	}

	return	FTM_BOOL_FALSE;
}

FTM_BOOL	FTNM_snmpIsCompleted(FTNM_NODE_PTR pNode)
{
	FTNM_SNMP_CONTEXT_PTR	pSNMP = (FTNM_SNMP_CONTEXT_PTR)pNode->pData;

	if (pSNMP->nStatus & FTNM_SNMP_STATUS_COMPLETED)
	{
		return	FTM_BOOL_TRUE;	
	}

	return	FTM_BOOL_FALSE;
}

FTM_RET FTNM_snmpClientAsyncCall(FTNM_NODE_PTR pNode)
{
	/* startup all hosts */
	struct snmp_pdu 		*pPDU;
	struct snmp_session 	xSession;
	FTNM_SNMP_OID_PTR		pOID;
	FTNM_SNMP_CONTEXT_PTR	pSNMP = (FTNM_SNMP_CONTEXT_PTR)pNode->pData;


	list_iterator_start(&pSNMP->xOIDList);
	pOID = (FTNM_SNMP_OID_PTR)list_iterator_next(&pSNMP->xOIDList);
	if (pOID == NULL)
	{
		return	FTM_RET_OK;	
	}

	snmp_sess_init(&xSession);			/* initialize session */
	xSession.version 		= pNode->xInfo.xOption.xSNMP.nVersion;
	xSession.peername 		= strdup(pNode->xInfo.xOption.xSNMP.pURL);
	xSession.community 		= (FTM_BYTE_PTR)strdup(pNode->xInfo.xOption.xSNMP.pCommunity);
	xSession.community_len 	= strlen(pNode->xInfo.xOption.xSNMP.pCommunity);
	xSession.callback 		= FTNM_snmpClientAsyncResponse;
	xSession.callback_magic = pNode;

	pSNMP->pSession = snmp_open(&xSession);
	if (pSNMP->pSession == 0)
	{
		snmp_perror("snmp_open");
		return	FTM_RET_COMM_ERROR;
	}

	
	pPDU = snmp_pdu_create(SNMP_MSG_GET);	/* send the first GET */
	if (pPDU == NULL)
	{
		snmp_close(pSNMP->pSession);
		pSNMP->pSession = NULL;

		snmp_perror("snmp_pdu_create");
		return	FTM_RET_COMM_ERROR;
	}

	pSNMP->nStatus = 0;

	snmp_add_null_var(pPDU, pOID->pOID, pOID->nOIDLen);
	if (snmp_send(pSNMP->pSession, pPDU) == 0)
	{
		snmp_perror("snmp_send");
		snmp_free_pdu(pPDU);
		return	FTM_RET_COMM_ERROR;
	}

	pSNMP->nStatus = FTNM_SNMP_STATUS_RUNNING;

	return	FTM_RET_OK;
}

FTM_INT	FTNM_snmpClientAsyncResponse
(
	FTM_INT				nOperation, 
	struct snmp_session *pSession, 
	FTM_INT				nReqID,
	struct snmp_pdu 	*pRespPDU, 
	FTM_VOID_PTR		pParams
)
{
	FTNM_NODE_PTR			pNode = (FTNM_NODE_PTR)pParams;
	FTNM_SNMP_CONTEXT_PTR	pSNMP = (FTNM_SNMP_CONTEXT_PTR)pNode->pData;
	struct snmp_pdu 		*pReqPDU;

	MESSAGE("pSNMP = %08lx\n", pSNMP);
	switch(nOperation)
	{
	case	NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE: 
		{
			if (pRespPDU->errstat == SNMP_ERR_NOERROR) 
			{
				FTNM_SNMP_OID_PTR	pOID = (FTNM_SNMP_OID_PTR)list_iterator_next(&pSNMP->xOIDList);
				if (pOID != NULL)
				{
					struct variable_list *pVariableList;
					FTM_INT	nLen = 0;

					pVariableList = pRespPDU->variables;
					while (pVariableList) 
					{
						nLen += pVariableList->name_length;
						pVariableList = pVariableList->next_variable;
					}

					if (pOID->pValue != NULL)
					{
						free(pOID->pValue);	
						pOID->pValue = NULL;
					}

					pOID->pValue = (FTM_BYTE_PTR)calloc(nLen, 1);

					pVariableList = pRespPDU->variables;
					while (pVariableList) 
					{
						snprint_variable((FTM_CHAR_PTR)&pOID->pValue[pOID->nValueLen], nLen - pOID->nValueLen, 
								pVariableList->name, pVariableList->name_length, pVariableList);
						pVariableList = pVariableList->next_variable;
					}
				}
			}								    

			if (pSNMP->nCurrentOID < list_size(&pSNMP->xEPList))
			{
				FTNM_SNMP_OID_PTR	pOID = list_get_at(&pSNMP->xEPList, pSNMP->nCurrentOID);

				if (pOID != NULL)
				{
					pReqPDU = snmp_pdu_create(SNMP_MSG_GET);

					snmp_add_null_var(pReqPDU, pOID->pOID, pOID->nOIDLen);
					if (snmp_send(pSNMP->pSession, pReqPDU))
					{
						return 1;
					}
					else 
					{
						snmp_perror("snmp_send");
						snmp_free_pdu(pReqPDU);
					}
				}
				else
				{
					pSNMP->nStatus = FTNM_SNMP_STATUS_COMPLETED;
				}
			}
			else
			{
				pSNMP->nStatus = FTNM_SNMP_STATUS_COMPLETED;
			}
		}
		break;

	default:
		{	
		}
	}

	return 1;

}

