#include <ctype.h>
#include <stdlib.h>
#include <errno.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <net-snmp/agent/net-snmp-agent-includes.h>
#include <net-snmp/output_api.h>

#include "ftm.h"
#include "ftom.h"
#include "ftom_msg.h"
#include "ftom_snmptrap.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"

#undef	__MODULE__
#define	__MODULE__	FTOM_TRACE_MODULE_SNMPTRAP

#ifndef	FTOM_TRACE_SNMPTRAP_IO	
#define	FTOM_TRACE_SNMPTRAP_IO	0
#endif

static 
netsnmp_session* FTOM_SNMPTRAP_addSession
(
	FTOM_SNMPTRAP_PTR pSNMPTRAP
);

static 
FTM_RET	FTOM_SNMPTRAP_closeSessions
(
	FTOM_SNMPTRAP_PTR pSNMPTRAP, 
	netsnmp_session 	*pSessionList
);

static 
FTM_BOOL	FTOM_SNMPTRAP_seekTrapCB
(
	const FTM_VOID_PTR 	pElement, 
	const FTM_VOID_PTR 	pIndicator
);

static 
FTM_RET	FTOM_SNMPTRAP_receiveTrap
(
	FTOM_SNMPTRAP_PTR 	pSNMPTRAP, 
	FTM_SNMP_OID_PTR	pOID,
	FTM_CHAR_PTR 		pMsg
);

static 
FTM_RET	FTOM_SNMPTRAP_alert
(
	FTOM_SNMPTRAP_PTR 	pSNMPTRAP, 
	FTM_SNMP_OID_PTR	pOID,
	FTM_CHAR_PTR 		pMsg
);

static 
FTM_RET	FTOM_SNMPTRAP_discovery
(
	FTOM_SNMPTRAP_PTR 	pSNMPTRAP, 
	FTM_SNMP_OID_PTR	pOID,
	FTM_CHAR_PTR 		pMsg
);

FTM_RET	FTOM_SNMPTRAP_create
(
	FTOM_SNMPTRAP_PTR _PTR_ ppSNMPTRAP
)
{
	ASSERT(ppSNMPTRAP != NULL);

	FTM_RET	xRet;
	FTOM_SNMPTRAP_PTR pSNMPTRAP;

	pSNMPTRAP = (FTOM_SNMPTRAP_PTR)FTM_MEM_malloc(sizeof(FTOM_SNMPTRAP));
	if (pSNMPTRAP == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", sizeof(FTOM_SNMPTRAP));
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_SNMPTRAP_init(pSNMPTRAP);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pSNMPTRAP);	
		ERROR2(xRet, "Initialize failed.\n");
		return	xRet;
	}

	*ppSNMPTRAP = pSNMPTRAP;
	TRACE("create ppSNMPTRAP : %08x\n", *ppSNMPTRAP);

	return	xRet;
}

FTM_RET	FTOM_SNMPTRAP_destroy
(
	FTOM_SNMPTRAP_PTR _PTR_ ppSNMPTRAP
)
{
	ASSERT(ppSNMPTRAP != NULL);

	FTM_RET	xRet;

	TRACE("destroy ppSNMPTRAP : %08x\n", *ppSNMPTRAP);
	xRet = FTOM_SNMPTRAP_final(*ppSNMPTRAP);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Finalize failed.\n");
	}

	xRet = FTM_MEM_free(*ppSNMPTRAP);	
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Memory free failed.\n");
	}

	*ppSNMPTRAP = NULL;

	return	xRet;
}

FTM_RET	FTOM_SNMPTRAP_init
(
	FTOM_SNMPTRAP_PTR 	pSNMPTRAP
)
{
	FTM_RET	xRet;

	memset(pSNMPTRAP, 0, sizeof(FTOM_SNMPTRAP));
	strcpy(pSNMPTRAP->xConfig.pName, FTOM_SNMPTRAP_NAME);
	pSNMPTRAP->xConfig.usPort= FTOM_SNMPTRAP_PORT;
	pSNMPTRAP->bStop = FTM_TRUE;

	xRet = FTM_LIST_init(&pSNMPTRAP->xTrapCBList);
	if (xRet == FTM_RET_OK)
	{
		FTM_LIST_setSeeker(&pSNMPTRAP->xTrapCBList, FTOM_SNMPTRAP_seekTrapCB);
	}
	
	return	xRet;
}

FTM_RET	FTOM_SNMPTRAP_final
(
	FTOM_SNMPTRAP_PTR pSNMPTRAP
)
{
	ASSERT(pSNMPTRAP != NULL);
	FTOM_SNMPTRAP_TRAP_INFO_PTR	pTrap;

	if (pSNMPTRAP->bStop)
	{
		FTOM_SNMPTRAP_stop(pSNMPTRAP);	
	}

	FTM_LIST_iteratorStart(&pSNMPTRAP->xTrapCBList);
	while(FTM_LIST_iteratorNext(&pSNMPTRAP->xTrapCBList, (FTM_VOID_PTR _PTR_)&pTrap) == FTM_RET_OK)
	{
		FTM_LIST_remove(&pSNMPTRAP->xTrapCBList, pTrap);
		FTM_MEM_free(pTrap);
	}

	FTM_LIST_final(&pSNMPTRAP->xTrapCBList);

	return	FTM_RET_OK;
}

FTM_RET FTOM_SNMPTRAP_start
(
	FTOM_SNMPTRAP_PTR pSNMPTRAP
)
{
	ASSERT(pSNMPTRAP != NULL);

    netsnmp_session		*pSessionList = NULL;
    netsnmp_session		*pSession = NULL;
	FTM_CHAR			pPort[16];

	if (!pSNMPTRAP->bStop)
	{
		INFO("SNMP Trap already started!\n");
		return	FTM_RET_ALREADY_STARTED;	
	}

	sprintf(pPort, "udp:%d", pSNMPTRAP->xConfig.usPort);
	pSNMPTRAP->pTransport = netsnmp_transport_open_server(pSNMPTRAP->xConfig.pName, pPort); 
	if (pSNMPTRAP->pTransport == NULL) 
	{
		ERROR2(FTM_RET_ERROR, "Couldn't open %d -- errno %d(\"%s\")\n", 
				pSNMPTRAP->xConfig.usPort,
				errno, strerror(errno));
		FTOM_SNMPTRAP_closeSessions(pSNMPTRAP, pSessionList);
		SOCK_CLEANUP;

		return	FTM_RET_ERROR;
	} 
	else 
	{
		pSession  = FTOM_SNMPTRAP_addSession(pSNMPTRAP);
		if (pSession == NULL) 
		{
			/*   
			 * Shouldn't happen?  We have already opened the transport
			 * successfully so what could have gone wrong?  
			 */
			FTOM_SNMPTRAP_closeSessions(pSNMPTRAP, pSessionList);
			netsnmp_transport_free(pSNMPTRAP->pTransport);
			pSNMPTRAP->pTransport = NULL;
			ERROR2(FTM_RET_ERROR, "couldn't open snmp - %s", strerror(errno));
			SOCK_CLEANUP;

			return	FTM_RET_ERROR;
		} 
		else 
		{
			pSession->next = pSessionList;
			pSessionList = pSession;
		}    
	}    

	pSNMPTRAP->bStop = FTM_FALSE;
	TRACE("SNMP Trap started!\n");

	return	FTM_RET_OK;
}

FTM_RET FTOM_SNMPTRAP_stop
(
	FTOM_SNMPTRAP_PTR pSNMPTRAP
)
{
	ASSERT(pSNMPTRAP != NULL);
	void*	pRet;

	if (pSNMPTRAP->bStop)
	{
		return	FTM_RET_NOT_START;	
	}

	pSNMPTRAP->bStop = FTM_TRUE;
	pthread_join(pSNMPTRAP->xPThread, &pRet);

	return	FTM_RET_OK;
}

FTM_RET FTOM_SNMPTRAP_loadConfig
(
	FTOM_SNMPTRAP_PTR 	pSNMPTRAP, 
	FTM_CONFIG_PTR		pConfig
)
{
	ASSERT(pSNMPTRAP != NULL);
	ASSERT(pConfig != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_ITEM		xSection;

	xRet = FTM_CONFIG_getItem(pConfig, "snmptrapd", &xSection);
	if (xRet == FTM_RET_OK)
	{
		FTM_CONFIG_ITEM	xList;
		FTM_CHAR		pTarget[64];

		memset(pSNMPTRAP->xConfig.pName, 0, sizeof(pSNMPTRAP->xConfig.pName));
		FTM_CONFIG_ITEM_getItemString(&xSection, "name", pSNMPTRAP->xConfig.pName, sizeof(pSNMPTRAP->xConfig.pName) - 1);
		FTM_CONFIG_ITEM_getItemUSHORT(&xSection, "port", &pSNMPTRAP->xConfig.usPort);
	
		memset(pTarget, 0, sizeof(pTarget));
		FTM_CONFIG_ITEM_getItemString(&xSection, "target", pTarget, sizeof(pTarget) - 1);
	
		if (strcmp(pTarget, "ft") == 0)
		{
			pSNMPTRAP->xConfig.xTarget = FTOM_SNMPTRAP_TARGET_FT;
		}
		else if (strcmp(pTarget, "tpgw") ==0)
		{
			pSNMPTRAP->xConfig.xTarget = FTOM_SNMPTRAP_TARGET_TPGW;
		}
		else
		{
			pSNMPTRAP->xConfig.xTarget = FTOM_SNMPTRAP_TARGET_UNKNOWN;
		}

		xRet = FTM_CONFIG_ITEM_getChildItem(&xSection, "traps", &xList);
		if (xRet == FTM_RET_OK)
        {
			FTM_ULONG	i, ulCount = 0;
		
			FTM_CONFIG_LIST_getItemCount(&xList, &ulCount);	
			for(i = 0 ; i < ulCount ; i++)
			{
				FTOM_SNMPTRAP_MSG_TYPE	xMsgType;
				FTM_CONFIG_ITEM			xElement;

				xRet = FTM_CONFIG_LIST_getItemAt(&xList, i, &xElement);
				if (xRet == FTM_RET_OK)
				{
					FTM_CHAR	pValue[256];
					FTM_CHAR	pOID[256];

					memset(pValue, 0, sizeof(pValue));
					xRet = FTM_CONFIG_ITEM_getItemString(&xElement, "msg", pValue, sizeof(pValue) - 1);
					if (xRet == FTM_RET_OK)
					{
						if (strcasecmp("alert", pValue) == 0)
						{
							xMsgType = FTOM_SNMPTRAP_MSG_TYPE_ALERT;			
						}
						else if (strcasecmp("discovery", pValue) == 0)
						{
							xMsgType = FTOM_SNMPTRAP_MSG_TYPE_DISCOVERY;			
						}
						else
						{
							WARN("Invalid message type[%s]\n", pValue);
							continue;	
						}
					}
				
					memset(pOID, 0, sizeof(pOID));
					xRet = FTM_CONFIG_ITEM_getItemString(&xElement, "oid", pOID, sizeof(pOID) - 1);
					if (xRet == FTM_RET_OK)
					{
						FTM_INT			nRet;
						FTM_SNMP_OID	xOID;

						xOID.nLen = FTM_SNMP_OID_LENGTH;
						nRet = read_objid(pOID, xOID.pIDs, &xOID.nLen);
						if (nRet == 1)
						{
							switch(xMsgType)
							{
							case FTOM_SNMPTRAP_MSG_TYPE_UNKNOWN:
							case FTOM_SNMPTRAP_MSG_TYPE_EP_CHANGED:
								{
									xRet = FTOM_SNMPTRAP_addTrapOID(pSNMPTRAP, xMsgType, pOID, &xOID, FTOM_SNMPTRAP_receiveTrap);
								}
								break;


							case	FTOM_SNMPTRAP_MSG_TYPE_ALERT:
								{
									xRet = FTOM_SNMPTRAP_addTrapOID(pSNMPTRAP, xMsgType, pOID, &xOID, FTOM_SNMPTRAP_alert);
								}
								break;

							case	FTOM_SNMPTRAP_MSG_TYPE_DISCOVERY:
								{
									xRet = FTOM_SNMPTRAP_addTrapOID(pSNMPTRAP, xMsgType, pOID, &xOID, FTOM_SNMPTRAP_discovery);
								}
								break;
							
							default:
								{
									xRet = FTM_RET_ERROR;	
								}
							}
							if (xRet == FTM_RET_OK)
							{
								TRACE("The trap OID has been registered. - %s\n", FTM_SNMP_OID_print(&xOID));
							}
							else
							{
								TRACE("The trap OID failed to register. - %s\n", FTM_SNMP_OID_print(&xOID));
							}
						}
						else
						{
							TRACE("Can't know the OID for %s.\n", pOID);
						}
					}
				}
			}
		}

	}

	return	FTM_RET_OK;
}

FTM_RET FTOM_SNMPTRAP_loadConfigFromFile
(
	FTOM_SNMPTRAP_PTR 	pSNMPTRAP, 
	FTM_CHAR_PTR 			pFileName
)
{
	ASSERT(pSNMPTRAP != NULL);
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_PTR		pConfig;

	xRet = FTM_CONFIG_create(pFileName, &pConfig, FTM_FALSE);	
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	xRet = FTOM_SNMPTRAP_loadConfig(pSNMPTRAP, pConfig);

	FTM_CONFIG_destroy(&pConfig);

	return	xRet;
}

FTM_RET FTOM_SNMPTRAP_saveConfig
(
	FTOM_SNMPTRAP_PTR 	pSNMPTRAP, 
	FTM_CONFIG_PTR		pConfig
)
{
	ASSERT(pSNMPTRAP != NULL);
	ASSERT(pConfig != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_ITEM		xSection;

	xRet = FTM_CONFIG_getItem(pConfig, "snmptrapd", &xSection);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_CONFIG_addItem(pConfig, "snmptrapd", &xSection);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;
		}
	}

	FTM_CONFIG_ITEM	xList;

	FTM_CONFIG_ITEM_setItemString(&xSection, "name", pSNMPTRAP->xConfig.pName);
	FTM_CONFIG_ITEM_setItemUSHORT(&xSection, "port", pSNMPTRAP->xConfig.usPort);

	switch(pSNMPTRAP->xConfig.xTarget)
	{
	case	FTOM_SNMPTRAP_TARGET_FT: FTM_CONFIG_ITEM_setItemString(&xSection, "target", "ft"); break;
	case	FTOM_SNMPTRAP_TARGET_TPGW: FTM_CONFIG_ITEM_setItemString(&xSection, "target", "tpgw"); break;
	default: FTM_CONFIG_ITEM_setItemString(&xSection, "target", "unknown"); break;
	}

	xRet = FTM_CONFIG_ITEM_getChildItem(&xSection, "traps", &xList);
	if (xRet != FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_createChildList(&xSection, "traps", &xList);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;	
		}
	}

	FTM_ULONG	i, ulCount = 0;

	FTM_CONFIG_LIST_getItemCount(&xList, &ulCount);	
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_CONFIG_LIST_deleteItemAt(&xList, 0);
	}

	ulCount = 0;
	FTM_LIST_count(&pSNMPTRAP->xTrapCBList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTOM_SNMPTRAP_TRAP_INFO_PTR 	pTrap;

		xRet = FTM_LIST_getAt(&pSNMPTRAP->xTrapCBList, i,(FTM_VOID_PTR _PTR_) &pTrap);
		if (xRet == FTM_RET_OK)
		{
			FTM_CONFIG_ITEM	xItem;

			xRet = FTM_CONFIG_LIST_addItem(&xList, &xItem);
			if (xRet == FTM_RET_OK)
			{
				switch(pTrap->xMsgType)
				{
				case FTOM_SNMPTRAP_MSG_TYPE_EP_CHANGED:
					{
						FTM_CONFIG_ITEM_addItemString(&xItem, "msg", "ep_changed");
					}
					break;

				case FTOM_SNMPTRAP_MSG_TYPE_ALERT:
					{
						FTM_CONFIG_ITEM_addItemString(&xItem, "msg", "alert");
					}
					break;

				case FTOM_SNMPTRAP_MSG_TYPE_DISCOVERY:
					{
						FTM_CONFIG_ITEM_addItemString(&xItem, "msg", "discovery");
					}
					break;

				default:
					{
						FTM_CONFIG_ITEM_addItemString(&xItem, "msg", "unknown");
					}
					break;
				}

				FTM_CONFIG_ITEM_addItemString(&xItem, "oid", pTrap->pName);
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET FTOM_SNMPTRAP_showConfig
(
	FTOM_SNMPTRAP_PTR pSNMPTRAP
)
{
	ASSERT(pSNMPTRAP != NULL);

	MESSAGE("\n[ SNMPTRAP CONFIGURATION ]\n");
	MESSAGE("%16s : %s\n", "Name", pSNMPTRAP->xConfig.pName);
	MESSAGE("%16s : %d\n", "Port", pSNMPTRAP->xConfig.usPort);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SNMPTRAP_setServiceCallback
(
	FTOM_SNMPTRAP_PTR 	pSNMPTRAP, 
	FTOM_SERVICE_ID 	xServiceID,  
	FTOM_SERVICE_CB 	fServiceCB
)
{
	ASSERT(pSNMPTRAP != NULL);
	ASSERT(fServiceCB != NULL);

	pSNMPTRAP->xServiceID = xServiceID;
	pSNMPTRAP->fServiceCB = fServiceCB;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SNMPTRAP_getTrapOIDCount
(
	FTOM_SNMPTRAP_PTR 		pSNMPTRAP, 
	FTM_ULONG_PTR			pulCount
)
{
	ASSERT(pSNMPTRAP != NULL);
	ASSERT(pulCount != NULL);

	return	FTM_LIST_count(&pSNMPTRAP->xTrapCBList, pulCount);
}

FTM_RET	FTOM_SNMPTRAP_addTrapOID
(
	FTOM_SNMPTRAP_PTR 		pSNMPTRAP, 
	FTOM_SNMPTRAP_MSG_TYPE	xMsgType,
	FTM_CHAR_PTR			pName,
	FTM_SNMP_OID_PTR 		pOID,
	FTOM_SNMPTRAP_CB		fTrapCB
)
{
	ASSERT(pSNMPTRAP != NULL);
	ASSERT(pOID != NULL);

	FTM_RET			xRet;
	FTOM_SNMPTRAP_TRAP_INFO_PTR 	pTrap;
	
	pTrap = (FTOM_SNMPTRAP_TRAP_INFO_PTR)FTM_MEM_malloc(sizeof(FTOM_SNMPTRAP_TRAP_INFO));
	if (pTrap == NULL)
	{
		ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", sizeof(FTOM_SNMPTRAP_TRAP_INFO));
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pTrap->xMsgType = xMsgType;
	strncpy(pTrap->pName, pName, sizeof(pTrap->pName) -1 );
	memcpy(&pTrap->xOID, pOID, sizeof(FTM_SNMP_OID));
	pTrap->fCB = fTrapCB;

	TRACE("Add Trap : %s\n",FTM_SNMP_OID_print(pOID));
	xRet = FTM_LIST_append(&pSNMPTRAP->xTrapCBList, pTrap);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to append trap to list!\n");
		FTM_MEM_free(pTrap);	
	}

	TRACE("OID : %s\n", FTM_SNMP_OID_print(pOID));

	return	FTM_RET_OK;
}

FTM_BOOL	FTOM_SNMPTRAP_seekTrapCB
(
	const FTM_VOID_PTR 	pElement, 
	const FTM_VOID_PTR 	pIndicator
)
{
	ASSERT(pElement != NULL);
	ASSERT(pIndicator != NULL);

	FTOM_SNMPTRAP_TRAP_INFO_PTR		pTrap = (FTOM_SNMPTRAP_TRAP_INFO_PTR)pElement;
	FTM_SNMP_OID_PTR	pOID = (FTM_SNMP_OID_PTR)pIndicator;

	return	snmp_oid_compare(pTrap->xOID.pIDs, pTrap->xOID.nLen, pOID->pIDs, pOID->nLen) == 0;
}


static 
FTM_INT FTOM_SNMPTRAP_preParse
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
FTOM_SNMPTRAP_inputCB
(
	FTM_INT 		nOP, 
	netsnmp_session *pSession,
	FTM_INT 		nReqID, 
	netsnmp_pdu 	*pPDU, 
	FTM_VOID_PTR 	pMagic
)
{
	FTOM_SNMPTRAP_PTR	pSNMPTRAP = (FTOM_SNMPTRAP_PTR)pMagic;

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
						FTOM_SNMPTRAP_TRAP_INFO_PTR	pTrap;
						FTM_SNMP_OID		xOID;

						memcpy(xOID.pIDs, vars->name, sizeof(oid) * vars->name_length);
						xOID.nLen  = vars->name_length;

						if (FTM_LIST_get(&pSNMPTRAP->xTrapCBList, &xOID, (FTM_VOID_PTR _PTR_)&pTrap) == FTM_RET_OK)
						{
							if (vars->type == ASN_OCTET_STR)
							{
								FTM_CHAR_PTR	pBuff = NULL;

								pBuff = FTM_MEM_malloc(vars->val_len + 1);
								if (pBuff == NULL)
								{
									ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory[size = %lu]!\n", vars->val_len+1);
									break;
								}

								memcpy(pBuff, vars->val.string, vars->val_len);
								pBuff[vars->val_len] = 0;
								if (pTrap->fCB != NULL)
								{
									pTrap->fCB(pSNMPTRAP, &xOID, pBuff);
								}
								else
								{
									FTOM_SNMPTRAP_receiveTrap(pSNMPTRAP, &xOID, pBuff);
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
						ERROR2(FTM_RET_ERROR, "Couldn't respond to inform pdu - %s\n", pErrMsg);
						SNMP_FREE(pErrMsg);

						snmp_free_pdu(reply);
					}
				}
			}
		}
		break;

	case NETSNMP_CALLBACK_OP_TIMED_OUT:
		{
			ERROR2(FTM_RET_TIMEOUT, "Timeout: This shouldn't happen!\n");
		}
        break;

    case NETSNMP_CALLBACK_OP_SEND_FAILED:
		{
			ERROR2(FTM_RET_ERROR, "Send Failed: This shouldn't happen either!\n");
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
        	ERROR2(FTM_RET_ERROR, "Unknown operation (%d): This shouldn't happen!\n", nOP);
		}
        break;
    }

	return	0;
}

static 
netsnmp_session * FTOM_SNMPTRAP_addSession
(
	FTOM_SNMPTRAP_PTR pSNMPTRAP
)
{
	ASSERT(pSNMPTRAP != NULL);

	netsnmp_session xSession, *pRet = NULL;

	snmp_sess_init(&xSession);
	xSession.peername 		= NULL;//SNMP_DEFAULT_PEERNAME;  /* Original code had NULL here */
	xSession.version 		= SNMP_DEFAULT_VERSION;
	xSession.community_len 	= SNMP_DEFAULT_COMMUNITY_LEN;
	xSession.retries 		= SNMP_DEFAULT_RETRIES;
	xSession.timeout 		= SNMP_DEFAULT_TIMEOUT;
	xSession.callback 		= FTOM_SNMPTRAP_inputCB;
	xSession.callback_magic = (void *)pSNMPTRAP;
	xSession.authenticator 	= NULL;
	xSession.isAuthoritative= SNMP_SESS_UNKNOWNAUTH;

	pRet = snmp_add(&xSession, pSNMPTRAP->pTransport, FTOM_SNMPTRAP_preParse, NULL);
	if (pRet == NULL) 
	{
		FTM_CHAR_PTR	pErrMsg = NULL;
		snmp_error(&xSession, NULL, NULL, &pErrMsg);
		ERROR2(FTM_RET_ERROR, "%s\n", pErrMsg);
		SNMP_FREE(pErrMsg);
	}
	return pRet;
}

static 
FTM_RET	FTOM_SNMPTRAP_closeSessions
(
	FTOM_SNMPTRAP_PTR pSNMPTRAP, 
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

FTM_RET	FTOM_SNMPTRAP_receiveTrap
(
	FTOM_SNMPTRAP_PTR 	pSNMPTRAP, 
	FTM_SNMP_OID_PTR	pOID,
	FTM_CHAR_PTR 		pMsg
)
{
	ASSERT(pSNMPTRAP != NULL);
	ASSERT(pMsg != NULL);
	
	FTM_RET			xRet;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTOM_EP_PTR		pEP = NULL;
	FTM_EP_DATA		xData;
	FTOM_SNMPTRAP_MSG_TYPE	xMsgType = FTOM_SNMPTRAP_MSG_TYPE_UNKNOWN;	
	cJSON _PTR_		pRoot;
	cJSON _PTR_		pItem;

	TRACE("SNMPTRAP : %s\n", pMsg);

	pRoot = cJSON_Parse(pMsg);
	if (pRoot == NULL)
	{
		ERROR2(FTM_RET_INVALID_ARGUMENTS,  "Invalid trap message[%s]\n", pMsg);
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	pItem = cJSON_GetObjectItem(pRoot, "type");
	if (pItem == NULL)
	{
		xMsgType = FTOM_SNMPTRAP_MSG_TYPE_EP_CHANGED;	
	}
	else
	{
		if ((pItem->type == cJSON_String) && (strcasecmp(pItem->valuestring, "ep_changed") == 0))
		{
			xMsgType = FTOM_SNMPTRAP_MSG_TYPE_EP_CHANGED;	
		}
		else
		{
			xRet = FTM_RET_INVALID_DATA;
			goto finish;
		}
	}

	switch(xMsgType)
	{
	case	FTOM_SNMPTRAP_MSG_TYPE_EP_CHANGED:
		{
			FTM_CHAR	pDID[FTM_DID_LEN+1];

			pItem = cJSON_GetObjectItem(pRoot, "did");
			if ((pItem == NULL) || (pItem->type != cJSON_String))
			{
				xRet = FTM_RET_INVALID_DATA;
				break;
			}

			memset(pDID, 0, sizeof(pDID));
			strncpy(pDID, pItem->valuestring, FTM_DID_LEN);

			pItem = cJSON_GetObjectItem(pRoot, "id");
			if ((pItem == NULL) || (pItem->type != cJSON_String))
			{
				xRet = FTM_RET_INVALID_DATA;
				break;
			}

			FTM_INT		nLen;

			nLen = strlen(pItem->valuestring);
			if (nLen == 8)
			{
				FTM_CHAR	pBuff[FTM_DID_LEN + FTM_EPID_LEN + 1];

				memset(pBuff, 0, sizeof(pBuff));
				snprintf(pBuff, sizeof(pBuff) -1,  "%s%s", pDID, pItem->valuestring);
			
				if (strlen(pBuff) > 14)
				{
					strcpy(pEPID, &pBuff[strlen(pBuff) - 14]);
				}
				else
				{
					strcpy(pEPID, pBuff);
				
				}
			}
			else
			{
				strcpy(pEPID, pItem->valuestring);
			}
	
			xRet = FTOM_EP_get(pEPID, &pEP);
			if (xRet != FTM_RET_OK)
			{
				xRet = FTM_RET_OBJECT_NOT_FOUND;
				ERROR2(xRet, "Can't found EP[%s]\n", pEPID);	
				break;
			}

			FTM_VALUE_TYPE		xDataType;
			FTOM_EP_PTR			pEP;

			FTOM_EP_getDataType(pEP, &xDataType);
	
			pItem = cJSON_GetObjectItem(pRoot, "value");
			if (pItem == NULL)
			{
				xRet = FTM_RET_INVALID_DATA;
				ERROR2(xRet, "TRAP : Value is not exist.\n");
				break;
			}
			
			switch(pItem->type)
			{
			case	cJSON_String:
				{
					TRACE("VALUE : %s\n", pItem->valuestring);
					FTM_EP_DATA_init(&xData, xDataType, (FTM_CHAR_PTR)pItem->valuestring);
				}
				break;
	
			case 	cJSON_Number:
				{
					switch(xDataType)
					{
					case	FTM_VALUE_TYPE_INT:
						{
							FTM_EP_DATA_initINT(&xData, pItem->valueint);
						}
						break;

					case	FTM_VALUE_TYPE_ULONG:
						{
							FTM_EP_DATA_initULONG(&xData, pItem->valueint);
						}
						break;

					case	FTM_VALUE_TYPE_FLOAT:
						{
							FTM_EP_DATA_initFLOAT(&xData, pItem->valuedouble);
						}
						break;
					
					case	FTM_VALUE_TYPE_BOOL:
						{
							FTM_EP_DATA_initBOOL(&xData, pItem->valueint);
						}
						break;
					}
				}
				break;
	
			default:
				{
					ERROR2(FTM_RET_INVALID_TYPE, "Invalid value type[%d].\n", pItem->type);
				}
				break;
			}
	
			pItem = cJSON_GetObjectItem(pRoot, "time");
			if (pItem != NULL)
			{
				if (pItem->type == cJSON_Number)
				{
					xData.ulTime = (FTM_ULONG)pItem->valueint;
				}
				else
				{
					ERROR2(FTM_RET_INVALID_TYPE, "Invalid value type[%d].\n", pItem->type);
				}
			}

			pItem = cJSON_GetObjectItem(pRoot, "state");
			if ((pItem != NULL) && (pItem->type == cJSON_String))
			{
				if (strcasecmp(pItem->valuestring, "enable") == 0)
				{
					xData.xState = FTM_EP_STATE_RUN;
				}
				else if (strcasecmp(pItem->valuestring, "disable") == 0)
				{
					xData.xState = FTM_EP_STATE_STOP;
				}
				else if (strcasecmp(pItem->valuestring, "error") == 0)
				{
					xData.xState = FTM_EP_STATE_ERROR;
				}
			}
	

			xRet = FTOM_EP_get(pEPID, &pEP);
			if (xRet == FTM_RET_OK)
			{
				FTOM_MSG_PTR	pMsg;

				xRet = FTOM_MSG_createEPData(pSNMPTRAP, pEPID, &xData, 1, &pMsg);
				if (xRet != FTM_RET_OK)
				{
					ERROR2(xRet, "Can't creation message!\n");	
				}
				else
				{
					xRet = FTOM_EP_sendMessage(pEP, pMsg);	
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet, "Can't send message!\n");	
						FTOM_MSG_destroy(&pMsg);
					}
				}
			}
			else
			{
				ERROR2(xRet, "Failed to get EP[%s].\n", pEPID);	
			}
		}
		break;

	default:
		{
			xRet = FTM_RET_INVALID_DATA;
		}
	}

finish:
	cJSON_Delete(pRoot);

	return	xRet;

}
FTM_RET	FTOM_SNMPTRAP_alert
(
	FTOM_SNMPTRAP_PTR 	pSNMPTRAP, 
	FTM_SNMP_OID_PTR	pOID,
	FTM_CHAR_PTR 		pMsg
)
{
	ASSERT(pSNMPTRAP != NULL);
	ASSERT(pMsg != NULL);
	
	FTM_RET			xRet;
	FTM_CHAR		pDID[FTM_DID_LEN+1];
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTOM_EP_PTR		pEP = NULL;
	FTM_VALUE_TYPE	xDataType;
	FTM_EP_DATA		xData;
	cJSON _PTR_		pRoot;
	cJSON _PTR_		pItem;

	TRACE("ALERT : %s\n", pMsg);

	pRoot = cJSON_Parse(pMsg);
	if (pRoot == NULL)
	{
		ERROR2(FTM_RET_INVALID_ARGUMENTS, "Invalid trap message[%s]\n", pMsg);
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	pItem = cJSON_GetObjectItem(pRoot, "did");
	if ((pItem == NULL) || (pItem->type != cJSON_String))
	{
		xRet = FTM_RET_SNMP_INVALID_MESSAGE_FORMAT;
		goto error;
	}
	memset(pDID, 0, sizeof(pDID));
	strncpy(pDID, pItem->valuestring, FTM_DID_LEN);

	pItem = cJSON_GetObjectItem(pRoot, "id");
	if ((pItem == NULL) || (pItem->type != cJSON_String))
	{
		xRet = FTM_RET_SNMP_INVALID_MESSAGE_FORMAT;
		goto error;
	}

	if (strlen(pItem->valuestring) == 8)
	{
		FTM_CHAR	pBuff[FTM_DID_LEN + FTM_EPID_LEN + 1];


		memset(pBuff, 0, sizeof(pBuff));
		sprintf(pBuff, "%s%s", pDID, pItem->valuestring);

		if (strlen(pBuff) > 14)
		{
			strcpy(pEPID, &pBuff[strlen(pBuff) - 14]);
		}
		else
		{
			strcpy(pEPID, pBuff);
		}
	}

	xRet = FTOM_EP_get(pEPID, &pEP);
	if (xRet != FTM_RET_OK)
	{
		WARN("EP[%s] does not exist!\n",pEPID);
		goto error;
	}

	FTOM_EP_getDataType(pEP, &xDataType);
		
	pItem = cJSON_GetObjectItem(pRoot, "value");
	if(pItem == NULL)
	{
		xRet = FTM_RET_INVALID_DATA;
		ERROR2(xRet, "TRAP : Value is not exist.\n");
		goto error;
	}
	
	switch(pItem->type)
	{
	case	cJSON_String:
		{
			FTM_EP_DATA_init(&xData, xDataType, (FTM_CHAR_PTR)pItem->valuestring);
		}
		break;
	
	case 	cJSON_Number:
		{
			switch(xDataType)
			{
			case	FTM_VALUE_TYPE_INT:
				{
					FTM_EP_DATA_initINT(&xData, pItem->valueint);
				}
				break;
	
			case	FTM_VALUE_TYPE_ULONG:
				{
					FTM_EP_DATA_initULONG(&xData, pItem->valueint);
				}
				break;
	
			case	FTM_VALUE_TYPE_FLOAT:
				{
					FTM_EP_DATA_initFLOAT(&xData, pItem->valuedouble);
				}
				break;
							
			case	FTM_VALUE_TYPE_BOOL:
				{
					FTM_EP_DATA_initBOOL(&xData, pItem->valueint);
				}
				break;
			}
		}
		break;
	
	default:
		{
			ERROR2(FTM_RET_INVALID_TYPE, "Invalid value type[%d].\n", pItem->type);
		}
		break;
	}
	
	pItem = cJSON_GetObjectItem(pRoot, "time");
	if ((pItem != NULL) && (pItem->type == cJSON_Number))
	{
		xData.ulTime = (FTM_ULONG)pItem->valueint;
	}
	
	pItem = cJSON_GetObjectItem(pRoot, "state");
	if ((pItem != NULL) && (pItem->type == cJSON_String))
	{
		if (strcasecmp(pItem->valuestring, "enable") == 0)
		{
			xData.xState = FTM_EP_STATE_RUN;
		}
		else if (strcasecmp(pItem->valuestring, "disable") == 0)
		{
			xData.xState = FTM_EP_STATE_STOP;
		}
		else if (strcasecmp(pItem->valuestring, "error") == 0)
		{
			xData.xState = FTM_EP_STATE_ERROR;
		}
	}
					
	xRet = FTOM_sendAlert(pEPID, &xData);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to send alert.\n");	
	}

error:
	cJSON_Delete(pRoot);

	return	xRet;

}
FTM_RET	FTOM_SNMPTRAP_discovery
(
	FTOM_SNMPTRAP_PTR 	pSNMPTRAP, 
	FTM_SNMP_OID_PTR	pOID,
	FTM_CHAR_PTR 		pMsg
)
{
	ASSERT(pSNMPTRAP != NULL);
	ASSERT(pMsg != NULL);
	
	FTM_RET			xRet;
	FTM_INT			i;
	FTM_ULONG		ulDIDLen;
	FTM_CHAR		pName[FTM_DEVICE_NAME_LEN + 1];
	FTM_CHAR		pDID[FTM_DID_LEN + 1];
	FTM_CHAR		pDeviceIP[32];
	FTM_EP_TYPE		pEPTypes[32];
	FTM_ULONG		ulEPTypes = 0;
	cJSON _PTR_		pRoot;
	cJSON _PTR_		pItem;
	cJSON _PTR_		pOIDs;

	TRACE("DISCOVERY : %s\n", pMsg);

	pRoot = cJSON_Parse(pMsg);
	if (pRoot == NULL)
	{
		ERROR2(FTM_RET_INVALID_ARGUMENTS, "Invalid trap message[%s]\n", pMsg);
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	pItem = cJSON_GetObjectItem(pRoot, "id");
	if ((pItem == NULL) || (pItem->type != cJSON_String))
	{
		xRet = FTM_RET_SNMP_INVALID_MESSAGE_FORMAT;
		ERROR2(xRet, "Invalid message format!\n");
		goto error;
	}
	memset(pName, 0, sizeof(pName));
	strncpy(pName, pItem->valuestring, FTM_DEVICE_NAME_LEN);


	memset(pDeviceIP, 0, sizeof(pDeviceIP));
	pItem = cJSON_GetObjectItem(pRoot, "ip");
	if (pItem != NULL)
	{ 
		if (pItem->type != cJSON_String)
		{
			xRet = FTM_RET_SNMP_INVALID_MESSAGE_FORMAT;
			ERROR2(xRet, "Invalid message format!\n");
			goto error;
		}
		strncpy(pDeviceIP, pItem->valuestring, sizeof(pDeviceIP) - 1);
	}

	pItem = cJSON_GetObjectItem(pRoot, "mac");
	if ((pItem == NULL) || (pItem->type != cJSON_String) || strlen(pItem->valuestring) != 17)
	{
		xRet = FTM_RET_SNMP_INVALID_MESSAGE_FORMAT;
		ERROR2(xRet, "Invalid message format!\n");
		goto error;
	}

	ulDIDLen  = 0;
	memset(pDID, 0, sizeof(pDID));
	for(i = 0 ; i < strlen(pItem->valuestring); i++)
	{
		if (((i + 1) % 3) != 0)
		{
			pDID[ulDIDLen++] = toupper(pItem->valuestring[i]);	
		}
	}

	pOIDs = cJSON_GetObjectItem(pRoot, "oids");
	if ((pOIDs == NULL) || (pOIDs->type != cJSON_Array))
	{
		xRet = FTM_RET_SNMP_INVALID_MESSAGE_FORMAT;
		ERROR2(xRet, "Invalid message format!\n");
		goto error;
	}

	ulEPTypes = 0;

	for(i = 0 ; i < cJSON_GetArraySize(pOIDs) ; i++)
	{
		FTM_ULONG	ulType;

		pItem = cJSON_GetArrayItem(pOIDs, i);
		if(pItem == NULL)
		{
			continue;		
		}

		if (pItem->type == cJSON_String)
		{
			ulType = strtol(pItem->valuestring, NULL, 10);
		}
		else if (pItem->type == cJSON_Number)
		{
			ulType = pItem->valueint;
		}
		else
		{
			continue;	
		}

		pEPTypes[ulEPTypes++] = (ulType << 16);
	}	
	
	FTOM_receivedDiscovery(pName, pDID, pDeviceIP, pEPTypes, ulEPTypes);

error:

	cJSON_Delete(pRoot);

	return	xRet;

}

#if 0
FTM_RET	FTOM_SNMPTRAP_dumpPDU(netsnmp_pdu 	*pPDU) 
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
