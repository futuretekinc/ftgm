#include "ftm_shell.h"
#include "ftom_blocker.h"
#include "ftm_trace.h"

#undef	__MODULE__
#define __MODULE__ FTOM_TRACE_MODULE_CLIENT

FTM_ULONG	ulGetheringTime = 3;

FTM_RET	FTOM_BLOCKER_SHELL_CMD_config
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR 	pData
);

FTM_RET	FTOM_BLOCKER_SHELL_CMD_object
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR 	pData
);


FTM_RET	FTOM_BLOCKER_SHELL_CMD_server
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR 	pData
);

FTM_RET	FTOM_BLOCKER_SHELL_CMD_trace
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
);

FTM_RET	FTOM_BLOCKER_SHELL_CMD_quit
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
);

FTM_SHELL_CMD	FTOM_BLOCKER_SHELL_cmds[] =
{
	{
		.pString	= "config",
		.function	= FTOM_BLOCKER_SHELL_CMD_config,
		.pShortHelp	= "Configuration Management",
		.pHelp		= "\n"\
					  "\tConfiguration Management.\n"
	},
	{
		.pString	= "server",
		.function	= FTOM_BLOCKER_SHELL_CMD_server,
		.pShortHelp	= "Server management.",
		.pHelp		= "\n"\
					  "\tServer management.\n"
	},
	{
		.pString	= "trace",
		.function	= FTOM_BLOCKER_SHELL_CMD_trace,
		.pShortHelp	= "Trace management.",
		.pHelp		= "\n"\
					  "\tTrace management.\n"
	},
	{
		.pString	= "quit",
		.function	= FTOM_BLOCKER_SHELL_CMD_quit,
		.pShortHelp	= "Quit.",
		.pHelp		= "\n"\
					  "\tQuit.\n"
	}
};

FTM_ULONG FTOM_BLOCKER_SHELL_cmdCount = sizeof(FTOM_BLOCKER_SHELL_cmds) / sizeof(FTM_SHELL_CMD);

FTM_RET	FTOM_BLOCKER_SHELL_CMD_config
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	FTM_RET	xRet = FTM_RET_INVALID_ARGUMENTS;	
	FTOM_BLOCKER_PTR	pClient = (FTOM_BLOCKER_PTR)pData;

	switch(nArgc)
	{
	case	1:
		FTOM_BLOCKER_CONFIG_show(pClient);
		break;

	case	3:
		{
			if (strcasecmp(pArgv[1], "save") == 0)
			{
				FTM_CONFIG_PTR	pConfig;

				xRet = FTM_CONFIG_create(pArgv[2], &pConfig, FTM_FALSE);
				if (xRet != FTM_RET_OK)
				{   
					MESSAGE("Failed to create configuration file!\n");
					break;
				}   

				xRet = FTOM_BLOCKER_CONFIG_save(pClient, pConfig);
				if (xRet != FTM_RET_OK)
				{   
					MESSAGE("Failed to save configuration!\n");
				}

				xRet = FTM_CONFIG_destroy(&pConfig);
				if (xRet == FTM_RET_OK)
				{
					MESSAGE("This configuration has been saved successfully[%s].\n", pArgv[2]);	
				}
			}
		}

		break;
	}

	return	xRet;
}

FTM_RET	FTOM_BLOCKER_SHELL_CMD_server
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	FTM_RET	xRet;
	FTOM_BLOCKER_PTR	pBlocker = (FTOM_BLOCKER_PTR)pData;
	FTM_ULONG	ulCount, i;
	
	if (pBlocker == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xRet = FTM_SIS_CMD_count(pBlocker->pSIS, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Failed to get service interface command count!\n");	
		return	xRet;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_SIS_CMD_PTR	pCmd;

		xRet = FTM_SIS_CMD_getAt(pBlocker->pSIS, i, &pCmd);	
		if (xRet == FTM_RET_OK)
		{
			MESSAGE("%2lu : %s\n", i+1, pCmd->pName);	
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_BLOCKER_SHELL_CMD_trace
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{

	return	FTM_TRACE_shellCmd(pShell, nArgc, pArgv, pData);
}

FTM_RET	FTOM_BLOCKER_SHELL_CMD_quit
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	FTOM_BLOCKER_PTR	pBlocker = (FTOM_BLOCKER_PTR)pData;
	
	if (pBlocker == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	FTOM_BLOCKER_stop(pBlocker);

	return	FTM_RET_SHELL_QUIT;
}

