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

static FTDM_SERVER	xServer;

FTM_RET 	FTDM_init(FTM_VOID)
{
	FTM_RET	xRet;

	xRet = FTDM_DBIF_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR("FTDM initialization failed. [ %08lx ]\n", xRet);
		return	FTM_RET_OK;
	}

	xRet = FTDM_NODE_init();
	if (xRet == FTM_RET_OK)
	{
		ERROR("Node management initialization fialed.[%08x]\n", xRet);
	}

	xRet = FTDM_EP_init();
	if (xRet == FTM_RET_OK)
	{
		ERROR("EP management initialization fialed.[%08x]\n", xRet);
	}

	xRet = FTDM_TRIGGER_init();
	if (xRet == FTM_RET_OK)
	{
		ERROR("Trigger management initialization fialed.[%08x]\n", xRet);
	}

	xRet = FTDM_ACTION_init();
	if (xRet == FTM_RET_OK)
	{
		ERROR("Action management initialization fialed.[%08x]\n", xRet);
	}

	xRet = FTDM_RULE_init();
	if (xRet == FTM_RET_OK)
	{
		ERROR("Rule management initialization fialed.[%08x]\n", xRet);
	}

	xRet = FTDM_EP_CLASS_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP type management initialization failed.[%08x]\n", xRet);	
	}

	xRet = FTDMS_init(&xServer);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Server initialization failed.[%08x]\n", xRet);	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_final(FTM_VOID)
{
	FTM_RET	xRet;

	xRet = FTDMS_final(&xServer);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Server finalization failed.[%08x]\n", xRet);	
	}

	xRet = FTDM_EP_CLASS_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP Class management finalization failed.[%08x]\n", xRet);	
	}

	xRet = FTDM_RULE_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Rule management finalize failed.[%08x]\n", xRet);	
	}

	xRet = FTDM_ACTION_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Action management finalize failed.[%08x]\n", xRet);	
	}

	xRet = FTDM_TRIGGER_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Trigger management finalize failed.[%08x]\n", xRet);	
	}

	xRet = FTDM_EP_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP management finalize failed.[%08x]\n", xRet);	
	}

	xRet = FTDM_NODE_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node management finalize failed.[%08x]\n", xRet);	
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

FTM_RET 	FTDM_loadConfig(FTDM_CFG_PTR pConfig)
{
	FTM_RET	xRet;

	xRet = FTDM_DBIF_loadConfig(&pConfig->xDB);
	if (xRet != FTM_RET_OK)
	{
		ERROR("FTDM initialization failed. [ %08lx ]\n", xRet);
		return	xRet;
	}

	xRet = FTDM_EP_CLASS_loadConfig(&pConfig->xEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR("FTDM_initEPClassInfo failed\n");	
	}

	FTM_TRACE_configSet(&pConfig->xPrint);


	FTDMS_loadConfig(&xServer, &pConfig->xServer);

	TRACE("FTDM initialization completed successfully.\n");

	return	FTM_RET_OK;
}

FTM_RET	FTDM_loadFromFile(FTM_CHAR_PTR pFileName)
{
	FTDM_CFG	xConfig;

	/* load configuration  */
	FTDM_CFG_init(&xConfig);
	FTDM_CFG_readFromFile(&xConfig, pFileName);

	FTDM_loadConfig(&xConfig);

	FTDM_CFG_final(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_start(FTM_VOID)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_setDebugLevel
(
	FTM_ULONG		ulLevel
)
{
	
	FTM_TRACE_levelString(ulLevel);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_getServer(FTDM_SERVER_PTR _PTR_ ppServer)
{
	*ppServer = &xserver;
}

static FTM_VOID	_showUsage(FTM_CHAR_PTR pAppName);


extern char *program_invocation_short_name;


int main(int nArgc, char *pArgv[])
{
	FTM_INT		nOpt;
	FTM_INT		nDebugLevel = -1;
	FTM_BOOL	bDaemon = FTM_FALSE;
	FTM_BOOL	bShowUsage = FTM_FALSE;
	FTM_CHAR	pConfigFileName[1024];
	
	
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


#if 0
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
#endif

	/* apply configuration */
		
	FTDM_init();
	FTDM_loadFromFile(pConfigFileName);

	if (nDebugLevel >= 0)
	{
		FTDM_setDebugLevel(nDebugLevel);	
	}

	if (bDaemon)
	{ 
		if (fork() == 0)
		{
			FTDMS_start(&xServer);
			FTDMS_waitingForFinished(&xServer);
		}
		else
		{
			return	0;	
		}
	}
	else
	{
		FTDMS_start(&xServer);
		FTM_SHELL_init();
		FTM_SHELL_setPrompt(FTDMS_pPrompt);
		FTM_SHELL_addCmds(FTDMS_pCmdList,FTDMS_ulCmdCount);
		FTM_SHELL_run();

	}

	FTDM_final();

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

