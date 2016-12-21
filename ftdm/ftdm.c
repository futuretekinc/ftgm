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

extern 
char *program_invocation_short_name;

static 
FTDM_SERVER	xServer;

static 
FTM_VOID	_showUsage(FTM_CHAR_PTR pAppName);

FTDM_CONTEXT	xFTDM;


FTM_RET 	FTDM_init(FTDM_CONTEXT_PTR pDM)
{
	FTM_RET	xRet;

	xRet = FTDM_DBIF_init(&pDM->xDBIF);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "FTDM initialization failed.\n");
		return	FTM_RET_OK;
	}

	xRet = FTDM_NODEM_create(&pDM->pNodeM);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Node management initialization fialed.\n");
	}

	xRet = FTDM_EPM_create(&pDM->pEPM);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "EP management initialization fialed.\n");
	}

	xRet = FTDM_LOGGER_create(&pDM->pLogger);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create logger!\n");
	}

	xRet = FTDM_TRIGGER_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Trigger management initialization fialed.\n");
	}

	xRet = FTDM_ACTION_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Action management initialization fialed.\n");
	}

	xRet = FTDM_RULE_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Rule management initialization fialed\n");
	}

	xRet = FTDM_EP_CLASS_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "EP type management initialization failed.\n" );	
	}

	xRet = FTDMS_init(&xServer, pDM);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Server initialization failed.\n");	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_final
(	
	FTDM_CONTEXT_PTR pFTDM
)
{
	FTM_RET	xRet;

	xRet = FTDMS_final(&xServer);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Server finalization failed.\n");	
	}

	xRet = FTDM_EP_CLASS_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "EP Class management finalization failed.\n");	
	}

	xRet = FTDM_RULE_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Rule management finalize failed.[%08x]\n", xRet);	
	}

	xRet = FTDM_ACTION_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Action management finalize failed.\n");	
	}

	xRet = FTDM_TRIGGER_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Trigger management finalize failed.\n");	
	}

	xRet = FTDM_LOGGER_destroy(&pFTDM->pLogger);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to destroy logger.\n");	
	}

	xRet = FTDM_EPM_destroy(&pFTDM->pEPM);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "EP management finalize failed.\n");	
	}

	xRet = FTDM_NODEM_destroy(&pFTDM->pNodeM);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Node management finalize failed.\n");	
	}

	xRet = FTDM_DBIF_final(&pFTDM->xDBIF);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "FTDM finalization failed. [ %08lx ]\n", xRet);
		return	FTM_RET_OK;
	}

	TRACE("FTDM finalization completed successfully.\n");

	return	FTM_RET_OK;
}

FTM_RET 	FTDM_loadConfig
(
	FTDM_CONTEXT_PTR	pDM,
	FTM_CONFIG_PTR		pConfig
)
{
	FTM_RET	xRet;

	xRet = FTDM_DBIF_loadConfig(&pDM->xDBIF, pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "FTDM initialization failed.\n");
		return	xRet;
	}

	xRet = FTDM_EP_CLASS_loadConfig(pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "FTDM_initEPClassInfo failed\n");	
	}


	FTDMS_loadConfig(&xServer, pConfig);

	TRACE("FTDM initialization completed successfully.\n");

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
	
	FTM_TRACE_LEVEL_print(ulLevel, FTM_TRUE);

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

int main(int nArgc, char *pArgv[])
{
	FTM_RET		xRet;
	FTM_INT		nOpt;
	FTM_ULONG	ulTraceLevel = FTM_TRACE_LEVEL_TRACE;
	FTM_BOOL	bDaemon = FTM_FALSE;
	FTM_BOOL	bShowUsage = FTM_FALSE;
	FTM_BOOL	bLoadObjectFromFile = FTM_FALSE;
	FTM_BOOL	bDBFirst = FTM_TRUE;
	FTM_BOOL	bDBErase = FTM_FALSE;
	FTM_CHAR	pConfigFileName[1024];
	FTM_CHAR	pObjectFileName[1024];
	FTM_CONFIG_PTR		pConfig;

	FTM_MEM_init();
	
	sprintf(pConfigFileName, "/etc/%s.conf", program_invocation_short_name);

	/* set command line options */
	while((nOpt = getopt(nArgc, pArgv, "c:dv:a:i:e?")) != -1)
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

		case	'v':
			{
				if (FTM_TRACE_strToLevel(optarg, &ulTraceLevel) != FTM_RET_OK)
				{
					bShowUsage = FTM_TRUE; 
				}
			}
			break;

		case	'a':
			{
				bLoadObjectFromFile = FTM_TRUE;
				strcpy(pObjectFileName, optarg);
			}
			break;

		case	'i':
			{
				bDBFirst = FTM_FALSE;
				bLoadObjectFromFile = FTM_TRUE;
				strcpy(pObjectFileName, optarg);
			}
			break;

		case	'e':
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
		

	FTM_TRACE_setInfo2(FTDM_TRACE_MODULE_SERVER,"SERVER",	FTM_TRACE_LEVEL_DISABLE, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2(FTDM_TRACE_MODULE_EP,	"EP", 		FTM_TRACE_LEVEL_DISABLE, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2(FTDM_TRACE_MODULE_DBIF,	"DBIF", 	FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);

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

	xRet = FTDM_init(&xFTDM);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to initialize ftdm!\n");
		goto  finish2;	
	}

	xRet = FTDM_loadConfig(&xFTDM, pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load configuration!\n");
		goto finish1;	
	}

	xRet = FTM_CONFIG_destroy(&pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to destroy config object!\n");
		goto finish1;	
	}

	xRet = FTDM_DBIF_open(&xFTDM.xDBIF);
	if (xRet != FTM_RET_OK)
	{
		ERROR("DB open failed[%08x].\n", xRet);
		return	0;	
	}

	FTDM_LOGGER_init(xFTDM.pLogger);
	
	if (bDBErase)
	{
		MESSAGE("Erase all data!\n");	
		FTDM_DBIF_TABLE_deleteAllTables();
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
	//	FTDM_saveObjectToDB(&xFTDM);
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
		FTM_SHELL_run2(FTDMS_pPrompt, FTDMS_pCmdList, FTDMS_ulCmdCount, &xServer);
		FTDMS_stop(&xServer);
	}


	FTDM_DBIF_close(&xFTDM.xDBIF);

finish1:
	FTDM_final(&xFTDM);

finish2:
	FTM_MEM_final();

	return	0;
}

FTM_VOID	_showUsage(FTM_CHAR_PTR pAppName)
{
	MESSAGE("Usage : %s [-c FILE] [-d] [-v <level>]\n", pAppName);
	MESSAGE("\tFutureTek Data Manger for IoT gateway.\n");
	MESSAGE("OPTIONS:\n");
	MESSAGE("\t-c FILE\tConfiguration file\n");
	MESSAGE("\t-i FILE\tobject file\n");
	MESSAGE("\t-d\tRun as a daemon\n");
	MESSAGE("\t-e\terase all object\n");
	MESSAGE("\t-v LEVEL\tSet message output mode.\n");
	MESSAGE("PARAMETERS:\n");
	MESSAGE("\t%8s %s\n", "LEVEL", "Output level");
	MESSAGE("\t%8s %d:%8s - %s\n", "        ", FTM_TRACE_LEVEL_ALL, 	"all", 	"Output all message");
	MESSAGE("\t%8s %d:%8s - %s\n", "        ", FTM_TRACE_LEVEL_TRACE, "trace","Output trace message");
	MESSAGE("\t%8s %d:%8s - %s\n", "        ", FTM_TRACE_LEVEL_DEBUG, "debug","Output debug message");
	MESSAGE("\t%8s %d:%8s - %s\n", "        ", FTM_TRACE_LEVEL_INFO, 	"info", "Output information message");
	MESSAGE("\t%8s %d:%8s - %s\n", "        ", FTM_TRACE_LEVEL_WARN, 	"warn", "Output warning message");
	MESSAGE("\t%8s %d:%8s - %s\n", "        ", FTM_TRACE_LEVEL_ERROR, "error","Output error message");
	MESSAGE("\t%8s %d:%8s - %s\n", "        ", FTM_TRACE_LEVEL_FATAL, "fatal","Output fatal message");
}

