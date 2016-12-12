#include "ftm_shell.h"
#include "ftom_azure_client.h"
#include "ftom.h"

#undef	__MODULE__
#define __MODULE__ FTOM_TRACE_MODULE_CLIENT

FTM_ULONG	ulGetheringTime = 3;

FTM_RET	FTOM_AZURE_SHELL_CMD_config
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR 	pData
);

FTM_RET	FTOM_AZURE_SHELL_CMD_object
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR 	pData
);

FTM_RET	FTOM_AZURE_SHELL_CMD_task
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR 	pData
);

FTM_RET	FTOM_AZURE_SHELL_CMD_discovery
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR 	pData
);

FTM_RET	FTOM_AZURE_SHELL_CMD_server
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR 	pData
);

FTM_SHELL_CMD	FTOM_AZURE_shellCmds[] =
{
	{
		.pString	= "config",
		.function	= FTOM_AZURE_SHELL_CMD_config,
		.pShortHelp	= "Configuration Management",
		.pHelp		= "\n"\
					  "\tConfiguration Management.\n"
	},
	{
		.pString	= "server",
		.function	= FTOM_AZURE_SHELL_CMD_server,
		.pShortHelp	= "Server management.",
		.pHelp		= "\n"\
					  "\tServer management.\n"
	}
};

FTM_ULONG FTOM_AZURE_shellCmdCount = sizeof(FTOM_AZURE_shellCmds) / sizeof(FTM_SHELL_CMD);

FTM_RET	FTOM_AZURE_SHELL_CMD_config
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;

	switch(nArgc)
	{
	case	1:
		FTOM_CLIENT_showConfig(pClient);
		break;

	case	3:
		{
			if (strcasecmp(pArgv[1], "save") == 0)
			{
				FTM_RET	xRet;

				xRet = FTOM_CLIENT_saveConfigToFile(pClient, pArgv[2]);
				if (xRet == FTM_RET_OK)
				{
					MESSAGE("This configuration has been saved successfully[%s].\n", pArgv[2]);	
				}
				else
				{
					MESSAGE("Failed to save configuration to a file[%s].\n", pArgv[2]);	
				}
			}
		}		
		break;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_AZURE_SHELL_CMD_server
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	FTOM_AZURE_CLIENT_PTR	pClient = (FTOM_AZURE_CLIENT_PTR)pData;
	
	if (pClient == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	return	FTM_RET_OK;
}

