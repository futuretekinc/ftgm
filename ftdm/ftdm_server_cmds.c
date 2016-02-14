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
#include "ftdm_server_cmds.h"

FTM_RET	FTDMS_CONSOLE_CMD_config(FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[]);
FTM_RET	FTDMS_CONSOLE_CMD_session(FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[]);
FTM_RET	FTDMS_CONSOLE_CMD_debug(FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[]);
FTM_RET	FTDMS_CONSOLE_CMD_node(FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[]);
FTM_RET	FTDMS_CONSOLE_CMD_ep(FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[]);
FTM_RET	FTDMS_CONSOLE_CMD_EP_showData(FTM_EPID	xEPID, FTM_ULONG ulBegin, FTM_ULONG ulCount);

extern FTDM_CFG	xConfig;
FTM_CONSOLE_CMD	FTDMS_pCmdList[] =
{
	{
		.pString	= "config",
		.function	= FTDMS_CONSOLE_CMD_config,
		.pShortHelp	= "Configuration Data Manager.",
		.pHelp		= "\n"\
					  "\tConfiguration Data Manager.\n"
	},
	{
		.pString	= "debug",
		.function	= FTDMS_CONSOLE_CMD_debug,
		.pShortHelp	= "Debug Manager.",
		.pHelp		= "\n"\
					  "\tDebug Manager.\n"
	},
	{
		.pString	= "session",
		.function	= FTDMS_CONSOLE_CMD_session,
		.pShortHelp	= "Session Manager.",
		.pHelp		= "\n"\
					  "\tSession Manager.\n"
	},
	{
		.pString	= "node",
		.function	= FTDMS_CONSOLE_CMD_node,
		.pShortHelp	= "Node Manager.",
		.pHelp		= "\n"\
					  "\tNode Manager.\n"
	},
	{
		.pString	= "ep",
		.function	= FTDMS_CONSOLE_CMD_ep,
		.pShortHelp	= "End Point Manager.",
		.pHelp		= "\n"\
					  "\tEnd Point Manager.\n"
	},
};

FTM_ULONG		FTDMS_ulCmdCount = sizeof(FTDMS_pCmdList) / sizeof(FTM_CONSOLE_CMD);
FTM_CHAR_PTR	FTDMS_pPrompt = "FTDMS>";

FTM_RET	FTDMS_CONSOLE_CMD_config
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
)
{
	if (nArgc == 1)
	{
		FTDM_CFG_show(&xConfig);
		return	FTM_RET_OK;
	}

	return	FTM_RET_INVALID_ARGUMENTS;
}

FTM_RET	FTDMS_CONSOLE_CMD_session
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

FTM_RET FTDMS_CONSOLE_CMD_debug
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
			MESSAGE("MODE : %s\n", FTM_PRINT_levelString(ulLevel));
		}
		break;

	}
	return	FTM_RET_OK;
}

FTM_RET FTDMS_CONSOLE_CMD_node
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
)
{
	return	FTM_RET_OK;
}

FTM_RET FTDMS_CONSOLE_CMD_ep
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
)
{

	switch (nArgc)
	{
	case	1:	
		{
			FTM_ULONG	ulCount;

			MESSAGE("# PRE-REGISTERED ENDPOINT\n");
			MESSAGE("%-5s %-8s %-16s %-16s %-8s %-8s %-8s %-8s %-16s %-8s\n",
					"INDEX", "EPID", "TYPE", "NAME", "UNIT", "STATE", "INTERVAL", "TIMEOUT", "DID", "DEPID");
			if (FTDM_CFG_EP_INFO_count(&xConfig.xEP, &ulCount) == FTM_RET_OK)
			{
				FTM_ULONG	i;

				for(i = 0 ; i < ulCount ; i++)
				{
					FTM_EP_INFO	xEPInfo;
		
					FTDM_CFG_EP_INFO_getAt(&xConfig.xEP, i, &xEPInfo);
					MESSAGE("%5d %08lx %-16s %-16s %-8s ",
						i+1,
						xEPInfo.xEPID,
						FTDM_CFG_EP_getTypeString(xEPInfo.xType),
						xEPInfo.pName,
						xEPInfo.pUnit);
		
					switch(xEPInfo.xState)
					{
					case	FTM_EP_STATE_DISABLE: 	MESSAGE("%-8s ", "DISABLE");  	break; 
					case	FTM_EP_STATE_RUN: 		MESSAGE("%-8s ", "RUN"); 		break; 
					case	FTM_EP_STATE_STOP: 		MESSAGE("%-8s ", "STOP"); 		break;
					case	FTM_EP_STATE_ERROR: 	MESSAGE("%-8s ", "ERROR"); 		break;
					default: MESSAGE("%-8s ", "UNKNOWN");
					}
		
					MESSAGE("%-8lu %-8lu %-16s %08lx\n",
						xEPInfo.ulInterval,
						xEPInfo.ulTimeout,
						xEPInfo.pDID,
						xEPInfo.xDEPID);
				}
			}

		}
		break;

	case	3:
		{
			if (strcasecmp(pArgv[2], "show") == 0)
			{
				FTM_EPID 		xEPID;

				xEPID = strtoul(pArgv[1], NULL, 16);

				MESSAGE("xEPID = %08x\n", (FTM_ULONG)xEPID);
				FTDMS_CONSOLE_CMD_EP_showData(xEPID, 0, 100);
			}
		}
		break;
	}


	return	FTM_RET_OK;
}

FTM_RET	FTDMS_CONSOLE_CMD_EP_showData(FTM_EPID	xEPID, FTM_ULONG ulBegin, FTM_ULONG ulCount)
{
	FTM_RET			xRet;
	FTM_ULONG		i, ulTotalCount = 0;
	FTM_EP_DATA_PTR pData;

	xRet = FTDM_EP_DATA_count(xEPID, &ulTotalCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR("%08x is not exists.\n", xEPID);
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	if (ulTotalCount <= ulBegin) 
	{
		return	FTM_RET_OK;
	}

	if (ulCount > (ulTotalCount - ulBegin))
	{
		ulCount = ulTotalCount - ulBegin;
	}

	pData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA) * ulCount);
	if (pData == NULL)
	{
		ERROR("System is not enough memory!\n");
		return	FTM_RET_NOT_ENOUGH_MEMORY;
	}

	xRet = FTDM_EP_DATA_get(xEPID, ulBegin, pData, ulCount, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pData);
		return	xRet;
	}

	for(i = 0 ; i < ulCount; i++)
	{
		FTM_CHAR	pTime[64];

		strcpy(pTime, ctime((time_t *)&pData[i].ulTime));
		pTime[strlen(pTime) - 1] = '\0';
		MESSAGE("%4d : %16s %10d %d\n", ulBegin + i + 1, pTime, pData[i].xState, pData[i].xValue.ulValue);
	}

	FTM_MEM_free(pData);

	return	FTM_RET_OK;
}
