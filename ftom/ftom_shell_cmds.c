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
					  "  Commands:\n"\
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
		.pString	= "ep",
		.function	= FTOM_SHELL_CMD_ep,
		.pShortHelp	= "End Point management.",
		.pHelp		= "\n"\
					  "    End point management.\n"\
					  "  Commands:\n"\
					  "    <EPID>  Show EP Information.\n"
	},
	{
		.pString	= "log",
		.function	= FTOM_SHELL_CMD_log,
		.pShortHelp	= "Log management.",
		.pHelp		= "\n"\
					  "    Log management.\n"\
					  "  Commands:\n"\
					  "    <INDEX>  log index.\n"
	},
	{
		.pString	= "node",
		.function	= FTOM_SHELL_CMD_node,
		.pShortHelp	= "Node management.",
		.pHelp		= "\n"\
					  "    Node management.\n"\
					  "  Commands:\n"\
					  "    <EPID>  Show node Information.\n"
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
					  "  Commands:\n"\
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
					  "  Commands:\n"\
					  "    <module>  target module.\n"\
					  "    <level>   target level.\n"
	},
	{
		.pString	= "trigger",
		.function	= FTOM_SHELL_CMD_trigger,
		.pShortHelp	= "Trigger management.",
		.pHelp		= "\n"\
					  "    Trigger management.\n"\
					  "  Commands:\n"\
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

	FTM_RET	xRet;
	switch(nArgc)
	{
	case	1:
		{
			FTOM_NODE_printList();
		}
		break;

	case	2:
		{
			FTOM_NODE_PTR	pNode;

			xRet = FTOM_NODE_get(pArgv[1], &pNode);
			if (xRet != FTM_RET_OK)
			{
				MESSAGE("Can't found Node[%s]!\n", pArgv[1]);

				break;
			}

			FTOM_NODE_print(pNode);
		}
		break;

	case	3:
		{
			FTOM_NODE_PTR	pNode;

			xRet = FTOM_NODE_get(pArgv[2], &pNode);
			if (xRet != FTM_RET_OK)
			{
				MESSAGE("Can't found Node[%s]!\n", pArgv[1]);

				break;
			}

			if (strcasecmp(pArgv[1], "disable") == 0)
			{
				FTM_BOOL	bRun;

				xRet = FTOM_NODE_isRun(pNode, &bRun);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Faield to get node[%s] status!\n", pNode->xInfo.pDID);
					break;
				}

				if (!bRun)
				{
					MESSAGE("Node[%s] is already stopped!\n", pNode->xInfo.pDID);
					break;
				}

				xRet = FTOM_NODE_stop(pNode);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Failed to start node[%s]!\n", pNode->xInfo.pDID);	
				}
				else
				{
					MESSAGE("Node[%s] started!\n", pNode->xInfo.pDID);	
				}
			}
			else if (strcasecmp(pArgv[1], "stop") == 0)
			{
				FTM_BOOL	bRun;

				xRet = FTOM_NODE_isRun(pNode, &bRun);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Faield to get node[%s] status!\n", pNode->xInfo.pDID);
					break;
				}

				if (!bRun)
				{
					MESSAGE("Node[%s] is already stopped!\n", pNode->xInfo.pDID);
					break;
				}

				xRet = FTOM_NODE_stop(pNode);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Failed to start node[%s]!\n", pNode->xInfo.pDID);	
				}
				else
				{
					MESSAGE("Node[%s] started!\n", pNode->xInfo.pDID);	
				}
			}
			else if (strcasecmp(pArgv[1], "start") == 0)
			{
			
				FTM_BOOL	bRun;

				xRet = FTOM_NODE_isRun(pNode, &bRun);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Faield to get node[%s] status!\n", pNode->xInfo.pDID);
					break;
				}

				if (bRun)
				{
					MESSAGE("Node[%s] is already started!\n", pNode->xInfo.pDID);
					break;
				}

				xRet = FTOM_NODE_start(pNode);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Failed to stop node[%s]!\n", pNode->xInfo.pDID);	
				}
				else
				{
					MESSAGE("Node[%s] stopped!\n", pNode->xInfo.pDID);	
				}
			}
			else if (strcasecmp(pArgv[1], "del") == 0)
			{
			
				FTM_BOOL	bRun;

				xRet = FTOM_NODE_isRun(pNode, &bRun);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Faield to get node[%s] status!\n", pNode->xInfo.pDID);
					break;
				}

				if (bRun)
				{
					xRet = FTOM_NODE_stop(pNode);
					if (xRet != FTM_RET_OK)
					{
						MESSAGE("Failed to stop Node[%s]!\n", pNode->xInfo.pDID);
						break;
					}
				}

				xRet = FTOM_NODE_destroy(&pNode, FTM_TRUE);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Failed to remove node[%s]!\n", pNode->xInfo.pDID);	
				}
				else
				{
					MESSAGE("Node have been removed.\n");	
				}
			}
		}
		break;
	}

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

	FTM_RET	xRet;
	switch(nArgc)
	{
	case	1:
		{
			FTOM_EP_printList();
		}
		break;

	case	2:
		{
			FTOM_EP_PTR	pEP;

			if (strcasecmp(pArgv[1], "class") == 0)
			{
				FTM_INT	i;
				for(i = 0 ; ; i++)
				{
					FTOM_EP_CLASS_PTR pEPClass = NULL;

					xRet = FTOM_EP_CLASS_getAt( i, &pEPClass);
					if (xRet != FTM_RET_OK)
					{
						break;
					}

					FTOM_EP_CLASS_print(pEPClass);
				}
			}
			else
			{
				xRet = FTOM_EP_get(pArgv[1], &pEP);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Can't found EP[%s]!\n", pArgv[1]);
					break;
				}

				FTOM_EP_print(pEP);
			}

		}
		break;

	case	3:
		{
			FTOM_EP_PTR	pEP;

			xRet = FTOM_EP_get(pArgv[2], &pEP);
			if (xRet != FTM_RET_OK)
			{
				MESSAGE("Can't found EP[%s]!\n", pArgv[2]);
				break;
			}

			if (strcasecmp(pArgv[1], "stop") == 0)
			{
				FTM_BOOL	bRun;

				xRet = FTOM_EP_isRun(pEP, &bRun);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Faield to get EP[%s] status!\n", pEP->xInfo.pEPID);
					break;
				}

				if (!bRun)
				{
					MESSAGE("EP[%s] is already stopped!\n", pEP->xInfo.pEPID);
					break;
				}

				xRet = FTOM_EP_stop(pEP, FTM_TRUE);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Failed to stop EP[%s]!\n", pEP->xInfo.pEPID);	
				}
				else
				{
					MESSAGE("EP[%s] stopped!\n", pEP->xInfo.pEPID);	
				}
			}
			else if (strcasecmp(pArgv[1], "start") == 0)
			{
				FTM_BOOL	bRun;

				xRet = FTOM_EP_isRun(pEP, &bRun);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Faield to get EP[%s] status!\n", pEP->xInfo.pEPID);
					break;
				}

				if (bRun)
				{
					MESSAGE("EP[%s] is already started!\n", pEP->xInfo.pEPID);
					break;
				}

				xRet = FTOM_EP_start(pEP);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Failed to start EP[%s]!\n", pEP->xInfo.pEPID);	
				}
				else
				{
					MESSAGE("EP[%s] started!\n", pEP->xInfo.pEPID);	
				}
			}
			else if (strcasecmp(pArgv[1], "del") == 0)
			{
			
				FTM_BOOL	bRun;

				xRet = FTOM_EP_isRun(pEP, &bRun);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Faield to get EP[%s] status!\n", pEP->xInfo.pEPID);
					break;
				}

				if (bRun)
				{
					xRet = FTOM_EP_stop(pEP, FTM_TRUE);
					if (xRet != FTM_RET_OK)
					{
						MESSAGE("Failed to stop EP[%s]!\n", pEP->xInfo.pEPID);
						break;
					}
				}

				xRet = FTOM_EP_destroy(&pEP, FTM_TRUE);
				if (xRet != FTM_RET_OK)
				{
					MESSAGE("Failed to remove EP[%s]!\n", pEP->xInfo.pEPID);	
				}
				else
				{
					MESSAGE("Node have been removed.\n");	
				}
			}
		}
		break;
	}

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
	FTM_RET	xRet;

	switch(nArgc)
	{
	case	1:
		{
			FTM_INT	i;

			MESSAGE("%16s %8s %8s %8s\n", "MODULE", "ENABLED", "LEVEL", "OUTMODE");
			for(i = 0 ; i < FTM_TRACE_MAX_MODULES ; i++)
			{
				FTM_TRACE_INFO	xInfo;

				memset(&xInfo, 0, sizeof(xInfo));
				xRet = FTM_TRACE_getInfo(i, &xInfo);
				if (xRet == FTM_RET_OK)
				{
					if (xInfo.pName[0] != 0)
					{
						MESSAGE("%16s %8s %8s %8s\n", xInfo.pName, xInfo.bEnabled?"ON":"OFF", FTM_TRACE_LEVEL_print(xInfo.ulLevel, FTM_TRUE), FTM_TRACE_OUT_print(xInfo.xOut));	
					}
				}
			}
		}
		break;

	case	3:
		{
			FTM_BOOL	bEnabled = FTM_FALSE;

			if (strcasecmp(pArgv[2], "on") == 0)
			{
				bEnabled = FTM_TRUE;
			}
			else if (strcasecmp(pArgv[2], "off") == 0)
			{
				bEnabled = FTM_FALSE;
			}

			if (strcasecmp(pArgv[1], "object") == 0)
			{
				FTM_TRACE_setModule(FTOM_TRACE_MODULE_NODE, bEnabled);	
				FTM_TRACE_setModule(FTOM_TRACE_MODULE_EP, 	bEnabled);	
				FTM_TRACE_setModule(FTOM_TRACE_MODULE_TRIGGER, bEnabled);	
				FTM_TRACE_setModule(FTOM_TRACE_MODULE_ACTION, bEnabled);	
				FTM_TRACE_setModule(FTOM_TRACE_MODULE_RULE,	bEnabled);	
			}
			else
			{
				FTM_ULONG		ulIndex;

				xRet = FTM_TRACE_getID(pArgv[1], &ulIndex);
				if (xRet == FTM_RET_OK)
				{
					FTM_TRACE_setModule(ulIndex, bEnabled);	
				}
				else
				{
					MESSAGE("%s module not found!\n", pArgv[1]);
				}
			}
		}
		break;
	}

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
				MESSAGE("Discovery request was failed[%08lx].\n", xRet);	
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
					MESSAGE("Discovery request was failed[%08lx].\n", xRet);	
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

			xRet = FTOM_LOGGER_getAt(0, ulCount, pLogs, &ulGetCount);
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
		
				xRet = FTOM_LOGGER_remove(0, ulCount, &ulCount);
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
	
				xRet = FTOM_LOGGER_getAt(ulIndex, ulCount, pLogs, &ulCount);
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

