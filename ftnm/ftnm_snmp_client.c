#include <string.h>
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
	struct mib_tree *pMIBTree;

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

FTM_RET FTNM_snmpClientAsyncCall(FTNM_SNMP_INFO_PTR pInfo)
{
	/* startup all hosts */
	FTNM_SNMP_OID_PTR	pOID;
	struct snmp_pdu 	*pPDU;
	struct snmp_session xSession;

	pInfo->nCurrentOID = 0;
	pOID = (FTNM_SNMP_OID_PTR)list_get_at(&pInfo->xOIDList, pInfo->nCurrentOID);
	if (pOID == NULL)
	{
		return	FTM_RET_OK;	
	}

	snmp_sess_init(&xSession);			/* initialize session */
	xSession.version 		= pInfo->nVersion;
	xSession.peername 		= strdup(pInfo->pPeerName);
	xSession.community 		= (FTM_BYTE_PTR)strdup(pInfo->pCommunity);
	xSession.community_len 	= strlen(pInfo->pCommunity);
	xSession.callback 		= FTNM_snmpClientAsyncResponse;
	xSession.callback_magic = pInfo;

	pInfo->pSession = snmp_open(&xSession);
	if (pInfo->pSession == 0)
	{
		snmp_perror("snmp_open");
		return	FTM_RET_COMM_ERROR;
	}

	
	pPDU = snmp_pdu_create(SNMP_MSG_GET);	/* send the first GET */
	if (pPDU == NULL)
	{
		snmp_close(pInfo->pSession);
		pInfo->pSession = NULL;

		snmp_perror("snmp_pdu_create");
		return	FTM_RET_COMM_ERROR;
	}

	snmp_add_null_var(pPDU, pOID->pOID, pOID->nOIDLen);
	if (snmp_send(pInfo->pSession, pPDU) == 0)
	{
		snmp_perror("snmp_send");
		snmp_free_pdu(pPDU);
		return	FTM_RET_COMM_ERROR;
	}

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
	FTNM_SNMP_INFO_PTR	pInfo = (FTNM_SNMP_INFO_PTR)pParams;
	struct snmp_pdu 	*pReqPDU;

	switch(nOperation)
	{
	case	NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE: 
		{
			if (pRespPDU->errstat == SNMP_ERR_NOERROR) 
			{
				FTNM_SNMP_OID_PTR	pOID = list_get_at(&pInfo->xOIDList, pInfo->nCurrentOID);

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

			pInfo->nCurrentOID++;
			if (pInfo->nCurrentOID < list_size(&pInfo->xOIDList))
			{
				FTNM_SNMP_OID_PTR	pOID = list_get_at(&pInfo->xOIDList, pInfo->nCurrentOID);

				if (pOID != NULL)
				{
					pReqPDU = snmp_pdu_create(SNMP_MSG_GET);

					snmp_add_null_var(pReqPDU, pOID->pOID, pOID->nOIDLen);
					if (snmp_send(pInfo->pSession, pReqPDU))
					{
						return 1;
					}
					else 
					{
						snmp_perror("snmp_send");
						snmp_free_pdu(pReqPDU);
					}
				}
			}
		}
		break;

	default:
		{	
		}
	}

	return 1;

}

