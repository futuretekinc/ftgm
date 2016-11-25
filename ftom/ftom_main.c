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
	FTM_CONFIG_PTR		pConfig;

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
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_FTOM,		"FTOM", 	FTM_TRACE_LEVEL_FATAL, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_NODE,		"NODE", 	FTM_TRACE_LEVEL_FATAL, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_EP,		"EP", 		FTM_TRACE_LEVEL_FATAL, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_TRIGGER,	"TRIGGER", 	FTM_TRACE_LEVEL_FATAL, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_ACTION,	"ACTION", 	FTM_TRACE_LEVEL_FATAL, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_RULE,		"RULE", 	FTM_TRACE_LEVEL_FATAL, FTM_TRACE_OUT_TERM);

	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_DMC,		"DMC", 		FTM_TRACE_LEVEL_FATAL, FTM_TRACE_OUT_TERM);

	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_SNMPC,	"SNMPC", 	FTM_TRACE_LEVEL_FATAL, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_SNMPTRAP,	"SNMPTRAP", FTM_TRACE_LEVEL_FATAL, FTM_TRACE_OUT_TERM);

	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_MQTTC,	"MQTTC", 	FTM_TRACE_LEVEL_FATAL, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_DISCOVERY,"DISCOVERY",FTM_TRACE_LEVEL_FATAL, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_SERVER,	"SERVER",	FTM_TRACE_LEVEL_FATAL, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_LOGGER,	"LOGGER",	FTM_TRACE_LEVEL_FATAL, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_CLIENT,	"CLIENT",	FTM_TRACE_LEVEL_FATAL, FTM_TRACE_OUT_TERM);

	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_SERVICE,	"SERVICE", 	FTM_TRACE_LEVEL_FATAL, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_MSGQ,		"MSGQ", 	FTM_TRACE_LEVEL_FATAL, FTM_TRACE_OUT_TERM);

	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_SHELL,	"SHELL", 	FTM_TRACE_LEVEL_FATAL, FTM_TRACE_OUT_TERM);

	xRet =FTM_CONFIG_create(pConfigFileName, &pConfig, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "SERVER configuration file[%s] load failed\n", pConfigFileName);

		goto finish2;
	}

	xRet = FTM_TRACE_loadConfig(pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load configuration from file[%s]\n", pConfigFileName);
		FTM_CONFIG_destroy(&pConfig);

		goto finish2;
	}

	xRet = FTOM_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can't create object manager!\n");
		FTM_CONFIG_destroy(&pConfig);

		goto finish2;
	}

	xRet = FTOM_loadConfig(pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load configuration[%s]\n", pConfigFileName);
		FTM_CONFIG_destroy(&pConfig);

		goto finish1;
	}

	xRet = FTOM_SERVICE_loadConfig(FTOM_SERVICE_ALL, pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load configuration from file[%s]\n", pConfigFileName);
		FTM_CONFIG_destroy(&pConfig);

		goto finish1;
	}

	FTM_CONFIG_destroy(&pConfig);

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


finish1:
	FTOM_final();


finish2:
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
