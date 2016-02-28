#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "libconfig.h"
#include "ftdm.h"
#include "ftm_debug.h"
#include "ftm_shell.h"
#include "ftdm_config.h"
#include "ftdm_ep_info.h"
#include "ftdm_ep_class_info.h"
#include "ftdm_node_info.h"
#include "ftdm_params.h"
#include "ftdm_server_cmds.h"
#include "ftdm_server.h"
#include "ftdm_sqlite.h"
#include "ftdm_event.h"
#include "ftdm_act.h"


FTM_RET 	FTDM_init(FTDM_CFG_PTR pConfig)
{
	FTM_RET	xRet;

	xRet = FTDM_DBIF_init(&pConfig->xDB);
	if (xRet != FTM_RET_OK)
	{
		ERROR("FTDM initialization failed. [ %08lx ]\n", xRet);
		return	xRet;
	}

	xRet = FTDM_NODE_init(&pConfig->xNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR("FTDM_initNodeInfo failed\n");	
	}

	xRet = FTDM_EP_init(&pConfig->xEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR("FTDM_initEPInfo failed\n");	
	}
	
	xRet = FTDM_EP_CLASS_INFO_init(&pConfig->xEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR("FTDM_initEPClassInfo failed\n");	
	}

	FTM_PRINT_configSet(&pConfig->xPrint);


	TRACE("FTDM initialization completed successfully.\n");

	return	FTM_RET_OK;
}

FTM_RET	FTDM_final(void)
{
	FTM_RET	xRet;

	xRet = FTDM_EP_CLASS_INFO_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR("FTDM_finalNodeInfo failed\n");	
	}

	xRet = FTDM_EP_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR("FTDM_finalNodeInfo failed\n");	
	}

	xRet = FTDM_NODE_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR("FTDM_finalNodeInfo failed\n");	
	}


	xRet = FTDM_DBIF_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR("FTDM finalization failed. [ %08lx ]\n", xRet);

		return	FTM_RET_OK;
	}

	TRACE("FTDM finalization completed successfully.\n");

	return	FTM_RET_OK;
}


static FTM_VOID	_showUsage(FTM_CHAR_PTR pAppName);


extern char *program_invocation_short_name;
FTDM_CFG	xConfig;


int main(int nArgc, char *pArgv[])
{
	FTM_RET		xRet;
	FTM_INT		nOpt;
	FTM_INT		nDebugLevel = -1;
	FTM_BOOL	bDaemon = FTM_FALSE;
	FTM_BOOL	bShowUsage = FTM_FALSE;
	FTM_CHAR	pConfigFileName[1024];
	pthread_t 	xPThread;	
	FTM_SHELL_CONFIG	xShellConfig;
	
	xShellConfig.pCmdList 	= FTDMS_pCmdList;
	xShellConfig.ulCmdCount	= FTDMS_ulCmdCount;
	xShellConfig.pPrompt	= FTDMS_pPrompt;
	
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

	if (bShowUsage)
	{
		_showUsage(pArgv[0]);
		return	0;
	}

	FTM_SHELL_init(&xShellConfig);

	/* load configuration  */
	FTDM_CFG_init(&xConfig);
	FTDM_CFG_load(&xConfig, pConfigFileName);

	xRet = FTDM_EVENT_init();
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EVENT_loadFromFile(pConfigFileName);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Event configuration load failed.\n");	
			return	0;
		}
	}

	xRet = FTDM_ACT_init();
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_ACT_loadFromFile(pConfigFileName);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Actor configuration load failed.\n");	
			return	0;
		}
	}

	if (nDebugLevel >= 0)
	{
		xConfig.xPrint.ulLevel = nDebugLevel;	
	}

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
		FTM_SHELL_run();
	}

	FTDM_ACT_final();

	FTDM_EVENT_final();

	FTDM_CFG_final(&xConfig);

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

