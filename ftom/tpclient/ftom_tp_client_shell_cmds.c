#include "ftom.h"
#include "ftm_shell.h"
#include "ftom_tp_client.h"

FTM_ULONG	ulGetheringTime = 3;

FTM_RET	FTOM_SHELL_CMD_config
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR 	pData
);

FTM_RET	FTOM_SHELL_CMD_object
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR 	pData
);

FTM_RET	FTOM_SHELL_CMD_task
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR 	pData
);

FTM_RET	FTOM_SHELL_CMD_quit
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR 	pData
);

FTM_RET	FTOM_SHELL_CMD_discovery
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR 	pData
);

FTM_RET	FTOM_SHELL_CMD_server
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR 	pData
);

FTM_SHELL_CMD	FTOM_shellCmds[] = 
{
	{
		.pString	= "config",
		.function	= FTOM_SHELL_CMD_config,
		.pShortHelp	= "Configuration Management",
		.pHelp		= "\n"\
					  "\tConfiguration Management.\n"
	},
#if 0
	{
		.pString	= "object",
		.function	= FTOM_SHELL_CMD_object,
		.pShortHelp	= "show object.",
		.pHelp		= "\n"\
					  "\tShow object.\n"
	},
#endif
	{
		.pString	= "quit",
		.function	= FTOM_SHELL_CMD_quit,
		.pShortHelp	= "quit.",
		.pHelp		= "\n"\
					  "\tquit.\n"
	},
#if 0
	{
		.pString	= "task",
		.function	= FTOM_SHELL_CMD_task,
		.pShortHelp	= "task management",
		.pHelp		= "\n"\
					  "\ttask management.\n"
	},
#endif
	{
		.pString	= "server",
		.function	= FTOM_SHELL_CMD_server,
		.pShortHelp	= "Server management.",
		.pHelp		= "\n"\
					  "\tServer management.\n"
	},
#if 0
	{
		.pString	= "discovery",
		.function	= FTOM_SHELL_CMD_discovery,
		.pShortHelp	= "Node discovery.",
		.pHelp		= "\n"\
					  "\tNode discovery.\n"
	}
#endif
};

FTM_ULONG		FTOM_shellCmdCount = sizeof(FTOM_shellCmds) / sizeof(FTM_SHELL_CMD);


FTM_RET	FTOM_SHELL_CMD_config
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	FTOM_TP_CLIENT_PTR	pClient = (FTOM_TP_CLIENT_PTR)pData;

	switch(nArgc)
	{
	case	1:
		FTOM_TP_CLIENT_showConfig(pClient);
		break;

	case	3:
		{
			if (strcasecmp(pArgv[1], "save") == 0)
			{
				FTM_RET	xRet;

				xRet = FTOM_TP_CLIENT_saveConfigToFile(pClient, pArgv[2]);
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

FTM_RET	FTOM_SHELL_CMD_object
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	switch(nArgc)
	{
	case	1:
		{
			//FTOM_NODE_printList();
			//FTOM_EP_printList();
			//FTOM_TRIGGER_printList();
			//FTOM_ACTION_printList();
			//FTOM_RULE_printList();
		}
		break;

	case	2:
		{
#if 0
			FTM_RET	xRet;
			FTOM_NODE_PTR	pNode;
			FTOM_EP_PTR		pEP;

			xRet = FTOM_NODE_get(pArgv[1], &pNode);
			if (xRet == FTM_RET_OK)
			{
				FTOM_NODE_print(pNode);	
				break;
			}

			xRet = FTOM_EP_get(pArgv[1], &pEP);
			if (xRet == FTM_RET_OK)
			{
				FTOM_EP_print(pEP);
				break;
			}
#endif
		}
		break;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SHELL_CMD_quit
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	FTM_RET	xRet;
	FTOM_TP_CLIENT_PTR	pClient = (FTOM_TP_CLIENT_PTR)pData;
	
	xRet = FTOM_TP_CLIENT_stop(pClient);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_RET_SHELL_QUIT;
}

FTM_RET	FTOM_SHELL_CMD_task
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	return	FTM_RET_OK;

}

FTM_RET	FTOM_SHELL_CMD_server
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	FTM_RET	xRet ;
	FTOM_TP_CLIENT_PTR	pClient = (FTOM_TP_CLIENT_PTR)pData;
	
	if (pClient == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	switch(nArgc)
	{
	case	2:
		{
			if (strcasecmp(pArgv[1], "sync") == 0)
			{
				xRet = FTOM_TP_CLIENT_serverSyncStart(pClient, FTM_FALSE);
				if (xRet == FTM_RET_OK)
				{
					MESSAGE("Start the server synchronization.\n");
				}
				else
				{
					MESSAGE("Failed to start the server synchronization.\n");
				}
			}
		}
		break;

	case	3:
		{
			if (strcasecmp(pArgv[1], "sync") == 0)
			{
				if (strcasecmp(pArgv[2], "auto") == 0)
				{
					xRet = FTOM_TP_CLIENT_serverSyncStart(pClient, FTM_TRUE);
					if (xRet == FTM_RET_OK)
					{
						MESSAGE("Start the server synchronization.\n");
						MESSAGE("The new sensor will be automatically registered.\n");
					}
					else
					{
						MESSAGE("Failed to start the server synchronization.\n");
					}
				}
			}
		}
		break;


	}
	return	FTM_RET_OK;
}

#if 0
FTM_RET	FTOM_SHELL_CMD_discovery
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	FTM_RET		xRet;
	FTOM_SERVICE_PTR	pService;
	
	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DISCOVERY, &pService);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Discovery not supported!\n");			
		return	FTM_RET_OK;
	}

	switch(nArgc)
	{
	case	1:
		{
			FTM_BOOL	bFinished;
			FTM_INT		i;
			FTM_ULONG	ulCount;

			xRet = FTOM_discoveryStart("255.255.255.255", 1234, 1);
			if (xRet != FTM_RET_OK)
			{
				MESSAGE("Discovery request was failed[%08x].\n", xRet);	
			}
			else
			{
				MESSAGE("Discovery request is complete.\n");	
			}

			usleep(1000000);
			do
			{
				bFinished = FTM_FALSE;
				FTOM_discoveryIsFinished(&bFinished);
				usleep(100000);
			}
			while(!bFinished);

			MESSAGE("Discovery finished!\n");
			xRet = FTOM_TP_CLIENT_DISCOVERY_getEPInfoCount(pClient, pService->pData, &ulCount);
			if (xRet != FTM_RET_OK)
			{
				MESSAGE("Discovery get EP Info count failed.\n");
			}
			else
			{
				for(i = 0 ; i < ulCount ; i++)
				{
					FTM_EP	xEPInfo;

					xRet = FTOM_TP_CLIENT_DISCOVERY_getEPInfoAt(pClient, pService->pData, i, &xEPInfo);
					if (xRet == FTM_RET_OK)
					{
						MESSAGE("%2d : %16s %16s\n", i+1, xEPInfo.pEPID, xEPInfo.pName);	
					}
				}
			}
		}
		break;

	case	2:
		{
			FTM_ULONG			ulCount;
			FTM_INT				i;

			if (strcasecmp(pArgv[1], "start") == 0)
			{
				xRet = FTOM_discoveryStart("255.255.255.255", 1234, 1);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Discovery request was failed[%08x].\n", xRet);	
				}
				else
				{
					MESSAGE("Discovery request is complete.\n");	
				}

			}
			else if (strcasecmp(pArgv[1], "list") == 0)
			{
				xRet = FTOM_DISCOVERY_getEPInfoCount(pService->pData, &ulCount);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Discovery get EP Info count failed.\n");
					break;	
				}

				for(i = 0 ; i < ulCount ; i++)
				{
					FTM_EP	xEPInfo;

					xRet = FTOM_DISCOVERY_getEPInfoAt(pService->pData, i, &xEPInfo);
					if (xRet == FTM_RET_OK)
					{
						MESSAGE("%2d : %16s %16s\n", i+1, xEPInfo.pEPID, xEPInfo.pName);	
					}
				}
			}
				
		}
	}

	return	FTM_RET_OK;
}
#endif
