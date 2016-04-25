#include "ftom.h"
#include "ftom_shell_cmds.h"
#include "ftom_node.h"
#include "ftom_node_management.h"
#include "ftom_ep.h"
#include "ftom_ep_management.h"
#include "ftom_msg.h"
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
	FTOM_PTR pOM = (FTOM_PTR)pData;

	switch(nArgc)
	{
	case	1:
		FTOM_showConfig(pOM);
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
	FTM_RET				xRet;
	FTOM_NODE_PTR		pNode;
	FTOM_EP_PTR			pEP;
	FTM_ULONG			i,j, ulCount;
	FTOM_PTR	pOM = (FTOM_PTR)pData;

	MESSAGE("\n# Node Information\n");
	MESSAGE("%16s %16s %8s %8s %s\n", "DID", "STATE", "INTERVAL", "TIMEOUT", "EPs");
	FTOM_NODEM_countNode(pOM->pNodeM, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_ULONG	ulEPCount;

		FTOM_NODEM_getNodeAt(pOM->pNodeM, i, &pNode);
		MESSAGE("%16s ", pNode->xInfo.pDID);
		MESSAGE("%16s ", FTOM_NODE_stateToStr(pNode->xState));
		MESSAGE("%8d ", pNode->xInfo.ulInterval);
		MESSAGE("%8d ", pNode->xInfo.ulTimeout);

		FTOM_NODE_getEPCount(pNode, &ulEPCount);
		MESSAGE("%-3d [ ", ulCount);
		for(j = 0; j < ulEPCount ; j++)
		{
			if (FTOM_NODE_getEPAt(pNode, j, &pEP) == FTM_RET_OK)
			{
				MESSAGE("%16s ", pEP->xInfo.pEPID);
			}
		}
		MESSAGE("]\n");
	}

	MESSAGE("\n# EP Information\n");
	MESSAGE("%16s %16s %16s %16s %8s %24s\n", "EPID", "TYPE", "DID", "STATE", "VALUE", "TIME");
	FTOM_EPM_count(pOM->pEPM, 0, &ulCount);
	for(i = 0; i < ulCount ; i++)
	{
		if (FTOM_EPM_getEPAt(pOM->pEPM, i, &pEP) == FTM_RET_OK)
		{
			FTM_CHAR	pTimeString[64];
			FTM_EP_DATA	xData;
		
			FTOM_EP_getData(pEP, &xData);

			ctime_r((time_t *)&xData.ulTime, pTimeString);
			if (strlen(pTimeString) != 0)
			{
				pTimeString[strlen(pTimeString) - 1] = '\0';
			}
			
			MESSAGE("%16s ", pEP->xInfo.pEPID);
			MESSAGE("%16s ", FTM_EP_typeString(pEP->xInfo.xType));
			if (pEP->pNode != NULL)
			{
				MESSAGE("%16s ", pEP->pNode->xInfo.pDID);
			}
	
			MESSAGE("%16s ", (!pEP->bStop)?"RUN":"STOP");
			MESSAGE("%8s ", FTM_VALUE_print(&xData.xValue));
			MESSAGE("%24s\n", pTimeString);
		}
	}

	MESSAGE("\n# Trigger Information\n");
	FTOM_TRIGGERM_count(pOM->pTriggerM, &ulCount);
	MESSAGE("\t%16s %16s %16s %8s %8s %s\n", "ID", "EPID", "TYPE", "DETECT", "HOLD", "CONDITION");
	for(i = 0; i< ulCount ; i++)
	{
		FTOM_TRIGGER_PTR	pTrigger;

		xRet = FTOM_TRIGGERM_getAt(pOM->pTriggerM, i, &pTrigger);
		if (xRet == FTM_RET_OK)
		{
			FTM_CHAR	pCondition[1024];

			FTM_TRIGGER_conditionToString(&pTrigger->xInfo, pCondition, sizeof(pCondition));

			MESSAGE("\t%16s %16s %16s %8.3f %8.3f %s\n", 
				pTrigger->xInfo.pID, 
				pTrigger->xInfo.pEPID, 
				FTM_TRIGGER_typeString(pTrigger->xInfo.xType),
            	pTrigger->xInfo.xParams.xCommon.ulDetectionTime / 1000000.0,
				pTrigger->xInfo.xParams.xCommon.ulHoldingTime / 1000000.0,
				pCondition);
		}

	}

	MESSAGE("\n# Action Information\n");
	FTOM_ACTIONM_count(pOM->pActionM, &ulCount);
	MESSAGE("\t%16s %16s\n", "ID","TYPE");
	for(i = 0; i< ulCount ; i++)
	{
		FTOM_ACTION_PTR	pAction;

		xRet = FTOM_ACTIONM_getAt(pOM->pActionM, i, &pAction);
		if (xRet == FTM_RET_OK)
		{
			MESSAGE("\t%16s %16s\n", 
				pAction->xInfo.pID, 
				FTM_ACTION_typeString(pAction->xInfo.xType));
		}

	}
	
	MESSAGE("\n# Rule Information\n");
	FTOM_RULEM_count(pOM->pRuleM, &ulCount);
	MESSAGE("\t%16s %24s %24s\n", "ID","TRIGGER", "ACTION");
	for(i = 0; i< ulCount ; i++)
	{
		FTOM_RULE_PTR	pRule;

		xRet = FTOM_RULEM_getAt(pOM->pRuleM, i, &pRule);
		if (xRet == FTM_RET_OK)
		{
			MESSAGE("\t%16s", pRule->xInfo.pID);
			
			for(j = 0 ; j < sizeof(pRule->xInfo.xParams.pTriggers) / (FTM_ID_LEN+1) ; j++)
			{
				if (j < pRule->xInfo.xParams.ulTriggers)
				{
					MESSAGE(" %2d", pRule->xInfo.xParams.pTriggers[j]);
				}
				else
				{
					MESSAGE(" %2d", 0);
				}
			}

			for(j = 0 ; j < sizeof(pRule->xInfo.xParams.pActions) / (FTM_ID_LEN+1) ; j++)
			{
				if (j < pRule->xInfo.xParams.ulActions)
				{
					MESSAGE(" %2d", pRule->xInfo.xParams.pActions[j]);
				}
				else
				{
					MESSAGE(" %2d", 0);
				}
			}
			MESSAGE("\n");
		}

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
	FTOM_PTR	pOM = (FTOM_PTR)pData;

	return	FTOM_stop(pOM);

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
	ASSERT(pData != NULL);

	FTM_RET		xRet;
	FTOM_PTR 	pOM = (FTOM_PTR)pData;
	
	switch(nArgc)
	{
	case	1:
		{
			xRet = FTOM_discovery(pOM, "255.255.255.255", 1234);
			if (xRet != FTM_RET_OK)
			{
				MESSAGE("Discovery request was failed[%08x].\n", xRet);	
			}
			else
			{
				MESSAGE("Discovery request is complete.\n");	
			
			}
		}
		break;
	}

	return	FTM_RET_OK;
}

