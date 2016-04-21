#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "ftm.h"
#include "ftom.h"
#include "ftom_shell_cmds.h"

extern	FTM_SHELL_CMD	FTOM_shellCmds[];
extern	FTM_ULONG		FTOM_shellCmdCount;

int main(int nArgc, char *pArgv[])
{
	FTM_RET		xRet;
	FTM_INT		nOpt;
	FTM_BOOL	bDaemon = FTM_FALSE;
	FTM_ULONG	ulDebugLevel = FTM_TRACE_LEVEL_ERROR;
	FTM_CHAR	pConfigFileName[1024];
	FTOM_PTR	pOM;

	sprintf(pConfigFileName, "%s.conf", FTOM_getProgramName());

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

	FTM_MEM_init();
	FTM_TRACE_setLevel(ulDebugLevel);
	xRet = FTOM_create(&pOM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't create object manager!\n");
		return	-1;	
	}

	FTOM_loadFromFile(pOM, pConfigFileName);

	if (bDaemon)
	{
		if (fork() != 0)
		{
			return	0;
		}
	}

	FTOM_start(pOM);
	FTOM_waitingForFinished(pOM);
	
	FTOM_destroy(&pOM);
	FTM_MEM_final();

	return	0;
}

