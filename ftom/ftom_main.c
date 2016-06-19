#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "ftm.h"
#include "ftom.h"
#include "ftom_shell.h"

extern	FTM_SHELL_CMD	FTOM_shellCmds[];
extern	FTM_ULONG		FTOM_shellCmdCount;
static
FTM_VOID	FTOM_usage
(
	FTM_VOID
);

int main(int nArgc, char *pArgv[])
{
	FTM_RET		xRet;
	FTM_INT		nOpt;
	FTM_BOOL	bDaemon = FTM_FALSE;
	FTM_ULONG	ulDebugLevel = FTM_TRACE_LEVEL_ERROR;
	FTM_CHAR	pConfigFileName[1024];

	sprintf(pConfigFileName, "/etc/%s.conf", FTOM_getProgramName());

	while((nOpt = getopt(nArgc, pArgv, "c:dv:h?")) != -1)
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

		case	'v':
			{
				ulDebugLevel = strtoul(optarg, 0, 10);
			}
			break;

		default:
			FTOM_usage();
			return	0;
		}
	}

	FTM_MEM_init();
	FTM_TRACE_setLevel(ulDebugLevel);
	FTM_TRACE_setModule(0xFFFFFFFF, FTM_TRUE);

	xRet = FTOM_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't create object manager!\n");
		return	-1;	
	}

	FTOM_loadConfigFromFile(pConfigFileName);

	if (bDaemon)
	{
		if (fork() != 0)
		{
			return	0;
		}
		FTOM_start();
		FTOM_waitingForFinished();
	}
	else
	{
		FTOM_SHELL_PTR pShell;

		FTOM_start();
		FTOM_SHELL_create(&pShell);
		FTOM_SHELL_process(pShell);
		FTOM_SHELL_destroy(&pShell);
		FTOM_waitingForFinished();
	}

	
	FTOM_final();
	FTM_MEM_final();

	return	0;
}

FTM_VOID	FTOM_usage
(
	FTM_VOID
)
{
	MESSAGE("Usage: %s [-d] [-c CONFFILE] [-v LEVEL]\n\n", FTOM_getProgramName());
	MESSAGE("Object manager for IoT gateway.\n");
	MESSAGE("OPTIONS:\n");
	MESSAGE("\t-d\tRun as a daemon\n");
	MESSAGE("\t-c FILE\tConfiguration file (default /etc/%s.conf)\n", FTOM_getProgramName());
	MESSAGE("\t-v LEVEL\tVerbose level\n");
}
