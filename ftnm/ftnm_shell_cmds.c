#include "ftnm.h"
#include "ftnm_shell_cmds.h"
#include "ftnm_node.h"
#include "ftnm_ep.h"
#include "ftnm_ep_class.h"
#include "ftnm_msg.h"

extern	FTNM_CONTEXT	xFTNM;

FTM_RET	FTNM_SHELL_CMD_config(FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTNM_SHELL_CMD_list(FTM_INT		nArgc, FTM_CHAR_PTR	pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTNM_SHELL_CMD_task(FTM_INT		nArgc, FTM_CHAR_PTR	pArgv[], FTM_VOID_PTR pData);
FTM_RET	FTNM_SHELL_CMD_quit(FTM_INT		nArgc, FTM_CHAR_PTR	pArgv[], FTM_VOID_PTR pData);

FTM_SHELL_CMD	FTNM_shellCmds[] = 
{
	{
		.pString	= "config",
		.function	= FTNM_SHELL_CMD_config,
		.pShortHelp	= "Configuration Management",
		.pHelp		= "\n"\
					  "\tConfiguration Management.\n"
	},
	{
		.pString	= "list",
		.function	= FTNM_SHELL_CMD_list,
		.pShortHelp	= "show object list.",
		.pHelp		= "\n"\
					  "\tShow object list.\n"
	},
	{
		.pString	= "exit",
		.function	= FTNM_SHELL_CMD_quit,
		.pShortHelp	= "exit.",
		.pHelp		= "\n"\
					  "\texit.\n"
	},
	{
		.pString	= "task",
		.function	= FTNM_SHELL_CMD_task,
		.pShortHelp	= "task management",
		.pHelp		= "\n"\
					  "\ttask management.\n"
	}

};

FTM_ULONG		FTNM_shellCmdCount = sizeof(FTNM_shellCmds) / sizeof(FTM_SHELL_CMD);


FTM_RET	FTNM_SHELL_CMD_config
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	FTNM_CONTEXT_PTR pCTX = (FTNM_CONTEXT_PTR)pData;

	switch(nArgc)
	{
	case	1:
		FTNM_showConfig(pCTX);
		break;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_SHELL_CMD_list
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	FTM_RET				xRet;
	FTNM_NODE_PTR		pNode;
	FTNM_EP_PTR			pEP;
	FTM_ULONG			i,j, ulCount;
	FTNM_CONTEXT_PTR	pCTX = (FTNM_CONTEXT_PTR)pData;

	MESSAGE("\n# Node Information\n");
	MESSAGE("%16s %16s %8s %8s %s\n", "DID", "STATE", "INTERVAL", "TIMEOUT", "EPs");
	FTNM_NODE_count(&ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_ULONG	ulEPCount;

		FTNM_NODE_getAt(i, &pNode);
		MESSAGE("%16s ", pNode->xInfo.pDID);
		MESSAGE("%16s ", FTNM_NODE_stateToStr(pNode->xState));
		MESSAGE("%8d ", pNode->xInfo.ulInterval);
		MESSAGE("%8d ", pNode->xInfo.ulTimeout);

		FTNM_NODE_getEPCount(pNode, &ulEPCount);
		MESSAGE("%-3d [ ", ulCount);
		for(j = 0; j < ulEPCount ; j++)
		{
			if (FTNM_NODE_getEPAt(pNode, j, &pEP) == FTM_RET_OK)
			{
				MESSAGE("%08lx ", pEP->xInfo.xEPID);
			}
		}
		MESSAGE("]\n");
	}

	MESSAGE("\n# EP Information\n");
	MESSAGE("%16s %16s %16s %16s %8s %24s\n", "EPID", "TYPE", "DID", "STATE", "VALUE", "TIME");
		TRACE("%s[%d] pCTX = %x\n", __func__, __LINE__, pCTX);
	FTNM_EPM_count(pCTX->pEPM, 0, &ulCount);
	for(i = 0; i < ulCount ; i++)
	{
		TRACE("%s[%d]\n", __func__, __LINE__);
		if (FTNM_EPM_getAt(pCTX->pEPM, i, &pEP) == FTM_RET_OK)
		{
			FTM_CHAR	pTimeString[64];
			FTM_EP_DATA	xData;
		
		TRACE("%s[%d]\n", __func__, __LINE__);
			FTNM_EP_getData(pEP, &xData);

		TRACE("%s[%d]\n", __func__, __LINE__);
			ctime_r((time_t *)&xData.ulTime, pTimeString);
			if (strlen(pTimeString) != 0)
			{
				pTimeString[strlen(pTimeString) - 1] = '\0';
			}
			
		TRACE("%s[%d]\n", __func__, __LINE__);
			MESSAGE("%16lx ", pEP->xInfo.xEPID);
			MESSAGE("%16s ", FTM_EP_typeString(pEP->xInfo.xType));
			if (pEP->pNode != NULL)
			{
				MESSAGE("%16s ", pEP->pNode->xInfo.pDID);
			}
	
			switch(pEP->xState)
			{
			case	FTM_EP_STATE_DISABLE: MESSAGE("%16s ", "DISABLE"); break;
			case	FTM_EP_STATE_RUN: 	MESSAGE("%16s ", "RUN");		break;
			case	FTM_EP_STATE_STOP: 	MESSAGE("%16s ", "STOP");		break;
			case	FTM_EP_STATE_ERROR: MESSAGE("%16s ", "ERROR");		break;
			default:					MESSAGE("%16s ", "UNKNOWN");		break;
			}

			switch(xData.xType)
			{
			case	FTM_EP_DATA_TYPE_INT: 	MESSAGE("%8d ", xData.xValue.nValue); break;
			case	FTM_EP_DATA_TYPE_ULONG:	MESSAGE("%8lu ", xData.xValue.ulValue); break;
			case	FTM_EP_DATA_TYPE_FLOAT:	MESSAGE("%8.2lf ", xData.xValue.fValue); break;
			default: 						MESSAGE("%8s ", "UNKNOWN"); break;
			}
			MESSAGE("%24s\n", pTimeString);
		}
	}

	MESSAGE("\n# Trigger Information\n");
	FTNM_TRIGGERM_count(pCTX->pTriggerM, &ulCount);
	MESSAGE("\t%8s %8s %16s %8s %8s %s\n", "ID", "EPID", "TYPE", "DETECT", "HOLD", "CONDITION");
	for(i = 0; i< ulCount ; i++)
	{
		FTNM_TRIGGER_PTR	pTrigger;

		xRet = FTNM_TRIGGERM_getAt(pCTX->pTriggerM, i, &pTrigger);
		if (xRet == FTM_RET_OK)
		{
			FTM_CHAR	pCondition[1024];

			FTM_TRIGGER_conditionToString(&pTrigger->xInfo, pCondition, sizeof(pCondition));

			MESSAGE("\t%8d %08x %16s %8.3f %8.3f %s\n", 
				pTrigger->xInfo.xID, 
				pTrigger->xInfo.xEPID, 
				FTM_TRIGGER_typeString(pTrigger->xInfo.xType),
            	(pTrigger->xInfo.xParams.xCommon.xDetectionTime.xTimeval.tv_sec * 1000000 +  pTrigger->xInfo.xParams.xCommon.xDetectionTime.xTimeval.tv_usec) / 1000000.0,
				(pTrigger->xInfo.xParams.xCommon.xHoldingTime.xTimeval.tv_sec * 1000000 +  pTrigger->xInfo.xParams.xCommon.xHoldingTime.xTimeval.tv_usec) / 1000000.0,
				pCondition);
		}

	}

	MESSAGE("\n# Action Information\n");
	FTNM_ACTIONM_count(pCTX->pActionM, &ulCount);
	MESSAGE("\t%8s %16s\n", "ID","TYPE");
	for(i = 0; i< ulCount ; i++)
	{
		FTNM_ACTION_PTR	pAction;

		xRet = FTNM_ACTIONM_getAt(pCTX->pActionM, i, &pAction);
		if (xRet == FTM_RET_OK)
		{
			MESSAGE("\t%8d %16s\n", 
				pAction->xInfo.xID, 
				FTM_ACTION_typeString(pAction->xInfo.xType));
		}

	}
	
	MESSAGE("\n# Rule Information\n");
	FTNM_RULEM_count(pCTX->pRuleM, &ulCount);
	MESSAGE("\t%8s %24s %24s\n", "ID","TRIGGER", "ACTION");
	for(i = 0; i< ulCount ; i++)
	{
		FTNM_RULE_PTR	pRule;

		xRet = FTNM_RULEM_getAt(pCTX->pRuleM, i, &pRule);
		if (xRet == FTM_RET_OK)
		{
			MESSAGE("\t%8d", pRule->xInfo.xID);
			
			for(j = 0 ; j < sizeof(pRule->xInfo.xParams.pTriggers) / sizeof(FTM_TRIGGER_ID) ; j++)
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

			for(j = 0 ; j < sizeof(pRule->xInfo.xParams.pActions) / sizeof(FTM_ACTION_ID) ; j++)
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

FTM_RET	FTNM_SHELL_CMD_quit
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	FTNM_CONTEXT_PTR	pCTX = (FTNM_CONTEXT_PTR)pData;

	return	FTNM_NOTIFY_quit(pCTX);

}

FTM_RET	FTNM_SHELL_CMD_task
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	return	FTM_RET_OK;

}

