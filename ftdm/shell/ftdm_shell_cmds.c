#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "ftdm.h"
#include "ftm_mem.h"
#include "ftdm_params.h"
#include "ftdm_config.h"
#include "ftdm_server.h"
#include "ftdm_trigger.h"
#include "ftdm_action.h"
#include "ftdm_rule.h"
#include "shell/ftdm_shell_cmds.h"

static
FTM_RET	FTDM_SHELL_CMD_config
(
	FTM_SHELL_PTR 	pShell, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR	pData
);

static
FTM_RET	FTDM_SHELL_CMD_object
(
	FTM_SHELL_PTR 	pShell, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR	pData
);

static
FTM_RET	FTDM_SHELL_CMD_session
(
	FTM_SHELL_PTR 	pShell, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR	pData
);

static
FTM_RET	FTDM_SHELL_CMD_node
(
	FTM_SHELL_PTR 	pShell, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR	pData
);

static
FTM_RET	FTDM_SHELL_CMD_ep
(
	FTM_SHELL_PTR 	pShell, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR	pData
);

static
FTM_RET	FTDM_SHELL_CMD_log
(
	FTM_SHELL_PTR 	pShell, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR	pData
);

static 
FTDM_CFG	xConfig;
FTM_SHELL_CMD	FTDM_pCmdList[] =
{
	{
		.pString	= "config",
		.function	= FTDM_SHELL_CMD_config,
		.pShortHelp	= "Configuration Data Manager.",
		.pHelp		= "\n"\
					  "\tConfiguration Data Manager.\n"
	},
	{
		.pString	= "ep",
		.function	= FTDM_SHELL_CMD_ep,
		.pShortHelp	= "End Point Manager.",
		.pHelp		= " [<EPID> [<cmd>] [<index> <count>]]\n"\
					  "    EP management\n"\
				      "  Commands:\n"\
				      "    data  Display data\n"\
				      "  Parameters:\n"\
				      "    EPID  Identifer of EP\n"\
				      "    index The starting index of the data for display.\n"\
				      "    count Number of the data for display.\n"
	},
	{
		.pString	= "log",
		.function	= FTDM_SHELL_CMD_log,
		.pShortHelp	= "Log management.",
		.pHelp		= "\n"\
					  "\tLog management.\n"
	},
	{
		.pString	= "node",
		.function	= FTDM_SHELL_CMD_node,
		.pShortHelp	= "Node Manager.",
		.pHelp		= "\n"\
					  "\tNode Manager.\n"
	},
	{
		.pString	= "object",
		.function	= FTDM_SHELL_CMD_object,
		.pShortHelp	= "Object management.",
		.pHelp		= "\n"\
					  "\tObject management.\n"
	},
	{
		.pString	= "session",
		.function	= FTDM_SHELL_CMD_session,
		.pShortHelp	= "Session Manager.",
		.pHelp		= "\n"\
					  "\tSession Manager.\n"
	},
	{
		.pString	= "trace",
		.function	= FTM_TRACE_shellCmd,
		.pShortHelp	= "Trace configuration.",
		.pHelp		= "\n"\
					  "\tTrace configuration.\n"
	},
};

FTM_ULONG		FTDM_ulCmdCount = sizeof(FTDM_pCmdList) / sizeof(FTM_SHELL_CMD);
FTM_CHAR_PTR	FTDM_pPrompt = "ftdm";

FTM_RET	FTDM_SHELL_CMD_config
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
	switch (nArgc)
	{
	case	1:
		{
			FTDM_CFG_show(&xConfig);
		}
		break;

	default:
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_SHELL_CMD_object
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
	FTDM_SIS_PTR	pSIS = (FTDM_SIS_PTR)pData;


	TRACE("pSIS = %08x, pSIS->pFTDM = %08x\n", pSIS, pSIS->pFTDM);
	switch (nArgc)
	{
	case	1:
		{
			FTM_CHAR_PTR	pArgv[1];
			
			pArgv[0] = "node";
			FTDM_SHELL_CMD_node(pShell, 1, pArgv, pData);
			pArgv[0] = "ep";
			FTDM_SHELL_CMD_ep(pShell, 1, pArgv, pData);
#if 0
			pArgv[0] = "trigger";
			FTDM_SHELL_CMD_trigger(pShell, 1, pArgc, pData);
			pArgv[0] = "action";
			FTDM_SHELL_CMD_action(pShell, 1, pArgv, pData);
			pArgv[0] = "rule";
			FTDM_SHELL_CMD_rule(pShell, 1, pArgv, pData);
#endif
		}	
		break;
#if 0
	case	3:
		{
			if (strcasecmp(pArgv[1], "load") == 0)
			{
				xRet = FTDM_loadObjectFromFile(pSIS->pFTDM, pArgv[2]);	
				if (xRet != FTM_RET_OK)
				{
					ERROR("Objects loading failed.[%08x]\n", xRet);
					return	xRet;
				}

				MESSAGE("Objects loading completed.\n");
			}
		}
		break;
#endif
	default:
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_SHELL_CMD_session
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
	FTM_RET			xRet;
	FTM_INT			i;
	FTM_ULONG		ulCount;
	FTDM_SESSION	xSession;
	FTDM_SIS_PTR	pSIS = (FTDM_SIS_PTR)pData;

	FTDM_SIS_getSessionCount(pSIS, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_CHAR	pIPAddr[32];

		xRet = FTDM_SIS_getSessionInfo(pSIS, i, &xSession);
		if (xRet == FTM_RET_OK)
		{
			sprintf(pIPAddr,"%d.%d.%d.%d", 
					((xSession.xPeer.sin_addr.s_addr      ) & 0xFF),
					((xSession.xPeer.sin_addr.s_addr >>  8) & 0xFF),
					((xSession.xPeer.sin_addr.s_addr >> 16) & 0xFF),
					((xSession.xPeer.sin_addr.s_addr >> 24) & 0xFF));

			MESSAGE("%2d : %16s %5d %s\n", i+1, pIPAddr, ntohs(xSession.xPeer.sin_port), FTM_TIME_printf(&xSession.xStartTime, NULL));
		}
	}

	return	FTM_RET_OK;
}

FTM_RET FTDM_SHELL_CMD_node
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
	FTM_RET	xRet;
	FTDM_SIS_PTR	pSIS = (FTDM_SIS_PTR)pData;
	FTDM_CONTEXT_PTR	pFTDM = pSIS->pFTDM;

	switch(nArgc)
	{
	case	1:
		{
			FTM_ULONG	i, ulCount;

			MESSAGE("\n[ Node ]\n");
			MESSAGE("\t%16s %16s %8s %16s %16s %16s %8s %8s %16s\n",
					"DID", "NAME", "STATE", "MODEL", "TYPE", "LOCATION", "REPORT", "TIMEOUT", "OPT");
			xRet = FTDM_getNodeCount(pFTDM, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				for(i = 0 ; i < ulCount ; i++)
				{
					FTDM_NODE_PTR	pNode;
					
					xRet = FTDM_getNodeAt(pFTDM, i, &pNode);
					if (xRet == FTM_RET_OK)
					{
						FTM_NODE	xInfo;

						xRet = FTDM_NODE_getInfo(pNode, &xInfo);
						if (xRet == FTM_RET_OK)
						{
							MESSAGE("\t%16s %16s %8s %16s %16s %16s %8lu %8lu ",
									xInfo.pDID, xInfo.pName, "Running", xInfo.pModel, FTM_NODE_typeString(xInfo.xType),
									xInfo.pLocation, xInfo.ulReportInterval, xInfo.ulTimeout);

							switch(xInfo.xType)
							{
							case	FTM_NODE_TYPE_SNMP:
								{
									MESSAGE(" %8s %16s %8s %8s\n",
											FTDM_CFG_SNMP_getVersionString(xInfo.xOption.xSNMP.ulVersion),
											xInfo.xOption.xSNMP.pURL,
											xInfo.xOption.xSNMP.pCommunity,
											xInfo.xOption.xSNMP.pMIB);
								}
								break;


							case	FTM_NODE_TYPE_MODBUS_OVER_TCP:
								{
									MESSAGE(" %16s %16lu %16lu\n",
											xInfo.xOption.xMB.pURL,
											xInfo.xOption.xMB.ulPort,
											xInfo.xOption.xMB.ulSlaveID);
								}
								break;

							case	FTM_NODE_TYPE_FINS:
								{
									MESSAGE(" %16s %02x:%02x:%02x %02x:%02x:%02x %lu\n",
											xInfo.xOption.xFINS.pDIP,
											(FTM_UINT8)((xInfo.xOption.xFINS.ulDA >> 16) & 0xFF), 
											(FTM_UINT8)((xInfo.xOption.xFINS.ulDA >> 8) & 0xFF), 
											(FTM_UINT8)((xInfo.xOption.xFINS.ulDA >> 0) & 0xFF), 
											(FTM_UINT8)((xInfo.xOption.xFINS.ulSA >> 16) & 0xFF), 
											(FTM_UINT8)((xInfo.xOption.xFINS.ulSA >> 8) & 0xFF), 
											(FTM_UINT8)((xInfo.xOption.xFINS.ulSA >> 0) & 0xFF), 
											xInfo.xOption.xFINS.ulServerID);
								}
								break;

							default:
								MESSAGE("\n");
							}
						}
					}

				}
			}
		}
		break;

	case	2:
		{
			if (strcasecmp(pArgv[1], "help") == 0)
			{
				MESSAGE("Usage : %s [<DID> [<cmd>] [<index> <count>]]\n", pArgv[0]);	
				MESSAGE("    Node management\n");
				MESSAGE("  Commands:\n");
				MESSAGE("    %8s  Display data\n", "data");
				MESSAGE("  Parameters:\n");
				MESSAGE("    %8s  Identifer of DID\n", "DID");
				MESSAGE("    %8s  The starting index of the data for display.\n", "index");
				MESSAGE("    %8s  Number of the data for display.\n", "count");
			}
			else
			{
				FTDM_NODE_PTR	pNode = NULL;

				xRet = FTDM_getNode(pSIS->pFTDM, pArgv[1], &pNode);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Node[%s] not found.\n", pArgv[1]);
				}
				else
				{
					FTM_NODE	xInfo;

					FTDM_NODE_getInfo(pNode, &xInfo);

					MESSAGE("%-16s : %s\n", "DID", 		xInfo.pDID);	
					MESSAGE("%-16s : %s\n", "TYPE", 	FTM_NODE_typeString(xInfo.xType)); 
					MESSAGE("%-16s : %s\n", "LOCATION", xInfo.pLocation);
					MESSAGE("%-16s : %lu\n", "INTERVAL",xInfo.ulReportInterval);
					MESSAGE("%-16s : %lu\n", "TIMEOUT", xInfo.ulTimeout);
					MESSAGE("%-16s : %s\n", "OPT 0", 	FTDM_CFG_SNMP_getVersionString(xInfo.xOption.xSNMP.ulVersion));	
					MESSAGE("%-16s : %s\n", "OPT 1", 	xInfo.xOption.xSNMP.pURL);	
					MESSAGE("%-16s : %s\n", "OPT 2", 	xInfo.xOption.xSNMP.pCommunity);	

				}
			}
	
		}
		break;
	}
	return	FTM_RET_OK;
}

FTM_RET FTDM_SHELL_CMD_ep
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
	FTM_RET	xRet;
	FTDM_SIS_PTR	pSIS = (FTDM_SIS_PTR)pData;
	FTDM_CONTEXT_PTR	pFTDM = pSIS->pFTDM;

	switch (nArgc)
	{
	case	1:	
		{
			FTM_ULONG	i, ulCount;

			MESSAGE("\n[ Endpoint ]\n");
			MESSAGE("\t%16s %16s %8s %16s %8s %8s %8s %8s %16s %8s %8s\n",
					"EPID", "NAME", "STATE", "TYPE", "UNIT", "UPDATE", 
					"REPORT", "TIMEOUT", "DID", "COUNT", "TIME");
			if (FTDM_getEPCount(pFTDM, 0, &ulCount) == FTM_RET_OK)
			{
				for(i = 0 ; i < ulCount ; i++)
				{
					FTM_RET		xRet;
					FTDM_EP_PTR	pEP;
					FTM_EP		xInfo;

					xRet = FTDM_getEPAt(pFTDM, i, &pEP);
					if (xRet == FTM_RET_OK)
					{
						xRet = FTDM_EP_getInfo(pEP, &xInfo);
						if (xRet == FTM_RET_OK)
						{
							FTM_ULONG	ulFirstTime, ulLastTime, ulDataCount;

							xRet= FTDM_EP_getDataInfo(pEP, &ulFirstTime, &ulLastTime, &ulDataCount);
							if (xRet == FTM_RET_OK)
							{
								MESSAGE("\t%16s %16s %8s %16s %8s ",
										xInfo.pEPID,
										xInfo.pName,
										(xInfo.bEnable)?"Running":"Stopped",
										FTM_EP_typeString(xInfo.xType),
										xInfo.pUnit);

								MESSAGE("%8lu %8lu %8lu %16s %8lu %10lu %10lu\n",
										xInfo.ulUpdateInterval,
										xInfo.ulReportInterval,
										xInfo.ulTimeout,
										xInfo.pDID,
										ulCount,
										ulFirstTime,
										ulLastTime);
							}
						}
					}
				}
			}
		}
		break;

	case	2:
		{
			if (strcasecmp(pArgv[1], "help") == 0)
			{
								
				MESSAGE("Usage : %s [<EPID> [<cmd>] [<index> <count>]]\n", pArgv[0]);	
				MESSAGE("    EP management\n");
				MESSAGE("  Commands:\n");
				MESSAGE("    %8s  Display data\n", "data");
				MESSAGE("  Parameters:\n");
				MESSAGE("    %8s  Identifer of EP\n", "EPID");
				MESSAGE("    %8s  The starting index of the data for display.\n", "index");
				MESSAGE("    %8s  Number of the data for display.\n", "count");
			}
			else
			{
				FTDM_EP_PTR	pEP;

				xRet = FTDM_getEP(pSIS->pFTDM, pArgv[1], &pEP);
				if (xRet == FTM_RET_OK)
				{
					FTDM_EP_print(pEP);
				
				}

			}
		}
			
		break;

	case	3:
		{
			if (strcasecmp(pArgv[2], "data") == 0)
			{
				FTDM_EP_PTR	pEP;

				xRet = FTDM_getEP(pSIS->pFTDM, pArgv[1], &pEP);
				if (xRet == FTM_RET_OK)
				{
					FTDM_EP_showData(pEP, 0, 100);
				}	
			}
		}
		break;

	case	5:
		{
			if (strcasecmp(pArgv[2], "data") == 0)
			{
				FTDM_EP_PTR	pEP;
				FTM_ULONG		ulStart = 0, ulCount = 0;

				ulStart = strtoul(pArgv[3], NULL, 10);
				ulCount = strtoul(pArgv[4], NULL, 10);

				if (ulStart == 0)
				{
					MESSAGE("Invalid parameter.\n");	
					break;
				}

				xRet = FTDM_getEP(pSIS->pFTDM, pArgv[1], &pEP);
				if (xRet == FTM_RET_OK)
				{
					FTDM_EP_showData(pEP, ulStart - 1, ulCount);
				}	
			}
		}
		break;

	case	6:
		{
			if (strcasecmp(pArgv[2], "data") == 0)
			{
				if (strcasecmp(pArgv[3], "del") == 0)
				{
					FTDM_EP_PTR	pEP;
					FTM_INT			nIndex;
					FTM_ULONG		ulCount = 0;

					nIndex	= strtol(pArgv[4], NULL, 10);
					ulCount = strtoul(pArgv[5], NULL, 10);

					if (nIndex == 0)
					{
						MESSAGE("Invalid parameter.\n");	
						break;
					}

					xRet = FTDM_getEP(pSIS->pFTDM, pArgv[1], &pEP);
					if (xRet == FTM_RET_OK)
					{
						FTDM_EP_deleteData(pEP, nIndex, ulCount, &ulCount);
					}
				}
			}
		}
	}


	return	FTM_RET_OK;
}

FTM_RET FTDM_SHELL_CMD_log
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
	FTM_RET	xRet;
	FTDM_SIS_PTR	pSIS = (FTDM_SIS_PTR)pData;
	FTDM_LOGGER_PTR	pLogger;

	xRet = FTDM_getLogger(pSIS->pFTDM, &pLogger);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Logger is not supported!\n");
		return	FTM_RET_OK;
	}

	switch (nArgc)
	{
	case	1:	
		{
			FTM_ULONG	ulBeginTime, ulEndTime, ulCount;

			xRet = FTDM_LOGGER_info(pLogger, &ulBeginTime, &ulEndTime, &ulCount);
			if (xRet != FTM_RET_OK)
			{
				MESSAGE("Error : %08lx\n", xRet);	
			}
			else
			{
				FTM_CHAR	pTimeString[128];
				memset(pTimeString, 0, sizeof(pTimeString));

				MESSAGE("# Logger Information\n");	
				MESSAGE("%16s : %lu\n", "Count", ulCount);
				if (ulCount != 0)
				{
					strftime(pTimeString, sizeof(pTimeString) - 1, "%Y-%m-%d %H:%M:%S", localtime((const time_t *)&ulBeginTime));
					MESSAGE("%16s : %s\n", "Begin", pTimeString);
					strftime(pTimeString, sizeof(pTimeString) - 1, "%Y-%m-%d %H:%M:%S", localtime((const time_t *)&ulEndTime));
					MESSAGE("%16s : %s\n", "End", pTimeString);
				}
			}
		}
		break;

	case	2:
		{
			if(strcasecmp(pArgv[1], "show") == 0)
			{
				FTM_LOG_PTR	pLogs;
				FTM_ULONG	i, ulCount = 0;
				FTM_CHAR	pTimeString[128];

				memset(pTimeString, 0, sizeof(pTimeString));

				pLogs = (FTM_LOG_PTR)FTM_MEM_malloc(sizeof(FTM_LOG) * 100);
				if (pLogs == NULL)
				{
					MESSAGE("Error : not enough memory!\n");
					break;	
				}
				xRet = FTDM_LOGGER_get(pLogger, 0, pLogs,  100, &ulCount);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Error : %08lx\n", xRet);	
				}

				MESSAGE("# Log List\n");	
				for(i = 0 ; i < ulCount ; i++)
				{
					strftime(pTimeString, sizeof(pTimeString) - 1, "%Y-%m-%d %H:%M:%S", localtime((const time_t *)&pLogs[i].ulTime));
					MESSAGE("%3lu : %16llu %s - %s\n", i+1, pLogs[i].ullID, pTimeString, FTM_LOG_print(&pLogs[i]));
				}

				FTM_MEM_free(pLogs);
			}
		}
			
		break;

	case	3:
		{
		}
		break;
	}

	return	FTM_RET_OK;
}


