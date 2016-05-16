#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/output_api.h>
#include "nxjson.h"

#include "libconfig.h"
#include "ftm.h"
#include "ftom.h"
#include "ftom_msg.h"
#include "ftom_snmptrapd.h"
#include "ftom_node_snmpc.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"

#ifndef	FTOM_TRACE_SNMPTRAPD_IO	
#define	FTOM_TRACE_SNMPTRAPD_IO	0
#endif

typedef	struct
{
	FTM_SNMP_OID			xOID;
	FTOM_SNMPTRAPD_CALLBACK	fCB;
} FTOM_TRAP, _PTR_ FTOM_TRAP_PTR;

static FTM_VOID_PTR		FTOM_SNMPTRAPD_process
(
	FTM_VOID_PTR 	pData
);

static netsnmp_session* FTOM_SNMPTRAPD_addSession
(
	FTOM_SNMPTRAPD_PTR pSNMPTRAPD
);

static FTM_RET	FTOM_SNMPTRAPD_closeSessions
(
	FTOM_SNMPTRAPD_PTR pSNMPTRAPD, 
	netsnmp_session 	*pSessionList
);

static FTM_BOOL	FTOM_SNMPTRAPD_seekTrapCB
(
	const FTM_VOID_PTR 	pElement, 
	const FTM_VOID_PTR 	pIndicator
);

static FTM_RET	FTOM_SNMPTRAPD_receiveTrap
(
	FTOM_SNMPTRAPD_PTR 	pSNMPTRAPD, 
	FTM_SNMP_OID_PTR	pOID,
	FTM_CHAR_PTR 		pMsg
);

static FTM_RET	FTOM_SNMPTRAPD_alert
(
	FTOM_SNMPTRAPD_PTR 	pSNMPTRAPD, 
	FTM_SNMP_OID_PTR	pOID,
	FTM_CHAR_PTR 		pMsg
);
static FTM_RET	FTOM_SNMPTRAPD_discovery
(
	FTOM_SNMPTRAPD_PTR 	pSNMPTRAPD, 
	FTM_SNMP_OID_PTR	pOID,
	FTM_CHAR_PTR 		pMsg
);

static FTM_RET	FTOM_SNMPTRAPD_sendAlert
(
	FTOM_SNMPTRAPD_PTR 	pSNMPTRAPD, 
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR 	pData
);

static FTM_RET	FTOM_SNMPTRAPD_receivedDiscovery
(
	FTOM_SNMPTRAPD_PTR	pSNMPTRAPD,
	FTM_CHAR_PTR	pName,
	FTM_CHAR_PTR	pDID,
	FTM_CHAR_PTR	pIP,
	FTM_EP_TYPE_PTR	pTypes,
	FTM_ULONG		ulCount
);

FTM_RET	FTOM_SNMPTRAPD_create
(
	FTOM_SNMPTRAPD_PTR _PTR_ ppSNMPTRAPD
)
{
	ASSERT(ppSNMPTRAPD != NULL);

	FTM_RET	xRet;
	FTOM_SNMPTRAPD_PTR pSNMPTRAPD;

	pSNMPTRAPD = (FTOM_SNMPTRAPD_PTR)FTM_MEM_malloc(sizeof(FTOM_SNMPTRAPD));
	if (pSNMPTRAPD == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_SNMPTRAPD_init(pSNMPTRAPD);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pSNMPTRAPD);	
		ERROR("Initialize failed[%08x].\n", xRet);
		return	xRet;
	}

	*ppSNMPTRAPD = pSNMPTRAPD;

	return	xRet;
}

FTM_RET	FTOM_SNMPTRAPD_destroy
(
	FTOM_SNMPTRAPD_PTR _PTR_ ppSNMPTRAPD
)
{
	ASSERT(ppSNMPTRAPD != NULL);

	FTM_RET	xRet;

	xRet = FTOM_SNMPTRAPD_final(*ppSNMPTRAPD);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Finalize failed[%08x].\n", xRet);
	}

	xRet = FTM_MEM_free(*ppSNMPTRAPD);	
	if (xRet != FTM_RET_OK)
	{
		ERROR("Memory free failed[%08x].\n", xRet);
	}

	*ppSNMPTRAPD = NULL;

	return	xRet;
}

FTM_RET	FTOM_SNMPTRAPD_init
(
	FTOM_SNMPTRAPD_PTR 	pSNMPTRAPD
)
{
	FTM_RET	xRet;

	memset(pSNMPTRAPD, 0, sizeof(FTOM_SNMPTRAPD));
	strcpy(pSNMPTRAPD->xConfig.pName, FTOM_SNMPTRAPD_NAME);
	pSNMPTRAPD->xConfig.usPort= FTOM_SNMPTRAPD_PORT;
	pSNMPTRAPD->bStop = FTM_TRUE;

	xRet = FTM_LIST_init(&pSNMPTRAPD->xTrapCBList);
	if (xRet == FTM_RET_OK)
	{
		FTM_LIST_setSeeker(&pSNMPTRAPD->xTrapCBList, FTOM_SNMPTRAPD_seekTrapCB);
	}
	
	return	xRet;
}

FTM_RET	FTOM_SNMPTRAPD_final
(
	FTOM_SNMPTRAPD_PTR pSNMPTRAPD
)
{
	ASSERT(pSNMPTRAPD != NULL);
	FTOM_TRAP_PTR	pTrap;

	if (pSNMPTRAPD->bStop)
	{
		FTOM_SNMPTRAPD_stop(pSNMPTRAPD);	
	}

	FTM_LIST_iteratorStart(&pSNMPTRAPD->xTrapCBList);
	while(FTM_LIST_iteratorNext(&pSNMPTRAPD->xTrapCBList, (FTM_VOID_PTR _PTR_)&pTrap) == FTM_RET_OK)
	{
		FTM_LIST_remove(&pSNMPTRAPD->xTrapCBList, pTrap);
		FTM_MEM_free(pTrap);
	}

	FTM_LIST_final(&pSNMPTRAPD->xTrapCBList);

	return	FTM_RET_OK;
}

FTM_RET FTOM_SNMPTRAPD_start
(
	FTOM_SNMPTRAPD_PTR pSNMPTRAPD
)
{
	ASSERT(pSNMPTRAPD != NULL);
	FTM_INT	nRet;

	if (!pSNMPTRAPD->bStop)
	{
		return	FTM_RET_ALREADY_STARTED;	
	}

	nRet = pthread_create(&pSNMPTRAPD->xPThread, NULL, FTOM_SNMPTRAPD_process, pSNMPTRAPD);
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

		default:
			{
				MESSAGE("Unknown error[%d]\n", nRet); 
			}
			break;
		}

		return	FTM_RET_THREAD_CREATION_ERROR;
	}

	TRACE("SNMP TrapD started!\n");
	return	FTM_RET_OK;
}

FTM_RET FTOM_SNMPTRAPD_stop
(
	FTOM_SNMPTRAPD_PTR pSNMPTRAPD
)
{
	ASSERT(pSNMPTRAPD != NULL);
	void*	pRet;

	if (pSNMPTRAPD->bStop)
	{
		return	FTM_RET_NOT_START;	
	}

	pSNMPTRAPD->bStop = FTM_TRUE;
	pthread_join(pSNMPTRAPD->xPThread, &pRet);

	return	FTM_RET_OK;
}

FTM_RET FTOM_SNMPTRAPD_loadFromFile
(
	FTOM_SNMPTRAPD_PTR 	pSNMPTRAPD, 
	FTM_CHAR_PTR 			pFileName
)
{
	ASSERT(pSNMPTRAPD != NULL);
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	config_t			xConfig;
	config_setting_t	*pSection;

	config_init(&xConfig);
	if (config_read_file(&xConfig, pFileName) == CONFIG_FALSE)
	{
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	pSection = config_lookup(&xConfig, "snmptrapd");
	if (pSection != NULL)
	{
		config_setting_t	*pField;
		config_setting_t	*pList;

		pField = config_setting_get_member(pSection, "name");
		if (pField != NULL)
		{
			memset(pSNMPTRAPD->xConfig.pName, 0, sizeof(pSNMPTRAPD->xConfig.pName));
			strncpy(pSNMPTRAPD->xConfig.pName,  config_setting_get_string(pField), sizeof(pSNMPTRAPD->xConfig.pName) - 1);
		}

		pField = config_setting_get_member(pSection, "port");
		if (pField != NULL)
		{
			pSNMPTRAPD->xConfig.usPort =  config_setting_get_int(pField);
		}

		pField = config_setting_get_member(pSection, "target");
		if (pField != NULL)
		{
			if (strcmp(config_setting_get_string(pField), "ft") ==0)
			{
				pSNMPTRAPD->xConfig.xTarget = FTOM_SNMPTRAPD_TARGET_FT;
			}
			else if (strcmp(config_setting_get_string(pField), "tpgw") ==0)
			{
				pSNMPTRAPD->xConfig.xTarget = FTOM_SNMPTRAPD_TARGET_TPGW;
			}
			else
			{
				pSNMPTRAPD->xConfig.xTarget = FTOM_SNMPTRAPD_TARGET_UNKNOWN;
			}
		}

		pField = config_setting_get_member(pSection, "port");
		pList = config_setting_get_member(pSection, "traps");
		if (pList != NULL)
        {
			FTM_ULONG	i, ulCount;
			
			ulCount = config_setting_length(pList);
			for(i = 0 ; i < ulCount ; i++)
			{
				FTOM_SNMPTRAPD_MSG_TYPE	xMsgType;
				config_setting_t		*pElement;

				pElement = config_setting_get_elem(pList, i);
				if (pElement != NULL)
				{
					pField = config_setting_get_member(pElement, "msg");
					if (pField != NULL)
					{
						if (strcasecmp("alert", config_setting_get_string(pField)) == 0)
						{
							xMsgType = FTOM_SNMPTRAPD_MSG_TYPE_ALERT;			
						}
						else if (strcasecmp("discovery", config_setting_get_string(pField)) == 0)
						{
							xMsgType = FTOM_SNMPTRAPD_MSG_TYPE_DISCOVERY;			
						}
						else
						{
							WARN("Invalid message type[%s]\n", config_setting_get_string(pField));
							continue;	
						}
					}
					
					pField = config_setting_get_member(pElement, "oid");
					if (pField != NULL)
					{
						FTM_INT			nRet;
						FTM_SNMP_OID	xOID;

						xOID.ulOIDLen = FTM_SNMP_OID_LENGTH;

						nRet = read_objid(config_setting_get_string(pField), xOID.pOID, (size_t *)&xOID.ulOIDLen);
						if (nRet == 1)
						{
							switch(xMsgType)
							{
							case FTOM_SNMPTRAPD_MSG_TYPE_UNKNOWN:
							case FTOM_SNMPTRAPD_MSG_TYPE_EP_CHANGED:
								{
									xRet = FTOM_SNMPTRAPD_addTrapOID(pSNMPTRAPD, &xOID, FTOM_SNMPTRAPD_receiveTrap);
								}
								break;


							case	FTOM_SNMPTRAPD_MSG_TYPE_ALERT:
								{
									xRet = FTOM_SNMPTRAPD_addTrapOID(pSNMPTRAPD, &xOID, FTOM_SNMPTRAPD_alert);
								}
								break;

							case	FTOM_SNMPTRAPD_MSG_TYPE_DISCOVERY:
								{
									xRet = FTOM_SNMPTRAPD_addTrapOID(pSNMPTRAPD, &xOID, FTOM_SNMPTRAPD_discovery);
								}
								break;
							
							default:
								{
									xRet = FTM_RET_ERROR;	
								}
							}
							if (xRet == FTM_RET_OK)
							{
								TRACE("The trap OID has been registered. - %s\n", FTM_SNMP_OID_toStr(&xOID));
							}
							else
							{
								TRACE("The trap OID failed to register. - %s\n", FTM_SNMP_OID_toStr(&xOID));
							}
						}
						else
						{
							TRACE("Can't know the OID for %s.\n", config_setting_get_string(pField));
						}
					}
				}
			}
		}

	}
	config_destroy(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET FTOM_SNMPTRAPD_showConfig
(
	FTOM_SNMPTRAPD_PTR pSNMPTRAPD
)
{
	ASSERT(pSNMPTRAPD != NULL);

	MESSAGE("[ SNMPTRAPD CONFIGURATION ]\n");
	MESSAGE("%16s : %s\n", "Name", pSNMPTRAPD->xConfig.pName);
	MESSAGE("%16s : %d\n", "Port", pSNMPTRAPD->xConfig.usPort);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SNMPTRAPD_setServiceCallback
(
	FTOM_SNMPTRAPD_PTR 	pSNMPTRAPD, 
	FTOM_SERVICE_ID 		xServiceID,  
	FTOM_SERVICE_CALLBACK fServiceCB
)
{
	ASSERT(pSNMPTRAPD != NULL);
	ASSERT(fServiceCB != NULL);

	pSNMPTRAPD->xServiceID = xServiceID;
	pSNMPTRAPD->fServiceCB = fServiceCB;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SNMPTRAPD_addTrapOID
(
	FTOM_SNMPTRAPD_PTR 		pSNMPTRAPD, 
	FTM_SNMP_OID_PTR 		pOID,
	FTOM_SNMPTRAPD_CALLBACK	fTrapCB
)
{
	ASSERT(pSNMPTRAPD != NULL);
	ASSERT(pOID != NULL);

	FTM_RET			xRet;
	FTOM_TRAP_PTR 	pTrap;
	
	pTrap = (FTOM_TRAP_PTR)FTM_MEM_malloc(sizeof(FTOM_TRAP));
	if (pTrap == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(&pTrap->xOID, pOID, sizeof(FTM_SNMP_OID));
	pTrap->fCB = fTrapCB;

	TRACE("Add Trap : %s\n",FTM_SNMP_OID_toStr(pOID));
	xRet = FTM_LIST_append(&pSNMPTRAPD->xTrapCBList, pTrap);
	if (xRet != FTM_RET_OK)
	{
		ERROR("List append failed.!\n");
		FTM_MEM_free(pTrap);	
	}

	TRACE("OID : %s\n", FTM_SNMP_OID_toStr(pOID));

	return	FTM_RET_OK;
}

FTM_BOOL	FTOM_SNMPTRAPD_seekTrapCB
(
	const FTM_VOID_PTR 	pElement, 
	const FTM_VOID_PTR 	pIndicator
)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	FTOM_TRAP_PTR		pTrap = (FTOM_TRAP_PTR)pElement;
	FTM_SNMP_OID_PTR	pOID = (FTM_SNMP_OID_PTR)pIndicator;

	return	snmp_oid_compare(pTrap->xOID.pOID, pTrap->xOID.ulOIDLen, pOID->pOID, pOID->ulOIDLen) == 0;
}


static 
FTM_INT FTOM_SNMPTRAPD_preParse
(
	netsnmp_session 	*session, 
	netsnmp_transport 	*transport,
    void 				*transport_data, 
	int 				transport_data_length
)
{
#if NETSNMP_USE_LIBWRAP
	char *addr_string = NULL;

	if (transport != NULL && transport->f_fmtaddr != NULL) 
	{
		/*
		 * Okay I do know how to format this address for logging.  
		 */
		addr_string = transport->f_fmtaddr(transport, transport_data,
						transport_data_length);
		/*
		 * Don't forget to free() it.  
		 */
	}

	if (addr_string != NULL) 
	{
		/* Catch udp,udp6,tcp,tcp6 transports using "[" */
		char *tcpudpaddr = strstr(addr_string, "[");
		if ( tcpudpaddr != 0 ) 
		{
			char sbuf[64];
			char *xp;

			strlcpy(sbuf, tcpudpaddr + 1, sizeof(sbuf));
			xp = strstr(sbuf, "]");
			if (xp)
				*xp = '\0';

			if (hosts_ctl("snmptrapd", STRING_UNKNOWN, sbuf, STRING_UNKNOWN) == 0) 
			{
					TRACE("%s rejected", addr_string);
					SNMP_FREE(addr_string);
				return 0;
			}
		}
		SNMP_FREE(addr_string);
	} else 
	{
		if (hosts_ctl("snmptrapd", STRING_UNKNOWN, STRING_UNKNOWN, STRING_UNKNOWN) == 0) 
		{
			TRACE("[unknown] rejected");
			return 0;
		}
	}
#endif/*  NETSNMP_USE_LIBWRAP  */

	return 1;
}


FTM_INT
FTOM_SNMPTRAPD_inputCB
(
	FTM_INT 		nOP, 
	netsnmp_session *pSession,
	FTM_INT 		nReqID, 
	netsnmp_pdu 	*pPDU, 
	FTM_VOID_PTR 	pMagic
)
{
	FTOM_SNMPTRAPD_PTR	pSNMPTRAPD = (FTOM_SNMPTRAPD_PTR)pMagic;

    oid trapOid[MAX_OID_LEN+2] = {0};
    int trapOidLen;

	TRACE("SNMPTRAP Received.\n");
    switch (nOP) 
	{
    case NETSNMP_CALLBACK_OP_RECEIVED_MESSAGE:
		{
			/*
			 * Drops packets with reception problems
			 */
			if (pSession->s_snmp_errno) 
			{
				/* drop problem packets */
            	return 1;
        	}

        	/*
	 		 * Determine the OID that identifies the trap being handled
	 		 */
        	switch (pPDU->command) 
			{
        	case SNMP_MSG_TRAP:
				{
           			/*
	     		 	 * Convert v1 traps into a v2-style trap OID
	     		 	 *    (following RFC 2576)
	       		 	 */ 
					 if (pPDU->trap_type == SNMP_TRAP_ENTERPRISESPECIFIC) 
					 { 
						trapOidLen = pPDU->enterprise_length; 
					 	memcpy(trapOid, pPDU->enterprise, sizeof(oid) * trapOidLen);

                		if (trapOid[trapOidLen - 1] != 0) 
						{
                    		trapOid[trapOidLen++] = 0;
                		}
                		trapOid[trapOidLen++] = pPDU->specific_type;
            		} 
				}
   	        	break;

			case SNMP_MSG_TRAP2: 
			case SNMP_MSG_INFORM:
				{
    				netsnmp_variable_list *vars;

					/*
					 * v2c/v3 notifications *should* have snmpTrapOID as the
					 *    second varbind, so we can go straight there.
					 *    But check, just to make sure
					 */
					for ( vars = pPDU->variables; vars; vars=vars->next_variable) 
					{
						FTOM_TRAP_PTR	pTrap;
						FTM_SNMP_OID		xOID;

						memcpy(xOID.pOID, vars->name, sizeof(oid) * vars->name_length);
						xOID.ulOIDLen  = vars->name_length;

						if (FTM_LIST_get(&pSNMPTRAPD->xTrapCBList, &xOID, (FTM_VOID_PTR _PTR_)&pTrap) == FTM_RET_OK)
						{
							if (vars->type == ASN_OCTET_STR)
							{
								FTM_CHAR_PTR	pBuff = NULL;

								pBuff = FTM_MEM_malloc(vars->val_len + 1);
								if (pBuff == NULL)
								{
									ERROR("Not enough memory\n");	
									break;
								}

								memcpy(pBuff, vars->val.string, vars->val_len);
								pBuff[vars->val_len] = 0;
								if (pTrap->fCB != NULL)
								{
									pTrap->fCB(pSNMPTRAPD, &xOID, pBuff);
								}
								else
								{
									FTOM_SNMPTRAPD_receiveTrap(pSNMPTRAPD, &xOID, pBuff);
								}

								FTM_MEM_free(pBuff);
							}
						}	
					}
				}	
            	break;

        	default:
				{
            		/* SHOULDN'T HAPPEN! */
            		return 1;	/* ??? */
				}
			}

			if (pPDU->command == SNMP_MSG_INFORM) 
			{
				netsnmp_pdu *reply = snmp_clone_pdu(pPDU);
				if (!reply) 
				{
					snmp_log(LOG_ERR, "couldn't clone PDU for INFORM response\n");
				} 
				else 
				{
					reply->command = SNMP_MSG_RESPONSE;
					reply->errstat = 0;
					reply->errindex = 0;

					if (!snmp_send(pSession, reply)) 
					{
						FTM_CHAR_PTR	pErrMsg = NULL;

						snmp_error(pSession, NULL, NULL, &pErrMsg);
						ERROR("Couldn't respond to inform pdu - %s\n", pErrMsg);
						SNMP_FREE(pErrMsg);

						snmp_free_pdu(reply);
					}
				}
			}
		}
		break;

	case NETSNMP_CALLBACK_OP_TIMED_OUT:
		{
			ERROR("Timeout: This shouldn't happen!\n");
		}
        break;

    case NETSNMP_CALLBACK_OP_SEND_FAILED:
		{
			ERROR("Send Failed: This shouldn't happen either!\n");
		}
        break;

    case NETSNMP_CALLBACK_OP_CONNECT:
    case NETSNMP_CALLBACK_OP_DISCONNECT:
		{
        	/* Ignore silently */
		}
        break;

    default:
		{
        	ERROR("Unknown operation (%d): This shouldn't happen!\n", nOP);
		}
        break;
    }

	return	0;
}

static 
netsnmp_session * FTOM_SNMPTRAPD_addSession
(
	FTOM_SNMPTRAPD_PTR pSNMPTRAPD
)
{
	ASSERT(pSNMPTRAPD != NULL);

	netsnmp_session xSession, *pRet = NULL;

	snmp_sess_init(&xSession);
	xSession.peername 		= NULL;//SNMP_DEFAULT_PEERNAME;  /* Original code had NULL here */
	xSession.version 		= SNMP_DEFAULT_VERSION;
	xSession.community_len 	= SNMP_DEFAULT_COMMUNITY_LEN;
	xSession.retries 		= SNMP_DEFAULT_RETRIES;
	xSession.timeout 		= SNMP_DEFAULT_TIMEOUT;
	xSession.callback 		= FTOM_SNMPTRAPD_inputCB;
	xSession.callback_magic = (void *)pSNMPTRAPD;
	xSession.authenticator 	= NULL;
	xSession.isAuthoritative= SNMP_SESS_UNKNOWNAUTH;

	pRet = snmp_add(&xSession, pSNMPTRAPD->pTransport, FTOM_SNMPTRAPD_preParse, NULL);
	if (pRet == NULL) 
	{
		FTM_CHAR_PTR	pErrMsg = NULL;
		snmp_error(&xSession, NULL, NULL, &pErrMsg);
		ERROR("%s\n", pErrMsg);
		SNMP_FREE(pErrMsg);
	}
	return pRet;
}

static 
FTM_RET	FTOM_SNMPTRAPD_closeSessions
(
	FTOM_SNMPTRAPD_PTR pSNMPTRAPD, 
	netsnmp_session 	*pSessionList
)
{
	netsnmp_session *pSession = NULL;
	netsnmp_session *pNextSession = NULL;

	for (pSession = pSessionList ; pSession  != NULL; pSession = pNextSession) 
	{
		pNextSession = pSession->next;
		snmp_close(pSession);
	}

	return	FTM_RET_OK;
}

static 
FTM_VOID FTOM_SNMPTRAPD_loop
(
	FTOM_SNMPTRAPD_PTR pSNMPTRAPD
)
{
	pSNMPTRAPD->bStop = FTM_FALSE;

	while (!pSNMPTRAPD->bStop) 
	{
		FTM_INT	nFDS;
		FTM_INT	nBlock;
		FTM_INT	nCount;
		struct timeval  xTimeout = { .tv_sec = 1, .tv_usec = 0};
		fd_set 	xReadFDS, xWriteFDS, xExceptFDS;
#if 0
		if (reconfig) {
			/*
			 * If we are logging to a file, receipt of SIGHUP also
			 * indicates that the log file should be closed and
			 * re-opened.  This is useful for users that want to
			 * rotate logs in a more predictable manner.
			 */
			netsnmp_logging_restart();
			snmp_log(LOG_INFO, "NET-SNMP version %s restarted\n",
							netsnmp_get_version());
			trapd_update_config();
			if (trap1_fmt_str_remember) {
				parse_format( NULL, trap1_fmt_str_remember );
			}
			reconfig = 0;
		}
#endif
		nFDS = 0;
		nBlock = 0;
		nCount = 0;
		FD_ZERO(&xReadFDS);
		FD_ZERO(&xWriteFDS);
		FD_ZERO(&xExceptFDS);
		snmp_select_info(&nFDS, &xReadFDS, &xTimeout, &nBlock);

		nCount = select(nFDS, &xReadFDS, &xWriteFDS, &xExceptFDS, &xTimeout);
		if (nCount > 0) 
		{
			snmp_read(&xReadFDS);
		} 
		else  if (nCount == 0)
		{
			//snmp_timeout();
		}
		else
		{
			switch (nCount) 
			{
			case -1:
				{
					if (errno == EINTR)
					{
						continue;
					}
					ERROR("select - %s\n", strerror(errno));
				}
				break;

			default:
				{
					ERROR("select returned %d\n", nCount);
				}
			}
			pSNMPTRAPD->bStop = FTM_TRUE;
		}
		run_alarms();
	}

	TRACE("exit SNMP Trapd mainloop!\n");
}

FTM_VOID_PTR	FTOM_SNMPTRAPD_process
(
	FTM_VOID_PTR pData
)
{
	ASSERT(pData != NULL);

	FTOM_SNMPTRAPD_PTR	pSNMPTRAPD = (FTOM_SNMPTRAPD_PTR)pData;

    netsnmp_session		*pSessionList = NULL;
    netsnmp_session		*pSession = NULL;
	FTM_CHAR			pPort[16];

	sprintf(pPort, "udp:%d", pSNMPTRAPD->xConfig.usPort);
	pSNMPTRAPD->pTransport = netsnmp_transport_open_server(pSNMPTRAPD->xConfig.pName, pPort); 
	if (pSNMPTRAPD->pTransport == NULL) 
	{
		ERROR("Couldn't open %d -- errno %d(\"%s\")\n", 
				pSNMPTRAPD->xConfig.usPort,
				errno, strerror(errno));
		FTOM_SNMPTRAPD_closeSessions(pSNMPTRAPD, pSessionList);
		SOCK_CLEANUP;

		return	0;
	} 
	else 
	{
		pSession  = FTOM_SNMPTRAPD_addSession(pSNMPTRAPD);
		if (pSession == NULL) 
		{
			/*   
			 * Shouldn't happen?  We have already opened the transport
			 * successfully so what could have gone wrong?  
			 */
			FTOM_SNMPTRAPD_closeSessions(pSNMPTRAPD, pSessionList);
			netsnmp_transport_free(pSNMPTRAPD->pTransport);
			pSNMPTRAPD->pTransport = NULL;
			ERROR("couldn't open snmp - %s", strerror(errno));
			SOCK_CLEANUP;

			return	0;
		} 
		else 
		{
			pSession->next = pSessionList;
			pSessionList = pSession;
		}    
	}    

	FTOM_SNMPTRAPD_loop(pSNMPTRAPD);

	return  FTM_RET_OK;

}

FTM_RET	FTOM_SNMPTRAPD_receiveTrap
(
	FTOM_SNMPTRAPD_PTR 	pSNMPTRAPD, 
	FTM_SNMP_OID_PTR	pOID,
	FTM_CHAR_PTR 		pMsg
)
{
	ASSERT(pSNMPTRAPD != NULL);
	ASSERT(pMsg != NULL);
	
	FTM_RET			xRet;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTOM_EP_PTR		pEP = NULL;
	FTM_EP_DATA		xData;
	FTOM_SNMPTRAPD_MSG_TYPE	xMsgType = FTOM_SNMPTRAPD_MSG_TYPE_UNKNOWN;	
	const nx_json 	*pRoot, *pItem;

	TRACE("SNMPTRAPD : %s\n", pMsg);

	pRoot = nx_json_parse_utf8(pMsg);
	if (pRoot == NULL)
	{
		ERROR("Invalid trap message[%s]\n", pMsg);
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	pItem = nx_json_get(pRoot, "type");
	if (pItem->type == NX_JSON_NULL)
	{
		xMsgType = FTOM_SNMPTRAPD_MSG_TYPE_EP_CHANGED;	
	}
	else
	{
		
		if (strcasecmp(pItem->text_value, "ep_changed") == 0)
		{
			xMsgType = FTOM_SNMPTRAPD_MSG_TYPE_EP_CHANGED;	
		
		}
	}

	switch(xMsgType)
	{
	case	FTOM_SNMPTRAPD_MSG_TYPE_EP_CHANGED:
		{
			pItem = nx_json_get(pRoot, "id");
			if (pItem->type != NX_JSON_NULL)
			{
				strcpy(pEPID, pItem->text_value);
	
				xRet = FTOM_EP_get(pEPID, &pEP);
				if (xRet == FTM_RET_OK)
				{
					FTM_EP_DATA_TYPE	xDataType;
					FTOM_EP_PTR			pEP;

					FTOM_EP_getDataType(pEP, &xDataType);
		
					pItem = nx_json_get(pRoot, "value");
					if (pItem->type != NX_JSON_NULL)
					{
						switch(pItem->type)
						{
						case	NX_JSON_STRING:
							{
								TRACE("VALUE : %s\n", pItem->text_value);
								FTM_EP_DATA_init(&xData, xDataType, (FTM_CHAR_PTR)pItem->text_value);
							}
							break;
		
						case 	NX_JSON_INTEGER:
						case	NX_JSON_BOOL:
							{
								TRACE("VALUE : %d\n", pItem->int_value);
								FTM_EP_DATA_init(&xData, xDataType, NULL);

								switch(xData.xType)
								{
								case	FTM_EP_DATA_TYPE_INT:
									{
										FTM_EP_DATA_initINT(&xData, pItem->int_value);
									}
									break;

								case	FTM_EP_DATA_TYPE_ULONG:
									{
										FTM_EP_DATA_initULONG(&xData, pItem->int_value);
									}
									break;
		
								case	FTM_EP_DATA_TYPE_FLOAT:
									{
										FTM_EP_DATA_initFLOAT(&xData, pItem->int_value);
									}
									break;
								
								case	FTM_EP_DATA_TYPE_BOOL:
									{
										FTM_EP_DATA_initBOOL(&xData, pItem->int_value);
									}
									break;
								}

							}
							break;
		
						case	NX_JSON_DOUBLE:
							{
								TRACE("VALUE : %lu\n", pItem->dbl_value);
								xData.xType = xDataType;
								switch(xData.xType)
								{
								case	FTM_EP_DATA_TYPE_INT:
									{
										FTM_EP_DATA_initINT(&xData, pItem->dbl_value);
									}
									break;

								case	FTM_EP_DATA_TYPE_ULONG:
									{
										FTM_EP_DATA_initULONG(&xData, pItem->dbl_value);
									}
									break;
		
								case	FTM_EP_DATA_TYPE_FLOAT:
									{
										FTM_EP_DATA_initFLOAT(&xData, pItem->dbl_value);
									}
									break;
								
								case	FTM_EP_DATA_TYPE_BOOL:
									{
										FTM_EP_DATA_initBOOL(&xData, pItem->dbl_value);
									}
									break;
								}
							}
							break;
		
						default:
							{
								ERROR("Invalid value type[%d].\n", pItem->type);
							}
							break;
						}
		
						pItem = nx_json_get(pRoot, "time");
						if (pItem->type != NX_JSON_NULL)
						{
							xData.ulTime = (FTM_ULONG)pItem->int_value;
						}
		
						pItem = nx_json_get(pRoot, "state");
						if (pItem->type != NX_JSON_NULL)
						{
							if (pItem->type == NX_JSON_STRING)
							{
								if (strcasecmp(pItem->text_value, "enable") == 0)
								{
									xData.xState = FTM_EP_STATE_RUN;
								}
								else if (strcasecmp(pItem->text_value, "disable") == 0)
								{
									xData.xState = FTM_EP_STATE_STOP;
								}
								else if (strcasecmp(pItem->text_value, "error") == 0)
								{
									xData.xState = FTM_EP_STATE_ERROR;
								}
							}
						}
				

						xRet = FTOM_EP_get(pEPID, &pEP);
						if (xRet == FTM_RET_OK)
						{
							xRet = FTOM_EP_setData(pEP, &xData);	
						}
						else
						{
							ERROR("Notify failed.\n");	
						}
					}
					else
					{
							
						xRet = FTM_RET_INVALID_DATA;
						ERROR("TRAP : Value is not exist.\n");
					}
				}
				else
				{
					xRet = FTM_RET_OBJECT_NOT_FOUND;
					ERROR("Can't found EP[%s]\n", pEPID);	
				}
			}
			else
			{
				xRet = FTM_RET_INVALID_DATA;
				ERROR("TRAP : ID is not exist.\n");
			}
		}
		break;

	default:
		{
			xRet = FTM_RET_INVALID_DATA;
		}
	}

	nx_json_free(pRoot);

	return	xRet;

}
FTM_RET	FTOM_SNMPTRAPD_alert
(
	FTOM_SNMPTRAPD_PTR 	pSNMPTRAPD, 
	FTM_SNMP_OID_PTR	pOID,
	FTM_CHAR_PTR 		pMsg
)
{
	ASSERT(pSNMPTRAPD != NULL);
	ASSERT(pMsg != NULL);
	
	FTM_RET			xRet;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTOM_EP_PTR		pEP = NULL;
	FTM_EP_DATA_TYPE	xDataType;
	FTM_EP_DATA		xData;
	const nx_json 	*pRoot, *pItem;

	TRACE("ALERT : %s\n", pMsg);

	pRoot = nx_json_parse_utf8(pMsg);
	if (pRoot == NULL)
	{
		ERROR("Invalid trap message[%s]\n", pMsg);
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	pItem = nx_json_get(pRoot, "id");
	if (pItem->type == NX_JSON_NULL)
	{
		xRet = FTM_RET_SNMP_INVALID_MESSAGE_FORMAT;
		goto error;
	}

	strncpy(pEPID, pItem->text_value, FTM_EPID_LEN);

	xRet = FTOM_EP_get(pEPID, &pEP);
	if (xRet != FTM_RET_OK)
	{
		WARN("EP[%s] does not exist!\n",pEPID);
		return	xRet;
	}

	FTOM_EP_getDataType(pEP, &xDataType);
		
	pItem = nx_json_get(pRoot, "value");
	if (pItem->type != NX_JSON_NULL)
	{
		switch(pItem->type)
		{
		case	NX_JSON_STRING:
			{
				TRACE("VALUE : %s\n", pItem->text_value);
				FTM_EP_DATA_init(&xData, xDataType, (FTM_CHAR_PTR)pItem->text_value);
			}
			break;
		
		case 	NX_JSON_INTEGER:
		case	NX_JSON_BOOL:
			{
				TRACE("VALUE : %d\n", pItem->int_value);
				switch(xDataType)
				{
				case	FTM_EP_DATA_TYPE_INT:
					{
						FTM_EP_DATA_initINT(&xData, pItem->int_value);
					}
					break;
		
				case	FTM_EP_DATA_TYPE_ULONG:
					{
						FTM_EP_DATA_initULONG(&xData, pItem->int_value);
					}
					break;
		
				case	FTM_EP_DATA_TYPE_FLOAT:
					{
						FTM_EP_DATA_initFLOAT(&xData, pItem->int_value);
					}
					break;
								
				case	FTM_EP_DATA_TYPE_BOOL:
					{
						FTM_EP_DATA_initBOOL(&xData, pItem->int_value);
					}
					break;
				}
			}
			break;
		
		case	NX_JSON_DOUBLE:
			{
				TRACE("VALUE : %lu\n", pItem->dbl_value);
				switch(xDataType)
				{
				case	FTM_EP_DATA_TYPE_INT:
					{
						FTM_EP_DATA_initINT(&xData, pItem->dbl_value);
					}
					break;
		
				case	FTM_EP_DATA_TYPE_ULONG:
					{
						FTM_EP_DATA_initULONG(&xData, pItem->dbl_value);
					}
					break;
		
				case	FTM_EP_DATA_TYPE_FLOAT:
					{
						FTM_EP_DATA_initFLOAT(&xData, pItem->dbl_value);
					}
					break;

				case	FTM_EP_DATA_TYPE_BOOL:
					{
						FTM_EP_DATA_initBOOL(&xData, pItem->dbl_value);
					}
					break;
				}
			}
			break;
		
		default:
			{
				ERROR("Invalid value type[%d].\n", pItem->type);
			}
			break;
		}
		
		pItem = nx_json_get(pRoot, "time");
		if (pItem->type != NX_JSON_NULL)
		{
			xData.ulTime = (FTM_ULONG)pItem->int_value;
		}
		
		pItem = nx_json_get(pRoot, "state");
		if (pItem->type != NX_JSON_NULL)
		{
			if (pItem->type == NX_JSON_STRING)
			{
				if (strcasecmp(pItem->text_value, "enable") == 0)
				{
					xData.xState = FTM_EP_STATE_RUN;
				}
				else if (strcasecmp(pItem->text_value, "disable") == 0)
				{
					xData.xState = FTM_EP_STATE_STOP;
				}
				else if (strcasecmp(pItem->text_value, "error") == 0)
				{
					xData.xState = FTM_EP_STATE_ERROR;
				}
			}
		}
						
		xRet = FTOM_SNMPTRAPD_sendAlert(pSNMPTRAPD, pEPID, &xData);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Notify failed.\n");	
		}
	}
	else
	{
		xRet = FTM_RET_INVALID_DATA;
		ERROR("TRAP : Value is not exist.\n");
	}

error:
	nx_json_free(pRoot);

	return	xRet;

}
FTM_RET	FTOM_SNMPTRAPD_discovery
(
	FTOM_SNMPTRAPD_PTR 	pSNMPTRAPD, 
	FTM_SNMP_OID_PTR	pOID,
	FTM_CHAR_PTR 		pMsg
)
{
	ASSERT(pSNMPTRAPD != NULL);
	ASSERT(pMsg != NULL);
	
	FTM_RET			xRet;
	FTM_INT			i;
	FTM_ULONG		ulDIDLen;
	FTM_CHAR		pName[FTM_DEVICE_NAME_LEN + 1];
	FTM_CHAR		pDID[FTM_DID_LEN + 1];
	FTM_CHAR		pDeviceIP[32];
	FTM_EP_TYPE		pEPTypes[32];
	FTM_ULONG		ulEPTypes = 0;
	const nx_json 	*pRoot, *pItem, *pOIDs;

	TRACE("DISCOVERY : %s\n", pMsg);

	pRoot = nx_json_parse_utf8(pMsg);
	if (pRoot == NULL)
	{
		ERROR("Invalid trap message[%s]\n", pMsg);
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	pItem = nx_json_get(pRoot, "id");
	if (pItem->type != NX_JSON_STRING)
	{
		ERROR("Invalid message format!\n");
		xRet = FTM_RET_SNMP_INVALID_MESSAGE_FORMAT;
		goto error;
	}
	memset(pName, 0, sizeof(pName));
	strncpy(pName, pItem->text_value, FTM_DEVICE_NAME_LEN);


	memset(pDeviceIP, 0, sizeof(pDeviceIP));
	pItem = nx_json_get(pRoot, "ip");
	if (pItem->type != NX_JSON_NULL)
	{ 
		if (pItem->type != NX_JSON_STRING)
		{
			ERROR("Invalid message format!\n");
			xRet = FTM_RET_SNMP_INVALID_MESSAGE_FORMAT;
			goto error;
		}
		strncpy(pDeviceIP, pItem->text_value, sizeof(pDeviceIP) - 1);
	}

	pItem = nx_json_get(pRoot, "mac");
	if ((pItem->type != NX_JSON_STRING) || strlen(pItem->text_value) != 17)
	{
		ERROR("Invalid message format!\n");
		xRet = FTM_RET_SNMP_INVALID_MESSAGE_FORMAT;
		goto error;
	}

	ulDIDLen  = 0;
	memset(pDID, 0, sizeof(pDID));
	for(i = 0 ; i < strlen(pItem->text_value); i++)
	{
		if (((i + 1) % 3) != 0)
		{
			pDID[ulDIDLen++] = toupper(pItem->text_value[i]);	
		}
	}

	pOIDs = nx_json_get(pRoot, "oids");
	if (pOIDs->type != NX_JSON_ARRAY)
	{
		ERROR("Invalid message format!\n");
		xRet = FTM_RET_SNMP_INVALID_MESSAGE_FORMAT;
		goto error;
	}

	ulEPTypes = 0;

	for(i = 0 ; i < pOIDs->length ; i++)
	{
		FTM_ULONG	ulType;

		pItem = nx_json_item(pOIDs, i);
		if(pOIDs->type == NX_JSON_NULL)
		{
			continue;		
		}

		if (pItem->type == NX_JSON_STRING)
		{
			ulType = strtol(pItem->text_value, NULL, 10);
		}
		else if (pItem->type == NX_JSON_INTEGER)
		{
			ulType = pItem->int_value;
		}
		else
		{
			continue;	
		}

		pEPTypes[ulEPTypes++] = (ulType << 16);
	}	
	
	FTOM_SNMPTRAPD_receivedDiscovery(pSNMPTRAPD, pName, pDID, pDeviceIP, pEPTypes, ulEPTypes);

error:

	nx_json_free(pRoot);

	return	xRet;

}

FTM_RET	FTOM_SNMPTRAPD_sendAlert
(
	FTOM_SNMPTRAPD_PTR 	pSNMPTRAPD, 
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR 	pData
)
{
	ASSERT(pSNMPTRAPD != NULL);
	ASSERT(pData != NULL);

	return	FTOM_sendAlert(pEPID, pData);
}

FTM_RET	FTOM_SNMPTRAPD_receivedDiscovery
(
	FTOM_SNMPTRAPD_PTR	pSNMPTRAPD,
	FTM_CHAR_PTR	pName,
	FTM_CHAR_PTR	pDID,
	FTM_CHAR_PTR	pIP,
	FTM_EP_TYPE_PTR	pTypes,
	FTM_ULONG		ulCount
)
{
	ASSERT(pSNMPTRAPD != NULL);
	ASSERT(pDID != NULL);
	ASSERT(pTypes != NULL);

	return	FTOM_receivedDiscovery(pName, pDID, pIP, pTypes, ulCount);
}

#if 0
FTM_RET	FTOM_SNMPTRAPD_dumpPDU(netsnmp_pdu 	*pPDU) 
{
    netsnmp_variable_list *vars;

	for(vars = pPDU->variables; vars ; vars = vars->next_variable)
	{
		int	i;
		char	pBuff[1024];

		memset(pBuff, 0, sizeof(pBuff));
		snprint_objid(pBuff, sizeof(pBuff) - 1, vars->name, vars->name_length);
		//MESSAGE("%6s : %S\n", "NAME", pBuff);
		MESSAGE("%6s : ", "NAME");
		for(i = 0 ; i < vars->name_length ; i++)
		{
			MESSAGE(".%d", vars->name[i]);
		}
		MESSAGE("\n");
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
		MESSAGE("%6s : %s\n", "VALUE", pBuff);
	}

	return	FTM_RET_OK;
}

#endif
