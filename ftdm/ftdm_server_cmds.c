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

FTM_RET	FTDMS_SHELL_CMD_config(FTM_SHELL_PTR pShell, FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTDMS_SHELL_CMD_object(FTM_SHELL_PTR pShell, FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTDMS_SHELL_CMD_session(FTM_SHELL_PTR pShell, FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTDMS_SHELL_CMD_trace(FTM_SHELL_PTR pShell, FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTDMS_SHELL_CMD_node(FTM_SHELL_PTR pShell, FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTDMS_SHELL_CMD_ep(FTM_SHELL_PTR pShell, FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[], FTM_VOID_PTR	pData);
FTM_RET	FTDMS_SHELL_CMD_EP_showData(FTM_SHELL_PTR pShell, FTM_EP_ID	xEPID, FTM_ULONG ulBegin, FTM_ULONG ulCount);

static FTDM_CFG	xConfig;
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
		.pString	= "object",
		.function	= FTDMS_SHELL_CMD_object,
		.pShortHelp	= "Object management.",
		.pHelp		= "\n"\
					  "\tObject management.\n"
	},
	{
		.pString	= "trace",
		.function	= FTDMS_SHELL_CMD_trace,
		.pShortHelp	= "Trace configuration.",
		.pHelp		= "\n"\
					  "\tTrace configuration.\n"
	},
	{
		.pString	= "session",
		.function	= FTDMS_SHELL_CMD_session,
		.pShortHelp	= "Session Manager.",
		.pHelp		= "\n"\
					  "\tSession Manager.\n"
	},
	{
		.pString	= "node",
		.function	= FTDMS_SHELL_CMD_node,
		.pShortHelp	= "Node Manager.",
		.pHelp		= "\n"\
					  "\tNode Manager.\n"
	},
	{
		.pString	= "ep",
		.function	= FTDMS_SHELL_CMD_ep,
		.pShortHelp	= "End Point Manager.",
		.pHelp		= "\n"\
					  "\tEnd Point Manager.\n"
	},
};

FTM_ULONG		FTDMS_ulCmdCount = sizeof(FTDMS_pCmdList) / sizeof(FTM_SHELL_CMD);
FTM_CHAR_PTR	FTDMS_pPrompt = "FTDMS> ";

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
			FTDM_TRIGGER_showList();
			FTDM_ACTION_showList();
			FTDM_RULE_showList();
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
	FTM_ULONG		i, ulCount;
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

		MESSAGE("%2d : %16s %5d\n", i+1, pIPAddr, xSession.xPeer.sin_port);
	}

	return	FTM_RET_OK;
}

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
				MESSAGE("    %8s   Set trace level\n", "level");
				MESSAGE("  Parameters:\n");
				MESSAGE("    %8s   trace level (0 ~ 6)\n");
			}
		}
		break;

	case	3:
		{
			if (strcasecmp(pArgv[1], "level") == 0)
			{
				FTM_ULONG	ulLevel;

				ulLevel = strtoul(pArgv[2], NULL, 10);
				FTM_TRACE_setLevel(ulLevel);
			}
		}
		break;
	}

	return	FTM_RET_OK;
}

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
				FTM_EP_ID 		xEPID;

				xEPID = strtoul(pArgv[1], NULL, 16);

				FTDM_SHELL_showEPInfo(pServer->pDM->pEPM, xEPID);
			}
		}
			
		break;

	case	3:
		{
			if (strcasecmp(pArgv[2], "data") == 0)
			{
				FTM_EP_ID 		xEPID;

				xEPID = strtoul(pArgv[1], NULL, 16);

				FTDM_SHELL_showEPData(pServer->pDM->pEPM, xEPID, 0, 100);
			}
		}
		break;

	case	5:
		{
			if (strcasecmp(pArgv[2], "data") == 0)
			{
				FTM_EP_ID 		xEPID;
				FTM_ULONG		ulStart = 0, ulCount = 0;

				xEPID 	= strtoul(pArgv[1], NULL, 16);
				ulStart = strtoul(pArgv[3], NULL, 10);
				ulCount = strtoul(pArgv[4], NULL, 10);

				if (ulStart == 0)
				{
					MESSAGE("Invalid parameter.\n");	
					break;
				}

				FTDM_SHELL_showEPData(pServer->pDM->pEPM, xEPID, ulStart - 1, ulCount);
			}
		}
		break;

	case	6:
		{
			if (strcasecmp(pArgv[2], "data") == 0)
			{
				if (strcasecmp(pArgv[3], "del") == 0)
				{
					FTM_EP_ID 		xEPID;
					FTM_INT			nIndex;
					FTM_ULONG		ulCount = 0;

					xEPID 	= strtoul(pArgv[1], NULL, 16);
					nIndex	= strtol(pArgv[4], NULL, 10);
					ulCount = strtoul(pArgv[5], NULL, 10);

					if (nIndex == 0)
					{
						MESSAGE("Invalid parameter.\n");	
						break;
					}

					FTDM_SHELL_delEPData(pServer->pDM->pEPM, xEPID, nIndex, ulCount);
				}
			}
		}
	}


	return	FTM_RET_OK;
}

