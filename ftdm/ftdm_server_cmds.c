#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "ftdm.h"
#include "ftm_mem.h"
#include "ftdm_params.h"
#include "ftdm_shell_cmds.h"
#include "ftdm_config.h"
#include "ftdm_server.h"
#include "ftdm_server_cmds.h"
#include "ftdm_trigger.h"
#include "ftdm_action.h"
#include "ftdm_rule.h"

static
FTM_RET	FTDMS_SHELL_CMD_config
(
	FTM_SHELL_PTR 	pShell, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR	pData
);

static
FTM_RET	FTDMS_SHELL_CMD_object
(
	FTM_SHELL_PTR 	pShell, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR	pData
);

static
FTM_RET	FTDMS_SHELL_CMD_session
(
	FTM_SHELL_PTR 	pShell, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR	pData
);

#if 0
static
FTM_RET	FTDMS_SHELL_CMD_trace
(
	FTM_SHELL_PTR 	pShell, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR	pData
);
#endif

static
FTM_RET	FTDMS_SHELL_CMD_node
(
	FTM_SHELL_PTR 	pShell, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR	pData
);

static
FTM_RET	FTDMS_SHELL_CMD_ep
(
	FTM_SHELL_PTR 	pShell, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR	pData
);

static
FTM_RET	FTDMS_SHELL_CMD_log
(
	FTM_SHELL_PTR 	pShell, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR	pData
);

FTM_RET	FTDMS_SHELL_CMD_EP_showData
(
	FTM_SHELL_PTR 	pShell, 
	FTM_CHAR_PTR 	pEPID, 
	FTM_ULONG 		ulBegin, 
	FTM_ULONG 		ulCount
);

static 
FTDM_CFG	xConfig;
FTM_SHELL_CMD	FTDMS_pCmdList[] =
{
	{
		.pString	= "config",
		.function	= FTDMS_SHELL_CMD_config,
		.pShortHelp	= "Configuration Data Manager.",
		.pHelp		= "\n"\
					  "\tConfiguration Data Manager.\n"
	},
	{
		.pString	= "ep",
		.function	= FTDMS_SHELL_CMD_ep,
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
		.function	= FTDMS_SHELL_CMD_log,
		.pShortHelp	= "Log management.",
		.pHelp		= "\n"\
					  "\tLog management.\n"
	},
	{
		.pString	= "node",
		.function	= FTDMS_SHELL_CMD_node,
		.pShortHelp	= "Node Manager.",
		.pHelp		= "\n"\
					  "\tNode Manager.\n"
	},
	{
		.pString	= "object",
		.function	= FTDMS_SHELL_CMD_object,
		.pShortHelp	= "Object management.",
		.pHelp		= "\n"\
					  "\tObject management.\n"
	},
	{
		.pString	= "session",
		.function	= FTDMS_SHELL_CMD_session,
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

FTM_ULONG		FTDMS_ulCmdCount = sizeof(FTDMS_pCmdList) / sizeof(FTM_SHELL_CMD);
FTM_CHAR_PTR	FTDMS_pPrompt = "FTDMS";

FTM_RET	FTDMS_SHELL_CMD_config
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

FTM_RET	FTDMS_SHELL_CMD_object
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
	FTDM_SERVER_PTR	pServer = (FTDM_SERVER_PTR)pData;

	FTM_RET	xRet;

	TRACE("pServer = %08x, pServer->pDM = %08x\n", pServer, pServer->pDM);
	switch (nArgc)
	{
	case	1:
		{
			FTDM_NODEM_showList(pServer->pDM->pNodeM);
			FTDM_EPM_showList(pServer->pDM->pEPM);
			FTDM_RULE_showList();
			FTDM_TRIGGER_showList();
			FTDM_ACTION_showList();
		}	
		break;

	case	3:
		{
			if (strcasecmp(pArgv[1], "load") == 0)
			{
				xRet = FTDM_loadObjectFromFile(pServer->pDM, pArgv[2]);	
				if (xRet != FTM_RET_OK)
				{
					ERROR("Objects loading failed.[%08x]\n", xRet);
					return	xRet;
				}

				MESSAGE("Objects loading completed.\n");
			}
		}
		break;

	default:
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDMS_SHELL_CMD_session
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
	FTDM_SERVER_PTR	pServer;

	xRet = FTDM_getServer(&pServer);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't found server!\n");
		return	xRet;
	}

	FTDMS_getSessionCount(pServer, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_CHAR	pIPAddr[32];

		FTDMS_getSessionInfo(pServer, i, &xSession);

		sprintf(pIPAddr,"%d.%d.%d.%d", 
				((xSession.xPeer.sin_addr.s_addr      ) & 0xFF),
				((xSession.xPeer.sin_addr.s_addr >>  8) & 0xFF),
				((xSession.xPeer.sin_addr.s_addr >> 16) & 0xFF),
				((xSession.xPeer.sin_addr.s_addr >> 24) & 0xFF));

		MESSAGE("%2d : %16s %5d %s\n", i+1, pIPAddr, ntohs(xSession.xPeer.sin_port), FTM_TIME_printf(&xSession.xStartTime, NULL));
	}

	return	FTM_RET_OK;
}

#if 0
FTM_RET FTDMS_SHELL_CMD_trace
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
	switch(nArgc)
	{
	case	1:
		{
			MESSAGE("# Trace Configuration\n");
			FTM_TRACE_printConfig(&xConfig.xPrint);
		}
		break;

	case	2:
		{
			if (strcasecmp(pArgv[1], "help") == 0)
			{
				MESSAGE("Usage : %s [<cmd> <level>]\n",	pArgv[0]);
				MESSAGE("    Trace configuration\n");
				MESSAGE("  Commands:\n");
				MESSAGE("    %8s   Set trace level\n", "set");
				MESSAGE("  Parameters:\n");
				MESSAGE("    %8s   trace level (0 ~ 6)\n", "level");
			}
		}
		break;

	case	4:
		{
			FTM_RET					xRet;
			FTM_TRACE_MODULE_TYPE	xType;

			xRet = FTM_TRACE_getType(pArgv[1], &xType);
			if (xRet != FTM_RET_OK)
			{
				MESSAGE("Invalid module name[%s]!\n", pArgv[1]);
			}
			else
			{
				if (strcasecmp(pArgv[2], "level") == 0)
				{
					FTM_ULONG	ulLevel;
	
					xRet = FTM_TRACE_strToLevel(pArgv[3], &ulLevel);
					if (xRet != FTM_RET_OK)
					{
						MESSAGE("Invalid level[%s]!\n", pArgv[3]);
					}
					else
					{
						FTM_TRACE_setLevel(xType, ulLevel);
					}
				}
			}
		}
		break;
	}

	return	FTM_RET_OK;
}
#endif

FTM_RET FTDMS_SHELL_CMD_node
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
	FTDM_SERVER_PTR	pServer = (FTDM_SERVER_PTR)pData;

	switch(nArgc)
	{
	case	1:
		{
			FTDM_SHELL_showNodeList(pServer->pDM->pNodeM);
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
				FTDM_SHELL_showNodeInfo(pServer->pDM->pNodeM, pArgv[1]);
			}
	
		}
		break;
	}
	return	FTM_RET_OK;
}

FTM_RET FTDMS_SHELL_CMD_ep
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
	FTDM_SERVER_PTR	pServer = (FTDM_SERVER_PTR)pData;

	switch (nArgc)
	{
	case	1:	
		{
			FTDM_SHELL_showEPList(pServer->pDM->pEPM);
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
				FTDM_SHELL_showEPInfo(pServer->pDM->pEPM, pArgv[1]);
			}
		}
			
		break;

	case	3:
		{
			if (strcasecmp(pArgv[2], "data") == 0)
			{
				FTDM_SHELL_showEPData(pServer->pDM->pEPM, pArgv[1], 0, 100);
			}
		}
		break;

	case	5:
		{
			if (strcasecmp(pArgv[2], "data") == 0)
			{
				FTM_ULONG		ulStart = 0, ulCount = 0;

				ulStart = strtoul(pArgv[3], NULL, 10);
				ulCount = strtoul(pArgv[4], NULL, 10);

				if (ulStart == 0)
				{
					MESSAGE("Invalid parameter.\n");	
					break;
				}

				FTDM_SHELL_showEPData(pServer->pDM->pEPM, pArgv[1], ulStart - 1, ulCount);
			}
		}
		break;

	case	6:
		{
			if (strcasecmp(pArgv[2], "data") == 0)
			{
				if (strcasecmp(pArgv[3], "del") == 0)
				{
					FTM_INT			nIndex;
					FTM_ULONG		ulCount = 0;

					nIndex	= strtol(pArgv[4], NULL, 10);
					ulCount = strtoul(pArgv[5], NULL, 10);

					if (nIndex == 0)
					{
						MESSAGE("Invalid parameter.\n");	
						break;
					}

					FTDM_SHELL_delEPData(pServer->pDM->pEPM, pArgv[1], nIndex, ulCount);
				}
			}
		}
	}


	return	FTM_RET_OK;
}

FTM_RET FTDMS_SHELL_CMD_log
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
	FTM_RET	xRet;
	FTDM_SERVER_PTR	pServer = (FTDM_SERVER_PTR)pData;

	switch (nArgc)
	{
	case	1:	
		{
			FTM_ULONG	ulBeginTime, ulEndTime, ulCount;

			xRet = FTDM_LOGGER_info(pServer->pDM->pLogger, &ulBeginTime, &ulEndTime, &ulCount);
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
				xRet = FTDM_LOGGER_get(pServer->pDM->pLogger, 0, pLogs,  100, &ulCount);
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

