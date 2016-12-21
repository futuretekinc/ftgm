#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include "ftdm.h"
#include "ftm_trigger.h"
#include "ftdm_sqlite.h"

#undef	__MODULE__
#define	__MODULE__	FTDM_TRACE_MODULE_DBIF

static FTM_BOOL FTDM_DBIF_EP_DATA_bIOTrace = FTM_FALSE;
static int	_FTDM_DBIF_EP_getListCB(void *pData, int nArgc, char **pArgv, char **pColName);
static int	_FTDM_DBIF_EP_getCB(void *pData, int nArgc, char **pArgv, char **pColName);
static int	_FTDM_DBIF_EP_DATA_timeCB(void *pData, int nArgc, char **pArgv, char **pColName);
static int	_FTDM_DBIF_EP_DATA_countCB(void *pData, int nArgc, char **pArgv, char **pColName);
static int _FTDM_DBIF_EP_DATA_getCB(void *pData, int nArgc, char **pArgv, char **pColName);
static int _FTDM_DBIF_CB_isExistTable(void *pData, int nArgc, char **pArgv, char **pColName);
static int _FTDM_DBIF_CB_isExist(void *pData, int nArgc, char **pArgv, char **pColName);
static int _FTDM_DBIF_TRIGGER_getCB(void *pData, int nArgc, char **pArgv, char **pColName);
static int _FTDM_DBIF_TRIGGER_getListCB(void *pData, int nArgc, char **pArgv, char **pColName);
static int _FTDM_DBIF_ACTION_getCB(void *pData, int nArgc, char **pArgv, char **pColName);
static int _FTDM_DBIF_ACTION_getListCB(void *pData, int nArgc, char **pArgv, char **pColName);
static int _FTDM_DBIF_RULE_getCB(void *pData, int nArgc, char **pArgv, char **pColName);
static int _FTDM_DBIF_RULE_getListCB(void *pData, int nArgc, char **pArgv, char **pColName);

static 
FTM_INT	FTDM_DBIF_countCB
(
	FTM_VOID _PTR_ 		pData, 
	FTM_INT				nArgc, 
	FTM_CHAR_PTR _PTR_ 	ppArgv, 
	FTM_CHAR_PTR _PTR_ 	ppColName
);

static int _FTDM_DBIF_LOG_getCB(void *pData, int nArgc, char **pArgv, char **pColName);

static 
FTM_RET	FTDM_DBIF_count
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName,
	FTM_ULONG_PTR	pulCount
);

static
FTM_RET	FTDM_DBIF_countWithTime
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName,
	FTM_ULONG		xBeginTime,
	FTM_ULONG		xEndTime,
	FTM_ULONG_PTR	pulCount
);

FTM_RET _FTDM_DBIF_isExistTable
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName, 
	FTM_BOOL_PTR 	pExist
);

FTM_RET _FTDM_DBIF_NODE_createTable
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName
);

FTM_RET _FTDM_DBIF_NODE_isExist
(	
	FTM_CHAR_PTR	pDID,
	FTM_BOOL_PTR 	pExist
);

FTM_RET _FTDM_DBIF_EP_createTable
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName
);

FTM_RET _FTDM_DBIF_TRIGGER_createTable
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName
);

FTM_RET _FTDM_DBIF_ACTION_createTable
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName
);

FTM_RET _FTDM_DBIF_RULE_createTable
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName
);

static FTM_CHAR	_strDefaultDBName[FTM_FILE_NAME_LEN+1] = "./ftdm.db";

FTM_RET	FTDM_DBIF_init
(
	FTDM_DBIF_PTR	pDBIF
)
{
	ASSERT(pDBIF != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_final
(
	FTDM_DBIF_PTR	pDBIF
)
{
	ASSERT(pDBIF != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_open
(
	FTDM_DBIF_PTR	pDBIF
)
{
	ASSERT(pDBIF != NULL);

	FTM_RET	xRet;
	
	if (pDBIF->pSQLiteDB != NULL)
	{
		return	FTM_RET_ALREADY_INITIALIZED;	
	}

	xRet = sqlite3_open(_strDefaultDBName, &pDBIF->pSQLiteDB);
	if ( xRet )
	{
		ERROR2(xRet, "%s\n", sqlite3_errmsg(pDBIF->pSQLiteDB));

		return	(FTM_RET_DBIF_DB_ERROR | xRet); 	
	}

	xRet = FTDM_DBIF_NODE_initTable();
	if (xRet != FTM_RET_OK)
	{
		sqlite3_close(pDBIF->pSQLiteDB);
		return	xRet;	
	}

	xRet = FTDM_DBIF_EP_initTable();
	if (xRet != FTM_RET_OK)
	{
		sqlite3_close(pDBIF->pSQLiteDB);
		return	xRet;	
	}

	xRet = FTDM_DBIF_TRIGGER_initTable();
	if (xRet != FTM_RET_OK)
	{
		sqlite3_close(pDBIF->pSQLiteDB);
		return	xRet;	
	}

	xRet = FTDM_DBIF_ACTION_initTable();
	if (xRet != FTM_RET_OK)
	{
		sqlite3_close(pDBIF->pSQLiteDB);
		return	xRet;	
	}

	xRet = FTDM_DBIF_RULE_initTable();
	if (xRet != FTM_RET_OK)
	{
		sqlite3_close(pDBIF->pSQLiteDB);
		return	xRet;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_close
(
	FTDM_DBIF_PTR	pDBIF
)
{
	ASSERT(pDBIF != NULL);

	if (pDBIF->pSQLiteDB)
	{
		sqlite3_close(pDBIF->pSQLiteDB);
		pDBIF->pSQLiteDB = NULL;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_loadConfig
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CONFIG_PTR	pConfig
)
{
	ASSERT(pDBIF != NULL);
	ASSERT(pConfig != NULL);
	FTM_RET	xRet;
	FTM_CONFIG_ITEM	xSection;

	xRet = FTM_CONFIG_getItem(pConfig, "database", &xSection);
	if (xRet == FTM_RET_OK)
	{
		FTM_CHAR	pDBFileName[FTM_FILE_NAME_LEN+1];

		xRet = FTM_CONFIG_ITEM_getItemString(&xSection, "file", pDBFileName, sizeof(pDBFileName));
		if (xRet == FTM_RET_OK)
		{
			strncpy(_strDefaultDBName, pDBFileName, sizeof(_strDefaultDBName) - 1);
		}
	}

	return	FTM_RET_OK;
}

/*************************************************************************
 *	Item Count
 *************************************************************************/
typedef	struct
{
	FTM_ULONG	ulCount;
}	FTDM_DBIF_COUNT_PARAMS, _PTR_ FTDM_DBIF_COUNT_PARAMS_PTR;

FTM_INT FTDM_DBIF_countCB
(
	FTM_VOID_PTR 	pData, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR _PTR_ pArgv, 
	FTM_CHAR_PTR _PTR_ pColName
)
{
	FTDM_DBIF_COUNT_PARAMS_PTR pParams = (FTDM_DBIF_COUNT_PARAMS_PTR)pData;

	if (nArgc != 0)
	{
		pParams->ulCount = atoi(pArgv[0]);
	}

	return	0;
}

FTM_RET	FTDM_DBIF_countWithTime
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName,
	FTM_ULONG		xBeginTime,
	FTM_ULONG		xEndTime,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pDBIF != NULL);
	ASSERT(pTableName != NULL);
	ASSERT(pulCount != NULL);

    int     xRet;
	FTM_INT	nSQLLen = 0;
    char    pSQL[1024];
    char    *pErrMsg = NULL;
	FTDM_DBIF_COUNT_PARAMS	xParams;

   	nSQLLen = sprintf(pSQL, "SELECT COUNT(*) FROM %s", pTableName);

	if ((xBeginTime != 0) || (xEndTime != 0))
	{
		nSQLLen += 	sprintf(&pSQL[nSQLLen], " WHERE");
	}

	if (xBeginTime != 0)
	{
		nSQLLen += 	sprintf(&pSQL[nSQLLen], " (%lu <= TIME)", xBeginTime);
	}


	if ((xBeginTime != 0) && (xEndTime != 0))
	{
   		nSQLLen += sprintf(&pSQL[nSQLLen], " AND");

	}

	if (xEndTime != 0)
	{
    	nSQLLen += sprintf(&pSQL[nSQLLen], " (TIME <= %lu)", xEndTime);
	}

	memset(&xParams, 0, sizeof(xParams));

    xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, FTDM_DBIF_countCB, &xParams, &pErrMsg);
    if (xRet != SQLITE_OK)
    {
        sqlite3_free(pErrMsg);
		*pulCount = 0;
    }
	else
	{
		*pulCount = xParams.ulCount;
	}

	return	FTM_RET_OK;
}

/*********************************************************************
 * get table information
 *********************************************************************/
typedef	struct
{
	FTM_ULONG	ulTime;
}	FTDM_DBIF_GET_TIME_PARAMS, _PTR_ FTDM_DBIF_GET_TIME_PARAMS_PTR;


FTM_INT	_FTDM_DBIF_getTimeCB
(
	FTM_VOID _PTR_	pData, 
	FTM_INT			nArgc, 
	FTM_CHAR _PTR_ _PTR_ pArgv, 
	FTM_CHAR _PTR_ _PTR_ pColName
)
{
	FTDM_DBIF_GET_TIME_PARAMS_PTR pParams = (FTDM_DBIF_GET_TIME_PARAMS_PTR)pData;

	if (nArgc != 0)
	{
		if (strcmp(pColName[0], "TIME") == 0)
		{
			pParams->ulTime = strtoul(pArgv[0], 0, 10);
		}
    }

    return  0;
}

FTM_RET	FTDM_DBIF_getInfo
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName,
	FTM_ULONG_PTR	pulBeginTime,
	FTM_ULONG_PTR	pulEndTime,
	FTM_ULONG_PTR	pulCount
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_BOOL		bExist = FTM_FALSE;
	FTDM_DBIF_GET_TIME_PARAMS	xParams;

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	xRet = _FTDM_DBIF_isExistTable(pDBIF, pTableName, &bExist);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}
	else if (bExist != FTM_TRUE)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	sprintf(pSQL, "SELECT TIME from %s ORDER BY TIME DESC LIMIT 1", pTableName);
	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_getTimeCB, &xParams, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
        ERROR2(xRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}
	
	*pulBeginTime = xParams.ulTime;

	sprintf(pSQL, "SELECT TIME from %s  ORDER BY TIME ASC LIMIT 1", pTableName);
	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_getTimeCB, &xParams, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
        ERROR2(xRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	*pulEndTime = xParams.ulTime;

	return	FTM_RET_OK;
}

/*********************************************************************
 * get item 
 *********************************************************************/
FTM_RET	FTDM_DBIF_get
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName,
	FTM_ULONG		nStartIndex,
	FTM_ULONG		nMaxCount,
	FTM_INT 		(*fCB)(void *pData, int nArgc, char **pArgv, char **pColName),
	FTM_VOID_PTR	pData
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_INT			nSQLLen = 0;

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	if (nMaxCount == 0)
	{
		nMaxCount = 100;	
	}
	
	nSQLLen =  sprintf(pSQL,"SELECT * FROM %s ", pTableName);
	nSQLLen += sprintf(&pSQL[nSQLLen], " ORDER BY TIME DESC LIMIT %lu OFFSET %lu", nMaxCount, nStartIndex);

	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, fCB, pData, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
        ERROR2(xRet, "%s\n",pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

/*********************************************************************
 * get item based on time
 *********************************************************************/
FTM_RET	FTDM_DBIF_getWithTime
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName,
	FTM_ULONG		xBeginTime,
	FTM_ULONG		xEndTime,
	FTM_BOOL		bAscending,
	FTM_ULONG		nMaxCount,
	FTM_INT 		(*fCB)(void *pData, int nArgc, char **pArgv, char **pColName),
	FTM_VOID_PTR	pData
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_INT			nSQLLen = 0;

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	if (nMaxCount == 0)
	{
		nMaxCount = 100;	
	}

	nSQLLen =  sprintf(pSQL,"SELECT * FROM %s", pTableName);

	if ((xBeginTime != 0) || (xEndTime != 0))
	{
		nSQLLen += sprintf(&pSQL[nSQLLen], " WHERE");
	}

	if (xBeginTime != 0)
	{
		nSQLLen += sprintf(&pSQL[nSQLLen], " (%lu <= TIME)", xBeginTime);
	}

	if ((xBeginTime != 0) && (xEndTime != 0))
	{
		nSQLLen += sprintf(&pSQL[nSQLLen], " AND");
	}

	if (xEndTime != 0)
	{
		nSQLLen += sprintf(&pSQL[nSQLLen], " (TIME <= %lu)", xEndTime);
	}

	if (bAscending)
	{
		nSQLLen += sprintf(&pSQL[nSQLLen], " ORDER BY TIME LIMIT %lu", nMaxCount);
	}
	else
	{
		nSQLLen += sprintf(&pSQL[nSQLLen], " ORDER BY TIME DESC LIMIT %lu", nMaxCount);
	}

	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, fCB, pData, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
        ERROR2(xRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
static 
FTM_INT _FTDM_DBIF_TABLE_countCB
(
	FTM_VOID_PTR 	pData, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR _PTR_ pArgv, 
	FTM_CHAR_PTR _PTR_ pColName
)
{
	FTM_ULONG_PTR pnCount = (FTM_ULONG_PTR)pData;

	if (nArgc != 0)
	{
		*pnCount = atoi(pArgv[0]);
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_getTableCount
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_ULONG_PTR	pulCount
)
{
    FTM_INT			xRet;
    FTM_CHAR		pSQL[1024];
    FTM_CHAR_PTR	strErrMsg = NULL;

	if (pDBIF->pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

    sprintf(pSQL, "SELECT COUNT(*) FROM sqlite_master where type='table'");
    xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_TABLE_countCB, pulCount, &strErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR2(xRet, "%s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTM_RET_ERROR;
    }

	return	FTM_RET_OK;
}

/**********************************************************************************
 *
 **********************************************************************************/
typedef struct
{
	FTM_ULONG		ulMaxCount;
	FTM_ULONG		ulCount;
	FTM_CHAR_PTR _PTR_	pTables;
}   _FTDM_DBIF_CB_TABLE_LIST_PARAMS, *  _FTDM_DBIF_CB_TABLE_LIST_PARAMS_PTR;

FTM_INT	_FTDM_DBIF_CB_getTableList
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_VOID_PTR	pData, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR _PTR_	pArgv, 
	FTM_CHAR_PTR _PTR_ pColName
)
{
  	_FTDM_DBIF_CB_TABLE_LIST_PARAMS_PTR pParams = (_FTDM_DBIF_CB_TABLE_LIST_PARAMS_PTR)pData;

    if (nArgc != 0)
    {
		if (pParams->ulCount < pParams->ulMaxCount)
		{
			strncpy(pParams->pTables[pParams->ulCount++], pArgv[0], 63);
		}
    }

    return  0;
}

FTM_RET FTDM_DBIF_getTableList
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR _PTR_	pTables,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
    FTM_INT		nRet;
    _FTDM_DBIF_CB_TABLE_LIST_PARAMS	xParams = { .ulMaxCount = ulMaxCount, .ulCount = 0, .pTables = pTables };
    FTM_CHAR_PTR	pSQL = "select name from sqlite_master where type='table' order by name";
    FTM_CHAR_PTR	pErrMsg = NULL;

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

    nRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_CB_getTableList, &xParams, &pErrMsg);
    if (nRet != SQLITE_OK)
    {
        ERROR2(nRet, "%s\n", pErrMsg);
        sqlite3_free(pErrMsg);

        return  FTM_RET_ERROR;
    }

    *pulCount = xParams.ulCount;

    return  FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
FTM_RET	FTDM_DBIF_TABLE_deleteAllTables
(
	FTDM_DBIF_PTR	pDBIF
)
{
	FTM_RET		xRet;
	FTM_INT		i;
	FTM_ULONG	ulTableCount = 0;
	FTM_CHAR_PTR _PTR_ pTables;
	FTM_CHAR_PTR	pBuff;

	xRet = FTDM_DBIF_getTableCount(&ulTableCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (ulTableCount == 0)
	{
		return	FTM_RET_OK;	
	}

	pTables = (FTM_CHAR_PTR _PTR_)FTM_MEM_malloc(sizeof(FTM_CHAR_PTR) * ulTableCount);
	if (pTables == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pBuff = (FTM_CHAR_PTR)FTM_MEM_malloc(ulTableCount * 64);
	if (pBuff == NULL)
	{
		FTM_MEM_free(pTables);
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	for(i = 0 ; i < ulTableCount ; i++)
	{
		pTables[i] = &pBuff[i*64];	
	}

	xRet = FTDM_DBIF_getTableList(pTables, ulTableCount, &ulTableCount);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pTables);
		FTM_MEM_free(pBuff);
		return	xRet;	
	}

	for(i = 0 ; i < ulTableCount ; i++)
	{
		TRACE("Table[%s] delete \n", pTables[i]);
		xRet = FTDM_DBIF_deleteTable(pTables[i]);	
		if (xRet != FTM_RET_OK)
		{
			TRACE("Table[%s] delete failed.\n", pTables[i]);
		}
	}

	FTM_MEM_free(pTables);
	FTM_MEM_free(pBuff);

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
FTM_RET	FTDM_DBIF_deleteTable
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR		pTableName
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "DROP TABLE %s", pTableName);
	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
		ERROR2(xRet, "%s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
FTM_RET	FTDM_DBIF_NODE_initTable
(
	FTDM_DBIF_PTR	pDBIF
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pTableName = "node_info";
	FTM_BOOL		bExist = FTM_FALSE;

	xRet = _FTDM_DBIF_isExistTable(pTableName, &bExist);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to access DB.\n");  
		return	xRet;	
	}

	if (bExist != FTM_TRUE)
	{
		TRACE("%s table is not exist\n", pTableName);
		xRet = _FTDM_DBIF_NODE_createTable(pTableName);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Can't create a new table[%s]\n", pTableName);
			return	FTM_RET_DBIF_ERROR;	
		}
		TRACE("It created new NODE table[%s].\n", pTableName);
	}

	return	FTM_RET_OK;
}

FTM_BOOL	FTDM_DBIF_NODE_isTableExist
(
	FTM_VOID
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pTableName = "node_info";
	FTM_BOOL		bExist = FTM_FALSE;

	xRet = _FTDM_DBIF_isExistTable(pTableName, &bExist);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to access DB\n");  
		return	FTM_FALSE;	
	}

	return	bExist;
}
/***************************************************************
 *
 ***************************************************************/

FTM_RET	FTDM_DBIF_NODE_count
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_ULONG_PTR	pulCount
)
{
    FTM_INT			xRet;
    FTM_CHAR		pSQL[1024];
    FTM_CHAR_PTR	pErrMsg = NULL;
	FTDM_DBIF_COUNT_PARAMS	xParams;

	if (pDBIF->pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

    sprintf(pSQL, "SELECT COUNT(*) FROM node_info");
    xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, FTDM_DBIF_countCB, &xParams, &pErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR2(xRet, "%s\n", pErrMsg);
        sqlite3_free(pErrMsg);

    	return  FTM_RET_ERROR;
    }

	*pulCount = xParams.ulCount;

	return	FTM_RET_OK;
}

/***************************************************************
 * delete item based on time
 ***************************************************************/
FTM_RET	FTDM_DBIF_delWithTime
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName,
	FTM_ULONG		xBeginTime,
	FTM_ULONG		xEndTime,
	FTM_ULONG_PTR	pulDeletedCount
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_INT			nSQLLen = 0;
	FTM_ULONG		ulBeforeCount = 0;
	FTM_ULONG		ulAfterCount = 0;

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	xRet = FTDM_DBIF_count(pTableName, &ulBeforeCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	nSQLLen += sprintf(pSQL, "DELETE FROM %s", pTableName);
	if ((xBeginTime != 0) || (xEndTime != 0))
	{
		nSQLLen += sprintf(&pSQL[nSQLLen], " WHERE");
	}

	if (xBeginTime != 0)
	{
		nSQLLen += sprintf(&pSQL[nSQLLen], " (%lu <= TIME)", xBeginTime);
	}

	if ((xBeginTime != 0) && (xEndTime != 0))
	{
		nSQLLen += sprintf(&pSQL[nSQLLen], " AND");
	}

	if (xEndTime != 0)
	{
		nSQLLen += sprintf(&pSQL[nSQLLen], " (TIME <= %lu)", xEndTime);
	}

	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
        ERROR2(xRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	xRet = FTDM_DBIF_count(pTableName, &ulAfterCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	*pulDeletedCount = ulBeforeCount - ulAfterCount;

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
typedef struct
{
	FTM_ULONG			nMaxCount;
	FTM_ULONG_PTR		pulCount;
	FTM_NODE_PTR	pInfos;
}	FTDM_DBIF_CB_GET_DEVICE_LIST_PARAMS, _PTR_ FTDM_DBIF_CB_GET_DEVICE_LIST_PARAMS_PTR;

static 
FTM_INT	_FTDM_DBIF_NODE_getListCB
(
	FTM_VOID_PTR	pData, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR _PTR_	pArgv, 
	FTM_CHAR_PTR _PTR_ pColName
)
{
	FTDM_DBIF_CB_GET_DEVICE_LIST_PARAMS_PTR pParams = (FTDM_DBIF_CB_GET_DEVICE_LIST_PARAMS_PTR)pData;

	if (nArgc != 0)
	{
		FTM_INT	i;

		for(i = 0 ; i < nArgc ; i++)
		{
			if (strcmp(pColName[i], "VALUE") == 0)
			{
				memcpy(&pParams->pInfos[(*pParams->pulCount)], pArgv[i], sizeof(FTM_NODE));
				(*pParams->pulCount)++;
			}
		}
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_NODE_getList
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_NODE_PTR	pInfos, 
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pulCount
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[2048];
	FTDM_DBIF_CB_GET_DEVICE_LIST_PARAMS	xParams = 
	{
		.nMaxCount  = nMaxCount,
		.pulCount 	= pulCount,
		.pInfos		= pInfos
	};	

	if (pDBIF->pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "SELECT * FROM node_info");
	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_NODE_getListCB, &xParams, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
        ERROR2(xRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
FTM_RET	FTDM_DBIF_NODE_create
(
	FTDM_DBIF_PTR	pDBIF,
 	FTM_NODE_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);

	FTM_RET			xRet;
	FTM_INT			nRet;
	sqlite3_stmt 	*pStmt;
	FTM_CHAR		pSQL[1024];
	FTM_BOOL		bExist = FTM_FALSE;

	if (pDBIF->pSQLiteDB == NULL)
	{
		TRACE("DB is not initialize.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}
	
	xRet = _FTDM_DBIF_NODE_isExist(pInfo->pDID, &bExist);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to check node!\n");
		return	xRet;	
	}

	if (bExist == FTM_TRUE)
	{
		ERROR2(FTM_RET_ALREADY_EXISTS, "Node[%s] is already exist.\n", pInfo->pDID);
		return	FTM_RET_ALREADY_EXISTS;	
	}

	sprintf(pSQL, "INSERT INTO node_info (DID,VALUE) VALUES (?,?)");
	do 
	{
		nRet = sqlite3_prepare(pDBIF->pSQLiteDB, pSQL, -1, &pStmt, 0);
		if( nRet!=SQLITE_OK )
		{
			ERROR2(nRet, "SQLite3 prepare error!\n");
			return FTM_RET_ERROR;
		}

		pInfo->xFlags |= FTM_EP_FLAG_STATIC;

		sqlite3_bind_text(pStmt, 1, pInfo->pDID, strlen(pInfo->pDID), 0);
		sqlite3_bind_blob(pStmt, 2, pInfo, sizeof(FTM_NODE), SQLITE_STATIC);

		nRet = sqlite3_step(pStmt);
		ASSERT( nRet != SQLITE_ROW);

		nRet = sqlite3_finalize(pStmt);
	}  while (nRet == SQLITE_SCHEMA);

	return FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
FTM_RET	FTDM_DBIF_NODE_destroy
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR		pDID
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];

	TRACE("Remove node[%s] from DB.\n", pDID);
	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "DELETE FROM node_info WHERE DID = \'%s\'", pDID);
	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
        ERROR2(xRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
static 
FTM_INT	_FTDM_DBIF_NODE_getCB
(
	FTM_VOID_PTR	pData, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR _PTR_	pArgv, 
	FTM_CHAR_PTR _PTR_ pColName
)
{
	FTM_NODE_PTR pInfo = (FTM_NODE_PTR)pData;

	if (nArgc != 0)
	{
		if (strcmp(pColName[0], "VALUE") == 0)
		{
			memcpy(pInfo, pArgv[0], sizeof(FTM_NODE));
		}
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_NODE_get
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pDID, 
	FTM_NODE_PTR	pInfo
)
{
    FTM_INT			xRet;
    FTM_CHAR		pSQL[1024];
    FTM_CHAR_PTR	pErrMsg = NULL;

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

    sprintf(pSQL, "SELECT * FROM node_info WHERE DID = \'%s\'", pDID);
    xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_NODE_getCB, pInfo, &pErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR2(xRet, "%s\n", pErrMsg);
        sqlite3_free(pErrMsg);

    	return  FTM_RET_ERROR;
    }

	if (strcmp(pInfo->pDID, pDID) != 0)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_NODE_setURL
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR		pDID, 
	FTM_CHAR_PTR		pURL
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_NODE_getURL
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR		pDID, 
	FTM_CHAR_PTR		pBuff,
	FTM_ULONG			nBuffLen
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_NODE_setLocation
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR		pDID, 
	FTM_CHAR_PTR		pLocation
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_NODE_getLocation
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR		pDID, 
	FTM_CHAR_PTR		pBuff,
	FTM_ULONG			nBuffLen
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_initTable
(
	FTDM_DBIF_PTR	pDBIF
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pTableName = "ep_info";
	FTM_BOOL		bExist = FTM_FALSE;

	xRet = _FTDM_DBIF_isExistTable(pTableName, &bExist);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_DBIF_ERROR;	
	}

	if (bExist != FTM_TRUE)
	{
		TRACE("%s table is not exist.\n", pTableName);
		FTM_CHAR_PTR	pErrMsg = NULL;
		char			pSQL[1024];

		if (pDBIF->pSQLiteDB == NULL)
		{
			return	FTM_RET_NOT_INITIALIZED;	
		}

		sprintf(pSQL, "CREATE TABLE %s ("\
						"EPID	TEXT PRIMARY KEY,"\
						"VALUE	BLOB)" , pTableName);

		xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
		if (xRet != SQLITE_OK)
		{
        	ERROR2(xRet, "%s\n", pErrMsg);
			sqlite3_free(pErrMsg);

			return	FTM_RET_ERROR;
		}
	}

	return	FTM_RET_OK;
}


FTM_BOOL FTDM_DBIF_EP_isTableExist
(
	FTDM_DBIF_PTR	pDBIF
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pTableName = "ep_info";
	FTM_BOOL		bExist = FTM_FALSE;

	xRet = _FTDM_DBIF_isExistTable(pTableName, &bExist);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_FALSE;	
	}

	return	bExist;
}

FTM_RET	FTDM_DBIF_EP_append
(
	FTDM_DBIF_PTR	pDBIF,
 	FTM_EP_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);

	FTM_RET			xRet;
	FTM_INT			nRet;
	sqlite3_stmt 	*pStmt;
	FTM_CHAR		pSQL[1024];
	FTM_BOOL		bExist = FTM_FALSE;

	if (pDBIF->pSQLiteDB == NULL)
	{
		TRACE("DB is not initialize.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}
	
	xRet = FTDM_DBIF_EP_isExist(pInfo->pEPID, &bExist);
	if ( xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to append EP[%s]!\n", pInfo->pEPID);
		return	xRet;
	}

	if (bExist)
	{
		WARN("EP[%s] is already exist.\n", pInfo->pEPID);
		return	FTM_RET_ALREADY_EXISTS;	
	}

	sprintf(pSQL, "INSERT INTO ep_info (EPID,VALUE) VALUES (?,?)");
	do 
	{
		nRet = sqlite3_prepare(pDBIF->pSQLiteDB, pSQL, -1, &pStmt, 0);
		if( nRet!=SQLITE_OK )
		{
			return FTM_RET_ERROR;
		}

		pInfo->xFlags |= FTM_EP_FLAG_STATIC;

		sqlite3_bind_text(pStmt, 1, pInfo->pEPID, strlen(pInfo->pEPID), 0);
		sqlite3_bind_blob(pStmt, 2, pInfo, sizeof(FTM_EP), SQLITE_STATIC);

		nRet = sqlite3_step(pStmt);
		ASSERT( nRet != SQLITE_ROW);

		nRet = sqlite3_finalize(pStmt);
	}  while (nRet == SQLITE_SCHEMA);

	return FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_remove
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pEPID
)
{
	FTM_RET		xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "DELETE FROM ep_info WHERE EPID = \'%s\'", pEPID);
	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
        ERROR2(xRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
FTM_RET	FTDM_DBIF_EP_count
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_ULONG_PTR		pulCount
)
{
	return	FTDM_DBIF_count("ep_info", pulCount);
}

/***************************************************************
 *
 ***************************************************************/
static 
FTM_INT	_FTDM_DBIF_EP_countCB
(
	FTM_VOID_PTR	pData, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR _PTR_	pArgv, 
	FTM_CHAR_PTR _PTR_ pColName
)
{
	FTM_ULONG_PTR pnCount = (FTM_ULONG_PTR)pData;

	if (nArgc != 0)
	{
		*pnCount = atoi(pArgv[0]);
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_isExist
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pEPID,
	FTM_BOOL_PTR	pbExist
)
{
    FTM_INT			nRet;
    FTM_CHAR		pSQL[1024];
    FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_ULONG		ulCount = 0;

	if (pDBIF->pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	if (FTDM_DBIF_EP_isTableExist() == FTM_TRUE)
	{
    	sprintf(pSQL, "SELECT COUNT(*) FROM ep_info WHERE EPID = \'%s\'", pEPID);
    	nRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_EP_countCB, &ulCount, &pErrMsg);
    	if (nRet != SQLITE_OK)
    	{
        	ERROR2(nRet, "%s\n", pErrMsg);
        	sqlite3_free(pErrMsg);

    		return  FTM_RET_ERROR;
    	}
	}

	*pbExist = (ulCount != 0);

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
typedef struct
{
	FTM_ULONG			nMaxCount;
	FTM_ULONG			nCount;
	FTM_EP_PTR	pInfos;
}	FTDM_DBIF_CB_GET_EP_LIST_PARAMS, _PTR_ FTDM_DBIF_CB_GET_EP_LIST_PARAMS_PTR;

FTM_INT	_FTDM_DBIF_EP_getListCB
(
	FTM_VOID_PTR	pData, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR _PTR_	pArgv, 
	FTM_CHAR_PTR _PTR_ pColName
)
{
	FTDM_DBIF_CB_GET_EP_LIST_PARAMS_PTR pParams = (FTDM_DBIF_CB_GET_EP_LIST_PARAMS_PTR)pData;

	if (nArgc != 0)
	{
		FTM_INT	i;

		if (pParams->nCount < pParams->nMaxCount)
		{
			for(i = 0 ; i < nArgc ; i++)
			{
				if (strcmp(pColName[i], "VALUE") == 0)
				{
					memcpy(&pParams->pInfos[pParams->nCount], pArgv[i], sizeof(FTM_EP));
					pParams->nCount++;
				}
			}
		}
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_getList
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_EP_PTR		pInfos, 
	FTM_ULONG		nMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
    FTM_INT			xRet;
    FTM_CHAR		pSQL[1024];
    FTM_CHAR_PTR	pErrMsg = NULL;
	FTDM_DBIF_CB_GET_EP_LIST_PARAMS xParams= 
	{
		.nMaxCount 	= nMaxCount,
		.nCount		= 0,
		.pInfos		= pInfos
	};

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

    sprintf(pSQL, "SELECT VALUE FROM ep_info");
    xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_EP_getListCB, &xParams, &pErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR2(xRet, "%s\n", pErrMsg);
        sqlite3_free(pErrMsg);

    	return  FTM_RET_ERROR;
    }

	*pulCount = xParams.nCount;

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
int _FTDM_DBIF_EP_getCB(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTM_EP_PTR pInfo = (FTM_EP_PTR)pData;

	if (nArgc != 0)
	{
		if (strcmp(pColName[0], "EPID") == 0)
		{
			strncpy(pInfo->pEPID, pArgv[0], FTM_EPID_LEN);
		}
		else if (strcmp(pColName[0], "VALUE") == 0)
		{
			memcpy(pInfo, pArgv[0], sizeof(FTM_EP));
		}
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_get
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pEPID,
 	FTM_EP_PTR		pEP
)
{
    int     xRet;
    char    pSQL[1024];
    char    *pErrMsg = NULL;
	FTM_EP	xEP;

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

    sprintf(pSQL, "SELECT * FROM ep_info WHERE EPID = \'%s\'", pEPID);
    xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_EP_getCB, &xEP, &pErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR2(xRet, "%s\n", pErrMsg);
        sqlite3_free(pErrMsg);

    	return  FTM_RET_ERROR;
    }
	
	if (strcmp(xEP.pEPID, pEPID) != 0)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	memcpy(pEP, &xEP, sizeof(FTM_EP));

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_set
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pEPID,
 	FTM_EP_PTR		pInfo
)
{
	ASSERT(pInfo != NULL);

	FTM_INT			nRet;
	sqlite3_stmt 	*pStmt;
	FTM_CHAR		pSQL[1024];

	if (pDBIF->pSQLiteDB == NULL)
	{
		TRACE("DB is not initialize.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "UPDATE ep_info SET VALUE = ? WHERE EPID = \'%s\'", pEPID);
	do 
	{
		nRet = sqlite3_prepare(pDBIF->pSQLiteDB, pSQL, -1, &pStmt, 0);
		if( nRet!=SQLITE_OK )
		{
			ERROR2(nRet, "%s\n", sqlite3_errmsg(pDBIF->pSQLiteDB));
			return FTM_RET_ERROR;
		}

		sqlite3_bind_blob(pStmt, 1, pInfo, sizeof(FTM_EP), SQLITE_STATIC);

		nRet = sqlite3_step(pStmt);
		ASSERT( nRet != SQLITE_ROW);

		nRet = sqlite3_finalize(pStmt);
	}  while (nRet == SQLITE_SCHEMA);

	return FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_DATA_initTable
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pEPID
)
{
	ASSERT(pEPID != NULL);

	FTM_RET			xRet;
	FTM_BOOL		bExist = FTM_FALSE;
	FTM_CHAR		pTableName[FTM_ID_LEN + 16];

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pTableName, "ep_%s", pEPID);
	xRet = _FTDM_DBIF_isExistTable(pTableName, &bExist);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (bExist == FTM_FALSE)
	{ 
		FTM_RET		xRet;
		FTM_CHAR_PTR	pErrMsg = NULL;
		FTM_CHAR		pSQL[1024];

		sprintf(pSQL, "CREATE TABLE ep_%s (ID INT64,TIME INT,STATE INT,VALUE TEXT)", pEPID);

		xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
		if (xRet != SQLITE_OK)
		{
        	ERROR2(xRet, "%s\n", pErrMsg);
			sqlite3_free(pErrMsg);

			return	FTM_RET_ERROR;
		}

		TRACE("Table ep_%s creating has done.\n", pEPID);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_DATA_append
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR	pData
)
{
	int				xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	char			pSQL[1024];
	struct timeval	tv;

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	gettimeofday(&tv, NULL);

	switch (pData->xValue.xType)
	{
	case	FTM_VALUE_TYPE_INT:
		{
			if (FTDM_DBIF_EP_DATA_bIOTrace)
			{
				TRACE("INSERT INTO ep_%s VALUES (%llu, %lu, %lu, 'i%d')\n", 
						pEPID,
						tv.tv_sec * (long long)1000000 + tv.tv_usec, 
						pData->ulTime, 
						(FTM_ULONG)pData->xState,
						pData->xValue.xValue.nValue);
			}

			sprintf(pSQL, "INSERT INTO ep_%s VALUES (%llu, %lu, %lu, 'i%d')", 
					pEPID,
					tv.tv_sec * (long long)1000000 + tv.tv_usec, 
					pData->ulTime, 
					(FTM_ULONG)pData->xState,
					pData->xValue.xValue.nValue);
		}
		break;

	case	FTM_VALUE_TYPE_ULONG:
		{
			if (FTDM_DBIF_EP_DATA_bIOTrace)
			{
				TRACE("INSERT INTO ep_%s VALUES (%llu, %lu, %lu, 'u%lu')\n", 
						pEPID,
						tv.tv_sec * (long long)1000000 + tv.tv_usec, 
						pData->ulTime, 
						(FTM_ULONG)pData->xState,
						pData->xValue.xValue.ulValue);
			}
			sprintf(pSQL, "INSERT INTO ep_%s VALUES (%llu, %lu, %lu, 'u%lu')", 
					pEPID,
					tv.tv_sec * (long long)1000000 + tv.tv_usec, 
					pData->ulTime, 
					(FTM_ULONG)pData->xState,
					pData->xValue.xValue.ulValue);
		}
		break;

	case	FTM_VALUE_TYPE_FLOAT:
		{
			if (FTDM_DBIF_EP_DATA_bIOTrace)
			{
				TRACE("INSERT INTO ep_%s VALUES (%llu, %lu, %lu, 'f%8.3lf')\n", 
						pEPID, 
						tv.tv_sec * (long long)1000000 + tv.tv_usec, 
						pData->ulTime, 
						(FTM_ULONG)pData->xState,
						pData->xValue.xValue.fValue);
			}
			sprintf(pSQL, "INSERT INTO ep_%s VALUES (%llu, %lu, %lu, 'f%8.3lf')", 
					pEPID, 
					tv.tv_sec * (long long)1000000 + tv.tv_usec, 
					pData->ulTime, 
					(FTM_ULONG)pData->xState,
					pData->xValue.xValue.fValue);
		}
		break;

	default:
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xRet = FTDM_DBIF_EP_DATA_initTable(pEPID);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
        ERROR2(xRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

/********************************************************
 *
 ********************************************************/
FTM_INT	_FTDM_DBIF_EP_DATA_timeCB
(
	FTM_VOID_PTR	pData, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR _PTR_ pArgv, 
	FTM_CHAR_PTR _PTR_ pColName
)
{
	if (nArgc != 0)
	{
		if (strcmp(pColName[0], "TIME") == 0)
		{
			*((FTM_ULONG_PTR)pData)= strtoul(pArgv[0], 0, 10);
		}
    }

    return  FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_DATA_info
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR	pulBeginTime,
	FTM_ULONG_PTR	pulEndTime
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pTableName[FTM_ID_LEN + 16];
	FTM_CHAR		pSQL[1024];
	FTM_BOOL		bExist = FTM_FALSE;

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pTableName, "ep_%s", pEPID);
	xRet = _FTDM_DBIF_isExistTable(pTableName, &bExist);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (bExist != FTM_TRUE)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	sprintf(pSQL, "SELECT TIME from %s ORDER BY TIME DESC LIMIT 1", pTableName);
	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_EP_DATA_timeCB, pulEndTime, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
        ERROR2(xRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	sprintf(pSQL, "SELECT TIME from %s  ORDER BY TIME ASC LIMIT 1", pTableName);
	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_EP_DATA_timeCB, pulBeginTime, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
        ERROR2(xRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

/********************************************************
 *
 ********************************************************/

int	_FTDM_DBIF_EP_DATA_countCB(void *pData, int nArgc, char **pArgv, char **pColName)
{
       *((FTM_ULONG_PTR)pData) = strtoul(pArgv[0], 0, 10);

    return  FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_DATA_count
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR	pulCount
)
{
	FTM_CHAR		pTableName[FTM_ID_LEN + 16];

	sprintf(pTableName, "ep_%s", pEPID);
	return	FTDM_DBIF_count(pTableName, pulCount);
}


FTM_RET	FTDM_DBIF_EP_DATA_CountWithTime
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		xBeginTime,
	FTM_ULONG		xEndTime,
	FTM_ULONG_PTR	pulCount
)
{
	FTM_RET		xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_INT		nSQLLen = 0;
	FTM_BOOL		bConditionOn = FTM_FALSE;

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	nSQLLen = sprintf(pSQL, "SELECT COUNT(*) from ep_%s ", pEPID);
	if (xBeginTime != 0)
	{
		if (bConditionOn == FTM_FALSE)
		{
			bConditionOn = FTM_TRUE;
			nSQLLen += sprintf(&pSQL[nSQLLen], "WHERE ");
		}
		else
		{
			nSQLLen += sprintf(&pSQL[nSQLLen], "AND ");
		}
		nSQLLen += sprintf(&pSQL[nSQLLen], "TIME >= %lu ", xBeginTime);
	}

	if (xEndTime != 0)
	{
		if (bConditionOn == FTM_FALSE)
		{
			bConditionOn = FTM_TRUE;
			nSQLLen += sprintf(&pSQL[nSQLLen], "WHERE ");
		}
		else
		{
			nSQLLen += sprintf(&pSQL[nSQLLen], "AND ");
		}
		nSQLLen += sprintf(&pSQL[nSQLLen], "TIME <= %lu ", xEndTime);
	}


	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_EP_DATA_countCB, pulCount, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
        ERROR2(xRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

/***********************************************************
 *
 ***********************************************************/
typedef struct 
{
	FTM_EP_DATA_PTR	pEPData;
	FTM_INT			nMaxCount;
	FTM_INT			nCount;
}	_FTDM_DBIF_CB_GET_EP_DATA_PARAMS, _PTR_ _FTDM_DBIF_CB_GET_EP_DATA_PARAMS_PTR;

FTM_INT	_FTDM_DBIF_EP_DATA_getCB
(
	FTM_VOID _PTR_ pData, 
	FTM_INT	nArgc, 
	FTM_CHAR _PTR_ _PTR_ pArgv, 
	FTM_CHAR _PTR_ _PTR_ pColName
)
{
	_FTDM_DBIF_CB_GET_EP_DATA_PARAMS_PTR pParams = (_FTDM_DBIF_CB_GET_EP_DATA_PARAMS_PTR)pData;

	if ((nArgc != 0) && (pParams->nCount < pParams->nMaxCount))
	{
		FTM_INT	i;
		
		pParams->nCount++;
		for(i = 0 ; i < nArgc ; i++)
		{
			if (strcmp(pColName[i],"TIME") == 0)
			{
				if (pParams->nCount <= pParams->nMaxCount)
				{
					pParams->pEPData[pParams->nCount-1].ulTime = strtoul(pArgv[i], 0, 10);
				}
			}
			else if (strcmp(pColName[i], "STATE") == 0)
			{
				pParams->pEPData[pParams->nCount-1].xState = strtoul(&pArgv[i][1], NULL, 10);
			}
			else if (strcmp(pColName[i], "VALUE") == 0)
			{
				if (pParams->nCount <= pParams->nMaxCount)
				{
					if (pArgv[i][0] == 'i')
					{
						FTM_INT	nValue;
						
						nValue  = strtol(&pArgv[i][1], NULL, 10);
						FTM_VALUE_initINT(&pParams->pEPData[pParams->nCount-1].xValue, nValue);
					}
					else if (pArgv[i][0] == 'u')
					{
						FTM_ULONG	ulValue;
						
						ulValue = strtoul(&pArgv[i][1], NULL, 10);
						FTM_VALUE_initULONG(&pParams->pEPData[pParams->nCount-1].xValue, ulValue);
					}
					else if (pArgv[i][0] == 'f')
					{
						FTM_FLOAT	fValue;

						fValue = strtod(&pArgv[i][1], NULL);
						FTM_VALUE_initFLOAT(&pParams->pEPData[pParams->nCount-1].xValue, fValue);
					}
				}
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_DATA_get
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		nStartIndex,
	FTM_EP_DATA_PTR	pEPData,
	FTM_ULONG		nMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	FTM_RET			xRet;
	FTM_CHAR		pTableName[FTM_NAME_LEN+1];

	_FTDM_DBIF_CB_GET_EP_DATA_PARAMS	xParams;

	sprintf(pTableName, "ep_%s", pEPID);

	xParams.pEPData = pEPData;
	xParams.nMaxCount = nMaxCount;
	xParams.nCount = 0;
	
	xRet = FTDM_DBIF_get(pTableName, nStartIndex, nMaxCount, _FTDM_DBIF_EP_DATA_getCB, &xParams);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	*pulCount = xParams.nCount;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_DATA_getWithTime
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		xBeginTime,
	FTM_ULONG		xEndTime,
	FTM_BOOL		bAscending,
	FTM_EP_DATA_PTR	pEPData,
	FTM_ULONG		nMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	FTM_RET			xRet;
	FTM_CHAR		pTableName[FTM_NAME_LEN+1];
	_FTDM_DBIF_CB_GET_EP_DATA_PARAMS	xParams;

	sprintf(pTableName, "ep_%s", pEPID);
	xParams.pEPData 	= pEPData;
	xParams.nMaxCount	= nMaxCount;
	xParams.nCount		= 0;

	xRet = FTDM_DBIF_getWithTime(pTableName, xBeginTime, xEndTime, bAscending, nMaxCount, _FTDM_DBIF_EP_DATA_getCB, &xParams);
	if (xRet != FTM_RET_OK)
	{
		*pulCount = 0;
	}
	else
	{
		*pulCount = xParams.nCount;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_DATA_del
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pEPID,
	FTM_INT			nIndex,
	FTM_ULONG		nCount
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_ULONG		ulTotalCount = 0, ulRetCount = 0;
	FTM_EP_DATA		xFirstData, xLastData;

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	xRet = FTDM_DBIF_EP_DATA_count(pEPID, &ulTotalCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (ulTotalCount == 0)
	{
		return	FTM_RET_OK;	
	}

	if (nIndex >= 0)
	{
		if (nIndex >= ulTotalCount)
		{
			return	FTM_RET_OK;	
		}

		if (nIndex + nCount > ulTotalCount)
		{
			nCount = ulTotalCount - nIndex;	
		}
	}
	else
	{
		if (nCount > ulTotalCount)
		{
			nIndex = 0;
			nCount = ulTotalCount;
		}
		else
		{
			nIndex = ulTotalCount - nCount;	
		}
	
	}

	xRet = FTDM_DBIF_EP_DATA_get(pEPID, nIndex, &xFirstData, 1, &ulRetCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTDM_DBIF_EP_DATA_get(pEPID, nIndex + nCount - 1, &xLastData, 1, &ulRetCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	sprintf(pSQL, " DELETE FROM ep_%s WHERE %lu <= TIME AND TIME <= %lu", pEPID, xFirstData.ulTime, xLastData.ulTime);
	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
        ERROR2(xRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_DATA_delWithTime
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		xBeginTime,
	FTM_ULONG		xEndTime
)
{
	FTM_CHAR	pTableName[FTM_NAME_LEN+1];
	FTM_ULONG	ulCount = 0;

	sprintf(pTableName, "ep_%s", pEPID);

	return	FTDM_DBIF_delWithTime(pTableName, xBeginTime, xEndTime, &ulCount);
}


/*************************************************************************
 *
 *************************************************************************/
FTM_RET	FTDM_DBIF_EP_DATA_countWithTime
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		xBeginTime,
	FTM_ULONG		xEndTime,
	FTM_ULONG_PTR	pulCount
)
{
	FTM_CHAR	pTableName[128];
	
	sprintf(pTableName, "ep_%s", pEPID);

	return	FTDM_DBIF_countWithTime(pTableName, xBeginTime, xEndTime, pulCount);
}

/*************************************************************************/
typedef struct
{
	FTM_BOOL		bExist;
	FTM_CHAR_PTR	pName;
}   _FTDM_DBIF_CB_EXIST_TABLE_PARAMS, *  _FTDM_DBIF_CB_EXIST_TABLE_PARAMS_PTR;

FTM_INT	_FTDM_DBIF_CB_isExistTable
(
	FTM_VOID_PTR	pData, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR _PTR_	pArgv, 
	FTM_CHAR_PTR _PTR_ pColName
)
{
    int i;
  	_FTDM_DBIF_CB_EXIST_TABLE_PARAMS_PTR pParams = (_FTDM_DBIF_CB_EXIST_TABLE_PARAMS_PTR)pData;

    if (nArgc != 0)
    {
        for(i = 0 ; i < nArgc ; i++)
        {
            if (strcmp(pParams->pName, pArgv[i]) == 0)
            {
                pParams->bExist = FTM_TRUE;
                break;
            }
        }

    }

    return  0;
}

FTM_RET _FTDM_DBIF_isExistTable
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName, 
	FTM_BOOL_PTR 	pExist
)
{
    int     xRet;
    _FTDM_DBIF_CB_EXIST_TABLE_PARAMS xParams = { .bExist = FTM_FALSE, .pName = pTableName};
    FTM_CHAR_PTR	pSQL = "select name from sqlite_master where type='table' order by name";
    FTM_CHAR_PTR	pErrMsg = NULL;

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB is not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

    xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_CB_isExistTable, &xParams, &pErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR2(xRet, "%s\n", pErrMsg);
        sqlite3_free(pErrMsg);

        return  FTM_RET_ERROR;
    }

    *pExist = xParams.bExist;

    return  FTM_RET_OK;
}

FTM_RET	_FTDM_DBIF_NODE_createTable
(
	FTM_CHAR_PTR	pTableName
)
{
	int	xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	char			pSQL[1024];

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "CREATE TABLE %s ("\
					"DID	TEXT PRIMARY KEY,"\
					"VALUE	BLOB)" , pTableName);

	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
        ERROR2(xRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

/*************************************************************************/
typedef struct
{
	FTM_BOOL	bExist;
}   _FTDM_DBIF_CB_IS_EXIST_PARAMS, * _FTDM_DBIF_CB_IS_EXIST_PARAMS_PTR;

FTM_INT	_FTDM_DBIF_CB_isExist
(
	FTM_VOID_PTR	pData, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR _PTR_	pArgv, 
	FTM_CHAR_PTR _PTR_ pColName
)
{
    _FTDM_DBIF_CB_IS_EXIST_PARAMS_PTR pParams = (_FTDM_DBIF_CB_IS_EXIST_PARAMS_PTR)pData;

	if (nArgc != 0)
	{
		if (atoi(pArgv[0]) != 0)
		{
			pParams->bExist = FTM_TRUE;
		}
    }

    return  FTM_RET_OK;
}

FTM_RET _FTDM_DBIF_NODE_isExist
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR pDID, 
	FTM_BOOL_PTR pExist
)
{
	_FTDM_DBIF_CB_IS_EXIST_PARAMS xParams = {.bExist = FTM_FALSE };
    FTM_INT			xRet;
    FTM_CHAR		pSQL[1024];
    FTM_CHAR_PTR	pErrMsg = NULL;

	if (pDBIF->pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

    sprintf(pSQL, "SELECT COUNT(DID) FROM node_info WHERE DID = '%s'", pDID);
    xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_CB_isExist, &xParams, &pErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR2(xRet, "%s\n", pErrMsg);
        sqlite3_free(pErrMsg);

    	return  FTM_RET_ERROR;
    }

    *pExist = xParams.bExist;

    return  FTM_RET_OK;
}

/*******************************************************
 *
 *******************************************************/
FTM_RET FTDM_DBIF_setTrace
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_BOOL			bTraceON
)
{
	FTDM_DBIF_EP_DATA_bIOTrace = bTraceON;

	return	FTM_RET_OK;
}

FTM_RET FTDM_DBIF_getTrace
(	
	FTDM_DBIF_PTR	pDBIF,
	FTM_BOOL_PTR	pbTraceON
)
{
	*pbTraceON = FTDM_DBIF_EP_DATA_bIOTrace;

	return FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
FTM_RET	FTDM_DBIF_TRIGGER_initTable
(
	FTDM_DBIF_PTR	pDBIF
)
{
	FTM_INT			xRet;
	FTM_CHAR_PTR	pTableName = "trigger";
	FTM_BOOL		bExist = FTM_FALSE;

	if (pDBIF->pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	xRet = _FTDM_DBIF_isExistTable(pTableName, &bExist);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_DBIF_ERROR;	
	}

	if (bExist != FTM_TRUE)
	{
		xRet = _FTDM_DBIF_TRIGGER_createTable(pTableName);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Can't create a new tables[%s].\n", pTableName);
			return	xRet;	
		}
		TRACE("It created new TRIGGER table[%s].\n", pTableName);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_TRIGGER_count
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_ULONG_PTR	pulCount
)
{
	return	FTDM_DBIF_count("trigger", pulCount);
}

FTM_RET	FTDM_DBIF_TRIGGER_create
(
	FTDM_DBIF_PTR	pDBIF,
 	FTM_TRIGGER_PTR	pTrigger
)
{
	ASSERT(pTrigger != NULL);

	FTM_INT			nRet;
	sqlite3_stmt 	*pStmt;
	FTM_CHAR		pSQL[1024];

	if (pDBIF->pSQLiteDB == NULL)
	{
		TRACE("DB is not initialize.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "INSERT INTO trigger (ID,VALUE) VALUES (?,?)");
	do 
	{
		nRet = sqlite3_prepare(pDBIF->pSQLiteDB, pSQL, -1, &pStmt, 0);
		if( nRet!=SQLITE_OK )
		{
			return FTM_RET_ERROR;
		}

		sqlite3_bind_text(pStmt, 1, pTrigger->pID, strlen(pTrigger->pID), 0);
		sqlite3_bind_blob(pStmt, 2, pTrigger, sizeof(FTM_TRIGGER), SQLITE_STATIC);

		nRet = sqlite3_step(pStmt);
		ASSERT( nRet != SQLITE_ROW);

		nRet = sqlite3_finalize(pStmt);
	}  while (nRet == SQLITE_SCHEMA);

	return FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
FTM_RET	FTDM_DBIF_TRIGGER_destroy
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pID
)
{
	FTM_RET		xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "DELETE FROM trigger WHERE ID = '%s'", pID);
	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
        ERROR2(xRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}
/***************************************************************
 *
 ***************************************************************/
FTM_RET	_FTDM_DBIF_TRIGGER_createTable
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName
)
{
	FTM_INT			xRet;
	FTM_CHAR		pSQL[1024];
	FTM_CHAR_PTR	pErrMsg = NULL;

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "CREATE TABLE %s (ID	TEXT PRIMARY KEY,VALUE BLOB)", pTableName);

	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
        ERROR2(xRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}


int _FTDM_DBIF_TRIGGER_getCB(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTM_TRIGGER_PTR	pTrigger = (FTM_TRIGGER_PTR)pData;

	if (nArgc != 0)
	{
		if (strcmp(pColName[0], "ID") == 0)
		{
			strncpy(pTrigger->pID, pArgv[0], FTM_ID_LEN);
		}
		else if (strcmp(pColName[0], "VALUE") == 0)
		{
			memcpy(pTrigger, pArgv[0], sizeof(FTM_TRIGGER));
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_TRIGGER_get
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pID,
 	FTM_TRIGGER_PTR	pTrigger
)
{
    FTM_INT			xRet;
    FTM_CHAR		pSQL[1024];
    FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_TRIGGER		xTrigger;

	if (pDBIF->pSQLiteDB == NULL)
	{
		TRACE("DB is not initialize.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	memset(&xTrigger, 0, sizeof(xTrigger));

    sprintf(pSQL, "SELECT * FROM trigger WHERE ID = '%s'", pID);
    xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_TRIGGER_getCB, &xTrigger, &pErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR2(xRet, "%s\n", pErrMsg);
        sqlite3_free(pErrMsg);

    	return  FTM_RET_ERROR;
    }

	if (strcpy(xTrigger.pID, pID) != 0)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	memcpy(pTrigger, &xTrigger, sizeof(FTM_TRIGGER));
	return	FTM_RET_OK;
}

typedef struct
{
	FTM_ULONG		ulMaxCount;
	FTM_ULONG		ulCount;
	FTM_TRIGGER_PTR	pTriggers;
}	FTDM_DBIF_CB_GET_TRIGGER_LIST_PARAMS, _PTR_ FTDM_DBIF_CB_GET_TRIGGER_LIST_PARAMS_PTR;

FTM_INT	_FTDM_DBIF_TRIGGER_getListCB
(
	FTM_VOID_PTR	pData, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR _PTR_	pArgv, 
	FTM_CHAR_PTR _PTR_ pColName
)
{
	FTDM_DBIF_CB_GET_TRIGGER_LIST_PARAMS_PTR pParams = (FTDM_DBIF_CB_GET_TRIGGER_LIST_PARAMS_PTR)pData;

	if (nArgc != 0)
	{
		FTM_INT	i;
		FTM_TRIGGER_PTR	pTrigger = &pParams->pTriggers[pParams->ulCount++];

		for(i = 0 ; i < nArgc ; i++)
		{
			if (strcasecmp(pColName[i], "ID") == 0)
			{
				strncpy(pTrigger->pID , pArgv[i], FTM_ID_LEN);
			}
			else if (strcasecmp(pColName[i], "VALUE") == 0)
			{
				memcpy(pTrigger, pArgv[i], sizeof(FTM_TRIGGER));
			}
		}

	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_TRIGGER_getList
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_TRIGGER_PTR		pTriggers, 
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pulCount
)
{
    FTM_INT			xRet;
    FTM_CHAR		pSQL[1024];
    FTM_CHAR_PTR	pErrMsg = NULL;
	FTDM_DBIF_CB_GET_TRIGGER_LIST_PARAMS xParams= 
	{
		.ulMaxCount = nMaxCount,
		.ulCount	= 0,
		.pTriggers	= pTriggers
	};

	if (pDBIF->pSQLiteDB == NULL)
	{
		TRACE("DB is not initialize.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

    sprintf(pSQL, "SELECT * FROM trigger");
    xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_TRIGGER_getListCB, &xParams, &pErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR2(xRet, "%s\n", pErrMsg);
        sqlite3_free(pErrMsg);

    	return  FTM_RET_ERROR;
    }

	*pulCount = xParams.ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_TRIGGER_set
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTriggerID,
 	FTM_TRIGGER_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);

	FTM_INT			nRet;
	FTM_CHAR		pSQL[1024];
	sqlite3_stmt 	*pStmt;

	if (pDBIF->pSQLiteDB == NULL)
	{
		TRACE("DB is not initialize.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "UPDATE trigger set VALUE = ? where ID= ?");

	do 
	{
		nRet = sqlite3_prepare(pDBIF->pSQLiteDB, pSQL, -1, &pStmt, 0);
		if( nRet !=SQLITE_OK )
		{
			return FTM_RET_ERROR;
		}

		sqlite3_bind_blob(pStmt, 1, pInfo, sizeof(FTM_TRIGGER), SQLITE_STATIC);
		sqlite3_bind_text(pStmt, 2, pInfo->pID, strlen(pInfo->pID), 0);

		nRet = sqlite3_step(pStmt);
		ASSERT( nRet != SQLITE_ROW);

		nRet = sqlite3_finalize(pStmt);
	}  while (nRet == SQLITE_SCHEMA);

	return FTM_RET_OK;
}
/***************************************************************
 *
 ***************************************************************/
FTM_RET	FTDM_DBIF_ACTION_initTable
(
	FTDM_DBIF_PTR	pDBIF
)
{
	FTM_INT			xRet;
	FTM_CHAR_PTR	pTableName = "action";
	FTM_BOOL		bExist = FTM_FALSE;

	if (pDBIF->pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	if (_FTDM_DBIF_isExistTable(pTableName, &bExist) != FTM_RET_OK)
	{
		ERROR2(FTM_RET_DBIF_ERROR, "%s table check error.\n", pTableName);  
		return	FTM_RET_DBIF_ERROR;	
	}

	if (bExist != FTM_TRUE)
	{
		xRet = _FTDM_DBIF_ACTION_createTable(pTableName);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Can't not create a new tables[%s].\n", pTableName);
			return	xRet;	
		}
		TRACE("It created new ACTION table[%s].\n", pTableName);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_ACTION_count
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_ULONG_PTR	pulCount
)
{
	return	FTDM_DBIF_count("action", pulCount);
}

FTM_RET	FTDM_DBIF_ACTION_create
(
	FTDM_DBIF_PTR	pDBIF,
 	FTM_ACTION_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);

	FTM_INT			nRet;
	FTM_CHAR		pSQL[1024];
	sqlite3_stmt 	*pStmt;

	if (pDBIF->pSQLiteDB == NULL)
	{
		TRACE("DB is not initialize.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "INSERT INTO action (ID,VALUE) VALUES (?,?)");

	do 
	{
		nRet = sqlite3_prepare(pDBIF->pSQLiteDB, pSQL, -1, &pStmt, 0);
		if( nRet !=SQLITE_OK )
		{
			return FTM_RET_ERROR;
		}

		sqlite3_bind_text(pStmt, 1, pInfo->pID, strlen(pInfo->pID), 0);
		sqlite3_bind_blob(pStmt, 2, pInfo, sizeof(FTM_ACTION), SQLITE_STATIC);

		nRet = sqlite3_step(pStmt);
		ASSERT( nRet != SQLITE_ROW);

		nRet = sqlite3_finalize(pStmt);
	}  while (nRet == SQLITE_SCHEMA);

	return FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_ACTION_set
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pActionID,
 	FTM_ACTION_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);

	FTM_INT			nRet;
	FTM_CHAR		pSQL[1024];
	sqlite3_stmt 	*pStmt;

	if (pDBIF->pSQLiteDB == NULL)
	{
		TRACE("DB is not initialize.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "UPDATE action set VALUE = ? where ID= ?");

	do 
	{
		nRet = sqlite3_prepare(pDBIF->pSQLiteDB, pSQL, -1, &pStmt, 0);
		if( nRet !=SQLITE_OK )
		{
			return FTM_RET_ERROR;
		}

		sqlite3_bind_blob(pStmt, 1, pInfo, sizeof(FTM_ACTION), SQLITE_STATIC);
		sqlite3_bind_text(pStmt, 2, pInfo->pID, strlen(pInfo->pID), 0);

		nRet = sqlite3_step(pStmt);
		ASSERT( nRet != SQLITE_ROW);

		nRet = sqlite3_finalize(pStmt);
	}  while (nRet == SQLITE_SCHEMA);

	return FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
FTM_RET	FTDM_DBIF_ACTION_destroy
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pActionID
)
{
	FTM_RET		xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "DELETE FROM action WHERE ID = '%s'", pActionID);
	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
        ERROR2(xRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}
/***************************************************************
 *
 ***************************************************************/
FTM_RET	_FTDM_DBIF_ACTION_createTable
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName
)
{
	FTM_INT			nRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "CREATE TABLE %s ("\
						"ID TEXT PRIMARY KEY,"\
						"VALUE BLOB)", pTableName);

	nRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (nRet != SQLITE_OK)
	{
        ERROR2(nRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_INT	_FTDM_DBIF_ACTION_getCB
(
	FTM_VOID_PTR	pData, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR _PTR_	pArgv, 
	FTM_CHAR_PTR _PTR_ pColName
)
{
	FTM_ACTION_PTR	pAction = (FTM_ACTION_PTR)pData;

	if (nArgc != 0)
	{
		if (strcmp(pColName[0], "ID") == 0)
		{
			strncpy(pAction->pID, pArgv[0], FTM_ID_LEN);
		}
		else if (strcmp(pColName[0], "VALUE") == 0)
		{
			memcpy(pAction, pArgv[0], sizeof(FTM_ACTION));
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_ACTION_get
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pActionID,
 	FTM_ACTION_PTR	pAction
)
{
    FTM_INT			nRet;
    FTM_CHAR		pSQL[1024];
    FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_ACTION		xAction;

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	memset(&xAction, 0, sizeof(xAction));

    sprintf(pSQL, "SELECT * FROM action WHERE ID = '%s'", pActionID);
    nRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_ACTION_getCB, &xAction, &pErrMsg);
    if (nRet != SQLITE_OK)
    {
        ERROR2(nRet, "%s\n", pErrMsg);
        sqlite3_free(pErrMsg);

    	return  FTM_RET_ERROR;
    }

	if (strcasecmp(xAction.pID,pActionID) != 0)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	memcpy(pAction, &xAction, sizeof(xAction));

	return	FTM_RET_OK;
}

typedef struct
{
	FTM_ULONG		ulMaxCount;
	FTM_ULONG		ulCount;
	FTM_ACTION_PTR	pActions;
}	FTDM_DBIF_CB_GET_ACTION_LIST_PARAMS, _PTR_ FTDM_DBIF_CB_GET_ACTION_LIST_PARAMS_PTR;

FTM_INT	_FTDM_DBIF_ACTION_getListCB
(
	FTM_VOID_PTR	pData, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR _PTR_	pArgv, 
	FTM_CHAR_PTR _PTR_ pColName
)
{
	FTDM_DBIF_CB_GET_ACTION_LIST_PARAMS_PTR pParams = (FTDM_DBIF_CB_GET_ACTION_LIST_PARAMS_PTR)pData;

	if (nArgc != 0)
	{
		FTM_INT	i;
		FTM_ACTION_PTR	pAction = &pParams->pActions[pParams->ulCount++];

		for(i = 0 ; i < nArgc ; i++)
		{
			if (strcasecmp(pColName[i], "ID") == 0)
			{
				strncpy( pAction->pID, pArgv[i], FTM_ID_LEN);
			}
			else if (strcasecmp(pColName[i], "VALUE") == 0)
			{
				memcpy(pAction, pArgv[i], sizeof(FTM_ACTION));
			}	
		}
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_ACTION_getList
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_ACTION_PTR		pActions, 
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pulCount
)
{
    FTM_INT			nRet;
    FTM_CHAR		pSQL[1024];
    FTM_CHAR_PTR	pErrMsg = NULL;
	FTDM_DBIF_CB_GET_ACTION_LIST_PARAMS xParams= 
	{
		.ulMaxCount = nMaxCount,
		.ulCount	= 0,
		.pActions	= pActions
	};

	if (pDBIF->pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

    sprintf(pSQL, "SELECT * FROM action");
    nRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_ACTION_getListCB, &xParams, &pErrMsg);
    if (nRet != SQLITE_OK)
    {
        ERROR2(nRet, "%s\n", pErrMsg);
        sqlite3_free(pErrMsg);

    	return  FTM_RET_ERROR;
    }

	*pulCount = xParams.ulCount;

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
FTM_RET	FTDM_DBIF_RULE_initTable
(
	FTDM_DBIF_PTR	pDBIF
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pTableName = "rule";
	FTM_BOOL		bExist = FTM_FALSE;

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	xRet = _FTDM_DBIF_isExistTable(pTableName, &bExist);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "%s table check error.\n", pTableName);  
		return	FTM_RET_DBIF_ERROR;	
	}

	if (bExist != FTM_TRUE)
	{
		TRACE("%s table is not exist.\n", pTableName);
		xRet = _FTDM_DBIF_RULE_createTable(pTableName);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Can't create a new table[%s][%08x]\n", pTableName, xRet);
			return	xRet;	
		}
		TRACE("It created new RULE table[%s].\n", pTableName);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_RULE_count
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_ULONG_PTR	pulCount
)
{
	return	FTDM_DBIF_count("rule", pulCount);
}

FTM_RET	FTDM_DBIF_RULE_append
(
	FTDM_DBIF_PTR	pDBIF,
 	FTM_RULE_PTR	pRule
)
{
	ASSERT(pRule != NULL);

	FTM_INT			nRet;
	FTM_CHAR		pSQL[1024];
  	sqlite3_stmt 	*pStmt;

	if (pDBIF->pSQLiteDB == NULL)
	{
		TRACE("DB is not initialize.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "INSERT INTO rule (ID,VALUE) VALUES(?, ?)");
	do 
	{
		nRet = sqlite3_prepare(pDBIF->pSQLiteDB, pSQL, -1, &pStmt, 0);
		if (nRet != SQLITE_OK)
		{
			return	FTM_RET_ERROR;
		}
	
	    sqlite3_bind_text(pStmt, 1, pRule->pID, strlen(pRule->pID), 0);
		sqlite3_bind_blob(pStmt, 2, pRule, sizeof(FTM_RULE), SQLITE_STATIC);
	
		nRet = sqlite3_step(pStmt);
	    ASSERT( nRet != SQLITE_ROW );
	
	   	nRet = sqlite3_finalize(pStmt);
	
  	} while( nRet == SQLITE_SCHEMA );

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
FTM_RET	_FTDM_DBIF_RULE_createTable
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName
)
{
	FTM_INT			nRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];

	if (pDBIF->pSQLiteDB == NULL)
	{
		TRACE("DB is not initialize.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "CREATE TABLE %s ("\
					"ID TEXT PRIMARY KEY,"\
					"VALUE BLOB)", pTableName);

	nRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (nRet != SQLITE_OK)
	{
        ERROR2(nRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_INT	_FTDM_DBIF_RULE_getCB
(
	FTM_VOID_PTR	pData, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR _PTR_	pArgv, 
	FTM_CHAR_PTR _PTR_ pColName
)
{
	FTM_RULE_PTR	pRule = (FTM_RULE_PTR)pData;

	if (nArgc != 0)
	{
		if (strcmp(pColName[0], "ID") == 0)
		{
			strncpy(pRule->pID, pArgv[0], FTM_ID_LEN);
		}
		else 
		{
			memcpy(&pRule->xParams, pArgv[0], sizeof(pRule->xParams));
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_RULE_create
(
	FTDM_DBIF_PTR	pDBIF,
 	FTM_RULE_PTR	pRule
)
{
	ASSERT(pRule != NULL);

	FTM_INT			nRet;
	sqlite3_stmt 	*pStmt;
	FTM_CHAR		pSQL[1024];

	if (pDBIF->pSQLiteDB == NULL)
	{
		TRACE("DB is not initialize.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "INSERT INTO rule (ID,VALUE) VALUES (?,?)");

	do 
	{
		nRet = sqlite3_prepare(pDBIF->pSQLiteDB, pSQL, -1, &pStmt, 0);
		if( nRet!=SQLITE_OK )
		{
			ERROR2(nRet, "SQLite3 prepare failed[%d].\n", nRet);
			return FTM_RET_ERROR;
		}

		sqlite3_bind_text(pStmt, 1, pRule->pID, strlen(pRule->pID), 0);
		sqlite3_bind_blob(pStmt, 2, pRule, sizeof(FTM_RULE), SQLITE_STATIC);

		nRet = sqlite3_step(pStmt);
		ASSERT( nRet != SQLITE_ROW);

		nRet = sqlite3_finalize(pStmt);
	}  while (nRet == SQLITE_SCHEMA);

	return FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
FTM_RET	FTDM_DBIF_RULE_destroy
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pRuleID
)
{
	FTM_RET		xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "DELETE FROM rule WHERE ID = '%s'", pRuleID);
	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
        ERROR2(xRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}
FTM_RET	FTDM_DBIF_RULE_get
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pRuleID,
 	FTM_RULE_PTR	pRule
)
{
    FTM_INT			nRet;
    FTM_CHAR		pSQL[1024];
    FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_RULE		xRule;

	if (pDBIF->pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	memset(&xRule, 0, sizeof(xRule));

    sprintf(pSQL, "SELECT * FROM rule WHERE ID = '%s'", pRuleID);
    nRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_RULE_getCB, &xRule, &pErrMsg);
    if (nRet != SQLITE_OK)
    {
        ERROR2(nRet, "%s\n", pErrMsg);
        sqlite3_free(pErrMsg);

    	return  FTM_RET_ERROR;
    }

	if (strcasecmp(xRule.pID, pRuleID) != 0)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	memcpy(pRule, &xRule, sizeof(xRule));
	return	FTM_RET_OK;
}

typedef struct
{
	FTM_ULONG		ulMaxCount;
	FTM_ULONG		ulCount;
	FTM_RULE_PTR	pRules;
}	FTDM_DBIF_CB_GET_RULE_LIST_PARAMS, _PTR_ FTDM_DBIF_CB_GET_RULE_LIST_PARAMS_PTR;

FTM_INT	_FTDM_DBIF_RULE_getListCB
(
	FTM_VOID_PTR	pData, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR _PTR_	pArgv, 
	FTM_CHAR_PTR _PTR_ pColName
)
{
	FTDM_DBIF_CB_GET_RULE_LIST_PARAMS_PTR pParams = (FTDM_DBIF_CB_GET_RULE_LIST_PARAMS_PTR)pData;

	if (nArgc != 0)
	{
		FTM_INT	i;
		FTM_RULE_PTR	pRule = &pParams->pRules[pParams->ulCount++];

		for(i = 0 ; i < nArgc ; i++)
		{
			if (strcasecmp(pColName[i], "ID") == 0)
			{
				strncpy(pRule->pID, pArgv[i], FTM_ID_LEN);
			}
			else if (strcasecmp(pColName[i], "VALUE") == 0)
			{
				memcpy(pRule, pArgv[i], sizeof(FTM_RULE));
			}
		}
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_RULE_getList
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_RULE_PTR		pRules, 
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pulCount
)
{
    FTM_INT			nRet;
    FTM_CHAR		pSQL[1024];
    FTM_CHAR_PTR	pErrMsg = NULL;
	FTDM_DBIF_CB_GET_RULE_LIST_PARAMS xParams= 
	{
		.ulMaxCount = nMaxCount,
		.ulCount	= 0,
		.pRules	= pRules
	};

	if (pDBIF->pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

    sprintf(pSQL, "SELECT * FROM rule");
    nRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_RULE_getListCB, &xParams, &pErrMsg);
    if (nRet != SQLITE_OK)
    {
        ERROR2(nRet, "%s\n", pErrMsg);
        sqlite3_free(pErrMsg);

    	return  FTM_RET_ERROR;
    }

	*pulCount = xParams.ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_RULE_set
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pRuleID,
 	FTM_RULE_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);

	FTM_INT			nRet;
	FTM_CHAR		pSQL[1024];
	sqlite3_stmt 	*pStmt;

	if (pDBIF->pSQLiteDB == NULL)
	{
		TRACE("DB is not initialize.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "UPDATE rule set VALUE = ? where ID= ?");

	do 
	{
		nRet = sqlite3_prepare(pDBIF->pSQLiteDB, pSQL, -1, &pStmt, 0);
		if( nRet !=SQLITE_OK )
		{
			return FTM_RET_ERROR;
		}

		sqlite3_bind_blob(pStmt, 1, pInfo, sizeof(FTM_RULE), SQLITE_STATIC);
		sqlite3_bind_text(pStmt, 2, pInfo->pID, strlen(pInfo->pID), 0);

		nRet = sqlite3_step(pStmt);
		ASSERT( nRet != SQLITE_ROW);

		nRet = sqlite3_finalize(pStmt);
	}  while (nRet == SQLITE_SCHEMA);

	return FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_count
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR		pTableName,
	FTM_ULONG_PTR		pulCount
)
{
    FTM_INT			xRet;
    FTM_CHAR		pSQL[1024];
    FTM_CHAR_PTR	pErrMsg = NULL;

	if (pDBIF->pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

    sprintf(pSQL, "SELECT COUNT(*) FROM %s", pTableName);
    xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, FTDM_DBIF_countCB, pulCount, &pErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR2(xRet, "%s\n", pErrMsg);
        sqlite3_free(pErrMsg);

    	return  FTM_RET_ERROR;
    }

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_LOG_initTable
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName
)
{
	FTM_RET			xRet;
	FTM_BOOL		bExist = FTM_FALSE;

	xRet = _FTDM_DBIF_isExistTable(pTableName, &bExist);
	if (xRet != FTM_RET_OK)
	{
		return	FTM_RET_DBIF_ERROR;	
	}

	if (bExist != FTM_TRUE)
	{
		TRACE("%s table is not exist.\n", pTableName);
		FTM_CHAR_PTR	pErrMsg = NULL;
		char			pSQL[1024];

		if (pDBIF->pSQLiteDB == NULL)
		{
			return	FTM_RET_NOT_INITIALIZED;	
		}

		sprintf(pSQL, "CREATE TABLE %s ("\
						"ID	INT PRIMARY KEY,"\
						"TIME INT,"\
						"LEVEL INT,"\
						"VALUE	BLOB)" , pTableName);

		xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
		if (xRet != SQLITE_OK)
		{
        	ERROR2(xRet, "%s\n", pErrMsg);
			sqlite3_free(pErrMsg);

			return	FTM_RET_ERROR;
		}
	}

	return	FTM_RET_OK;
}


FTM_RET	FTDM_DBIF_LOG_isTableExist
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName,
	FTM_BOOL_PTR	pbExist
)
{
	return _FTDM_DBIF_isExistTable(pTableName, pbExist);
}

FTM_RET	FTDM_DBIF_LOG_append
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName,
 	FTM_LOG_PTR	pLog
)
{
	ASSERT(pLog != NULL);

	FTM_INT			nRet;
	sqlite3_stmt 	*pStmt;
	FTM_CHAR		pSQL[1024];

	if (pDBIF->pSQLiteDB == NULL)
	{
		TRACE("DB is not initialize.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "INSERT INTO %s (ID, TIME, LEVEL, VALUE) VALUES (?,?,?,?)", pTableName);
	do 
	{
		nRet = sqlite3_prepare(pDBIF->pSQLiteDB, pSQL, -1, &pStmt, 0);
		if( nRet!=SQLITE_OK )
		{
			return FTM_RET_ERROR;
		}

		sqlite3_bind_int64(pStmt, 1, pLog->ullID);
		sqlite3_bind_int(pStmt, 2, pLog->ulTime);
		sqlite3_bind_int(pStmt, 3, pLog->xLevel);
		sqlite3_bind_blob(pStmt, 4, pLog, sizeof(FTM_LOG), SQLITE_STATIC);

		nRet = sqlite3_step(pStmt);
		ASSERT( nRet != SQLITE_ROW);

		nRet = sqlite3_finalize(pStmt);
	}  while (nRet == SQLITE_SCHEMA);

	return FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_LOG_del
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulDeletedCount
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_ULONG		ulTotalCount = 0, ulRetCount = 0;
	FTM_LOG			xFirstLog, xLastLog;

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	xRet = FTDM_DBIF_LOG_count("log", &ulTotalCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (ulTotalCount == 0)
	{
		return	FTM_RET_OK;	
	}

	if (ulIndex >= 0)
	{
		if (ulIndex >= ulTotalCount)
		{
			return	FTM_RET_OK;	
		}

		if (ulIndex + ulCount > ulTotalCount)
		{
			ulCount = ulTotalCount - ulIndex;	
		}
	}
	else
	{
		if (ulCount > ulTotalCount)
		{
			ulIndex = 0;
			ulCount = ulTotalCount;
		}
		else
		{
			ulIndex = ulTotalCount - ulCount;	
		}
	
	}

	xRet = FTDM_DBIF_LOG_get("log", ulIndex, &xFirstLog, 1, &ulRetCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTDM_DBIF_LOG_get("log", ulIndex + ulCount - 1, &xLastLog, 1, &ulRetCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	sprintf(pSQL, " DELETE FROM log WHERE %llu <= ID AND ID <= %llu", xFirstLog.ullID, xLastLog.ullID);
	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
        ERROR2(xRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	*pulDeletedCount = ulCount;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_LOG_delWithTime
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName,
	FTM_ULONG		xBeginTime,
	FTM_ULONG		xEndTime,
	FTM_ULONG_PTR	pulDeletedCount
)
{
	return	FTDM_DBIF_delWithTime(pTableName, xBeginTime, xEndTime, pulDeletedCount);
}
/***************************************************************
 *
 ***************************************************************/
FTM_RET	FTDM_DBIF_LOG_count
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR		pTableName,
	FTM_ULONG_PTR		pulCount
)
{
	return	FTDM_DBIF_count(pTableName, pulCount);
}

/***************************************************************
 *
 ***************************************************************/
FTM_RET	FTDM_DBIF_LOG_isExist
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName,
	FTM_UINT64		ullID,
	FTM_BOOL_PTR	pbExist
)
{
	FTM_RET			xRet;
    FTM_INT			nRet;
    FTM_CHAR		pSQL[1024];
    FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_BOOL		bTableExist;
	FTDM_DBIF_COUNT_PARAMS	xParams; 

	if (pDBIF->pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	xRet = FTDM_DBIF_LOG_isTableExist(pTableName, &bTableExist);
	if (xRet != FTM_RET_OK)
	{
		*pbExist = FTM_FALSE;
		return	FTM_RET_OK;
	}

   	sprintf(pSQL, "SELECT COUNT(*) FROM %s WHERE ID = \'%llu\'", pTableName, ullID);
   	nRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, FTDM_DBIF_countCB, &xParams, &pErrMsg);
   	if (nRet != SQLITE_OK)
   	{
       	ERROR2(nRet, "%s\n", pErrMsg);
       	sqlite3_free(pErrMsg);

   		return  FTM_RET_ERROR;
   	}

	*pbExist = (xParams.ulCount != 0);

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
typedef struct
{
	FTM_ULONG	ulMaxCount;
	FTM_ULONG	ulCount;
	FTM_LOG_PTR	pLogs;
}	FTDM_DBIF_CB_GET_LOG_LIST_PARAMS, _PTR_ FTDM_DBIF_CB_GET_LOG_LIST_PARAMS_PTR;

FTM_INT	_FTDM_DBIF_LOG_getListCB
(
	FTM_VOID_PTR	pData, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR _PTR_	pArgv, 
	FTM_CHAR_PTR _PTR_ pColName
)
{
	FTDM_DBIF_CB_GET_LOG_LIST_PARAMS_PTR pParams = (FTDM_DBIF_CB_GET_LOG_LIST_PARAMS_PTR)pData;

	if ((nArgc != 0) && (pParams->ulCount < pParams->ulMaxCount))
	{
		FTM_INT	i;

		for(i = 0 ; i < nArgc ; i++)
		{
			if (strcmp(pColName[i], "VALUE") == 0)
			{
				memcpy(&pParams->pLogs[pParams->ulCount++], pArgv[i], sizeof(FTM_LOG));
			}
		}
	}
	return	FTM_RET_OK;
}
			
FTM_RET	FTDM_DBIF_LOG_getList
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_LOG_PTR		pLogs, 
	FTM_ULONG_PTR	pulCount
)
{
    FTM_INT			xRet;
	FTM_INT			nSQLLen;
    FTM_CHAR		pSQL[1024];
    FTM_CHAR_PTR	pErrMsg = NULL;
	FTDM_DBIF_CB_GET_LOG_LIST_PARAMS xParams= 
	{
		.ulMaxCount	= ulMaxCount,
		.ulCount	= 0,
		.pLogs		= pLogs
	};

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

    nSQLLen = sprintf(pSQL, "SELECT VALUE FROM log");
	nSQLLen += sprintf(&pSQL[nSQLLen], " ORDER BY ID DESC LIMIT %lu OFFSET %lu", ulMaxCount, ulIndex);
    xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_LOG_getListCB, &xParams, &pErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR2(xRet, "%s\n", pErrMsg);
        sqlite3_free(pErrMsg);

    	return  FTM_RET_ERROR;
    }

	*pulCount = xParams.ulCount;

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
FTM_RET	FTDM_DBIF_LOG_info
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName,
	FTM_ULONG_PTR	pulCount,
	FTM_ULONG_PTR	pulBeginTime,
	FTM_ULONG_PTR	pulEndTime
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_BOOL		bExist = FTM_FALSE;

	if (pDBIF->pSQLiteDB == NULL)
	{
		ERROR2(FTM_RET_NOT_INITIALIZED, "DB not initialized.\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	xRet = _FTDM_DBIF_isExistTable(pTableName, &bExist);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (bExist != FTM_TRUE)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	xRet = FTDM_DBIF_count(pTableName, pulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	sprintf(pSQL, "SELECT TIME from %s ORDER BY TIME DESC LIMIT 1", pTableName);
	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_EP_DATA_timeCB, pulEndTime, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
        ERROR2(xRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	sprintf(pSQL, "SELECT TIME from %s  ORDER BY TIME ASC LIMIT 1", pTableName);
	xRet = sqlite3_exec(pDBIF->pSQLiteDB, pSQL, _FTDM_DBIF_EP_DATA_timeCB, pulBeginTime, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
        ERROR2(xRet, "%s\n", pErrMsg);
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

/***********************************************************
 *
 ***********************************************************/
typedef struct 
{
	FTM_LOG_PTR	pLogs;
	FTM_INT		nMaxCount;
	FTM_INT		nCount;
}	_FTDM_DBIF_CB_GET_LOG_PARAMS, _PTR_ _FTDM_DBIF_CB_GET_LOG_PARAMS_PTR;

FTM_INT	_FTDM_DBIF_LOG_getCB
(
	FTM_VOID _PTR_ pData, 
	FTM_INT	nArgc, 
	FTM_CHAR _PTR_ _PTR_ pArgv, 
	FTM_CHAR _PTR_ _PTR_ pColName
)
{
	_FTDM_DBIF_CB_GET_LOG_PARAMS_PTR pParams = (_FTDM_DBIF_CB_GET_LOG_PARAMS_PTR)pData;

	if ((nArgc != 0) && (pParams->nCount < pParams->nMaxCount))
	{
		FTM_INT	i;
	
		for(i = 0 ; i < nArgc ; i++)
		{
			if (strcmp(pColName[i], "VALUE") == 0)
			{
				if (pParams->nCount <= pParams->nMaxCount)
				{
					memcpy(&pParams->pLogs[pParams->nCount++], pArgv[i], sizeof(FTM_LOG));
				}
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_LOG_get
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName,
	FTM_ULONG		nStartIndex,
	FTM_LOG_PTR		pLogs,
	FTM_ULONG		nMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	FTM_RET			xRet;

	_FTDM_DBIF_CB_GET_LOG_PARAMS	xParams;

	xParams.pLogs = pLogs;
	xParams.nMaxCount = nMaxCount;
	xParams.nCount = 0;

	xRet = FTDM_DBIF_get(pTableName, nStartIndex, nMaxCount, _FTDM_DBIF_LOG_getCB, &xParams);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	*pulCount = xParams.nCount;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_LOG_getWithTime
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName,
	FTM_ULONG		xBeginTime,
	FTM_ULONG		xEndTime,
	FTM_LOG_PTR		pLogs,
	FTM_ULONG		nMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pTableName != NULL);
	ASSERT(pLogs != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET	xRet;
	_FTDM_DBIF_CB_GET_LOG_PARAMS xParams;
	
	xParams.pLogs = pLogs;
	xParams.nMaxCount= nMaxCount;
	xParams.nCount = 0;

	xRet = 	FTDM_DBIF_getWithTime(pTableName, xBeginTime,xEndTime, FTM_FALSE, nMaxCount, _FTDM_DBIF_LOG_getCB, &xParams);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	*pulCount = xParams.nCount;

	return	FTM_RET_OK;
}

/*************************************************************************
 *
 *************************************************************************/
FTM_RET	FTDM_DBIF_LOG_countWithTime
(
	FTDM_DBIF_PTR	pDBIF,
	FTM_CHAR_PTR	pTableName,
	FTM_ULONG		xBeginTime,
	FTM_ULONG		xEndTime,
	FTM_ULONG_PTR	pulCount
)
{
	return	FTDM_DBIF_countWithTime(pTableName, xBeginTime, xEndTime, pulCount);
}

