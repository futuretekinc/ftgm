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
#include "ftdm_dbif.h"
#include "ftdm_trigger.h"
#include "ftdm_action.h"
#include "ftdm_rule.h"

#undef	__MODULE__
#define	__MODULE__	FTDM_TRACE_MODULE_FTDM

static 
FTDM_SERVER	xServer;

static 
FTM_VOID	_showUsage(FTM_CHAR_PTR pAppName);

FTDM_CONTEXT	xFTDM;


FTM_RET 	FTDM_init
(
	FTM_VOID
)	
{
	FTM_RET	xRet;

	xRet = FTDM_DBIF_create(&xFTDM.pDBIF);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "FTDM initialization failed.\n");
		return	FTM_RET_OK;
	}

	xRet = FTDM_NODEM_create(xFTDM.pDBIF, &xFTDM.pNodeM);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Node management initialization fialed.\n");
	}

	xRet = FTDM_EPM_create(xFTDM.pDBIF, &xFTDM.pEPM);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "EP management initialization fialed.\n");
	}

	xRet = FTDM_LOGGER_create(xFTDM.pDBIF, &xFTDM.pLogger);
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

	xRet = FTDM_SERVER_init(&xServer, &xFTDM);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Server initialization failed.\n");	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_final
(
	FTM_VOID
)
{
	FTM_RET	xRet;

	xRet = FTDM_SERVER_final(&xServer);
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

	xRet = FTDM_LOGGER_destroy(&xFTDM.pLogger);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to destroy logger.\n");	
	}

	xRet = FTDM_EPM_destroy(&xFTDM.pEPM);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "EP management finalize failed.\n");	
	}

	xRet = FTDM_NODEM_destroy(&xFTDM.pNodeM);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Node management finalize failed.\n");	
	}

	xRet = FTDM_DBIF_destroy(&xFTDM.pDBIF);
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
	FTM_CONFIG_PTR		pConfig
)
{
	FTM_RET	xRet;

	xRet = FTDM_DBIF_loadConfig(xFTDM.pDBIF, pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load DB configuration.\n");
		return	xRet;
	}
	

	xRet = FTDM_EP_CLASS_loadConfig(pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load EP CLASS configuration.\n");
		return	xRet;
	}
	

	xRet = FTDM_SERVER_loadConfig(&xServer, pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load server configuration.\n");
		return	xRet;
	}
	
	TRACE("FTDM initialization completed successfully.\n");

	return	FTM_RET_OK;
}

FTM_RET	FTDM_loadObject
(
	FTM_CONFIG_PTR		pConfig
)
{
	FTM_RET	xRet;

	TRACE("Object load from file\n");
	xRet = FTDM_NODEM_loadConfig(xFTDM.pNodeM, pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node configuration load failed.\n");
		return	FTM_RET_ERROR;
	}

	xRet = FTDM_EPM_loadConfig(xFTDM.pEPM, pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Node configuration load failed.\n");
		return	FTM_RET_ERROR;
	}

	xRet = FTDM_loadTriggerConfig(pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Event configuration load failed.\n");	
		return	FTM_RET_ERROR;
	}

	xRet = FTDM_loadActionConfig(pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Actor configuration load failed.\n");	
		return	FTM_RET_ERROR;
	}

	xRet = FTDM_loadRuleConfig(pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Actor configuration load failed.\n");	
		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_loadObjectFromDB
(	
	FTM_VOID
)
{
	FTM_RET	xRet;

	TRACE("Objects load from DB\n");
	xRet = FTDM_NODEM_loadFromDB(xFTDM.pNodeM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to load the node.[%08x]\n");
	}

	xRet = FTDM_EPM_loadFromDB(xFTDM.pEPM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to load the EP.[%08x]\n");
	}

	xRet = FTDM_loadTriggerFromDB();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to load the trigger.[%08x]\n");
	}

	xRet = FTDM_loadActionFromDB();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to load the action.[%08x]\n");
	}

	xRet = FTDM_loadRuleFromDB();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to load the rule.[%08x]\n");
	}

	return	FTM_RET_OK;
}
FTM_RET	FTDM_saveObjectToDB
(
	FTM_VOID
)
{
	FTM_RET	xRet;

	xRet = FTDM_NODEM_saveToDB(xFTDM.pNodeM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to save the node.[%08x]\n");
	}

	xRet = FTDM_EPM_saveToDB(xFTDM.pEPM);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to save the EP.[%08x]\n");
	}

	xRet = FTDM_saveTriggerToDB();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to save the trigger.[%08x]\n");
	}

	xRet = FTDM_saveActionToDB();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Failed to save the action.[%08x]\n");
	}

	xRet = FTDM_saveRuleToDB();
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
	FTM_VOID
)
{
	FTM_RET		xRet;
	FTM_ULONG	i, ulCount;

	xRet = FTDM_EPM_count(xFTDM.pEPM, 0, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTDM_EP_PTR	pEP;

		xRet = FTDM_EPM_getAt(xFTDM.pEPM, i, &pEP);
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
	FTM_CONFIG_PTR	pConfig = NULL;

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
		
	FTM_TRACE_setInfo2(FTDM_TRACE_MODULE_FTDM,	"FTDM",		FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2(FTDM_TRACE_MODULE_SERVER,"SERVER",	FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2(FTDM_TRACE_MODULE_EP,	"EP", 		FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);
	FTM_TRACE_setInfo2(FTDM_TRACE_MODULE_DBIF,	"DBIF", 	FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);

	xRet = FTM_CONFIG_create(pConfigFileName, &pConfig, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	xRet = FTM_TRACE_loadConfig(pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load configuration from file[%s]\n", pConfigFileName);
	}

	xRet = FTDM_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to initialize FTDM.\n");	
		return	xRet;
	}

	xRet = FTDM_loadConfig(pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load configuration!\n");
	}

	FTM_CONFIG_destroy(&pConfig);

	xRet = FTDM_DBIF_open(xFTDM.pDBIF);
	if (xRet != FTM_RET_OK)
	{
		ERROR("DB open failed[%08x].\n", xRet);
		return	0;	
	}

	FTDM_LOGGER_init(xFTDM.pLogger);
	
	if (bDBErase)
	{
		MESSAGE("Erase all data!\n");	
		FTDM_DBIF_deleteAllTables(xFTDM.pDBIF);
	}

	if (bDBFirst)
	{
		xRet = FTDM_loadObjectFromDB();
		if (xRet == FTM_RET_OK)
		{
			bLoadObjectFromFile = FTM_FALSE;
		}
	}

	if (bLoadObjectFromFile)
	{
		xRet = FTM_CONFIG_create(pObjectFileName, &pConfig, FTM_FALSE);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;
		}

		xRet = FTDM_loadConfig(pConfig);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Can't load object from file[%08x]\n", xRet);
			ERROR("File Name : %s\n", pObjectFileName);
			return	0;	
		}

		FTM_CONFIG_destroy(&pConfig);
	}

	if (bDaemon)
	{ 
		if (fork() == 0)
		{
			FTDM_SERVER_start(&xServer);
			FTDM_SERVER_waitingForFinished(&xServer);
		}
		else
		{
			return	0;	
		}
	}
	else
	{
		FTDM_SERVER_start(&xServer);
		FTM_SHELL_run2(FTDM_pPrompt, FTDM_pCmdList, FTDM_ulCmdCount, &xServer);
		FTDM_SERVER_stop(&xServer);
	}

	FTDM_final();

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

FTM_RET	FTDM_createAction
(
	FTM_ACTION_PTR 	pInfo,
	FTDM_ACTION_PTR	_PTR_ ppAction
)
{
	ASSERT(pInfo != NULL);
	FTM_RET				xRet;
	FTDM_ACTION_PTR	pAction;

	if (FTDM_ACTION_get(pInfo->pID, &pAction) == FTM_RET_OK)
	{
			ERROR("Action[%s] already exist.\n", pInfo->pID);
		return	FTM_RET_ALREADY_EXIST_OBJECT;
	}

	if (strlen(pInfo->pID) == 0)
	{
		do 
		{	
			struct timeval	xTime;

			gettimeofday(&xTime, NULL);
			sprintf(pInfo->pID, "%08lx%08lx", (FTM_ULONG)xTime.tv_sec, (FTM_ULONG)xTime.tv_usec);
			usleep(10);
		}
		while (FTDM_ACTION_get(pInfo->pID, &pAction) == FTM_RET_OK);
	}

	xRet = FTDM_DBIF_createAction(xFTDM.pDBIF, pInfo);
	if (xRet == FTM_RET_OK)
	{
		pAction = (FTDM_ACTION_PTR)FTM_MEM_malloc(sizeof(FTDM_ACTION));
		if (pAction == NULL)
		{
			ERROR("Not enough memory!\n");
			FTDM_DBIF_destroyAction(xFTDM.pDBIF, pInfo->pID);
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}

		memset(pAction, 0, sizeof(FTDM_ACTION));
		memcpy(&pAction->xInfo, pInfo, sizeof(FTM_ACTION));
		pAction->pDBIF = xFTDM.pDBIF;

		xRet = FTM_ACTION_append((FTM_ACTION_PTR)pAction);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Action[%s] append failed.\n", pAction->xInfo.pID);
			FTDM_DBIF_destroyAction(xFTDM.pDBIF, pInfo->pID);
			FTM_MEM_free(pAction);
		}
		else
		{
			if (ppAction != NULL)
			{
				*ppAction = pAction;
			}
		}
	}

	return	xRet;
}

FTM_RET	FTDM_destroyAction
(
	FTM_CHAR_PTR	pActionID
)
{
	FTM_RET				xRet;
	FTDM_ACTION_PTR	pAction = NULL;

	xRet = FTDM_ACTION_get(pActionID, &pAction);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTDM_DBIF_destroyAction(xFTDM.pDBIF, pActionID);
	FTM_ACTION_remove((FTM_ACTION_PTR)pAction);

	FTM_MEM_free(pAction);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_loadActionConfig
(
	FTM_CONFIG_PTR		pConfig
)
{
	ASSERT(pConfig != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_ITEM		xAction;
	FTM_CONFIG_ITEM		xActions;
	FTM_CONFIG_ITEM		xActionItem;
	FTM_CONFIG_ITEM		xParamsItem;

	xRet = FTM_CONFIG_getItem(pConfig, "event", &xAction);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_getChildItem(&xAction, "actions", &xActions);
		if (xRet == FTM_RET_OK)
		{
			FTM_ULONG	ulCount;

			xRet = FTM_CONFIG_LIST_getItemCount(&xActions, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				FTM_ULONG	i;

				for(i = 0 ; i < ulCount ; i++)
				{
					xRet = FTM_CONFIG_LIST_getItemAt(&xActions, i, &xActionItem);	
					if (xRet == FTM_RET_OK)
					{
						FTM_ACTION		xInfo;
						FTDM_ACTION_PTR	pAction = NULL;
						FTM_ULONG		ulIndex = 0;

						FTM_ACTION_setDefault(&xInfo);

						FTM_CONFIG_ITEM_getItemString(&xActionItem, "id", xInfo.pID, FTM_ID_LEN);
						FTM_CONFIG_ITEM_getItemString(&xActionItem, "name", xInfo.pName, FTM_NAME_LEN);
						FTM_CONFIG_ITEM_getItemULONG(&xActionItem, "index", &ulIndex);
						xRet = FTM_CONFIG_ITEM_getItemINT(&xActionItem, "type", (FTM_INT_PTR)&xInfo.xType);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}

						switch(xInfo.xType)
						{
						case	FTM_ACTION_TYPE_SET:
							{
								xRet = FTM_CONFIG_ITEM_getChildItem(&xActionItem, "params", &xParamsItem);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}
	
								xRet = FTM_CONFIG_ITEM_getItemString(&xParamsItem, "epid", xInfo.xParams.xSet.pEPID, FTM_EPID_LEN);
								if (xRet != FTM_RET_OK)
								{
									ERROR("EPID invalid!\n");
										continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemValue(&xParamsItem, "value", &xInfo.xParams.xSet.xValue);
								if (xRet != FTM_RET_OK)
								{
									ERROR("Action value invalid!\n");
										continue;
								}

								xRet = FTDM_createAction(&xInfo, &pAction);
								if (xRet != FTM_RET_OK)
								{
										ERROR("The new event can not creation.\n");
								}
								else if (ulIndex != 0)
								{
									pAction->ulIndex = ulIndex;	
								}
								//FTDM_LOG_createAction(xInfo.pID, xRet);
							}
							break;

						default:
							{
								ERROR("ACtion type[%08x] is not supported.\n", xInfo.xType);
								continue;
							}
						}
					
					}
				}
			}
		}
	}
	else
	{
		TRACE("Section [action] not found.\n");
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_loadActionFromFile
(
	FTM_CHAR_PTR			pFileName
)
{
	ASSERT(pFileName != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_PTR		pConfig;

	xRet = FTM_CONFIG_create(pFileName, &pConfig, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_CONFIG_LOAD_FAILED;
	}

	xRet = FTDM_loadActionConfig(pConfig);

	FTM_CONFIG_destroy(&pConfig);

	return	xRet;
}

FTM_RET	FTDM_loadActionFromDB
(
	FTM_VOID
)
{
	FTM_ULONG	nMaxCount = 0;

	if ((FTDM_DBIF_getActionCount(xFTDM.pDBIF, &nMaxCount) == FTM_RET_OK) &&
		(nMaxCount > 0))
	{

		FTM_ACTION_PTR	pActions;
		FTM_ULONG		nActionCount = 0;
		
		pActions = (FTM_ACTION_PTR)FTM_MEM_malloc(nMaxCount * sizeof(FTM_ACTION));
		if (pActions == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}
	
		if (FTDM_DBIF_getActionList(xFTDM.pDBIF, pActions, nMaxCount, &nActionCount) == FTM_RET_OK)
		{
			FTM_INT	i;
			FTDM_ACTION_PTR	pAction;

			for(i = 0 ; i < nActionCount ; i++)
			{
				FTDM_createAction(&pActions[i], &pAction);
			}
		}

		FTM_MEM_free(pActions);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_saveActionToDB
(
	FTM_VOID
)
{
	FTM_RET			i, xRet;
	FTM_ULONG		ulCount;
	FTM_ACTION_PTR	pAction;
	
	xRet = FTM_ACTION_count(&ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		xRet = FTM_ACTION_getAt(i, &pAction);
		if (xRet == FTM_RET_OK)
		{
			FTM_ACTION	xInfo;

			xRet = FTDM_DBIF_getAction(xFTDM.pDBIF, pAction->pID, &xInfo);
			if (xRet != FTM_RET_OK)
			{
				xRet = FTDM_DBIF_createAction(xFTDM.pDBIF, &xInfo);	
				if (xRet != FTM_RET_OK)
				{
					ERROR("Failed to save the new action.[%08x]\n", xRet);
				}
			}
		}
		else
		{
			ERROR("Failed to get action information[%08x]\n", xRet);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_createRule
(
	FTM_RULE_PTR 	pInfo,
	FTDM_RULE_PTR _PTR_ ppRule
)
{
	ASSERT(pInfo != NULL);
	FTM_RET			xRet;
	FTDM_RULE_PTR	pRule;

	if (FTDM_RULE_get(pInfo->pID, &pRule) == FTM_RET_OK)
	{
		ERROR2(FTM_RET_ALREADY_EXIST_OBJECT, "Rule[%s] already exist.\n", pInfo->pID);
		return	FTM_RET_ALREADY_EXIST_OBJECT;
	}

	if (strlen(pInfo->pID) == 0)
	{
		do 
		{	
			struct timeval	xTime;

			gettimeofday(&xTime, NULL);
			sprintf(pInfo->pID, "%08lx%08lx", (FTM_ULONG)xTime.tv_sec, (FTM_ULONG)xTime.tv_usec);
			usleep(10);
		}
		while (FTDM_RULE_get(pInfo->pID, &pRule) == FTM_RET_OK);
	}

	xRet = FTDM_DBIF_createRule(xFTDM.pDBIF, pInfo);
	if (xRet == FTM_RET_OK)
	{
		pRule = (FTDM_RULE_PTR)FTM_MEM_malloc(sizeof(FTDM_RULE));
		if (pRule == NULL)
		{
			ERROR2(FTM_RET_NOT_ENOUGH_MEMORY, "Not enough memory!\n");
			FTDM_DBIF_destroyRule(xFTDM.pDBIF, pInfo->pID);
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}

		memset(pRule, 0, sizeof(FTDM_RULE));
		memcpy(&pRule->xInfo, pInfo, sizeof(FTM_RULE));
		pRule->pDBIF = xFTDM.pDBIF;

		xRet = FTM_RULE_append((FTM_RULE_PTR)pRule);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Rule[%s] append failed.\n", pRule->xInfo.pID);
			FTDM_DBIF_destroyRule(xFTDM.pDBIF, pInfo->pID);
			FTM_MEM_free(pRule);
		}
		else
		{
			if (ppRule != NULL)
			{
				*ppRule = pRule;	
			}
		}
	}
	else
	{
		ERROR2(xRet, "Rule[%s] DB append failed.\n", pInfo->pID);	
	}

	return	xRet;
}

FTM_RET	FTDM_destroyRule
(
	FTM_CHAR_PTR	pRuleID
)
{
	FTM_RET				xRet;
	FTDM_RULE_PTR	pRule = NULL;

	xRet = FTDM_RULE_get(pRuleID, &pRule);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTDM_DBIF_destroyRule(xFTDM.pDBIF, pRuleID);
	FTM_RULE_remove((FTM_RULE_PTR)pRule);

	FTM_MEM_free(pRule);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_loadRuleConfig
(
	FTM_CONFIG_PTR		pConfig
)
{
	ASSERT(pConfig != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_ITEM		xRule;
	FTM_CONFIG_ITEM		xRules;
	FTM_CONFIG_ITEM		xRuleItem;

	xRet = FTM_CONFIG_getItem(pConfig, "event", &xRule);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_getChildItem(&xRule, "rules", &xRules);
		if (xRet == FTM_RET_OK)
		{
			FTM_ULONG	ulCount;

			xRet = FTM_CONFIG_LIST_getItemCount(&xRules, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				FTM_ULONG	i;

				for(i = 0 ; i < ulCount ; i++)
				{
					xRet = FTM_CONFIG_LIST_getItemAt(&xRules, i, &xRuleItem);	
					if (xRet == FTM_RET_OK)
					{
						FTM_RULE		xInfo;
						FTM_CONFIG_ITEM	xRulesItem;
						FTM_CONFIG_ITEM	xActionsItem;

						FTM_RULE_setDefault(&xInfo);

						FTM_CONFIG_ITEM_getItemString(&xRuleItem, "id", xInfo.pID, FTM_ID_LEN);
						FTM_CONFIG_ITEM_getItemString(&xRuleItem, "name", xInfo.pName, FTM_NAME_LEN);
						xInfo.xParams.ulTriggers = 0;

						xRet = FTM_CONFIG_ITEM_getChildItem(&xRuleItem, "triggers", &xRulesItem);
						if (xRet != FTM_RET_OK)
						{
							ERROR2(xRet, "Failed to get trigger items!\n");
							continue;
						}
				
						FTM_ULONG	j, ulRuleCount;

						xRet = FTM_CONFIG_LIST_getItemCount(&xRulesItem, &ulRuleCount);
						if (xRet != FTM_RET_OK)
						{
							ERROR2(xRet, "Failed to get trigger count!\n");
							continue;
						}
				
						for(j = 0 ; j < ulRuleCount ; j++)
						{
							FTM_CONFIG_ITEM	xRuleItem;

							xRet = FTM_CONFIG_LIST_getItemAt(&xRulesItem, j, &xRuleItem);	
							if (xRet == FTM_RET_OK)
							{
								FTM_ULONG	ulIndex;
								FTDM_TRIGGER_PTR	pTrigger;

								xRet = FTM_CONFIG_ITEM_getULONG(&xRuleItem, &ulIndex);
								if (xRet != FTM_RET_OK)
								{
									ERROR2(xRet, "Failed to get rule index!\n");
									continue;	
								}

								xRet = FTDM_TRIGGER_getByIndex(ulIndex, &pTrigger);
								if (xRet != FTM_RET_OK)
								{
									ERROR2(xRet, "Failed to get tirgger at %lu!\n", ulIndex);
									continue;	
								}

								strcpy(xInfo.xParams.pTriggers[xInfo.xParams.ulTriggers++], pTrigger->xInfo.pID);
							}
							else
							{
								ERROR2(xRet, "Get rule[%d] load failed.\n", j);	
							}
						
						}

						xInfo.xParams.ulActions = 0;
						xRet = FTM_CONFIG_ITEM_getChildItem(&xRuleItem, "actions", &xActionsItem);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}
						FTM_ULONG	ulActionCount;

						xRet = FTM_CONFIG_LIST_getItemCount(&xActionsItem, &ulActionCount);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}
			
						for(j = 0 ; j < ulActionCount ; j++)
						{
							FTM_CONFIG_ITEM	xActionItem;

							xRet = FTM_CONFIG_LIST_getItemAt(&xActionsItem, j, &xActionItem);	
							if (xRet == FTM_RET_OK)
							{
								FTM_ULONG	ulIndex;
								FTDM_ACTION_PTR	pAction;

								xRet = FTM_CONFIG_ITEM_getULONG(&xActionItem, &ulIndex);
								if (xRet != FTM_RET_OK)
								{
									ERROR2(xRet, "Action index get failed.\n");
									continue;	
								}

								xRet = FTDM_ACTION_getByIndex(ulIndex, &pAction);
								if (xRet != FTM_RET_OK)
								{
									ERROR2(xRet, "Action[%d] get failed.\n");
									continue;	
								}

								strcpy(xInfo.xParams.pActions[xInfo.xParams.ulActions++], pAction->xInfo.pID);
							}
						
						}

						xRet = FTDM_createRule(&xInfo, NULL);
						//FTDM_LOG_createRule(xInfo.pID, xRet);
					
					}
				}
			}
		}
	}
	else
	{
		TRACE("Section [action] not found.\n");
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_loadRuleFromDB
(
	FTM_VOID
)
{
	FTM_ULONG	nMaxCount = 0;

	if ((FTDM_DBIF_getRuleCount(xFTDM.pDBIF, &nMaxCount) == FTM_RET_OK) &&
		(nMaxCount > 0))
	{

		FTM_RULE_PTR	pRules;
		FTM_ULONG		nRuleCount = 0;
		
		pRules = (FTM_RULE_PTR)FTM_MEM_malloc(nMaxCount * sizeof(FTM_RULE));
		if (pRules == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}
	
		if (FTDM_DBIF_getRuleList(xFTDM.pDBIF, pRules, nMaxCount, &nRuleCount) == FTM_RET_OK)
		{
			FTM_INT	i;

			for(i = 0 ; i < nRuleCount ; i++)
			{
				FTM_RULE_create(&pRules[i]);
			}
		}

		FTM_MEM_free(pRules);
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_saveRuleToDB
(	
	FTM_VOID
)
{
	FTM_RET			i, xRet;
	FTM_ULONG		ulCount;
	FTM_RULE_PTR	pRule;
	
	xRet = FTM_RULE_count(&ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		xRet = FTM_RULE_getAt(i, &pRule);
		if (xRet == FTM_RET_OK)
		{
			FTM_RULE	xInfo;

			xRet = FTDM_DBIF_getRule(xFTDM.pDBIF, pRule->pID, &xInfo);
			if (xRet != FTM_RET_OK)
			{
				xRet = FTDM_DBIF_createRule(xFTDM.pDBIF, &xInfo);	
				if (xRet != FTM_RET_OK)
				{
					ERROR2(xRet, "Failed to save the new trigger.[%08x]\n", xRet);
				}
			}
		}
		else
		{
			ERROR2(xRet, "Failed to get trigger information[%08x]\n", xRet);
		}
	}

	return	FTM_RET_OK;
}

/*************************************************************************
 *
 *************************************************************************/
FTM_RET	FTDM_createTrigger
(
	FTM_TRIGGER_PTR 	pInfo,
	FTDM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	ASSERT(pInfo != NULL);
	FTM_RET				xRet;
	FTDM_TRIGGER_PTR	pTrigger;

	if (FTDM_TRIGGER_get(pInfo->pID, &pTrigger) == FTM_RET_OK)
	{
		ERROR("Object already exist!\n");
		return	FTM_RET_ALREADY_EXIST_OBJECT;
	}

	if (strlen(pInfo->pID) == 0)
	{
		do 
		{	
			struct timeval	xTime;

			gettimeofday(&xTime, NULL);
			sprintf(pInfo->pID, "%08lx%08lx", (FTM_ULONG)xTime.tv_sec, (FTM_ULONG)xTime.tv_usec);
			usleep(10);
		}
		while (FTDM_TRIGGER_get(pInfo->pID, &pTrigger) == FTM_RET_OK);
	}

	xRet = FTDM_DBIF_createTrigger(xFTDM.pDBIF, pInfo);
	if (xRet == FTM_RET_OK)
	{
		pTrigger = (FTDM_TRIGGER_PTR)FTM_MEM_malloc(sizeof(FTDM_TRIGGER));
		if (pTrigger == NULL)
		{
			ERROR("Not enough memory!\n");
			FTDM_DBIF_destroyTrigger(xFTDM.pDBIF, pInfo->pID);
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}

		memset(pTrigger, 0, sizeof(FTDM_TRIGGER));
		memcpy(&pTrigger->xInfo, pInfo, sizeof(FTM_TRIGGER));

		xRet = FTM_TRIGGER_append((FTM_TRIGGER_PTR)pTrigger);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Trigger[%s] append failed\n", pTrigger->xInfo.pID);
			FTDM_DBIF_destroyTrigger(xFTDM.pDBIF, pInfo->pID);
		}
	}
	else
	{
		ERROR("Trigger[%s] DB creation failed.\n", pInfo->pID);
	}

	if ((xRet == FTM_RET_OK) && (ppTrigger != NULL))
	{
		*ppTrigger = pTrigger;	
	}
	
	return	xRet;
}

FTM_RET	FTDM_destroyTrigger
(
	FTM_CHAR_PTR	pID
)
{
	FTM_RET				xRet;
	FTDM_TRIGGER_PTR	pTrigger = NULL;

	xRet = FTDM_TRIGGER_get(pID, &pTrigger);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	FTDM_DBIF_destroyTrigger(xFTDM.pDBIF, pID);
	FTM_TRIGGER_remove((FTM_TRIGGER_PTR)pTrigger);

	FTM_MEM_free(pTrigger);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_loadTriggerConfig
(
	FTM_CONFIG_PTR	pConfig
)
{
	ASSERT(pConfig != NULL);

	FTM_RET				xRet;
	FTM_CONFIG_ITEM		xTrigger;
	FTM_CONFIG_ITEM		xTriggers;
	FTM_CONFIG_ITEM		xTriggerItem;

	xRet = FTM_CONFIG_getItem(pConfig, "event", &xTrigger);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_CONFIG_ITEM_getChildItem(&xTrigger, "triggers", &xTriggers);
		if (xRet == FTM_RET_OK)
		{
			FTM_ULONG	ulCount;

			xRet = FTM_CONFIG_LIST_getItemCount(&xTriggers, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				FTM_ULONG	i;
				FTDM_TRIGGER_PTR	pTrigger;

				for(i = 0 ; i < ulCount ; i++)
				{
					xRet = FTM_CONFIG_LIST_getItemAt(&xTriggers, i, &xTriggerItem);	
					if (xRet == FTM_RET_OK)
					{
						FTM_TRIGGER		xInfo;
						FTM_ULONG		ulIndex = 0;
						FTM_CHAR		pTypeString[64];

						FTM_TRIGGER_setDefault(&xInfo);

						FTM_CONFIG_ITEM_getItemString(&xTriggerItem, "id", xInfo.pID, FTM_ID_LEN);
						FTM_CONFIG_ITEM_getItemString(&xTriggerItem, "name", xInfo.pName, FTM_NAME_LEN);
						FTM_CONFIG_ITEM_getItemULONG(&xTriggerItem, "index", &ulIndex);
						
						xRet = FTM_CONFIG_ITEM_getItemString(&xTriggerItem, "epid", xInfo.pEPID, FTM_EPID_LEN);
						if (xRet != FTM_RET_OK)
						{
							TRACE("Trigger epid get failed.\n");
							continue;
						}

						xRet = FTM_CONFIG_ITEM_getItemString(&xTriggerItem, "type", pTypeString, sizeof(pTypeString) - 1);
						if (xRet != FTM_RET_OK)
						{
							TRACE("Trigger type get failed.\n");
							continue;
						}

						xRet = FTM_TRIGGER_strToType(pTypeString, &xInfo.xType);
						if (xRet != FTM_RET_OK)
						{
							TRACE("Trigger type[%s] is invalid.\n", pTypeString);
							continue;
						}

						switch(xInfo.xType)
						{
						case	FTM_TRIGGER_TYPE_ABOVE:
							{
								xRet = FTM_CONFIG_ITEM_getItemULONG(&xTriggerItem, "detect",&xInfo.xParams.xAbove.ulDetectionTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemULONG(&xTriggerItem, "hold",&xInfo.xParams.xAbove.ulHoldingTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemValue(&xTriggerItem, "value", &xInfo.xParams.xAbove.xValue);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

							}
							break;

						case	FTM_TRIGGER_TYPE_BELOW:
							{
								xRet = FTM_CONFIG_ITEM_getItemULONG(&xTriggerItem, "detect",&xInfo.xParams.xBelow.ulDetectionTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemULONG(&xTriggerItem, "hold",&xInfo.xParams.xBelow.ulHoldingTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemValue(&xTriggerItem, "value", &xInfo.xParams.xBelow.xValue);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

							}
							break;

						case	FTM_TRIGGER_TYPE_INCLUDE:
							{
								xRet = FTM_CONFIG_ITEM_getItemULONG(&xTriggerItem, "detect",&xInfo.xParams.xInclude.ulDetectionTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemULONG(&xTriggerItem, "hold",&xInfo.xParams.xInclude.ulHoldingTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemValue(&xTriggerItem, "upper", &xInfo.xParams.xInclude.xUpper);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemValue(&xTriggerItem, "lower", &xInfo.xParams.xInclude.xLower);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

							}
							break;

						case	FTM_TRIGGER_TYPE_EXCEPT:
							{
								xRet = FTM_CONFIG_ITEM_getItemULONG(&xTriggerItem, "detect",&xInfo.xParams.xExcept.ulDetectionTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemULONG(&xTriggerItem, "hold",&xInfo.xParams.xExcept.ulHoldingTime);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemValue(&xTriggerItem, "upper", &xInfo.xParams.xExcept.xUpper);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

								xRet = FTM_CONFIG_ITEM_getItemValue(&xTriggerItem, "lower", &xInfo.xParams.xExcept.xLower);
								if (xRet != FTM_RET_OK)
								{
									continue;
								}

							}
							break;

						case	FTM_TRIGGER_TYPE_CHANGE:
						default:
							{
							}
							break;
						}

						xRet = FTDM_createTrigger(&xInfo, &pTrigger);
						if (xRet != FTM_RET_OK)
						{
							ERROR("The new trigger can not creation.\n");
						}
						else if (ulIndex != 0)
						{
							pTrigger->ulIndex = ulIndex;	
						}
						//FTDM_LOG_createTrigger(xInfo.pID, xRet);
					}
				}
			}
		}
		else
		{
			TRACE("Triggers not found.\n");	
		}
	}
	else
	{
		TRACE("Section [trigger] not found.\n");
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_loadTriggerFromDB
(
	FTM_VOID
)
{

	FTM_RET		xRet;
	FTM_ULONG	nMaxCount = 0;

	if ((FTDM_DBIF_getTriggerCount(xFTDM.pDBIF, &nMaxCount) == FTM_RET_OK) &&
		(nMaxCount > 0))
	{

		FTM_TRIGGER_PTR	pInfos;
		FTM_ULONG		nTriggerCount = 0;
		
		pInfos = (FTM_TRIGGER_PTR)FTM_MEM_malloc(nMaxCount * sizeof(FTM_TRIGGER));
		if (pInfos == NULL)
		{
			return	FTM_RET_NOT_ENOUGH_MEMORY;	
		}
	
		if (FTDM_DBIF_getTriggerList(xFTDM.pDBIF, pInfos, nMaxCount, &nTriggerCount) == FTM_RET_OK)
		{
			FTM_INT	i;

			for(i = 0 ; i < nTriggerCount ; i++)
			{
				FTDM_TRIGGER_PTR	pTrigger;

				pTrigger = (FTDM_TRIGGER_PTR)FTM_MEM_malloc(sizeof(FTDM_TRIGGER));
				if (pTrigger == NULL)
				{
					ERROR("Not enough memory!\n");
					break;	
				}

				memcpy(&pTrigger->xInfo, &pInfos[i], sizeof(FTM_TRIGGER));

				xRet = FTM_TRIGGER_append((FTM_TRIGGER_PTR)pTrigger);
				if (xRet != FTM_RET_OK)
				{
					FTM_MEM_free(pTrigger);	
				}
			}
		}

		FTM_MEM_free(pInfos);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_saveTriggerToDB
(
	FTM_VOID
)
{
	FTM_RET			i, xRet;
	FTM_ULONG		ulCount;
	FTM_TRIGGER_PTR	pTrigger;
	
	xRet = FTM_TRIGGER_count(&ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_TRIGGER	xInfo;

		xRet = FTM_TRIGGER_getAt(i, &pTrigger);
		if (xRet != FTM_RET_OK)
		{
			ERROR("Failed to get trigger information[%08x]\n", xRet);
			continue;
		}
		

		xRet = FTDM_DBIF_getTrigger(xFTDM.pDBIF, pTrigger->pID, &xInfo);
		if (xRet != FTM_RET_OK)
		{
			xRet = FTDM_DBIF_createTrigger(xFTDM.pDBIF, pTrigger);	
			if (xRet != FTM_RET_OK)
			{
				ERROR("Failed to save the new trigger.[%08x]\n", xRet);
			}
		}
	}

	return	FTM_RET_OK;
}

