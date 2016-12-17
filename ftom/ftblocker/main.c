#include <stdio.h>
#include <stdlib.h>

#include "ftom.h"
#include "ftom_blocker.h"
#include "ftom_blocker_shell_cmds.h"

#undef	__MODULE__
#define __MODULE__ FTOM_TRACE_MODULE_CLIENT

static
FTM_VOID	FTOM_BLOCKER_usage
(
	FTM_VOID
);


FTM_INT	main
(	
	FTM_INT nArgc, 
	FTM_CHAR_PTR pArgv[]
)
{
	FTM_RET			xRet;
	FTM_INT			nOpt;
	FTM_BOOL		bDaemon = FTM_FALSE;
	FTM_ULONG		ulDebugLevel = FTM_TRACE_LEVEL_ALL;
	FTOM_BLOCKER_PTR	pBlocker = NULL;
	FTM_CHAR		pConfigFileName[1024];
	FTM_CONFIG_PTR	pConfig = NULL;

	sprintf(pConfigFileName, "/etc/%s.conf", FTM_getProgramName());

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
			{
				FTOM_BLOCKER_usage();
			
				goto finish;
			}
		}
	}

	xRet = FTM_MEM_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Memory initialization failed.\n");
		goto finish;	
	}

	FTM_TRACE_setLevel(FTM_TRACE_MAX_MODULES, ulDebugLevel);
	FTM_TRACE_setInfo2(FTOM_TRACE_MODULE_CLIENT,"client", FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2(FTOM_TRACE_MODULE_MQTTC,"mqtt", FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);

	xRet = FTM_CONFIG_create(pConfigFileName, &pConfig, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Configuration loading failed!\n");
		goto finish;
	}

	xRet = FTOM_BLOCKER_create(FTM_getProgramName(), pConfig, (FTOM_BLOCKER_PTR _PTR_)&pBlocker);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can't create a client.\n");
		goto finish;
	}

	FTM_CONFIG_destroy(&pConfig);

	xRet = FTOM_BLOCKER_init(pBlocker);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to blocker initialize!\n");
		goto finish;
	}

	if (bDaemon)
	{
		if (fork() != 0)
		{
			return	0;
		}
	}

	FTOM_BLOCKER_start(pBlocker);

	if (!bDaemon)
	{
		FTM_SHELL_run2(pBlocker->pCloudClientModule->pName, FTOM_BLOCKER_SHELL_cmds, FTOM_BLOCKER_SHELL_cmdCount, pBlocker);
	}

	xRet = FTOM_BLOCKER_waitingForFinished(pBlocker);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "The client was finished abnormally!!.\n");	
	}

finish:

	if (pBlocker != NULL)
	{
		FTOM_BLOCKER_final(pBlocker);

		xRet = FTOM_BLOCKER_destroy(&pBlocker);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Remove the Client failed\n");
		}
	}

	if (pConfig != NULL)
	{
		FTM_CONFIG_destroy(&pConfig);	
	}

	xRet = FTM_MEM_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Memory finalization failed.\n");	
	}

  	return 0;
}

FTM_VOID	FTOM_BLOCKER_usage
(
	FTM_VOID
)
{
	MESSAGE("Usage: %s [-d] [-c CONFFILE] [-v LEVEL]\n\n", FTM_getProgramName());
	MESSAGE("Connect ftom to azure.\n");
	MESSAGE("OPTIONS:\n");
	MESSAGE("\t-d\tRun as a daemon\n");
	MESSAGE("\t-c FILE\tConfiguration file (default /etc/%s.conf)\n", FTM_getProgramName());
	MESSAGE("\t-v LEVEL\tVerbose level\n");
}
