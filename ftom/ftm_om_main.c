#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "ftm.h"
#include "ftm_om.h"
#include "ftm_om_shell_cmds.h"

extern char *program_invocation_short_name;
extern	FTM_SHELL_CMD	FTM_OM_shellCmds[];
extern	FTM_ULONG		FTM_OM_shellCmdCount;

int main(int nArgc, char *pArgv[])
{
	FTM_RET		xRet;
	FTM_INT		nOpt;
	FTM_BOOL	bDaemon = FTM_FALSE;
	FTM_ULONG	ulDebugLevel = FTM_TRACE_LEVEL_ERROR;
	FTM_CHAR	pConfigFileName[1024];
	FTM_OM_PTR	pOM;

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
	xRet = FTM_OM_create(&pOM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't create object manager!\n");
		return	-1;	
	}

	FTM_OM_loadFromFile(pOM, pConfigFileName);

	if (bDaemon)
	{
		if (fork() != 0)
		{
			return	0;
		}
	}

	FTM_OM_start(pOM);
	FTM_OM_waitingForFinished(pOM);

	FTM_OM_destroy(&pOM);
	FTM_MEM_final();

	return	0;
}

