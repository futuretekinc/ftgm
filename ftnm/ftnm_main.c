#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "ftnm.h"
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
static FTNM_CFG	xConfig;

int main(int nArgc, char *pArgv[])
{
	FTM_INT				nOpt;
	FTM_CHAR			pConfigFileName[1024];
	FTM_BOOL			bDaemon = FTM_BOOL_FALSE;
	FTM_CONSOLE_CMD_PTR	pConsoleCmd;

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

	FTNM_CFG_init(&xConfig);
	FTNM_CFG_load(&xConfig, pConfigFileName);
	FTNM_CFG_show(&xConfig);

	FTNM_init(&xContext, &xConfig);

	FTNM_DMC_run(&xContext);
	
	FTM_CONSOLE_run();

	FTM_CONSOLE_final();
	FTNM_final(&xContext);
	FTNM_CFG_final(&xConfig);

	return	0;
}

FTM_RET	FTM_CONSOLE_cmdList
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
)
{
	FTNM_NODE_PTR	pNode;
	FTM_ULONG		i, ulCount;

	FTNM_NODE_count(&ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTNM_NODE_getAt(i, &pNode);
		TRACE("DID = %s\n", pNode->xInfo.pDID);
		TRACE("STATE = %08lx\n", pNode->xState);
	}
	
	return	FTM_RET_OK;
}
