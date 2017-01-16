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
#include "ftdm_server.h"
#include "ftdm_sqlite.h"
#include "ftdm_trigger.h"
#include "ftdm_action.h"
#include "ftdm_rule.h"
#include "shell/ftdm_shell_cmds.h"

static 
FTM_VOID	_showUsage(FTM_CHAR_PTR pAppName);


int main(int nArgc, char *pArgv[])
{
	FTM_RET		xRet;
	FTM_INT		nOpt;
	FTM_ULONG	ulTraceLevel = FTM_TRACE_LEVEL_TRACE;
	FTM_BOOL	bDaemon = FTM_FALSE;
	FTM_BOOL	bShowUsage = FTM_FALSE;
	FTM_BOOL	bLoadObjectFromFile = FTM_FALSE;
	FTM_BOOL	bDBErase = FTM_FALSE;
	FTM_CHAR	pConfigFileName[1024];
	FTM_CHAR	pObjectFileName[1024];
	FTM_CONFIG_PTR		pConfig;
	FTDM_PTR	pFTDM;
	FTDM_DBIF_PTR	pDBIF;
	FTDM_SIS_PTR	pSIS;


	FTM_MEM_init();
	
	sprintf(pConfigFileName, "/etc/%s.conf", FTM_getProgramName());

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
		ERROR2(xRet, "Failed to create configuration[filename = %s]\n", pConfigFileName);

		goto finish2;
	}

	xRet = FTM_TRACE_loadConfig(pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load configuration from file[%s]\n", pConfigFileName);
		FTM_CONFIG_destroy(&pConfig);

		goto finish2;
	}

	xRet = FTDM_create(&pFTDM);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create ftdm!\n");
		goto  finish2;	
	}

	xRet = FTDM_loadConfig(pFTDM, pConfig);
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

	xRet = FTDM_getDBIF(pFTDM, &pDBIF);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get DB interface!\n");	
		goto finish1;
	}

	xRet = FTDM_DBIF_open(pDBIF);
	if (xRet != FTM_RET_OK)
	{
		ERROR("DB open failed[%08x].\n", xRet);
		goto finish1;
	}

	//FTDM_LOGGER_init(pTDM.pLogger);
	
	if (bDBErase)
	{
		MESSAGE("Erase all data!\n");	
		FTDM_DBIF_deleteAllTables(pDBIF);
	}
	
	xRet = FTDM_loadObjectFromDB(pFTDM);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load object from DB!\n");
	}

	if (bLoadObjectFromFile)
	{
		xRet = FTDM_loadObjectFromFile(pFTDM, pObjectFileName);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to load object from file[%s]\n", pObjectFileName);
			goto finish1;	
		}
	}

	FTDM_getSIS(pFTDM, &pSIS);

	if (bDaemon)
	{ 
		if (fork() == 0)
		{

			FTDM_SIS_start(pSIS);
			FTDM_SIS_waitingForFinished(pSIS);
		}
		else
		{
			return	0;	
		}
	}
	else
	{
		FTDM_SIS_start(pSIS);
		FTM_SHELL_run2(FTDM_pPrompt, FTDM_pCmdList, FTDM_ulCmdCount, pSIS);
		FTDM_SIS_stop(pSIS);
	}

	FTDM_DBIF_close(pDBIF);

finish1:
	if (pConfig != NULL)
	{
		xRet = FTM_CONFIG_destroy(&pConfig);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to destroy config object!\n");
		}
	}

	FTDM_destroy(&pFTDM);

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

