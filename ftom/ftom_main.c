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
				FTM_TRACE_strToLevel(optarg, &ulDebugLevel);
			}
			break;

		default:
			FTOM_usage();
			return	0;
		}
	}

	FTM_MEM_init();
	FTM_TRACE_setLevel(FTM_TRACE_MODULE_ALL, ulDebugLevel);
	//FTM_TRACE_setModule(0, FTM_TRUE);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_FTOM,		FTM_TRUE, "FTOM", 		FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_NODE,		FTM_TRUE, "NODE", 		FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_EP,		FTM_TRUE, "EP", 		FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_TRIGGER,	FTM_FALSE, "TRIGGER", 	FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_ACTION,	FTM_FALSE, "ACTION", 	FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_RULE,		FTM_FALSE, "RULE", 		FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);

	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_DMC,		FTM_FALSE, "DMC", 		FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);

	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_SNMPC,	FTM_TRUE, 	"SNMPC", 	FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_SNMPTRAP,	FTM_FALSE, "SNMPTRAP", 	FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);

	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_MQTTC,	FTM_FALSE, "MQTTC", 	FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_DISCOVERY,FTM_TRUE, "DISCOVERY",	FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_SERVER,	FTM_TRUE, "SERVER",	FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_LOGGER,	FTM_TRUE, "LOGGER",	FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_CLIENT,	FTM_TRUE, "CLIENT",	FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);

	xRet = FTOM_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can't create object manager!\n");
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
