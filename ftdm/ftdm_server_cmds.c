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
#include "ftdm_event.h"
#include "ftdm_act.h"

FTM_RET	FTDMS_SHELL_CMD_config(FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[]);
FTM_RET	FTDMS_SHELL_CMD_session(FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[]);
FTM_RET	FTDMS_SHELL_CMD_debug(FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[]);
FTM_RET	FTDMS_SHELL_CMD_node(FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[]);
FTM_RET	FTDMS_SHELL_CMD_ep(FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[]);
FTM_RET	FTDMS_SHELL_CMD_EP_showData(FTM_EPID	xEPID, FTM_ULONG ulBegin, FTM_ULONG ulCount);

extern FTDM_CFG	xConfig;
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
		.pString	= "debug",
		.function	= FTDMS_SHELL_CMD_debug,
		.pShortHelp	= "Debug Manager.",
		.pHelp		= "\n"\
					  "\tDebug Manager.\n"
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
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
)
{
	if (nArgc == 1)
	{
		FTDM_CFG_show(&xConfig);
		FTDM_EVENT_showList();
		FTDM_ACT_showList();
		return	FTM_RET_OK;
	}

	return	FTM_RET_INVALID_ARGUMENTS;
}

FTM_RET	FTDMS_SHELL_CMD_session
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
)
{
	FTM_ULONG	i, ulCount;
	FTDM_SESSION	xSession;

	FTDMS_getSessionCount(&ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_CHAR	pIPAddr[32];

		FTDMS_getSessionInfo(i, &xSession);

		sprintf(pIPAddr,"%d.%d.%d.%d", 
				((xSession.xPeer.sin_addr.s_addr      ) & 0xFF),
				((xSession.xPeer.sin_addr.s_addr >>  8) & 0xFF),
				((xSession.xPeer.sin_addr.s_addr >> 16) & 0xFF),
				((xSession.xPeer.sin_addr.s_addr >> 24) & 0xFF));

		MESSAGE("%2d : %16s %5d\n", i+1, pIPAddr, xSession.xPeer.sin_port);
	}

	return	FTM_RET_OK;
}

FTM_RET FTDMS_SHELL_CMD_debug
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
)
{
	switch(nArgc)
	{
	case	1:
		{
			FTM_ULONG	ulLevel;

			FTM_PRINT_getLevel(&ulLevel);
			MESSAGE("LEVEL : %s\n", FTM_PRINT_levelString(ulLevel));
		}
		break;

	case	2:
		{
			if (strcasecmp(pArgv[1], "help") == 0)
			{
				MESSAGE("Usage : %s [<cmd> <level>]\n",	pArgv[0]);
				MESSAGE("    Debugging configuration\n");
				MESSAGE("  Commands:\n");
				MESSAGE("    %8s   Set debugging level\n", "level");
				MESSAGE("  Parameters:\n");
				MESSAGE("    %8s   Debugging level (0 ~ 10)\n");
			}
		}
		break;

	case	3:
		{
			if (strcasecmp(pArgv[1], "level") == 0)
			{
				FTM_ULONG	ulLevel;

				ulLevel = strtoul(pArgv[2], NULL, 10);
				FTM_PRINT_setLevel(ulLevel);
			}
		}
		break;
	}

	return	FTM_RET_OK;
}

FTM_RET FTDMS_SHELL_CMD_node
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
)
{
	switch(nArgc)
	{
	case	1:
		{
			FTDM_SHELL_showNodeList();
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
				FTDM_SHELL_showNodeInfo(pArgv[1]);
			}
	
		}
		break;
	}
	return	FTM_RET_OK;
}

FTM_RET FTDMS_SHELL_CMD_ep
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
)
{

	switch (nArgc)
	{
	case	1:	
		{
			FTDM_SHELL_showEPList();
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
				FTM_EPID 		xEPID;

				xEPID = strtoul(pArgv[1], NULL, 16);

				FTDM_SHELL_showEPInfo(xEPID);
			}
		}
			
		break;

	case	3:
		{
			if (strcasecmp(pArgv[2], "data") == 0)
			{
				FTM_EPID 		xEPID;

				xEPID = strtoul(pArgv[1], NULL, 16);

				FTDM_SHELL_showEPData(xEPID, 0, 100);
			}
		}
		break;

	case	5:
		{
			if (strcasecmp(pArgv[2], "data") == 0)
			{
				FTM_EPID 		xEPID;
				FTM_ULONG		ulStart = 0, ulCount = 0;

				xEPID 	= strtoul(pArgv[1], NULL, 16);
				ulStart = strtoul(pArgv[3], NULL, 10);
				ulCount = strtoul(pArgv[4], NULL, 10);

				if (ulStart == 0)
				{
					MESSAGE("Invalid parameter.\n");	
					break;
				}

				FTDM_SHELL_showEPData(xEPID, ulStart - 1, ulCount);
			}
		}
		break;
	}


	return	FTM_RET_OK;
}

