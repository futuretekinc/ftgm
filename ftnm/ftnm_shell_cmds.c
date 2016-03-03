#include "ftnm.h"
#include "ftnm_shell_cmds.h"
#include "ftnm_node.h"
#include "ftnm_ep.h"
#include "ftnm_ep_class.h"

extern	FTNM_CONTEXT	xFTNM;

FTM_RET	FTNM_SHELL_CMD_config
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
);

FTM_RET	FTNM_SHELL_CMD_list
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
);

FTM_SHELL_CMD	FTNM_xCmds[] = 
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
	}
};

FTM_ULONG		FTNM_ulCmds = sizeof(FTNM_xCmds) / sizeof(FTM_SHELL_CMD);

FTM_RET	FTNM_SHELL_CMD_config
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
)
{
	switch(nArgc)
	{
	case	1:
		FTNM_showConfig();
		break;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_SHELL_CMD_list
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
)
{
	FTM_RET			xRet;
	FTNM_NODE_PTR	pNode;
	FTNM_EP_PTR		pEP;
	FTM_ULONG		i,j, ulCount;

	MESSAGE("\n< NODE LIST >\n");
	MESSAGE("%-16s %-16s %-8s %-8s %-8s \n", "DID", "STATE", "INTERVAL", "TIMEOUT", "EPs");
	FTNM_NODE_count(&ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTNM_NODE_getAt(i, &pNode);
		MESSAGE("%-16s %-16s %-8d %-8d ", pNode->xInfo.pDID, FTNM_NODE_stateToStr(pNode->xState), pNode->xInfo.ulInterval, pNode->xInfo.ulTimeout);

		FTNM_NODE_getEPCount(pNode, &ulCount);
		MESSAGE("%-3d [ ", ulCount);
		for(j = 0; j < ulCount ; j++)
		{
			if (FTNM_NODE_getEPAt(pNode, j, &pEP) == FTM_RET_OK)
			{
				MESSAGE("%08lx ", pEP->xInfo.xEPID);
			}
		}
		MESSAGE("]\n");
	}

	MESSAGE("\n< EP LIST >\n");
	MESSAGE("%-16s %-16s %-16s %-16s %-8s %-24s\n", "EPID", "TYPE", "DID", "STATE", "VALUE", "TIME");
	FTNM_EP_count(0, &ulCount);
	for(i = 0; i < ulCount ; i++)
	{
		if (FTNM_EP_getAt(i, &pEP) == FTM_RET_OK)
		{
			FTM_CHAR	pTimeString[64];
			FTM_EP_DATA	xData;
		
			FTNM_EP_getData(pEP, &xData);

			ctime_r((time_t *)&xData.ulTime, pTimeString);
			if (strlen(pTimeString) != 0)
			{
				pTimeString[strlen(pTimeString) - 1] = '\0';
			}
			
			MESSAGE("%08lx         ", pEP->xInfo.xEPID);
			MESSAGE("%-16s ", FTM_getEPTypeString(pEP->xInfo.xType));
			if (pEP->pNode != NULL)
			{
				MESSAGE("%-16s ", pEP->pNode->xInfo.pDID);
			}
	
			switch(pEP->xState)
			{
			case	FTM_EP_STATE_DISABLE: MESSAGE("%-16s ", "DISABLE"); break;
			case	FTM_EP_STATE_RUN: 	MESSAGE("%-16s ", "RUN");		break;
			case	FTM_EP_STATE_STOP: 	MESSAGE("%-16s ", "STOP");		break;
			case	FTM_EP_STATE_ERROR: MESSAGE("%-16s ", "ERROR");		break;
			default:					MESSAGE("%-16s ", "UNKNOWN");		break;
			}

			switch(xData.xType)
			{
			case	FTM_EP_DATA_TYPE_INT: 	MESSAGE("%-8d ", xData.xValue.nValue); break;
			case	FTM_EP_DATA_TYPE_ULONG:	MESSAGE("%-8lu ", xData.xValue.ulValue); break;
			case	FTM_EP_DATA_TYPE_FLOAT:	MESSAGE("%-8.2lf ", xData.xValue.fValue); break;
			default: 						MESSAGE("%-8s ", "UNKNOWN"); break;
			}
			MESSAGE("%24s\n", pTimeString);
		}
	}

	MESSAGE("\n< TRIGGER >\n");
	FTNM_TRIG_count(&ulCount);
	MESSAGE("\t%-8s %-8s %-16s %s\n", "ID", "EPID", "TYPE", "CONDITION");
	for(i = 0; i< ulCount ; i++)
	{
		FTNM_TRIG_PTR	pTrigger;

		xRet = FTNM_TRIG_getAt(i, &pTrigger);
		if (xRet == FTM_RET_OK)
		{
			FTM_CHAR	pCondition[1024];

			FTM_EVENT_conditionToString(&pTrigger->xEvent, pCondition, sizeof(pCondition));

			MESSAGE("\t%08x %08x %-16s %s\n", 
				pTrigger->xEvent.xID, 
				pTrigger->xEvent.xEPID, 
				FTM_EVENT_typeString(pTrigger->xEvent.xType),
				pCondition);
		}

	}
	return	FTM_RET_OK;
}
