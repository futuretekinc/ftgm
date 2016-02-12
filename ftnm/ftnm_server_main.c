#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "ftnm.h"
#include "ftm_mem.h"
#include "ftm_debug.h"
#include "ftnm_cmds.h"
#include "ftnm_config.h"

extern char *program_invocation_short_name;

FTM_VOID_PTR	FTNMS_process(FTM_VOID_PTR pData);

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
	FTNM_init(pConfigFileName);

	if (bDaemon)
	{
		if (fork() == 0)
		{
			FTNMS_process(NULL);
		}
	}
	else
	{
		FTNMS_process(NULL);
		FTM_CONSOLE_run();
	}

	FTNM_final();
	FTM_MEM_final();

	return	0;
}

FTM_VOID_PTR	FTNMS_process(FTM_VOID_PTR pData)
{
	FTNM_run();

	return	0;
}
