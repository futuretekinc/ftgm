#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "ftnm.h"
#include "ftm_mem.h"
#include "ftm_debug.h"
#include "ftnm_console_cmds.h"

extern char *program_invocation_short_name;

FTNM_CONTEXT	xFTNM;

int main(int nArgc, char *pArgv[])
{
	FTM_INT				nOpt;
	FTM_BOOL			bDaemon = FTM_FALSE;
	FTM_CONSOLE_CONFIG	xConsoleConfig;
	FTM_CHAR			pConfigFileName[1024];

	sprintf(pConfigFileName, "%s.conf", program_invocation_short_name);

	while((nOpt = getopt(nArgc, pArgv, "c:d?")) != -1)
	{
		switch(nOpt)
		{
		case	'c':
			{
				strncpy(pConfigFileName, optarg, sizeof(pConfigFileName));
			}
			break;
		
		case	'd':
			{
				bDaemon = FTM_TRUE;	
			}
			break;
		}
	}

	FTM_MEM_init();
	FTM_DEBUG_printModeSet(2);

	xConsoleConfig.pPrompt 		= "FTNMS>";
	xConsoleConfig.pCmdList 	= FTNM_xCmds;
	xConsoleConfig.ulCmdCount 	= FTNM_ulCmds;

	FTM_CONSOLE_init(&xConsoleConfig);
	FTNM_init(&xFTNM);
	
	FTNM_loadConfig(&xFTNM, pConfigFileName);

	if (bDaemon)
	{
		if (fork() == 0)
		{
			FTNM_run(&xFTNM);
			FTNM_waitingForFinished(&xFTNM);
		}
	}
	else
	{
		FTNM_run(&xFTNM);
		FTM_CONSOLE_run();
	}

	FTNM_final(&xFTNM);
	FTM_MEM_final();

	return	0;
}

