#include "ftnm.h"
#include "ftnm_console_cmds.h"
#include "ftnm_node.h"
#include "ftnm_ep.h"
#include "ftnm_ep_class.h"

extern	FTNM_CONTEXT	xFTNM;

FTM_RET	FTNM_CONSOLE_CMD_config
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
);

FTM_RET	FTNM_CONSOLE_CMD_list
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
);

FTM_CONSOLE_CMD	FTNM_xCmds[] = 
{
	{
		.pString	= "config",
		.function	= FTNM_CONSOLE_CMD_config,
		.pShortHelp	= "Configuration Management",
		.pHelp		= "\n"\
					  "\tConfiguration Management.\n"
	},
	{
		.pString	= "list",
		.function	= FTNM_CONSOLE_CMD_list,
		.pShortHelp	= "show object list.",
		.pHelp		= "\n"\
					  "\tShow object list.\n"
	}
};

FTM_ULONG		FTNM_ulCmds = sizeof(FTNM_xCmds) / sizeof(FTM_CONSOLE_CMD);

FTM_RET	FTNM_CONSOLE_CMD_config
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
)
{
	switch(nArgc)
	{
	case	1:
		FTNM_showConfig(&xFTNM);
		break;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNM_CONSOLE_CMD_list
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
)
{
	FTNM_NODE_PTR	pNode;
	FTNM_EP_PTR		pEP;
	FTM_ULONG		i, ulNodeCount;
	FTM_ULONG		j, ulEPCount;

	MESSAGE("\n< NODE LIST >\n");
	MESSAGE("%-16s %-16s %-16s\n", "DID", "STATE", "EPs");
	FTNM_NODE_count(&ulNodeCount);
	for(i = 0 ; i < ulNodeCount ; i++)
	{
		FTNM_NODE_getAt(i, &pNode);
		MESSAGE("%-16s %-16s ", pNode->xInfo.pDID, FTNM_NODE_stateString(pNode->xState));

		FTNM_NODE_EP_count(pNode, &ulEPCount);
		MESSAGE("%3d [ ", ulEPCount);
		for(j = 0; j < ulEPCount ; j++)
		{
			if (FTNM_NODE_EP_getAt(pNode, j, &pEP) == FTM_RET_OK)
			{
				MESSAGE("%08lx ", pEP->xInfo.xEPID);
			}
		}
		MESSAGE("]\n");
	}

	MESSAGE("\n< EP LIST >\n");
	MESSAGE("%-16s %-16s %-16s %-8s %-24s\n", "EPID", "TYPE", "DID", "VALUE", "TIME");
	FTNM_EP_count(0, &ulEPCount);
	for(i = 0; i < ulEPCount ; i++)
	{
		if (FTNM_EP_getAt(i, &pEP) == FTM_RET_OK)
		{
			FTM_CHAR	pTimeString[64];
			
			ctime_r((time_t *)&pEP->xData.ulTime, pTimeString);
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
		
			switch(pEP->xData.xType)
			{
			case	FTM_EP_DATA_TYPE_INT: 	MESSAGE("%-8d ", pEP->xData.xValue.nValue); break;
			case	FTM_EP_DATA_TYPE_ULONG:	MESSAGE("%-8lu ", pEP->xData.xValue.ulValue); break;
			case	FTM_EP_DATA_TYPE_FLOAT:	MESSAGE("%-8.2lf ", pEP->xData.xValue.fValue); break;
			default: 						MESSAGE("%-8s ", "UNKNOWN"); break;
			}
			MESSAGE("%24s\n", pTimeString);
		}
	}
	return	FTM_RET_OK;
}
