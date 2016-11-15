#include "ftom.h"
#include "ftom_shell_cmds.h"
#include "ftom_msg.h"
#include "ftom_node.h"
#include "ftom_ep.h"
#include "ftom_trigger.h"
#include "ftom_action.h"
#include "ftom_rule.h"
#include "ftom_discovery.h"
#include "ftom_logger.h"
#include "ftom_modules.h"

#undef	__MODULE__
#define	__MODULE__	FTOM_TRACE_MODULE_SHELL

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

FTM_RET	FTOM_SHELL_CMD_node
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR 	pData
);

FTM_RET	FTOM_SHELL_CMD_ep
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR 	pData
);

FTM_RET	FTOM_SHELL_CMD_action
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR 	pData
);

FTM_RET	FTOM_SHELL_CMD_trigger
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR 	pData
);

FTM_RET	FTOM_SHELL_CMD_rule
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

FTM_RET	FTOM_SHELL_CMD_log
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR 	pData
);

FTM_RET	FTOM_SHELL_CMD_trace
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[], 
	FTM_VOID_PTR 	pData
);

FTM_SHELL_CMD	FTOM_shellCmds[] = 
{
	{
		.pString	= "action",
		.function	= FTOM_SHELL_CMD_action,
		.pShortHelp	= "Action management.",
		.pHelp		= "\n"\
					  "    Action management.\n"\
					  "  COMMANDS:\n"\
					  "    <ActionID>  Show Action Information.\n"
	},
	{
		.pString	= "config",
		.function	= FTOM_SHELL_CMD_config,
		.pShortHelp	= "Configuration Management",
		.pHelp		= "\n"\
					  "    Configuration Management.\n"
	},
	{
		.pString	= "discovery",
		.function	= FTOM_SHELL_CMD_discovery,
		.pShortHelp = "Discovery NODEs and EPs.",
		.pHelp      = "<cmds> [<opts>]\n"\
					  "    Discovery NODEs and EPs.\n"\
					  "  COMMANDS:\n"\
					  "    search     Search NODE & EP.\n"\
					  "    list       Show a list of discovered devices.\n"\
					  "  OPtions:\n"\
					  "    -u         Unregistered devices only.\n"\
					  "    -n <NETWORK>  Network to search.\n"\
					  "    -p <PORT>  Service port.\n"
					  
	},
	{
		.pString	= "ep",
		.function	= FTOM_SHELL_CMD_ep,
		.pShortHelp	= "End point management.",
		.pHelp		= "<cmds> [<opts>] [<EPID> <EPID> ...]\n"
					  "    End point management.\n"\
					  "  COMMANDS:\n"\
					  "    import     Get the EP from the discovered list and register.\n"\
					  "    enable	  Set the EP to the enabled state.\n"\
					  "    disable	  Set the EP to the disabled state.\n"\
					  "    start	  Activate the EP.\n"\
					  "    stop	  	  Stop the EP.\n"
					  "  OPTIONS:\n"\
					  "    -a         Apply all end points.\n"\
					  "    <EPID>     EP ID.\n"
	},
	{
		.pString	= "log",
		.function	= FTOM_SHELL_CMD_log,
		.pShortHelp	= "Log management.",
		.pHelp		= "\n"\
					  "    Log management.\n"\
					  "  COMMANDS:\n"\
					  "    <INDEX>  log index.\n"
	},
	{
		.pString	= "node",
		.function	= FTOM_SHELL_CMD_node,
		.pShortHelp	= "Node management.",
		.pHelp		= "<cmds> [<opts>] [<DID> <DID> ...]\n"\
					  "    Node management.\n"\
					  "  COMMANDS:\n"\
					  "    import     Get the NODE from the discovered list and register.\n"\
					  "    start	  Activate the NODE.\n"\
					  "    stop	  	  Stop the NODE.\n"\
					  "  OPTIONS:\n"\
					  "    -a         Apply all nodes.\n"\
					  "    <DID>      Node ID.\n"
	},
	{
		.pString	= "object",
		.function	= FTOM_SHELL_CMD_object,
		.pShortHelp	= "show object.",
		.pHelp		= "\n"\
					  "    Show object.\n"
	},
	{
		.pString	= "quit",
		.function	= FTOM_SHELL_CMD_quit,
		.pShortHelp	= "quit.",
		.pHelp		= "\n"\
					  "    quit.\n"
	},
	{
		.pString	= "rule",
		.function	= FTOM_SHELL_CMD_rule,
		.pShortHelp	= "Rule management.",
		.pHelp		= "\n"\
					  "    Rule management.\n"\
					  "  COMMANDS:\n"\
					  "    <RuleID>  Show Rule Information.\n"
	},
	{
		.pString	= "task",
		.function	= FTOM_SHELL_CMD_task,
		.pShortHelp	= "task management",
		.pHelp		= "\n"\
					  "    task management.\n"
	},
	{
		.pString	= "trace",
		.function	= FTOM_SHELL_CMD_trace,
		.pShortHelp	= "Trace management.",
		.pHelp		= "[<module> [<level>]]\n"\
					  "    Trace management.\n"\
					  "  COMMANDS:\n"\
					  "    <module>  target module.\n"\
					  "    <level>   target level.\n"
	},
	{
		.pString	= "trigger",
		.function	= FTOM_SHELL_CMD_trigger,
		.pShortHelp	= "Trigger management.",
		.pHelp		= "\n"\
					  "    Trigger management.\n"\
					  "  COMMANDS:\n"\
					  "    <TriggerID>  Show Trigger Information.\n"
	},
	{
		.pString	= "server",
		.function	= FTOM_SHELL_CMD_server,
		.pShortHelp	= "Server management.",
		.pHelp		= "\n"\
					  "    Server management.\n"
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

	case	3:
		{
			if (strcasecmp(pArgv[1], "save") == 0)
			{
				FTM_RET	xRet;

				xRet = FTOM_saveConfigToFile(pArgv[2]);
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

FTM_RET	FTOM_SHELL_CMD_node
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	ASSERT(pShell != NULL);
	ASSERT(pArgv != NULL);

	FTM_RET			xRet;
	FTM_CHAR_PTR	pCmd = NULL;
	FTM_BOOL		bAll = FTM_FALSE;
	FTM_INT			nOpt;

	if (nArgc == 1)
	{
		return	FTOM_NODE_printList();
	}
 	if (nArgc == 2)
	{
		if (strcasecmp(pArgv[1], "show") == 0)
		{
			return	FTOM_NODE_printList();
		}
		else
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}

	pCmd = pArgv[1];
	nOpt = 2;

	while(nOpt < nArgc)
	{
		if (pArgv[nOpt][0] == '-')
		{
			switch(pArgv[nOpt][1])
			{
			case 'a':
				bAll = FTM_TRUE;
				break;

			default: /* '?' */
				ERROR2(FTM_RET_INVALID_ARGUMENTS, "Invalid argument [Opt = %02x]!\n", (FTM_UINT8)nOpt);
				return	FTM_RET_INVALID_ARGUMENTS;
			}
		}
		else
		{
			break;	
		}

		nOpt++;
	}

	if(bAll == FTM_FALSE)
	{
		if (nArgc < 3)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}

		if (strcasecmp(pCmd, "import") == 0)
		{
			FTOM_NODE_PTR	pNODE;
			FTOM_SERVICE_PTR	pService;
			FTM_NODE			xNodeInfo;
	
			xRet = FTOM_SERVICE_get(FTOM_SERVICE_DISCOVERY, &pService);
			if (xRet != FTM_RET_OK)
			{
				MESSAGE("Discovery not supported!\n");			
				goto finished;
			}

			xRet = FTOM_DISCOVERY_getNodeInfo(pService->pData, pArgv[nOpt], &xNodeInfo);
			if (xRet == FTM_RET_OK)
			{
				xRet = FTOM_NODE_create(&xNodeInfo, TRUE, &pNODE);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("NODE[%s] creation failed!\n", pArgv[nOpt]);
				}
				else
				{
					MESSAGE("NODE[%s] creation was completed successfully.\n", pArgv[nOpt]);
				}
			}
		}
		else 
		{
			FTOM_NODE_PTR	pNODE;

			xRet = FTOM_NODE_get(pArgv[nOpt], &pNODE);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "NODE[%s] can't found!\n",	pArgv[nOpt]);
			}

			if (strcasecmp(pCmd, "show") == 0)
			{
				FTOM_NODE_print(pNODE);
			}
			else if (strcasecmp(pCmd, "stop") == 0)
			{
				xRet = FTOM_NODE_stop(pNODE);
				if ((xRet != FTM_RET_OK) && (xRet == FTM_RET_NOT_START))	
				{
					MESSAGE("Failed to stop node[%s]!\n", pNODE->xInfo.pDID);	
				}
				else
				{
					MESSAGE("NODE[%s] stopped!\n", pNODE->xInfo.pDID);	
				}
			}
			else if (strcasecmp(pCmd, "start") == 0) 
			{
				xRet = FTOM_NODE_start(pNODE);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Failed to start node[%s]!\n", pNODE->xInfo.pDID);	
				}
				else
				{
					MESSAGE("NODE[%s] started!\n", pNODE->xInfo.pDID);	
				}
			}
		}
	}
	else if (strcasecmp(pCmd, "import") == 0)
	{
		FTOM_SERVICE_PTR	pService;
		FTM_NODE			xNodeInfo;
		FTM_INT			i;
		FTM_ULONG		ulCount;

		xRet = FTOM_SERVICE_get(FTOM_SERVICE_DISCOVERY, &pService);
		if (xRet != FTM_RET_OK)
		{
			MESSAGE("Discovery not supported!\n");			
			goto finished;
		}

		xRet = FTOM_DISCOVERY_getNodeInfoCount(pService->pData, &ulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Discovered node count get failed.\n");
			goto finished;
		}

		for(i = 0 ; i < ulCount ; i++)
		{
			xRet = FTOM_DISCOVERY_getNodeInfoAt(pService->pData, i, &xNodeInfo);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Discovered node[%d] information get failed.\n", i);
			}
			else
			{
				FTM_CHAR_PTR	pNewArgv[3];

				pNewArgv[0] = pArgv[0];
				pNewArgv[1] = pArgv[1];
				pNewArgv[2] = xNodeInfo.pDID;

				FTOM_SHELL_CMD_node(pShell, 3, pNewArgv, pData);
			}
		}
	}
	else 
	{
		FTM_INT		i;
		FTM_ULONG	ulCount;
		FTM_CHAR_PTR	pNewArgv[3];
		FTM_CHAR	pDID[FTM_DID_LEN+1];

		pNewArgv[0] = pArgv[0];
		pNewArgv[1] = pArgv[1];
		pNewArgv[2] = pDID;
			
		xRet = FTOM_NODE_count(&ulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "NODE count get failed!\n");	
			goto finished;
		}

		for(i = 0 ; i < ulCount ; i++)
		{
			FTOM_NODE_PTR	pNODE;
		
			xRet = FTOM_NODE_getAt(i, &pNODE);
			if (xRet == FTM_RET_OK)
			{
				strncpy(pDID, pNODE->xInfo.pDID, FTM_DID_LEN);	
				FTOM_SHELL_CMD_node(pShell, 3, pNewArgv, pData);
			}
		}
	}

finished:
	return	FTM_RET_OK;
}

FTM_RET	FTOM_SHELL_CMD_ep
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	ASSERT(pShell != NULL);
	ASSERT(pArgv != NULL);

	FTM_RET			xRet;
	FTM_CHAR_PTR	pCmd = NULL;
	FTM_BOOL		bAll = FTM_FALSE;
	FTM_INT			nOpt;

	if (nArgc == 1)
	{
		return	FTOM_EP_printList();
	}
	else if (nArgc == 2)
	{
		if (strcasecmp(pArgv[1], "show") == 0)
		{
			return	FTOM_EP_printList();
		}
		else
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}

	pCmd = pArgv[1];
	nOpt = 2;

	while(nOpt < nArgc)
	{
		if (pArgv[nOpt][0] == '-')
		{
			switch(pArgv[nOpt][1])
			{
			case 'a':
				bAll = FTM_TRUE;
				break;

			default: /* '?' */
				ERROR2(FTM_RET_INVALID_ARGUMENTS, "Invalid argument [Opt = %02x]!\n", (FTM_UINT8)nOpt);
				return	FTM_RET_INVALID_ARGUMENTS;
			}
		}
		else
		{
			break;	
		}

		nOpt++;
	}

	if(bAll == FTM_FALSE)
	{
		if (nArgc < 3)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}

		if (strcasecmp(pCmd, "import") == 0)
		{
			FTOM_EP_PTR	pEP;
			FTOM_SERVICE_PTR	pService;
			FTM_EP			xEPInfo;
	
			xRet = FTOM_SERVICE_get(FTOM_SERVICE_DISCOVERY, &pService);
			if (xRet != FTM_RET_OK)
			{
				MESSAGE("Discovery not supported!\n");			
				goto finished;
			}

			xRet = FTOM_DISCOVERY_getEPInfo(pService->pData, pArgv[nOpt], &xEPInfo);
			if (xRet == FTM_RET_OK)
			{
				xRet = FTOM_EP_create(&xEPInfo, TRUE, &pEP);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("EP[%s] creation failed!\n", pArgv[nOpt]);
				}
				else
				{
					MESSAGE("EP[%s] creation was completed successfully.\n", pArgv[nOpt]);
				}
			}
		}
		else 
		{
			FTOM_EP_PTR	pEP;

			xRet = FTOM_EP_get(pArgv[nOpt], &pEP);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "EP[%s] can't found!\n",	pArgv[nOpt]);
			}

			if (strcasecmp(pCmd, "show") == 0)
			{
				FTOM_EP_print(pEP);
			}
			else if (strcasecmp(pCmd, "disable") == 0)
			{
				FTM_EP	xInfo;

				xInfo.bEnable = FTM_FALSE;

				xRet = FTOM_EP_setInfo(pEP, FTM_EP_FIELD_ENABLE, &xInfo);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("The EP[%s] setting changing failed!\n", pEP->xInfo.pEPID);	
				}
				else
				{
					MESSAGE("EP[%s] is disabled!\n", pEP->xInfo.pEPID);	
				}
			}
			else if (strcasecmp(pCmd, "enable") == 0)
			{
				FTM_EP	xInfo;

				xInfo.bEnable = FTM_TRUE;

				xRet = FTOM_EP_setInfo(pEP, FTM_EP_FIELD_ENABLE, &xInfo);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("The EP[%s] setting changing failed!\n", pEP->xInfo.pEPID);	
				}
				else
				{
					MESSAGE("EP[%s] is enabled!\n", pEP->xInfo.pEPID);	
				}
			}
			else if (strcasecmp(pCmd, "stop") == 0)
			{
				xRet = FTOM_EP_stop(pEP, FALSE);
				if ((xRet != FTM_RET_OK) && (xRet == FTM_RET_NOT_START))	
				{
					MESSAGE("Failed to stop ep[%s]!\n", pEP->xInfo.pEPID);	
				}
				else
				{
					MESSAGE("EP[%s] stopped!\n", pEP->xInfo.pEPID);	
				}
			}
			else if (strcasecmp(pCmd, "start") == 0)
			{
				xRet = FTOM_EP_start(pEP);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Failed to start ep[%s]!\n", pEP->xInfo.pEPID);	
				}
				else
				{
					MESSAGE("EP[%s] started!\n", pEP->xInfo.pEPID);	
				}
			}
		}
	}
	else if (strcasecmp(pCmd, "import") == 0)
	{
		FTOM_SERVICE_PTR	pService;
		FTM_EP			xEPInfo;
		FTM_INT			i;
		FTM_ULONG		ulCount;

		xRet = FTOM_SERVICE_get(FTOM_SERVICE_DISCOVERY, &pService);
		if (xRet != FTM_RET_OK)
		{
			MESSAGE("Discovery not supported!\n");			
			goto finished;
		}

		xRet = FTOM_DISCOVERY_getEPInfoCount(pService->pData, &ulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Discovered node count get failed.\n");
			goto finished;
		}

		for(i = 0 ; i < ulCount ; i++)
		{
			xRet = FTOM_DISCOVERY_getEPInfoAt(pService->pData, i, &xEPInfo);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Discovered node[%d] information get failed.\n", i);
			}
			else
			{
				FTM_CHAR_PTR	pNewArgv[3];

				pNewArgv[0] = pArgv[0];
				pNewArgv[1] = pArgv[1];
				pNewArgv[2] = xEPInfo.pEPID;

				FTOM_SHELL_CMD_ep(pShell, 3, pNewArgv, pData);
			}
		}
	}
	else 
	{
		FTM_INT		i;
		FTM_ULONG	ulCount;
		FTM_CHAR_PTR	pNewArgv[3];
		FTM_CHAR	pEPID[FTM_EPID_LEN+1];

		pNewArgv[0] = pArgv[0];
		pNewArgv[1] = pArgv[1];
		pNewArgv[2] = pEPID;
			
		xRet = FTOM_EP_count(0, NULL, &ulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "EP count get failed!\n");	
			goto finished;
		}

		for(i = 0 ; i < ulCount ; i++)
		{
			FTOM_EP_PTR	pEP;
		
			xRet = FTOM_EP_getAt(i, &pEP);
			if (xRet == FTM_RET_OK)
			{
				strncpy(pEPID, pEP->xInfo.pEPID, FTM_EPID_LEN);	
				FTOM_SHELL_CMD_ep(pShell, 3, pNewArgv, pData);
			}
		}
	}

finished:
	return	FTM_RET_OK;
}

FTM_RET	FTOM_SHELL_CMD_trigger
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	ASSERT(pShell != NULL);
	ASSERT(pArgv != NULL);

	FTM_RET	xRet;
	switch(nArgc)
	{
	case	1:
		{
			FTOM_TRIGGER_printList();
		}
		break;

	case	2:
		{
			FTOM_TRIGGER_PTR	pTrigger;

			xRet = FTOM_TRIGGER_get(pArgv[1], &pTrigger);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Can't found Trigger[%s]!\n", pArgv[1]);
				break;
			}

			FTOM_TRIGGER_print(pTrigger);

		}
		break;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SHELL_CMD_action
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	ASSERT(pShell != NULL);
	ASSERT(pArgv != NULL);

	FTM_RET	xRet;
	switch(nArgc)
	{
	case	1:
		{
			FTOM_ACTION_printList();
		}
		break;

	case	2:
		{
			FTOM_ACTION_PTR	pAction;

			xRet = FTOM_ACTION_get(pArgv[1], &pAction);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Can't found Action[%s]!\n", pArgv[1]);
				break;
			}

			FTOM_ACTION_print(pAction);

		}
		break;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SHELL_CMD_rule
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	ASSERT(pShell != NULL);
	ASSERT(pArgv != NULL);

	FTM_RET	xRet;
	switch(nArgc)
	{
	case	1:
		{
			FTOM_RULE_printList();
		}
		break;

	case	2:
		{
			FTOM_RULE_PTR	pRule;

			xRet = FTOM_RULE_get(pArgv[1], &pRule);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Can't found Rule[%s]!\n", pArgv[1]);
				break;
			}

			FTOM_RULE_print(pRule);

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
			FTOM_NODE_printList();
			FTOM_EP_printList();
			FTOM_TRIGGER_printList();
			FTOM_ACTION_printList();
			FTOM_RULE_printList();
		}
		break;

	case	2:
		{
			FTM_RET	xRet;
			FTOM_NODE_PTR		pNode;
			FTOM_EP_PTR			pEP;
			FTOM_TRIGGER_PTR	pTrigger;
			FTOM_ACTION_PTR		pAction;
			FTOM_RULE_PTR		pRule;

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

			xRet = FTOM_TRIGGER_get(pArgv[1], &pTrigger);
			if (xRet == FTM_RET_OK)
			{
				FTOM_TRIGGER_print(pTrigger);
				break;	
			}

			xRet = FTOM_ACTION_get(pArgv[1], &pAction);
			if (xRet == FTM_RET_OK)
			{
				FTOM_ACTION_print(pAction);
				break;	
			}

			xRet = FTOM_RULE_get(pArgv[1], &pRule);
			if (xRet == FTM_RET_OK)
			{
				FTOM_RULE_print(pRule);
				break;	
			}
				
		}
		break;

	case	3:
		{
			FTM_RET	xRet;
			FTOM_NODE_PTR		pNode;
			FTOM_EP_PTR			pEP;
			FTOM_TRIGGER_PTR	pTrigger;
			FTOM_ACTION_PTR		pAction;
			FTOM_RULE_PTR		pRule;
			FTM_BOOL			bEnable = FTM_FALSE;

			if ((strcasecmp(pArgv[2], "enable") == 0)
			   || (strcasecmp(pArgv[2], "run") == 0))
			{
				bEnable = FTM_TRUE;
			}
			else if ((strcasecmp(pArgv[2], "disable") == 0)
			       || (strcasecmp(pArgv[2], "stop") == 0))
			{
				bEnable = FTM_FALSE;
			}
			else
			{
				MESSAGE("Invalid arguments!\n");
			}

			xRet = FTOM_NODE_get(pArgv[1], &pNode);
			if (xRet == FTM_RET_OK)
			{
				if (bEnable)
				{
					FTOM_NODE_start(pNode);
				}
				else
				{
					FTOM_NODE_stop(pNode);
				}

				break;
			}

			xRet = FTOM_EP_get(pArgv[1], &pEP);
			if (xRet == FTM_RET_OK)
			{
				if (bEnable)
				{
					FTOM_EP_start(pEP);
				}
				else
				{
					FTOM_EP_stop(pEP, FTM_FALSE);
				}
				break;
			}

			xRet = FTOM_TRIGGER_get(pArgv[1], &pTrigger);
			if (xRet == FTM_RET_OK)
			{
				if (bEnable)
				{
					FTOM_TRIGGER_start(pTrigger);
				}
				else
				{
					FTOM_TRIGGER_stop(pTrigger);
				}
				break;	
			}

			xRet = FTOM_ACTION_get(pArgv[1], &pAction);
			if (xRet == FTM_RET_OK)
			{
				FTOM_ACTION_print(pAction);
				break;	
			}

			xRet = FTOM_RULE_get(pArgv[1], &pRule);
			if (xRet == FTM_RET_OK)
			{
				FTOM_RULE_print(pRule);
				break;	
			}
				
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

FTM_RET	FTOM_SHELL_CMD_trace
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	return	FTM_TRACE_shellCmd(pShell, nArgc, pArgv, pData);
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

	switch(nArgc)
	{
	case	2:
		{
			if (strcasecmp(pArgv[1], "sync") == 0)
			{
				xRet = FTOM_serverSync(FTM_FALSE);
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
					xRet = FTOM_serverSync(FTM_TRUE);
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
	FTM_INT			nOpt;
	FTM_BOOL		bSearch = FTM_FALSE;
	FTM_BOOL		bList = FTM_FALSE;
	FTM_BOOL		bUnregisteredOnly = FTM_FALSE;
	FTM_CHAR_PTR	pNetwork = "255.255.255.255";
	FTM_INT			i, nIndex;
	FTM_ULONG		ulCount;
	FTM_USHORT		usPort = 1234;

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	if (strcasecmp(pArgv[1], "search") == 0)
	{
		TRACE("CMD : search\n");
		bSearch = FTM_TRUE;
		bList = FTM_TRUE;
	}
	else if (strcasecmp(pArgv[1], "list") == 0)
	{
		TRACE("CMD : list\n");
		bList = FTM_TRUE;
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	while ((nOpt = getopt(nArgc, pArgv, "un:l")) != -1) 
	{
		switch (nOpt) 
		{
		case 'u':
			bUnregisteredOnly = FTM_TRUE;
			break;

		case 'n':
			pNetwork = optarg; 
			break;

		case 'p':
			usPort = (FTM_USHORT)strtoul(optarg, 0, 10);
			break;

		default: /* '?' */
			ERROR2(FTM_RET_INVALID_ARGUMENTS, "Invalid argument [Opt = %02x]!\n", (FTM_UINT8)nOpt);
			return	FTM_RET_INVALID_ARGUMENTS;
		}
	}

	xRet = FTOM_SERVICE_get(FTOM_SERVICE_DISCOVERY, &pService);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Discovery not supported!\n");			
		return	FTM_RET_OK;
	}

	if (bSearch)
	{
		FTM_BOOL	bFinished;

		xRet = FTOM_DISCOVERY_startSearch(pService->pData, pNetwork, usPort, 1);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Discovery request was failed.\n");	
		}
		else
		{
			TRACE("Discovery request is complete.\n");	

			usleep(1000000);
			do
			{
				bFinished = FTM_FALSE;
				FTOM_discoveryIsFinished(&bFinished);
				usleep(100000);
			}
			while(!bFinished);

	
			TRACE("Discovery finished!\n");
		}
	}

	if (bList)
	{
		xRet = FTOM_DISCOVERY_getNodeInfoCount(pService->pData, &ulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Discovery get EP Info count failed.\n");
			goto finished;
		}

		MESSAGE("\n[ Discovered NODE ]\n");

		nIndex = 0;
		for(i = 0 ; i < ulCount ; i++)
		{
			FTM_NODE	xNodeInfo;
	
			xRet = FTOM_DISCOVERY_getNodeInfoAt(pService->pData, i, &xNodeInfo);
			if (xRet == FTM_RET_OK)
			{
				if (bUnregisteredOnly)
				{
					FTOM_NODE_PTR	pNode;
	
					xRet = FTOM_NODE_get(xNodeInfo.pDID, &pNode);
					if (xRet != FTM_RET_OK)
					{
						MESSAGE("%2d : %16s %16s\n", ++nIndex, xNodeInfo.pDID, xNodeInfo.pName);	
					}
				}
				else
				{
					MESSAGE("%2d : %16s %16s\n", ++nIndex, xNodeInfo.pDID, xNodeInfo.pName);	
				}
			}
		}
		
		MESSAGE("%16s : %lu\n", "Discovered", ulCount);
		MESSAGE("%16s : %d\n", "Unregistered", nIndex);

		xRet = FTOM_DISCOVERY_getEPInfoCount(pService->pData, &ulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Discovery get EP Info count failed.\n");
			goto finished;
		}
	
		MESSAGE("\n[ Discovered EP ]\n");

		nIndex = 0;
		for(i = 0 ; i < ulCount ; i++)
		{
			FTM_EP	xEPInfo;
	
			xRet = FTOM_DISCOVERY_getEPInfoAt(pService->pData, i, &xEPInfo);
			if (xRet == FTM_RET_OK)
			{
				if (bUnregisteredOnly)
				{
					FTOM_EP_PTR	pEP;
	
					xRet = FTOM_EP_get(xEPInfo.pEPID, &pEP);
					if (xRet != FTM_RET_OK)
					{
						MESSAGE("%2d : %16s %16s %16s\n", ++nIndex, xEPInfo.pEPID, xEPInfo.pName, FTM_EP_typeString(xEPInfo.xType));	
					}
				}
				else
				{
					MESSAGE("%2d : %16s %16s %16s\n", ++nIndex, xEPInfo.pEPID, xEPInfo.pName, FTM_EP_typeString(xEPInfo.xType));	
				}
			}
		}

		MESSAGE("%16s : %lu\n", "Discovered", ulCount);
		MESSAGE("%16s : %d\n", "Unregistered", nIndex);
	}

finished:

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SHELL_CMD_log
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	ASSERT(pShell != NULL);
	ASSERT(pArgv != NULL);

	FTM_RET	xRet;
	switch(nArgc)
	{
	case	1:
		{
		}
		break;

	case	2:
		{
			FTM_LOG_PTR	pLogs ;
			FTM_ULONG	ulCount = 0, ulGetCount = 0;
			FTM_INT		i;
			FTM_CHAR    pTimeString[128];

			ulCount = strtoul(pArgv[1], 0, 10);
			if (ulCount == 0)
			{
				break;	
			}
			
			if (ulCount > 100)
			{
				ulCount = 100;	
			}

			pLogs = (FTM_LOG_PTR)FTM_MEM_malloc(sizeof(FTM_LOG) * ulCount);

			xRet = FTOM_getLogsAt(0, ulCount, pLogs, &ulGetCount);
			if (xRet != FTM_RET_OK)
			{
				MESSAGE("Failed to get log!\n");	
				FTM_MEM_free(pLogs);
				break;
			}

			for(i = 0 ; i < ulGetCount ; i++)
			{
				strftime(pTimeString, sizeof(pTimeString) - 1, "%Y-%m-%d %H:%M:%S", localtime((const time_t *)&pLogs[i].ulTime));
				MESSAGE("%3d : %16llu %s - %s\n", i+1, pLogs[i].ullID, pTimeString, FTM_LOG_print(&pLogs[i]));
			}


			FTM_MEM_free(pLogs);
		}
		break;
	
	case	3:
		{
			FTM_LOG_PTR	pLogs ;
			FTM_ULONG	ulIndex, ulCount = 0;
			FTM_INT		i;
			FTM_CHAR    pTimeString[128];

			if (strcasecmp(pArgv[1], "del") == 0)
			{
				ulCount = strtoul(pArgv[2], 0, 10);
				if (ulCount == 0)
				{
					break;	
				}
		
				xRet = FTOM_removeLogsFrom(0, ulCount, &ulCount);
				if(xRet == FTM_RET_OK)
				{
					MESSAGE("Removed %lu logs.\n", ulCount);
				}
				else
				{
					MESSAGE("Failed to remove logs.\n");
				}
			}
			else
			{
				ulIndex = strtoul(pArgv[1], 0, 10);
				if (ulIndex == 0)
				{
					break;	
				}
	
				ulCount = strtoul(pArgv[2], 0, 10);
				if (ulCount == 0)
				{
					break;	
				}
				
				if (ulCount > 100)
				{
					ulCount = 100;	
				}
	
				pLogs = (FTM_LOG_PTR)FTM_MEM_malloc(sizeof(FTM_LOG) * ulCount);
	
				xRet = FTOM_getLogsAt(ulIndex, ulCount, pLogs, &ulCount);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Failed to get log!\n");	
					break;
				}
	
				for(i = 0 ; i < ulCount ; i++)
				{
					strftime(pTimeString, sizeof(pTimeString) - 1, "%Y-%m-%d %H:%M:%S", localtime((const time_t *)&pLogs[i].ulTime));
					MESSAGE("%3d : %16llu %s - %s\n", i+1, pLogs[i].ullID, pTimeString, FTM_LOG_print(&pLogs[i]));
				}
			}
		}
		break;
	}

	return	FTM_RET_OK;
}

