#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "ftdm.h"
#include "libconfig.h"
#include "ftm_console.h"
#include "ftdm_params.h"
#include "ftdm_server.h"
#include "ftdm_config.h"
#include "ftdm_server_cmds.h"
#include "ftm_debug.h"

static FTM_VOID	_showUsage(FTM_CHAR_PTR pAppName);


extern char *program_invocation_short_name;
FTDM_CFG	xConfig;


int main(int nArgc, char *pArgv[])
{
	FTM_INT		nOpt;
	FTM_INT		nDebugLevel = -1;
	FTM_BOOL	bDaemon = FTM_FALSE;
	FTM_BOOL	bShowUsage = FTM_FALSE;
	FTM_CHAR	pConfigFileName[1024];
	pthread_t 	xPThread;	
	FTM_CONSOLE_CONFIG	xConsoleConfig;
	
	xConsoleConfig.pCmdList 	= FTDMS_pCmdList;
	xConsoleConfig.ulCmdCount	= FTDMS_ulCmdCount;
	xConsoleConfig.pPrompt		= FTDMS_pPrompt;
	
	sprintf(pConfigFileName, "%s.conf", program_invocation_short_name);

	/* set command line options */
	while((nOpt = getopt(nArgc, pArgv, "c:dm:?")) != -1)
	{
		switch(nOpt)
		{
		case	'c':
			{
				strcpy(pConfigFileName, optarg);
			}
			break;

		case	'd':
			{
				bDaemon = FTM_TRUE;
			}
			break;

		case	'm':
			{
				nDebugLevel = atoi(optarg);
				if (nDebugLevel < 0 || 5 < nDebugLevel)
				{
					bShowUsage = FTM_TRUE; 
				}
			}
			break;

		case	'?':
		default:
			bShowUsage = FTM_TRUE;
		}
	}

	if (!bShowUsage)
	{
		/* load configuration  */
		FTDM_CFG_init(&xConfig);
		FTDM_CFG_load(&xConfig, pConfigFileName);

		if (nDebugLevel >= 0)
		{
			xConfig.xDebug.ulMode = nDebugLevel;	
		}

		/* apply configuration */
		
		FTDM_init(&xConfig);
		FTM_CONSOLE_init(&xConsoleConfig);

		if (bDaemon)
		{ 
			if (fork() == 0)
			{
				FTDMS_run(&xConfig.xServer, &xPThread);
				pthread_join(xPThread, NULL);
			}
		}
		else
		{
			FTDMS_run(&xConfig.xServer, &xPThread);
			FTM_CONSOLE_run();
		}
	
		FTDM_CFG_final(&xConfig);
	}
	else
	{
		_showUsage(pArgv[0]);
	
	}

	return	0;
}

FTM_VOID	_showUsage(FTM_CHAR_PTR pAppName)
{
	MESSAGE("Usage : %s [-c file] [-d] [-m 0|1|2]\n", pAppName);
	MESSAGE("\tFutureTek Data Manger for M2M gateway.\n");
	MESSAGE("OPTIONS:\n");
	MESSAGE("    -c <file> Configuration file\n");
	MESSAGE("    -d        Run as a daemon\n");
	MESSAGE("    -m <n>    Set message output mode.\n");
	MESSAGE("              0 - Don't output any messages.\n");
	MESSAGE("              1 - Only general message\n");
	MESSAGE("              2 - All message(include debugging message).\n");
}

