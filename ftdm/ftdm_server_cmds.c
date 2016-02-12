#include <sys/socket.h>
#include <arpa/inet.h>
#include "ftdm.h"
#include "ftdm_params.h"
#include "ftdm_server.h"
#include "ftdm_server_cmds.h"

FTM_RET	FTDMS_CONSOLE_CMD_config(FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[]);
FTM_RET	FTDMS_CONSOLE_CMD_session(FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[]);

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
		.pString	= "session",
		.function	= FTDMS_CONSOLE_CMD_session,
		.pShortHelp	= "Session Manager.",
		.pHelp		= "\n"\
					  "\tSession Manager.\n"
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


