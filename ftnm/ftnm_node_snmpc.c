#include <stdlib.h>
#include "ftnm.h"
#include "ftnm_node_snmpc.h"
#include "ftnm_dmc.h"
#include "ftnm_ep.h"
#include "ftnm_ep_class.h"

FTM_ULONG		active_hosts = 0;
FTM_VOID_PTR	FTNM_NODE_SNMPC_process(FTM_VOID_PTR pData);
FTM_RET			FTNM_NODE_SNMPC_start(FTNM_NODE_SNMPC_PTR pNode);
FTM_RET			FTNM_NODE_SNMPC_stop(FTNM_NODE_SNMPC_PTR pNode);
FTM_RET			FTNM_NODE_SNMPC_openSession(FTNM_NODE_SNMPC_PTR pNode);

FTM_RET	FTNM_NODE_SNMPC_create(FTM_NODE_INFO_PTR pInfo, FTNM_NODE_PTR _PTR_ ppNode)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppNode != NULL);

	FTNM_NODE_SNMPC_PTR	pNode;
	
	
	pNode = (FTNM_NODE_SNMPC_PTR)FTM_MEM_malloc(sizeof(FTNM_NODE_SNMPC));
	if (pNode == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memset(pNode, 0, sizeof(FTNM_NODE_SNMPC));

	memcpy(&pNode->xCommon.xInfo, pInfo, sizeof(FTM_NODE_INFO));
	FTM_LIST_init(&pNode->xCommon.xEPList);
	pthread_mutex_init(&pNode->xCommon.xMutexLock, NULL);
	
	pNode->xCommon.fStart	= (FTNM_NODE_START)FTNM_NODE_SNMPC_start;
	pNode->xCommon.fStop 	= (FTNM_NODE_STOP)FTNM_NODE_SNMPC_stop;
	*ppNode = (FTNM_NODE_PTR)pNode;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_SNMPC_destroy(FTNM_NODE_SNMPC_PTR pNode)
{
	ASSERT(pNode != NULL);

	FTM_LIST_final(&pNode->xCommon.xEPList);

	FTM_MEM_free(pNode);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_SNMPC_init(FTNM_NODE_SNMPC_PTR pNode)
{
	FTM_RET				nRet;
	FTM_ULONG			ulEPCount;

	ASSERT(pNode != NULL);

	nRet = FTNM_NODE_EP_count((FTNM_NODE_PTR)pNode, &ulEPCount);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	TRACE("NODE(%08x)[%s] has %d EPs\n", pNode, pNode->xCommon.xInfo.pDID, ulEPCount);
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
			FTM_LIST_append(&pNode->xCommon.xEPList, pEP);
		}
	}
	pNode->xCommon.xState = FTNM_NODE_STATE_INITIALIZED;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_SNMPC_final(FTNM_NODE_SNMPC_PTR pNode)
{
	ASSERT(pNode != NULL);

	FTM_LIST_final(&pNode->xCommon.xEPList);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_SNMPC_start(FTNM_NODE_SNMPC_PTR pNode)
{
	if (pthread_create(&pNode->xCommon.xPThread, NULL, FTNM_NODE_SNMPC_process, pNode) == 0)
	{
		return	FTM_RET_OK;
	}

	return	FTM_RET_OK;
}


FTM_RET	FTNM_NODE_SNMPC_stop(FTNM_NODE_SNMPC_PTR pNode)
{
	ASSERT(pNode != NULL);

	FTM_VOID_PTR	xRet;

	pthread_cancel(pNode->xCommon.xPThread);

	pthread_join(pNode->xCommon.xPThread, (FTM_VOID_PTR _PTR_)&xRet);
	if (xRet != PTHREAD_CANCELED)
	{
		ERROR("The node is not stopped. [ID = %s]\n", pNode->xCommon.xInfo.pDID);
		return	FTM_RET_NODE_IS_NOT_STOPPED;
	}

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTNM_NODE_SNMPC_process(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);

	FTNM_NODE_SNMPC_PTR	pNode = (FTNM_NODE_SNMPC_PTR)pData;
	FTNM_EP_PTR			pEP;

	FTM_LIST_iteratorStart(&pNode->xCommon.xEPList);
	while (FTM_LIST_iteratorNext(&pNode->xCommon.xEPList, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
	{
		if (pEP->xInfo.xState == FTM_EP_STATE_RUN)
		{
			pEP->xState = FTNM_EP_STATE_RUN;	
		}
		else
		{
			pEP->xState = FTNM_EP_STATE_STOP;	
		}
	}

	pNode->bRun = FTM_TRUE;
	pNode->xCommon.xState = FTNM_NODE_STATE_RUNNING;

	if (FTNM_NODE_SNMPC_openSession(pNode) != FTM_RET_OK)
	{
		pNode->xCommon.xState = FTNM_NODE_STATE_CANT_OPEN_SESSION;
		return	0;	
	}

	FTNM_TIMER_init(&pNode->xTimer, 0);

	while(pNode->bRun)
	{
		FTNM_TIMER_add(&pNode->xTimer, pNode->xCommon.xInfo.ulTimeout * 1000000);
	
		FTM_LIST_iteratorStart(&pNode->xCommon.xEPList);
		while (pNode->bRun && (FTM_LIST_iteratorNext(&pNode->xCommon.xEPList, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK))
		{
			if (pEP->xState == FTNM_EP_STATE_RUN)
			{
				netsnmp_pdu 		*pReqPDU;
				netsnmp_pdu			*pRespPDU = NULL; 

				pReqPDU = snmp_pdu_create(SNMP_MSG_GET);	/* send the first GET */
				if (pReqPDU == NULL)
				{
					ERROR("SNMP PDU creation error - %s\n", snmp_errstring(snmp_errno));
					pEP->xState = FTNM_EP_STATE_ERROR;
					break;
				}

				pReqPDU->time = pNode->xCommon.xInfo.ulTimeout;
				snmp_add_null_var(pReqPDU, pEP->xOption.xSNMP.pOID, pEP->xOption.xSNMP.nOIDLen);
				pNode->xStatistics.ulRequest++;

				int nRet = snmp_synch_response(pNode->pSession, pReqPDU, &pRespPDU);
				if (nRet != 0)
				{
					ERROR("EP(%08x) is occurred synch response error! - %s\n", pEP->xInfo.xEPID, snmp_errstring(nRet));
					if (FTNM_SNMPC_getMaxRetryCount() > pEP->ulRetryCount)
					{
						pEP->ulRetryCount++;
					}
					else
					{
						ERROR("EP(%08x) is not stable.\n", pEP->xInfo.xEPID);
						pEP->xState = FTNM_EP_STATE_ERROR;
					}
				}
				else
				{
					if (pRespPDU->errstat == SNMP_ERR_NOERROR) 
					{
						struct variable_list *pVariable;

						pVariable= pRespPDU->variables;
						while (pVariable) 
						{
							switch(pVariable->name[pVariable->name_length-2])
							{
							case	6:
								{
									FTM_CHAR	pBuff[1024];
									FTM_EP_DATA	xData;

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


									xData.ulTime = time(NULL);
									xData.xType  = FTM_EP_DATA_TYPE_FLOAT;
									xData.xValue.fValue = strtod(pBuff, NULL);

									FTNM_setEPData(pEP->xInfo.xEPID, &xData);
								}
								break;
							};

							pVariable= pVariable->next_variable;
						}
					}

					pNode->xStatistics.ulResponse++;
					pNode->xCommon.ulRetry = 0;
				}

				if (pRespPDU != NULL)
				{
					snmp_free_pdu(pRespPDU);
				}
			}
		}
	
		FTNM_TIMER_waitForExpired(&pNode->xTimer);
	}

	snmp_close(pNode->pSession);
	pNode->pSession = NULL;

	pNode->xCommon.xState = FTNM_NODE_STATE_STOP;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_NODE_SNMPC_openSession(FTNM_NODE_SNMPC_PTR pNode)
{
	struct snmp_session	xSession;

	snmp_sess_init(&xSession);			/* initialize session */

	xSession.version 		= pNode->xCommon.xInfo.xOption.xSNMP.ulVersion;
	xSession.peername 		= pNode->xCommon.xInfo.xOption.xSNMP.pURL;
	xSession.community 		= (u_char *)pNode->xCommon.xInfo.xOption.xSNMP.pCommunity;
	xSession.community_len	= strlen(pNode->xCommon.xInfo.xOption.xSNMP.pCommunity);

	pNode->pSession = snmp_open(&xSession);
	if (pNode->pSession == NULL)
	{
		ERROR("snmp_open: %s\n", snmp_errstring(snmp_errno));
		pNode->xCommon.xState = FTNM_NODE_STATE_CANT_OPEN_SESSION;
		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

