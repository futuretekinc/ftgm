#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "ftdm.h"
#include "libconfig.h"
#include "simclist.h"
#include "ftm_console.h"
#include "ftdm_params.h"
#include "ftdm_server.h"
#include "ftdm_config.h"
#include "debug.h"

static FTM_VOID			_showUsage(FTM_CHAR_PTR pAppName);

extern char *program_invocation_short_name;

static FTDM_CFG	xConfig;

int main(int nArgc, char *pArgv[])
{
	FTM_INT		nOpt;
	FTM_BOOL	bDaemon = FTM_BOOL_FALSE;
	FTM_CHAR	pConfigFileName[1024];
	pthread_t 	xPThread;	
		

	
	sprintf(pConfigFileName, "%s.conf", program_invocation_short_name);

	/* set command line options */
	while((nOpt = getopt(nArgc, pArgv, "c:d?")) != -1)
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
				bDaemon = FTM_BOOL_TRUE;
			}
			break;

		case	'?':
		default:
			_showUsage(pArgv[0]);
			return	0;
		}
	}


	setPrintMode(2);

	/* load configuration  */
	FTDM_CFG_init(&xConfig);
	FTDM_CFG_load(&xConfig, pConfigFileName);
	FTDM_CFG_show(&xConfig);

	/* apply configuration */

	FTDM_init(&xConfig);

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

		FTM_CONSOLE_init();
		FTM_CONSOLE_run();
		FTM_CONSOLE_final();
	}


	FTDM_CFG_final(&xConfig);

	return	0;
}

FTM_VOID	_showUsage(FTM_CHAR_PTR pAppName)
{
	MESSAGE("Usage : %s [-d] [-m 0|1|2]\n", pAppName);
	MESSAGE("\tFutureTek Data Manger for M2M gateway.\n");
	MESSAGE("OPTIONS:\n");
	MESSAGE("    -d      Run as a daemon\n");
	MESSAGE("    -m <n>  Set message output mode.\n");
	MESSAGE("            0 - Don't output any messages.\n");
	MESSAGE("            1 - Only general message\n");
	MESSAGE("            2 - All message(include debugging message).\n");
}
