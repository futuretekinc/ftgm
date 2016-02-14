#include <stdlib.h>
#include <errno.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>

#include "libconfig.h"
#include "ftm_mem.h"
#include "ftnm.h"
#include "ftnm_snmptrapd.h"
#include "ftnm_node_snmpc.h"
#include "ftnm_dmc.h"
#include "ftnm_ep.h"
#include "ftnm_ep_class.h"

static FTM_VOID_PTR	FTNM_SNMPTRAPD_process(FTM_VOID_PTR pData);
static FTM_RET		FTNM_SNMPTRAPD_dumpPDU(netsnmp_pdu 	*pPDU) ;

FTM_RET	FTNM_SNMPTRAPD_create(FTNM_SNMPTRAPD_PTR _PTR_ ppCTX)
{
	ASSERT(ppCTX != NULL);
	FTNM_SNMPTRAPD_PTR	pCTX;

	pCTX = (FTNM_SNMPTRAPD_PTR)FTM_MEM_malloc(sizeof(FTNM_SNMPTRAPD));
	if (pCTX == NULL)
	{
		ERROR("Not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	memset(pCTX, 0, sizeof(FTNM_SNMPTRAPD));
	strcpy(pCTX->xConfig.pName, FTNM_SNMPTRAPD_NAME);
	pCTX->xConfig.usPort= FTNM_SNMPTRAPD_PORT;
	pCTX->bRunning		= FTM_FALSE;

	*ppCTX = pCTX;

	return	FTM_RET_OK;
}

FTM_RET	FTNM_SNMPTRAPD_destroy(FTNM_SNMPTRAPD_PTR pCTX)
{
	ASSERT(pCTX != NULL);

	if (pCTX->bRunning)
	{
		FTNM_SNMPTRAPD_stop(pCTX);	
	}

	FTM_MEM_free(pCTX);

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

	pCTX->bRunning = FTM_FALSE;
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

FTM_RET FTNM_SNMPTRAPD_loadConfig(FTNM_SNMPTRAPD_PTR pCTX, FTM_CHAR_PTR pFileName)
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

FTM_RET	FTNM_SNMPTRAPD_setTrapCB(FTNM_SNMPTRAPD_PTR pCTX, FTNM_SNMPTRAPD_CALLBACK fTrapCB)
{
	ASSERT(pCTX != NULL);
	
	pCTX->fTrapCB = fTrapCB;

	return	FTM_RET_OK;
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
	FTM_INT 		op, 
	netsnmp_session *pSession,
	FTM_INT 		nReqID, 
	netsnmp_pdu 	*pPDU, 
	FTM_VOID_PTR 	pMagic
)
{
    oid stdTrapOidRoot[] = { 1, 3, 6, 1, 6, 3, 1, 1, 5 };
    oid snmpTrapOid[]    = { 1, 3, 6, 1, 6, 3, 1, 1, 4, 1, 0 };
    oid trapOid[MAX_OID_LEN+2] = {0};
    int trapOidLen;
    netsnmp_variable_list *vars;
    //netsnmp_trapd_handler *traph;
#if 0
    netsnmp_transport *pTransport = (netsnmp_transport *) pMagic;
#endif
    switch (op) 
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
        	TRACE("input: %x\n", pPDU->command);
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
					else 
					{
                		memcpy(trapOid, stdTrapOidRoot, sizeof(stdTrapOidRoot));
                		trapOidLen = OID_LENGTH(stdTrapOidRoot);  /* 9 */
                		trapOid[trapOidLen++] = pPDU->trap_type+1;
					}
				}
   	        	break;

			case SNMP_MSG_TRAP2: 
			case SNMP_MSG_INFORM:
				{
					/*
					 * v2c/v3 notifications *should* have snmpTrapOID as the
					 *    second varbind, so we can go straight there.
					 *    But check, just to make sure
					 */
					vars = pPDU->variables;
					if (vars)
					{
						vars = vars->next_variable;
						if (!vars || snmp_oid_compare(vars->name, vars->name_length,
													snmpTrapOid, OID_LENGTH(snmpTrapOid))) 
						{
							/*
						 	 * Didn't find it!
						 	 * Let's look through the full list....
						 	 */
							for ( vars = pPDU->variables; vars; vars=vars->next_variable) 
							{
                    			if (!snmp_oid_compare(vars->name, vars->name_length, snmpTrapOid, OID_LENGTH(snmpTrapOid)))
								{
                        			break;
								}
							}
                		}

                		if (!vars) 
						{
	            			/*
		     				 * Still can't find it!  Give up.
		     				 */
		    				snmp_log(LOG_ERR, "Cannot find TrapOID in TRAP2 PDU\n");
							FTNM_SNMPTRAPD_dumpPDU(pPDU) ;
		    				return 1;		/* ??? */
						}
	    			}

            		memcpy(trapOid, vars->val.objid, vars->val_len);
            		trapOidLen = vars->val_len /sizeof(oid);

				}	
            	break;

        	default:
				{
            		/* SHOULDN'T HAPPEN! */
            		return 1;	/* ??? */
				}
			}

			/*
			 *  OK - We've found the Trap OID used to identify this trap.
			 *  Call each of the various lists of handlers:
			 *     a) authentication-related handlers,
			 *     b) other handlers to be applied to all traps
			 *		(*before* trap-specific handlers)
			 *     c) the handler(s) specific to this trap
			 *     d) any other global handlers
			 *
			 *  In each case, a particular trap handler can abort further
			 *     processing - either just for that particular list,
			 *     or for the trap completely.
			 *
			 *  This is particularly designed for authentication-related
			 *     handlers, but can also be used elsewhere.
			 *
			 *  OK - Enough waffling, let's get to work.....
			*/
#if 0
			for( idx = 0; handlers[idx].descr; ++idx ) 
			{
				DEBUGMSGTL(("snmptrapd", "Running %s handlers\n",
							handlers[idx].descr));
				if (NULL == handlers[idx].handler) /* specific */
				{
					traph = netsnmp_get_traphandler(trapOid, trapOidLen);
				}
				else
				{
					traph = *handlers[idx].handler;
				}

				for( ; traph; traph = traph->nexth) 
				{
					if (!netsnmp_trapd_check_auth(traph->authtypes))
					{
						continue; /* we continue on and skip this one */
					}

					ret = (*(traph->handler))(pdu, pTransport, traph);
					if(NETSNMPTRAPD_HANDLER_FINISH == ret)
					{
						return 1;
					}

					if (ret == NETSNMPTRAPD_HANDLER_BREAK)
					{
						break; /* move on to next type */
					}
				} /* traph */
			} /* handlers */

#endif
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
						snmp_sess_perror("snmptrapd: Couldn't respond to inform pdu", pSession);
						snmp_free_pdu(reply);
					}
				}
			}
		}
		break;

	case NETSNMP_CALLBACK_OP_TIMED_OUT:
		{
			snmp_log(LOG_ERR, "Timeout: This shouldn't happen!\n");
		}
        break;

    case NETSNMP_CALLBACK_OP_SEND_FAILED:
		{
			snmp_log(LOG_ERR, "Send Failed: This shouldn't happen either!\n");
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
        	snmp_log(LOG_ERR, "Unknown operation (%d): This shouldn't happen!\n", op);
		}
        break;
    }

	return	0;
}

static 
netsnmp_session * FTNM_SNMPTRAPD_addSession(netsnmp_transport *pTransport)
{
	netsnmp_session xSession, *pRet = NULL;

	snmp_sess_init(&xSession);
	xSession.peername 		= SNMP_DEFAULT_PEERNAME;  /* Original code had NULL here */
	xSession.version 		= SNMP_DEFAULT_VERSION;
	xSession.community_len 	= SNMP_DEFAULT_COMMUNITY_LEN;
	xSession.retries 		= SNMP_DEFAULT_RETRIES;
	xSession.timeout 		= SNMP_DEFAULT_TIMEOUT;
	xSession.callback 		= FTNM_SNMPTRAPD_inputCB;
	xSession.callback_magic = (void *) pTransport;
	xSession.authenticator 	= NULL;
	xSession.isAuthoritative= SNMP_SESS_UNKNOWNAUTH;

	pRet = snmp_add(&xSession, pTransport, FTNM_SNMPTRAPD_preParse, NULL);
	if (pRet == NULL) 
	{
		snmp_sess_perror("snmptrapd", &xSession);
	}
	return pRet;
}

static 
FTM_RET	FTNM_SNMPTRAPD_closeSessions(netsnmp_session * pSessionList)
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
FTNM_SNMPTRAPD_mainLoop(FTNM_SNMPTRAPD_PTR pTrapd)
{
	int             count, numfds, block;
	fd_set          readfds,writefds,exceptfds;
	struct timeval  timeout, *tvp;

	while (pTrapd->bRunning) 
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
		tvp->tv_sec = 5;
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
					snmp_log_perror("select");
					pTrapd->bRunning = FTM_FALSE;
				}
				break;

			default:
				{
					ERROR("select returned %d\n", count);
					pTrapd->bRunning = FTM_FALSE;
				}
			}
		}
		run_alarms();
	}
}

FTM_VOID_PTR	FTNM_SNMPTRAPD_process(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);

	FTNM_SNMPTRAPD_PTR	pTrapd = (FTNM_SNMPTRAPD_PTR)pData;

    netsnmp_session		*pSessionList = NULL;
    netsnmp_session		*pSession = NULL;
	netsnmp_transport 	*pTransport = NULL;
	FTM_CHAR			pPort[16];

	sprintf(pPort, "%d", pTrapd->xConfig.usPort);
	pTransport = netsnmp_transport_open_server(pTrapd->xConfig.pName, pPort); 
	if (pTransport == NULL) 
	{
		ERROR("Couldn't open %d -- errno %d(\"%s\")\n", 
				pTrapd->xConfig.usPort
				errno, strerror(errno));
		FTNM_SNMPTRAPD_closeSessions(pSessionList);
		SOCK_CLEANUP;

		return	0;
	} 
	else 
	{
		pSession  = FTNM_SNMPTRAPD_addSession(pTransport);
		if (pSession == NULL) 
		{
			/*   
			 * Shouldn't happen?  We have already opened the transport
			 * successfully so what could have gone wrong?  
			 */
			FTNM_SNMPTRAPD_closeSessions(pSessionList);
			netsnmp_transport_free(pTransport);
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

	FTNM_SNMPTRAPD_mainLoop(pTrapd);

	return  FTM_RET_OK;

}


FTM_RET	FTNM_SNMPTRAPD_dumpPDU(netsnmp_pdu 	*pPDU) 
{
    netsnmp_variable_list *vars;

	for(vars = pPDU->variables; vars ; vars = vars->next_variable)
	{
		int	i ;
		char	pBuff[1024];

		MESSAGE("Name : ");
		for(i = 0 ; i < vars->name_length ; i++)
		{
			MESSAGE(".%d", vars->name[i]);
		}
		MESSAGE("\n");
		print_objid(vars->name, vars->name_length);
		MESSAGE("TYPE : %d\n", vars->type);
		memcpy(pBuff, vars->val.string, vars->val_len);
		pBuff[vars->val_len] = 0;
		MESSAGE("VAL : %s\n", pBuff);
	}

	return	FTM_RET_OK;
}
