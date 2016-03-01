#include <stdlib.h>
#include <errno.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "libconfig.h"
#include "ftnm.h"
#include "ftnm_node_snmpc.h"
#include "ftnm_dmc.h"
#include "ftnm_ep.h"
#include "ftnm_ep_class.h"
#include "ftnm_snmptrapd.h"

FTM_VOID_PTR	FTNM_SNMPC_asyncResponseManager(FTM_VOID_PTR pData);
FTM_VOID_PTR	FTNM_SNMPTRAPD_process(FTM_VOID_PTR pData);

extern int	active_hosts;

FTM_RET	FTNM_SNMPC_init(void)
{
	init_agent("ftnm:snmpc");
	init_snmp("ftnm:snmpc");

	return	FTM_RET_OK;
}

FTM_RET	FTNM_SNMPC_final(void)
{
	return	FTM_RET_OK;
}

FTM_RET	FTNM_SNMPC_create(FTNM_SNMPC_PTR _PTR_ ppCTX)
{
	ASSERT(ppCTX != NULL);
	FTNM_SNMPC_PTR	pCTX;

	pCTX = (FTNM_SNMPC_PTR)FTM_MEM_malloc(sizeof(FTNM_SNMPC));
	if (pCTX == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}
	
	memset(pCTX, 0, sizeof(FTNM_SNMPC));

	strcpy(pCTX->xConfig.pName, FTNM_SNMPC_NAME);
	FTM_LIST_init(&pCTX->xConfig.xMIBList);
	pCTX->xConfig.ulMaxRetryCount = FTNM_SNMPC_RETRY_COUNT;

	*ppCTX = pCTX;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_SNMPC_destroy(FTNM_SNMPC_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTM_ULONG i, ulCount;

	FTM_LIST_count(&pCTX->xConfig.xMIBList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_VOID_PTR pValue;

		if (FTM_LIST_getAt(&pCTX->xConfig.xMIBList, i, &pValue) == FTM_RET_OK)
		{
			FTM_MEM_free(pValue);
		}
	}

	FTM_LIST_final(&pCTX->xConfig.xMIBList);


	FTM_MEM_free(pCTX);

	return	FTM_RET_OK;
}

FTM_RET FTNM_SNMPC_start(FTNM_SNMPC_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTM_INT	nRet;

	nRet = pthread_create(&pCTX->xPThread, NULL, FTNM_SNMPC_asyncResponseManager, 0);
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


FTM_RET	FTNM_SNMPC_stop(FTNM_SNMPC_PTR pCTX)
{
	ASSERT(pCTX != NULL);
	
	FTM_INT			nRet;
	FTM_VOID_PTR 	pRet = NULL;

	nRet = pthread_join(pCTX->xPThread, &pRet);
	if (nRet != 0)
	{
		switch(nRet)
		{ 
		case	EDEADLK: 
			{
				MESSAGE("A deadlock was detected (e.g., two threads tried to join with each other); or thread specifies the calling thread.\n"); 
			}
			break;

		case	EINVAL: 
			{
				MESSAGE("thread is not a joinable thread. Another thread is already waiting to join with this thread.\n"); 
			}
			break;

		case	ESRCH:  
			{	
				MESSAGE("No thread with the ID thread could be found.\n");
			}
			break;

		default:
			{
				MESSAGE("Unknown error[%d]\n", nRet); 
			}
			break;
		}

		return	FTM_RET_THREAD_JOIN_ERROR;
	}


	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTNM_SNMPC_asyncResponseManager(FTM_VOID_PTR pData)
{
	while (FTM_TRUE)
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

FTM_RET FTNM_SNMPC_loadConfig(FTNM_SNMPC_PTR pCTX, FTM_CHAR_PTR pFileName)
{
	ASSERT(pCTX != NULL);
	ASSERT(pFileName != NULL);

	config_t			xConfig;
	config_setting_t	*pSection;
	
	config_init(&xConfig);
	printf("pFileName = %s\n", pFileName);
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
						FTM_LIST_append(&pCTX->xConfig.xMIBList, pBuff);
				read_mib(pBuff);
					}
				}
			}
		}

		pField = config_setting_get_member(pSection, "retry_count");
		if (pField != NULL)
		{
			pCTX->xConfig.ulMaxRetryCount = config_setting_get_int(pField);		
		}
		else
		{
			pCTX->xConfig.ulMaxRetryCount = 1;
		}
	}
	config_destroy(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET FTNM_SNMPC_showConfig(FTNM_SNMPC_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	FTM_ULONG	ulCount;

	MESSAGE("\n[ SNMP CONFIGURATION ]\n");
	MESSAGE("%16s : %d\n", "MAX RETRY COUNT", pCTX->xConfig.ulMaxRetryCount);
	if (FTM_LIST_count(&pCTX->xConfig.xMIBList, &ulCount) == FTM_RET_OK)
	{
		FTM_ULONG i;

		MESSAGE("%16s : %d\n", "MIBs", ulCount);
		for(i = 0 ; i < ulCount ; i++)
		{
			FTM_VOID_PTR	pValue;

			if (FTM_LIST_getAt(&pCTX->xConfig.xMIBList, i, &pValue) == FTM_RET_OK)
			{
				MESSAGE("%16d - %s\n", i+1, (FTM_CHAR_PTR)pValue);
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_SNMPC_getEPData(FTNM_NODE_SNMPC_PTR pNode, FTNM_EP_PTR pEP, FTM_EP_DATA_PTR pData)
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
		
			int nRet = snmp_synch_response(pSession, pReqPDU, &pRespPDU);
		
			if ((nRet == STAT_SUCCESS) && (pRespPDU->errstat == SNMP_ERR_NOERROR))
			{
				struct variable_list *pVariable = pRespPDU->variables;
				while (pVariable) 
				{
					switch(pVariable->name[pVariable->name_length-2])
					{
					case	6:
						{
							FTM_EP_DATA	xLastData;
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

							xRet = FTNM_EP_getData(pEP, &xLastData);
							if (xRet != FTM_RET_OK)
							{
								break;
							}

							pData->ulTime = time(NULL);
							pData->xState = FTM_EP_STATE_RUN;
							pData->xType  = xLastData.xType;
							switch(pData->xType)
							{
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
		ERROR("snmp_open: %s\n", snmp_errstring(snmp_errno));
		xRet = FTM_RET_SNMP_CANT_OPEN_SESSION;
	}

	sem_post(&pEP->xLock);
	sem_post(&pNode->xLock);

	return	xRet;
}

FTM_RET		FTNM_SNMPC_getOID(FTM_CHAR_PTR pInput, oid *pOID, size_t	*pnOIDLen)
{
	if (read_objid(pInput, pOID, pnOIDLen) == 1)
	{
		return	FTM_RET_OK;
	}

	return	FTM_RET_SNMP_CANT_FIND_OID;

}
