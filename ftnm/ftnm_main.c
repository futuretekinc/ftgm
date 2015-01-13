#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "ftnm.h"
#include "ftm_mem.h"
#include "ftm_debug.h"
#include "ftm_console.h"
#include "ftnm_config.h"

static FTM_RET	FTM_CONSOLE_cmdList
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
);

extern char *program_invocation_short_name;

FTM_CONSOLE_CMD	xConsoleCmds[] = 
{
	{
		.pString	= "list",
		.function	= FTM_CONSOLE_cmdList,
		.pShortHelp	= "show object list.",
		.pHelp		= "\n"\
					  "\tShow object list.\n"
	},
	{
		.pString	= NULL,	
	}
};

static FTNM_CONTEXT	xContext;

int main(int nArgc, char *pArgv[])
{
	FTM_INT				nOpt;
	FTM_CHAR			pConfigFileName[1024];
	FTM_BOOL			bDaemon = FTM_BOOL_FALSE;
	FTNM_CFG_PTR		pConfig;
	FTM_CONSOLE_CMD_PTR	pConsoleCmd;

	FTM_MEM_init();

	sprintf(pConfigFileName, "%s.conf", program_invocation_short_name);

	while((nOpt = getopt(nArgc, pArgv, "c:d?")) != -1)
	{
		switch(nOpt)
		{
		case	'c':
			break;
		
		case	'd':
			{
				bDaemon = FTM_BOOL_TRUE;	
			}
			break;
		}
	}

	setPrintMode(2);


	FTM_CONSOLE_init();
	pConsoleCmd = xConsoleCmds;
	while(pConsoleCmd->pString != NULL)
	{
		FTM_CONSOLE_appendCmd(pConsoleCmd);
		pConsoleCmd++;	
	}

	FTNM_CFG_create(&pConfig);
	FTNM_CFG_load(pConfig, pConfigFileName);
	FTNM_CFG_show(pConfig);

	FTNM_init(&xContext, pConfig);
	FTNM_CFG_destroy(pConfig);

	FTNM_DMC_run(&xContext);
	
	FTM_CONSOLE_run();

	FTM_CONSOLE_final();
	FTNM_final(&xContext);

	FTM_MEM_final();

	return	0;
}

FTM_RET	FTM_CONSOLE_cmdList
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
	MESSAGE("%-16s %-8s %-16s\n", "DID", "STATE", "EPs");
	FTNM_NODE_count(&ulNodeCount);
	for(i = 0 ; i < ulNodeCount ; i++)
	{
		FTNM_NODE_getAt(i, &pNode);
		MESSAGE("%-16s %08lx ", pNode->xInfo.pDID, pNode->xState);

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
	MESSAGE("%-16s %-16s %-16s\n", "EPID", "TYPE", "DID");
	FTNM_EP_count(&ulEPCount);
	for(i = 0; i < ulEPCount ; i++)
	{
		if (FTNM_EP_getAt(i, &pEP) == FTM_RET_OK)
		{
			MESSAGE("%08lx         ", pEP->xInfo.xEPID);
			MESSAGE("%-16s ", FTM_getEPTypeString(pEP->xInfo.xType));
			if (pEP->pNode != NULL)
			{
				MESSAGE("%-16s\n", pEP->pNode->xInfo.pDID);
			}
			else
			{
				MESSAGE("\n");
			}
		}
	}
	return	FTM_RET_OK;
}
