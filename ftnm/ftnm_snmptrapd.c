#include <stdlib.h>
#include <errno.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/output_api.h>
#include "nxjson.h"

#include "libconfig.h"
#include "ftm.h"
#include "ftnm.h"
#include "ftnm_msg.h"
#include "ftnm_snmptrapd.h"
#include "ftnm_node_snmpc.h"
#include "ftnm_dmc.h"
#include "ftnm_ep.h"
#include "ftnm_ep_class.h"

typedef	struct
{
	FTNM_SNMP_OID				xOID;
	FTNM_SNMPTRAPD_CALLBACK		fCallback;
} FTNM_CALLBACK, _PTR_ FTNM_CALLBACK_PTR;

static FTM_VOID_PTR		FTNM_SNMPTRAPD_process(FTM_VOID_PTR pData);
static netsnmp_session* FTNM_SNMPTRAPD_addSession(FTNM_SNMPTRAPD_PTR pCTX);
static FTM_RET			FTNM_SNMPTRAPD_closeSessions(FTNM_SNMPTRAPD_PTR pCTX, netsnmp_session * pSessionList);
static FTM_BOOL			FTNM_SNMPTRAPD_seekTrapCB(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
static FTM_RET			FTNM_SNMPTRAPD_receiveTrap(FTNM_SNMPTRAPD_PTR pCTX, FTM_CHAR_PTR pMsg);
//static FTM_RET		FTNM_SNMPTRAPD_dumpPDU(netsnmp_pdu 	*pPDU) ;

FTM_RET	FTNM_SNMPTRAPD_init(FTNM_SNMPTRAPD_PTR pCTX)
{
	FTM_RET	xRet;

	memset(pCTX, 0, sizeof(FTNM_SNMPTRAPD));
	strcpy(pCTX->xConfig.pName, FTNM_SNMPTRAPD_NAME);
	pCTX->xConfig.usPort= FTNM_SNMPTRAPD_PORT;

	xRet = FTM_LIST_init(&pCTX->xTrapCBList);
	if (xRet == FTM_RET_OK)
	{
		FTM_LIST_setSeeker(&pCTX->xTrapCBList, FTNM_SNMPTRAPD_seekTrapCB);
	}

	return	xRet;
}

FTM_RET	FTNM_SNMPTRAPD_final(FTNM_SNMPTRAPD_PTR pCTX)
{
	ASSERT(pCTX != NULL);
	FTNM_CALLBACK_PTR	pCB;

	if (pCTX->bStop)
	{
		FTNM_SNMPTRAPD_stop(pCTX);	
	}

	FTM_LIST_iteratorStart(&pCTX->xTrapCBList);
	while(FTM_LIST_iteratorNext(&pCTX->xTrapCBList, (FTM_VOID_PTR _PTR_)&pCB) == FTM_RET_OK)
	{
		FTM_LIST_remove(&pCTX->xTrapCBList, pCB);
		FTM_MEM_free(pCB);
	}

	FTM_LIST_final(&pCTX->xTrapCBList);

	return	FTM_RET_OK;
}

FTM_RET FTNM_SNMPTRAPD_start(FTNM_SNMPTRAPD_PTR pCTX)
{
	ASSERT(pCTX != NULL);
	FTM_INT	nRet;

	nRet = pthread_create(&pCTX->xPThread, NULL, FTNM_SNMPTRAPD_process, pCTX);
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

	return	FTM_RET_OK;
}

FTM_RET FTNM_SNMPTRAPD_stop(FTNM_SNMPTRAPD_PTR pCTX)
{
	ASSERT(pCTX != NULL);
	FTM_INT	nRet;
	void*	pRet;

	pCTX->bStop = FTM_TRUE;
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

FTM_RET FTNM_SNMPTRAPD_loadFromFile(FTNM_SNMPTRAPD_PTR pCTX, FTM_CHAR_PTR pFileName)
{
	ASSERT(pCTX != NULL);
	ASSERT(pFileName != NULL);

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
			memset(pCTX->xConfig.pName, 0, sizeof(pCTX->xConfig.pName));
			strncpy(pCTX->xConfig.pName,  config_setting_get_string(pField), sizeof(pCTX->xConfig.pName) - 1);
		}

		pField = config_setting_get_member(pSection, "port");
		if (pField != NULL)
		{
			pCTX->xConfig.usPort =  config_setting_get_int(pField);
		}

		pList = config_setting_get_member(pSection, "traps");
		if (pList != NULL)
        {
			FTM_ULONG	i, ulCount;
			
			ulCount = config_setting_length(pList);
			for(i = 0 ; i < ulCount ; i++)
			{
				pField = config_setting_get_elem(pList, i);
				if (pField != NULL)
				{
					FTNM_SNMP_OID	xOID;
					
					xOID.ulOIDLen = FTNM_SNMP_OID_LENGTH;

					MESSAGE("pObID = %s\n", config_setting_get_string(pField));
					if (read_objid(config_setting_get_string(pField), xOID.pOID, (size_t *)&xOID.ulOIDLen) == 1)
					{
						MESSAGE("SNMP_PARSE_OID success!\n");
						FTNM_SNMPTRAPD_addTrapOID(pCTX, &xOID);
					}
				
				}
			}
		}

	}
	config_destroy(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET FTNM_SNMPTRAPD_showConfig(FTNM_SNMPTRAPD_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	MESSAGE("[ SNMPTRAPD CONFIGURATION ]\n");
	MESSAGE("%16s : %s\n", "Name", pCTX->xConfig.pName);
	MESSAGE("%16s : %d\n", "Port", pCTX->xConfig.usPort);

	return	FTM_RET_OK;
}

FTM_RET	FTNM_SNMPTRAPD_setServiceCallback(FTNM_SNMPTRAPD_PTR pCTX, FTNM_SERVICE_ID xServiceID,  FTNM_SERVICE_CALLBACK fServiceCB)
{
	ASSERT(pCTX != NULL);
	ASSERT(fServiceCB != NULL);

	pCTX->xServiceID = xServiceID;
	pCTX->fServiceCB = fServiceCB;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_SNMPTRAPD_addTrapOID(FTNM_SNMPTRAPD_PTR pCTX, FTNM_SNMP_OID_PTR pOID)
{
	ASSERT(pCTX != NULL);
	ASSERT(pOID != NULL);

	FTM_RET				xRet;
	FTNM_CALLBACK_PTR 	pCB;
	
	pCB = (FTNM_CALLBACK_PTR)FTM_MEM_malloc(sizeof(FTNM_CALLBACK));
	if (pCB == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(&pCB->xOID, pOID, sizeof(FTNM_SNMP_OID));

	xRet = FTM_LIST_append(&pCTX->xTrapCBList, pCB);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pCB);	
	}

	return	FTM_RET_OK;
}

FTM_BOOL	FTNM_SNMPTRAPD_seekTrapCB(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	FTNM_CALLBACK_PTR	pCB = (FTNM_CALLBACK_PTR)pElement;
	FTNM_SNMP_OID_PTR	pOID = (FTNM_SNMP_OID_PTR)pIndicator;

	return	snmp_oid_compare(pCB->xOID.pOID, pCB->xOID.ulOIDLen, pOID->pOID, pOID->ulOIDLen) == 0;
}


static 
FTM_INT FTNM_SNMPTRAPD_preParse
(
	netsnmp_session 	*session, 
	netsnmp_transport 	*transport,
    void 				*transport_data, 
	int 				transport_data_length
)
{
#if NETSNMP_USE_LIBWRAP
	char *addr_string = NULL;

	if (transport != NULL && transport->f_fmtaddr != NULL) {
		/*
		 * Okay I do know how to format this address for logging.  
		 */
		addr_string = transport->f_fmtaddr(transport, transport_data,
						transport_data_length);
		/*
		 * Don't forget to free() it.  
		 */
	}

	if (addr_string != NULL) {
		/* Catch udp,udp6,tcp,tcp6 transports using "[" */
		char *tcpudpaddr = strstr(addr_string, "[");
		if ( tcpudpaddr != 0 ) {
			char sbuf[64];
			char *xp;

			strlcpy(sbuf, tcpudpaddr + 1, sizeof(sbuf));
			xp = strstr(sbuf, "]");
			if (xp)
				*xp = '\0';

			if (hosts_ctl("snmptrapd", STRING_UNKNOWN,
							sbuf, STRING_UNKNOWN) == 0) {
					TRACE("%s rejected", addr_string);
					SNMP_FREE(addr_string);
				return 0;
			}
		}
		SNMP_FREE(addr_string);
	} else {
		if (hosts_ctl("snmptrapd", STRING_UNKNOWN,
						STRING_UNKNOWN, STRING_UNKNOWN) == 0) {
			TRACE("[unknown] rejected");
			return 0;
		}
	}
#endif/*  NETSNMP_USE_LIBWRAP  */

	return 1;
}


FTM_INT
FTNM_SNMPTRAPD_inputCB
(
	FTM_INT 		nOP, 
	netsnmp_session *pSession,
	FTM_INT 		nReqID, 
	netsnmp_pdu 	*pPDU, 
	FTM_VOID_PTR 	pMagic
)
{
	FTNM_SNMPTRAPD_PTR	pCTX = (FTNM_SNMPTRAPD_PTR)pMagic;

    oid trapOid[MAX_OID_LEN+2] = {0};
    int trapOidLen;

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
						FTNM_CALLBACK_PTR	pCB;
						FTNM_SNMP_OID		xOID;
						memcpy(xOID.pOID, vars->name, sizeof(oid) * vars->name_length);
						xOID.ulOIDLen  = vars->name_length;
						if (FTM_LIST_get(&pCTX->xTrapCBList, &xOID, (FTM_VOID_PTR _PTR_)&pCB) == FTM_RET_OK)
						{
							if (vars->type == ASN_OCTET_STR)
							{
								FTM_CHAR	pBuff[1024];
		
								memcpy(pBuff, vars->val.string, vars->val_len);
								pBuff[vars->val_len] = 0;
								FTNM_SNMPTRAPD_receiveTrap(pCTX, pBuff);
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
netsnmp_session * FTNM_SNMPTRAPD_addSession(FTNM_SNMPTRAPD_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	netsnmp_session xSession, *pRet = NULL;

	snmp_sess_init(&xSession);
	xSession.peername 		= SNMP_DEFAULT_PEERNAME;  /* Original code had NULL here */
	xSession.version 		= SNMP_DEFAULT_VERSION;
	xSession.community_len 	= SNMP_DEFAULT_COMMUNITY_LEN;
	xSession.retries 		= SNMP_DEFAULT_RETRIES;
	xSession.timeout 		= SNMP_DEFAULT_TIMEOUT;
	xSession.callback 		= FTNM_SNMPTRAPD_inputCB;
	xSession.callback_magic = (void *)pCTX;
	xSession.authenticator 	= NULL;
	xSession.isAuthoritative= SNMP_SESS_UNKNOWNAUTH;

	pRet = snmp_add(&xSession, pCTX->pTransport, FTNM_SNMPTRAPD_preParse, NULL);
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
FTM_RET	FTNM_SNMPTRAPD_closeSessions(FTNM_SNMPTRAPD_PTR pCTX, netsnmp_session * pSessionList)
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

static void
FTNM_SNMPTRAPD_mainLoop(FTNM_SNMPTRAPD_PTR pCTX)
{
	int             count, numfds, block;
	fd_set          readfds,writefds,exceptfds;
	struct timeval  timeout, *tvp;
	
	pCTX->bStop = FTM_FALSE;

	while (!pCTX->bStop) 
	{
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
		numfds = 0;
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&exceptfds);
		block = 0;
		tvp = &timeout;
		timerclear(tvp);
		tvp->tv_sec = 1;
		snmp_select_info(&numfds, &readfds, tvp, &block);
		if (block == 1)
		{
			tvp = NULL;         /* block without timeout */
		}
#ifndef NETSNMP_FEATURE_REMOVE_FD_EVENT_MANAGER
		netsnmp_external_event_info(&numfds, &readfds, &writefds, &exceptfds);
#endif /* NETSNMP_FEATURE_REMOVE_FD_EVENT_MANAGER */
		count = select(numfds, &readfds, &writefds, &exceptfds, tvp);
		if (count > 0) 
		{
#ifndef NETSNMP_FEATURE_REMOVE_FD_EVENT_MANAGER
			netsnmp_dispatch_external_events(&count, &readfds, &writefds,
										&exceptfds);
#endif /* NETSNMP_FEATURE_REMOVE_FD_EVENT_MANAGER */
	/* If there are any more events after external events, then
					 * try SNMP events. */
			if (count > 0) 
			{
					snmp_read(&readfds);
			}
		} 
		else 
		{
			switch (count) 
			{
			case 0:
				{
					snmp_timeout();
				}
				break;

			case -1:
				{
					if (errno == EINTR)
					{
						continue;
					}
					ERROR("select - %s\n", strerror(errno));
					pCTX->bStop = FTM_TRUE;
				}
				break;

			default:
				{
					ERROR("select returned %d\n", count);
					pCTX->bStop = FTM_TRUE;
				}
			}
		}
		run_alarms();
	}

	TRACE("exit SNMP Trapd mainloop!\n");
}

FTM_VOID_PTR	FTNM_SNMPTRAPD_process(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);

	FTNM_SNMPTRAPD_PTR	pCTX = (FTNM_SNMPTRAPD_PTR)pData;

    netsnmp_session		*pSessionList = NULL;
    netsnmp_session		*pSession = NULL;
	FTM_CHAR			pPort[16];

	sprintf(pPort, "%d", pCTX->xConfig.usPort);
	pCTX->pTransport = netsnmp_transport_open_server(pCTX->xConfig.pName, pPort); 
	if (pCTX->pTransport == NULL) 
	{
		ERROR("Couldn't open %d -- errno %d(\"%s\")\n", 
				pCTX->xConfig.usPort,
				errno, strerror(errno));
		FTNM_SNMPTRAPD_closeSessions(pCTX, pSessionList);
		SOCK_CLEANUP;

		return	0;
	} 
	else 
	{
		pSession  = FTNM_SNMPTRAPD_addSession(pCTX);
		if (pSession == NULL) 
		{
			/*   
			 * Shouldn't happen?  We have already opened the transport
			 * successfully so what could have gone wrong?  
			 */
			FTNM_SNMPTRAPD_closeSessions(pCTX, pSessionList);
			netsnmp_transport_free(pCTX->pTransport);
			pCTX->pTransport = NULL;
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

	FTNM_SNMPTRAPD_mainLoop(pCTX);

	return  FTM_RET_OK;

}

FTM_RET	FTNM_SNMPTRAPD_receiveTrap(FTNM_SNMPTRAPD_PTR pCTX, FTM_CHAR_PTR pMsg)
{
	ASSERT(pCTX != NULL);
	ASSERT(pMsg != NULL);
	
	FTM_RET					xRet;
	FTM_EP_ID				xEPID = 0;
	FTNM_EP_PTR				pEP = NULL;
	FTM_EP_DATA				xData;
	FTNM_SNMPTRAPD_MSG_TYPE	xMsgType = FTNM_SNMPTRAPD_MSG_TYPE_UNKNOWN;	
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
		xMsgType = FTNM_SNMPTRAPD_MSG_TYPE_EP_CHANGED;	
	}
	else
	{
		
		if (strcasecmp(pItem->text_value, "ep_changed") == 0)
		{
			xMsgType = FTNM_SNMPTRAPD_MSG_TYPE_EP_CHANGED;	
		
		}
	}

	switch(xMsgType)
	{
	case	FTNM_SNMPTRAPD_MSG_TYPE_EP_CHANGED:
		{
			pItem = nx_json_get(pRoot, "id");
			if (pItem->type != NX_JSON_NULL)
			{
				xEPID = strtoul(pItem->text_value, 0, 16);
	
				xRet = FTNM_EP_get(xEPID, &pEP);
				if (xRet == FTM_RET_OK)
				{
					FTNM_EP_getData(pEP, &xData);
		
					pItem = nx_json_get(pRoot, "value");
					if (pItem->type != NX_JSON_NULL)
					{
						switch(pItem->type)
						{
						case	NX_JSON_STRING:
							{
								switch(xData.xType)
								{
								case	FTM_EP_DATA_TYPE_INT:
									{
										xData.xValue.nValue = strtol(pItem->text_value, NULL, 10);
									}
									break;
		
								case	FTM_EP_DATA_TYPE_ULONG:
									{
										xData.xValue.ulValue = strtoul(pItem->text_value, NULL, 10);
									}
									break;
		
								case	FTM_EP_DATA_TYPE_FLOAT:
									{
										xData.xValue.fValue = atof(pItem->text_value);
									}
									break;
								}
		
							}
							break;
		
						case 	NX_JSON_INTEGER:
						case	NX_JSON_BOOL:
							{
								xData.xValue.nValue = pItem->int_value;
							}
							break;
		
						case	NX_JSON_DOUBLE:
							{
								switch(xData.xType)
								{
								case	FTM_EP_DATA_TYPE_INT:
									{
										xData.xValue.nValue = (FTM_INT)pItem->dbl_value;
									}
									break;
		
								case	FTM_EP_DATA_TYPE_ULONG:
									{
										xData.xValue.ulValue = (FTM_ULONG)pItem->dbl_value;
									}
									break;
		
								case	FTM_EP_DATA_TYPE_FLOAT:
									{
										xData.xValue.fValue = pItem->dbl_value;
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

						if (pCTX->fServiceCB != NULL)
						{
							FTNM_MSG_EP_CHANGED_PARAMS xParam;
							
							xParam.xEPID = xEPID;
							memcpy(&xParam.xData, &xData, sizeof(xData));

							pCTX->fServiceCB(pCTX->xServiceID, FTNM_MSG_TYPE_EP_CHANGED, &xParam);
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
					ERROR("Can't found EP[%08x]\n", xEPID);	
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

#if 0
FTM_RET	FTNM_SNMPTRAPD_dumpPDU(netsnmp_pdu 	*pPDU) 
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
