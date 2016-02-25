#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "ftnm.h"
#include "ftm_mem.h"
#include "ftm_debug.h"
#include "ftnm_shell_cmds.h"

extern char *program_invocation_short_name;

int main(int nArgc, char *pArgv[])
{
	FTM_INT				nOpt;
	FTM_BOOL			bDaemon = FTM_FALSE;
	FTM_SHELL_CONFIG	xShellConfig;
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
	FTM_PRINT_setLevel(0);

	xShellConfig.pPrompt 		= "FTNMS>";
	xShellConfig.pCmdList 	= FTNM_xCmds;
	xShellConfig.ulCmdCount 	= FTNM_ulCmds;

	FTM_SHELL_init(&xShellConfig);
	FTNM_init();

	FTNM_loadConfig(pConfigFileName);

	if (bDaemon)
	{
		if (fork() == 0)
		{
			FTNM_run();
			FTNM_waitingForFinished();
		}
	}
	else
	{
		FTNM_run();
		FTM_SHELL_run();
	}

	FTNM_final();

	FTM_MEM_final();

	return	0;
}

