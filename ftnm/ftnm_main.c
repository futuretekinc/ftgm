#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "ftm.h"
#include "ftnm.h"
#include "ftnm_shell_cmds.h"

extern char *program_invocation_short_name;
extern	FTM_SHELL_CMD	FTNM_shellCmds[];
extern	FTM_ULONG		FTNM_shellCmdCount;

int main(int nArgc, char *pArgv[])
{
	FTM_RET			xRet;
	FTM_INT			nOpt;
	FTM_BOOL		bDaemon = FTM_FALSE;
	FTM_ULONG		ulDebugLevel = FTM_TRACE_LEVEL_ERROR;
	FTM_CHAR		pConfigFileName[1024];
	FTNM_CONTEXT_PTR	pCTX;

	sprintf(pConfigFileName, "%s.conf", program_invocation_short_name);

	while((nOpt = getopt(nArgc, pArgv, "c:Dd:?")) != -1)
	{
		switch(nOpt)
		{
		case	'c':
			{
				strncpy(pConfigFileName, optarg, sizeof(pConfigFileName));
			}
			break;
		
		case	'D':
			{
				bDaemon = FTM_TRUE;	
			}
			break;

		case	'd':
			{
				ulDebugLevel = strtoul(optarg, 0, 10);
			}
			break;
		
		}
	}

//	FTM_DEBUG_initSignals();

	FTM_MEM_init();
	FTM_TRACE_setLevel(ulDebugLevel);
	xRet = FTNM_create(&pCTX);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't create object manager!\n");
		return	-1;	
	}

	FTNM_loadFromFile(pCTX, pConfigFileName);

	if (bDaemon)
	{
		if (fork() == 0)
		{
			FTNM_start(pCTX);
			FTNM_waitingForFinished(pCTX);

		}
	}
	else
	{
		FTNM_start(pCTX);

		FTM_SHELL_init();
		FTM_SHELL_setGlobalData(pCTX);
		FTM_SHELL_setPrompt("FTNM> ");
		FTM_SHELL_addCmds(FTNM_shellCmds,FTNM_shellCmdCount);
		FTM_SHELL_run();
	}

	FTNM_destroy(&pCTX);
	FTM_MEM_final();

	return	0;
}

