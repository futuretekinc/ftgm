#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "libconfig.h"
#include "ftm.h"
#include "ftdm.h"
#include "ftdm_config.h"
#include "ftdm_ep.h"
#include "ftdm_ep_management.h"
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

FTM_RET 	FTDM_init(FTDM_CONTEXT_PTR pDM)
{
	FTM_RET	xRet;

	xRet = FTDM_DBIF_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR("FTDM initialization failed. [ %08lx ]\n", xRet);
		return	FTM_RET_OK;
	}

	xRet = FTDM_NODEM_create(&pDM->pNodeM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node management initialization fialed.[%08x]\n", xRet);
	}

	xRet = FTDM_EPM_create(&pDM->pEPM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP management initialization fialed.[%08x]\n", xRet);
	}

	xRet = FTDM_TRIGGER_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Trigger management initialization fialed.[%08x]\n", xRet);
	}

	xRet = FTDM_ACTION_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Action management initialization fialed.[%08x]\n", xRet);
	}

	xRet = FTDM_RULE_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Rule management initialization fialed.[%08x]\n", xRet);
	}

	xRet = FTDM_EP_CLASS_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP type management initialization failed.[%08x]\n", xRet);	
	}

	xRet = FTDMS_init(&xServer, pDM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Server initialization failed.[%08x]\n", xRet);	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_final(FTDM_CONTEXT_PTR pFTDM)
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

	xRet = FTDM_EPM_destroy(&pFTDM->pEPM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("EP management finalize failed.[%08x]\n", xRet);	
	}

	xRet = FTDM_NODEM_destroy(&pFTDM->pNodeM);
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

FTM_RET 	FTDM_loadConfig
(
	FTDM_CONTEXT_PTR	pDM,
	FTDM_CFG_PTR 		pConfig
)
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

FTM_RET	FTDM_loadConfigFromFile
(
	FTDM_CONTEXT_PTR	pDM,
	FTM_CHAR_PTR pFileName
)
{
	FTDM_CFG	xConfig;

	/* load configuration  */
	FTDM_CFG_init(&xConfig);
	FTDM_CFG_readFromFile(&xConfig, pFileName);

	FTDM_loadConfig(pDM, &xConfig);

	FTDM_CFG_final(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_loadObjectFromFile
(
	FTDM_CONTEXT_PTR	pDM,
	FTM_CHAR_PTR		pFileName
)
{
	FTM_RET	xRet;

	TRACE("Object load from file\n");
	xRet = FTDM_NODEM_loadFromFile(pDM->pNodeM, pFileName);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node configuration load failed.\n");
		return	FTM_RET_ERROR;
	}

	xRet = FTDM_EPM_loadFromFile(pDM->pEPM, pFileName);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node configuration load failed.\n");
		return	FTM_RET_ERROR;
	}

	xRet = FTDM_TRIGGER_loadFromFile(pFileName);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Event configuration load failed.\n");	
		return	FTM_RET_ERROR;
	}

	xRet = FTDM_ACTION_loadFromFile(pFileName);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Actor configuration load failed.\n");	
		return	FTM_RET_ERROR;
	}

	xRet = FTDM_RULE_loadFromFile(pFileName);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Actor configuration load failed.\n");	
		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_loadObjectFromDB
(	
	FTDM_CONTEXT_PTR	pDM
)
{
	FTM_RET	xRet;

	TRACE("Objects load from DB\n");
	xRet = FTDM_NODEM_loadFromDB(pDM->pNodeM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to load the node.[%08x]\n");
	}

	xRet = FTDM_EPM_loadFromDB(pDM->pEPM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to load the EP.[%08x]\n");
	}

	xRet = FTDM_TRIGGER_loadFromDB();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to load the trigger.[%08x]\n");
	}

	xRet = FTDM_ACTION_loadFromDB();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to load the action.[%08x]\n");
	}

	xRet = FTDM_RULE_loadFromDB();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to load the rule.[%08x]\n");
	}

	return	FTM_RET_OK;
}
FTM_RET	FTDM_saveObjectToDB
(
	FTDM_CONTEXT_PTR	pDM
)
{
	FTM_RET	xRet;

	xRet = FTDM_NODEM_saveToDB(pDM->pNodeM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to save the node.[%08x]\n");
	}

	xRet = FTDM_EPM_saveToDB(pDM->pEPM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to save the EP.[%08x]\n");
	}

	xRet = FTDM_TRIGGER_saveToDB();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to save the trigger.[%08x]\n");
	}

	xRet = FTDM_ACTION_saveToDB();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to save the action.[%08x]\n");
	}

	xRet = FTDM_RULE_saveToDB();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to save the rule.[%08x]\n");
	}

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

FTM_RET	FTDM_getServer
(
	FTDM_SERVER_PTR _PTR_ ppServer
)
{
	*ppServer = &xServer;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_removeInvalidData
(
	FTDM_CONTEXT_PTR pDM
)
{
	ASSERT(pDM != NULL);
	FTM_RET		xRet;
	FTM_ULONG	i, ulCount;

	xRet = FTDM_EPM_count(pDM->pEPM, 0, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTDM_EP_PTR	pEP;

		xRet = FTDM_EPM_getAt(pDM->pEPM, i, &pEP);
		if (xRet == FTM_RET_OK)
		{
		
		}
	}
	return	FTM_RET_OK;
}

static FTM_VOID	_showUsage(FTM_CHAR_PTR pAppName);


extern char *program_invocation_short_name;


static 	FTDM_CONTEXT	xFTDM;

int main(int nArgc, char *pArgv[])
{
	FTM_RET		xRet;
	FTM_INT		nOpt;
	FTM_ULONG	ulTraceLevel = FTM_TRACE_LEVEL_ERROR;
	FTM_BOOL	bDaemon = FTM_FALSE;
	FTM_BOOL	bShowUsage = FTM_FALSE;
	FTM_BOOL	bLoadObjectFromFile = FTM_FALSE;
	FTM_BOOL	bDBFirst = FTM_TRUE;
	FTM_BOOL	bDBErase = FTM_FALSE;
	FTM_CHAR	pConfigFileName[1024];
	FTM_CHAR	pObjectFileName[1024];
	FTM_SHELL	xShell;

	FTM_MEM_init();
	
	sprintf(pConfigFileName, "%s.conf", program_invocation_short_name);

	/* set command line options */
	while((nOpt = getopt(nArgc, pArgv, "c:dm:o:O:E?")) != -1)
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
				if (FTM_TRACE_strToLevel(optarg, &ulTraceLevel) != FTM_RET_OK)
				{
					bShowUsage = FTM_TRUE; 
				}
			}
			break;

		case	'o':
			{
				bLoadObjectFromFile = FTM_TRUE;
				strcpy(pObjectFileName, optarg);
			}
			break;

		case	'O':
			{
				bDBFirst = FTM_FALSE;
				bLoadObjectFromFile = FTM_TRUE;
				strcpy(pObjectFileName, optarg);
			}
			break;

		case	'E':
			{
				bDBErase = FTM_TRUE;	
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


	/* apply configuration */
		
	FTDM_init(&xFTDM);
	FTDM_loadConfigFromFile(&xFTDM, pConfigFileName);

	if (ulTraceLevel >= 0)
	{
		FTDM_setDebugLevel(ulTraceLevel);	
	}

	xRet = FTDM_DBIF_open();
	if (xRet != FTM_RET_OK)
	{
		ERROR("DB open failed[%08x].\n", xRet);
		return	0;	
	}

	if (bDBErase)
	{
		MESSAGE("Erase all data!\n");	
	}

	if (bDBFirst)
	{
		xRet = FTDM_loadObjectFromDB(&xFTDM);
		if (xRet == FTM_RET_OK)
		{
			bLoadObjectFromFile = FTM_FALSE;
		}
	}

	if (bLoadObjectFromFile)
	{
		xRet = FTDM_loadObjectFromFile(&xFTDM, pObjectFileName);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Can't load object from file[%08x]\n", xRet);
			ERROR("File Name : %s\n", pObjectFileName);
			return	0;	
		}

		FTDM_saveObjectToDB(&xFTDM);
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
		FTM_SHELL_init(&xShell, &xServer);
		FTM_SHELL_setPrompt(&xShell, FTDMS_pPrompt);
		FTM_SHELL_addCmds(&xShell, FTDMS_pCmdList,FTDMS_ulCmdCount);
		FTM_SHELL_run(&xShell);
		FTM_SHELL_final(&xShell);
		FTDMS_stop(&xServer);
	}

	FTDM_final(&xFTDM);

	FTDM_DBIF_close();
	
	FTM_MEM_final();

	return	0;
}

FTM_VOID	_showUsage(FTM_CHAR_PTR pAppName)
{
	MESSAGE("Usage : %s [-c file] [-d] [-m <level>]\n", pAppName);
	MESSAGE("\tFutureTek Data Manger for M2M gateway.\n");
	MESSAGE("OPTIONS:\n");
	MESSAGE("    -c <file> Configuration file\n");
	MESSAGE("    -o <file> Configuration file\n");
	MESSAGE("    -d        Run as a daemon\n");
	MESSAGE("    -m <leve> Set message output mode.\n");
	MESSAGE("PARAMETERS:\n");
	MESSAGE("    %8s %s\n", "level", "Output level");
	MESSAGE("    %8s %d:%8s - %s\n", "        ", FTM_TRACE_LEVEL_ALL, 	"all", 	"Output all message");
	MESSAGE("    %8s %d:%8s - %s\n", "        ", FTM_TRACE_LEVEL_TRACE, "trace","Output trace message");
	MESSAGE("    %8s %d:%8s - %s\n", "        ", FTM_TRACE_LEVEL_DEBUG, "debug","Output debug message");
	MESSAGE("    %8s %d:%8s - %s\n", "        ", FTM_TRACE_LEVEL_INFO, 	"info", "Output information message");
	MESSAGE("    %8s %d:%8s - %s\n", "        ", FTM_TRACE_LEVEL_WARN, 	"warn", "Output warning message");
	MESSAGE("    %8s %d:%8s - %s\n", "        ", FTM_TRACE_LEVEL_ERROR, "error","Output error message");
	MESSAGE("    %8s %d:%8s - %s\n", "        ", FTM_TRACE_LEVEL_FATAL, "fatal","Output fatal message");
}

