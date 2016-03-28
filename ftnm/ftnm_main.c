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
extern	FTNM_CONTEXT	xCTX;

int main(int nArgc, char *pArgv[])
{
	FTM_INT				nOpt;
	FTM_BOOL			bDaemon = FTM_FALSE;
	FTM_ULONG			ulDebugLevel = FTM_TRACE_LEVEL_ERROR;
	FTM_CHAR			pConfigFileName[1024];

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

	if (bDaemon)
	{
		if (fork() == 0)
		{
			FTM_MEM_init();
			FTM_TRACE_setLevel(ulDebugLevel);
			FTNM_init();

			FTNM_loadFromFile(pConfigFileName);

			FTNM_start();
			FTNM_waitingForFinished();

			FTNM_final();
			FTM_MEM_final();

		}
	}
	else
	{
		FTM_MEM_init();
		FTM_TRACE_setLevel(ulDebugLevel);
		FTNM_init();
	TRACE("%s[%d] : xCTX = %08x, xCTX.pEPM = %08x\n", __func__, __LINE__,&xCTX, xCTX.pEPM);

		FTNM_loadFromFile(pConfigFileName);

		FTNM_start();

		FTM_SHELL_init();
		TRACE("xCTX = %08x\n", &xCTX);
		FTM_SHELL_setGlobalData(&xCTX);
		FTM_SHELL_setPrompt("FTNM> ");
		FTM_SHELL_addCmds(FTNM_shellCmds,FTNM_shellCmdCount);
		FTM_SHELL_run();

		FTNM_final();
		FTM_MEM_final();
	}

	return	0;
}

