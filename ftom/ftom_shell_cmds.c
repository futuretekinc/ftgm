#include "ftom.h"
#include "ftom_shell_cmds.h"
#include "ftom_msg.h"
#include "ftom_node.h"
#include "ftom_ep.h"
#include "ftom_trigger.h"
#include "ftom_action.h"
#include "ftom_rule.h"
#include "ftom_discovery.h"

FTM_ULONG	ulGetheringTime = 3;

FTM_RET	FTOM_SHELL_CMD_config
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR 	pData
);

FTM_RET	FTOM_SHELL_CMD_list
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

FTM_SHELL_CMD	FTOM_shellCmds[] = 
{
	{
		.pString	= "config",
		.function	= FTOM_SHELL_CMD_config,
		.pShortHelp	= "Configuration Management",
		.pHelp		= "\n"\
					  "\tConfiguration Management.\n"
	},
	{
		.pString	= "list",
		.function	= FTOM_SHELL_CMD_list,
		.pShortHelp	= "show object list.",
		.pHelp		= "\n"\
					  "\tShow object list.\n"
	},
	{
		.pString	= "quit",
		.function	= FTOM_SHELL_CMD_quit,
		.pShortHelp	= "quit.",
		.pHelp		= "\n"\
					  "\tquit.\n"
	},
	{
		.pString	= "task",
		.function	= FTOM_SHELL_CMD_task,
		.pShortHelp	= "task management",
		.pHelp		= "\n"\
					  "\ttask management.\n"
	},
	{
		.pString	= "discovery",
		.function	= FTOM_SHELL_CMD_discovery,
		.pShortHelp	= "Node discovery.",
		.pHelp		= "\n"\
					  "\tNode discovery.\n"
	}

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
	switch(nArgc)
	{
	case	1:
		FTOM_showConfig();
		break;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SHELL_CMD_list
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	FTOM_NODE_printList();
	FTOM_EP_printList();
	FTOM_TRIGGER_printList();
	FTOM_ACTION_printList();
	FTOM_RULE_printList();

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

	xRet = FTOM_stop();
	if (xRet == FTM_RET_OK)
	{
		return	FTM_RET_SHELL_QUIT;
	}

	return	xRet;
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
			xRet = FTOM_DISCOVERY_getEPInfoCount(pService->pData, &ulCount);
			if (xRet != FTM_RET_OK)
			{
				MESSAGE("Discovery get EP Info count failed.\n");
			}
			else
			{
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

