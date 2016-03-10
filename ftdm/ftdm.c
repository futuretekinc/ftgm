#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "libconfig.h"
#include "ftm.h"
#include "ftdm.h"
#include "ftdm_config.h"
#include "ftdm_ep.h"
#include "ftdm_ep_class.h"
#include "ftdm_node.h"
#include "ftdm_params.h"
#include "ftdm_server_cmds.h"
#include "ftdm_server.h"
#include "ftdm_sqlite.h"
#include "ftdm_trigger.h"
#include "ftdm_action.h"
#include "ftdm_rule.h"

FTM_RET 	FTDM_init(FTM_VOID)
{
	FTM_RET	xRet;

#if 0
	xRet = FTDM_EP_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP management initialization failed.\n");	
	}
#endif	
	xRet = FTDM_EP_CLASS_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP type management initialization failed.\n");	
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_final(FTM_VOID)
{
	FTM_RET	xRet;

	xRet = FTDM_EP_CLASS_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR("FTDM_finalNodeInfo failed\n");	
	}
#if 0
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
#endif

	xRet = FTDM_DBIF_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR("FTDM finalization failed. [ %08lx ]\n", xRet);

		return	FTM_RET_OK;
	}

	TRACE("FTDM finalization completed successfully.\n");

	return	FTM_RET_OK;
}

FTM_RET 	FTDM_loadConfig(FTDM_CFG_PTR pConfig)
{
	FTM_RET	xRet;

	xRet = FTDM_DBIF_loadConfig(&pConfig->xDB);
	if (xRet != FTM_RET_OK)
	{
		ERROR("FTDM initialization failed. [ %08lx ]\n", xRet);
		return	xRet;
	}
/*
	xRet = FTDM_EP_loadConfig(&pConfig->xEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR("FTDM_initEPInfo failed\n");	
	}
*/	
	xRet = FTDM_EP_CLASS_loadConfig(&pConfig->xEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR("FTDM_initEPClassInfo failed\n");	
	}

	FTM_TRACE_configSet(&pConfig->xPrint);


	TRACE("FTDM initialization completed successfully.\n");

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


	/* load configuration  */
	FTDM_CFG_init(&xConfig);
	FTDM_CFG_readFromFile(&xConfig, pConfigFileName);

	xRet = FTDM_NODE_init();
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_NODE_loadFromFile(pConfigFileName);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Node configuration load failed.\n");
			return	0;
		}
	}

	xRet = FTDM_EP_init();
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_loadFromFile(pConfigFileName);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Node configuration load failed.\n");
			return	0;
		}
	}

	xRet = FTDM_TRIGGER_init();
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_TRIGGER_loadFromFile(pConfigFileName);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Event configuration load failed.\n");	
			return	0;
		}
	}

	xRet = FTDM_ACTION_init();
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_ACTION_loadFromFile(pConfigFileName);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Actor configuration load failed.\n");	
			return	0;
		}
	}

	xRet = FTDM_RULE_init();
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_RULE_loadFromFile(pConfigFileName);
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
		
	FTDM_init();
	FTDM_loadConfig(&xConfig);


	if (bDaemon)
	{ 
		if (fork() == 0)
		{
			FTDMS_run(&xConfig.xServer, &xPThread);
			pthread_join(xPThread, NULL);
		}
		else
		{
			return	0;	
		}
	}
	else
	{
		FTDMS_run(&xConfig.xServer, &xPThread);
		FTM_SHELL_init();
		FTM_SHELL_setPrompt(FTDMS_pPrompt);
		FTM_SHELL_addCmds(FTDMS_pCmdList,FTDMS_ulCmdCount);
		FTM_SHELL_run();

	}

	FTDM_RULE_final();
	FTDM_ACTION_final();
	FTDM_TRIGGER_final();
	FTDM_EP_final();
	FTDM_NODE_final();
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

