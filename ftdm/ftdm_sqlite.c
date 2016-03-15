#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sqlite3.h>
#include <sys/time.h>
#include "ftdm.h"
#include "ftm_trigger.h"
#include "ftdm_sqlite.h"

static FTM_BOOL FTDM_DBIF_EP_DATA_bIOTrace = FTM_FALSE;

FTM_RET _FTDM_DBIF_isExistTable
(
	FTM_CHAR_PTR	pTableName, 
	FTM_BOOL_PTR 	pExist
);

FTM_RET _FTDM_DBIF_NODE_createTable
(
	FTM_CHAR_PTR	pTableName
);

FTM_RET _FTDM_DBIF_isExistNode
(	
	FTM_CHAR_PTR	pTableName, 
	FTM_CHAR_PTR	pDID,
	FTM_BOOL_PTR 	pExist
);

FTM_RET _FTDM_DBIF_EP_createTable
(
	FTM_CHAR_PTR	pTableName
);

static FTM_CHAR_PTR	_strDefaultDBName = "./ftdm.db";

static sqlite3		*_pSQLiteDB= 0;

FTM_RET	FTDM_DBIF_init
(
	FTM_VOID
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_final
(
	void
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_open
(
	FTM_VOID
)
{
	FTM_RET	xRet;
	
	if (_pSQLiteDB != NULL)
	{
		return	FTM_RET_ALREADY_INITIALIZED;	
	}

	xRet = sqlite3_open(_strDefaultDBName, &_pSQLiteDB);
	if ( xRet )
	{
		ERROR("SQL error : %s\n", sqlite3_errmsg(_pSQLiteDB));

		return	(FTM_RET_DBIF_DB_ERROR | xRet); 	
	}

	xRet = FTDM_DBIF_NODE_initTable();
	if (xRet != FTM_RET_OK)
	{
		sqlite3_close(_pSQLiteDB);
		return	xRet;	
	}

	xRet = FTDM_DBIF_EP_initTable();
	if (xRet != FTM_RET_OK)
	{
		sqlite3_close(_pSQLiteDB);
		return	xRet;	
	}

	xRet = FTDM_DBIF_TRIGGER_initTable();
	if (xRet != FTM_RET_OK)
	{
		sqlite3_close(_pSQLiteDB);
		return	xRet;	
	}

	xRet = FTDM_DBIF_ACTION_initTable();
	if (xRet != FTM_RET_OK)
	{
		sqlite3_close(_pSQLiteDB);
		return	xRet;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_close
(
	FTM_VOID
)
{
	if (_pSQLiteDB)
	{
		sqlite3_close(_pSQLiteDB);
		_pSQLiteDB = NULL;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_loadConfig
(
	FTDM_CFG_DB_PTR	pConfig
)
{
	if (_pSQLiteDB)
	{
		return	FTM_RET_DBIF_ALREADY_INITIALIZED;
	}

	if ((pConfig != NULL) && (pConfig->pFileName != NULL))
	{
		_strDefaultDBName = pConfig->pFileName;
	}

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
FTM_RET	FTDM_DBIF_NODE_initTable
(
	void
)
{
	FTM_CHAR_PTR	pTableName = "node_info";
	FTM_BOOL		bExist = FTM_FALSE;

	if (_FTDM_DBIF_isExistTable(pTableName, &bExist) != FTM_RET_OK)
	{
		ERROR("_FTDM_DBIF_isExistTable(%s,bExist)\n", pTableName);  
		return	FTM_RET_DBIF_ERROR;	
	}

	if (bExist != FTM_TRUE)
	{
		ERROR("%s is not exist\n", pTableName);
		if (_FTDM_DBIF_NODE_createTable(pTableName) != FTM_RET_OK)
		{
			return	FTM_RET_DBIF_ERROR;	
		}
	}

	return	FTM_RET_OK;
}

FTM_BOOL	FTDM_DBIF_NODE_isTableExist
(
	FTM_VOID
)
{
	FTM_CHAR_PTR	pTableName = "node_info";
	FTM_BOOL		bExist = FTM_FALSE;

	if (_FTDM_DBIF_isExistTable(pTableName, &bExist) != FTM_RET_OK)
	{
		ERROR("_FTDM_DBIF_isExistTable(%s,bExist)\n", pTableName);  
		return	FTM_FALSE;	
	}

	return	bExist;
}
/***************************************************************
 *
 ***************************************************************/
static int _FTDM_DBIF_NODE_countCB(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTM_ULONG_PTR pnCount = (FTM_ULONG_PTR)pData;

	if (nArgc != 0)
	{
		*pnCount = atoi(pArgv[0]);
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_NODE_count
(
	FTM_ULONG_PTR		pulCount
)
{
    int     xRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

    sprintf(strSQL, "SELECT COUNT(*) FROM node_info");
    xRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_NODE_countCB, pulCount, &strErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTM_RET_ERROR;
    }

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

static int _FTDM_DBIF_NODE_getListCB(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTDM_DBIF_CB_GET_DEVICE_LIST_PARAMS_PTR pParams = (FTDM_DBIF_CB_GET_DEVICE_LIST_PARAMS_PTR)pData;

	if (nArgc != 0)
	{
		FTM_INT	i;
		FTM_NODE_PTR	pNodeInfo = &pParams->pInfos[(*pParams->pulCount)++];

		for(i = 0 ; i < nArgc ; i++)
		{
			if (strcasecmp(pColName[i], "DID") == 0)
			{
				strncpy(pNodeInfo->pDID, pArgv[i], 32);
			}
			else if (strcasecmp(pColName[i], "TYPE") == 0)
			{
				pNodeInfo->xType = strtoul(pArgv[i], 0, 10);
			}
			else if (strcasecmp(pColName[i], "LOC") == 0)
			{
				strncpy(pNodeInfo->pLocation, pArgv[i], FTM_LOCATION_LEN);
			}
			else if (strcasecmp(pColName[i], "INTERVAL") == 0)
			{
				pNodeInfo->ulInterval = strtoul(pArgv[i], NULL, 10);
			}
			else if (strcasecmp(pColName[i], "TIMEOUT") == 0)
			{
				pNodeInfo->ulTimeout = strtoul(pArgv[i], NULL, 10);
			}
			else if (strcasecmp(pColName[i], "OPT0") == 0)
			{
				switch (pNodeInfo->xType)
				{
				case	FTM_NODE_TYPE_SNMP:
					pNodeInfo->xOption.xSNMP.ulVersion = strtoul(pArgv[i], 0, 10);
					break;
				}
			}
			else if (strcasecmp(pColName[i], "OPT1") == 0)
			{
				switch (pNodeInfo->xType)
				{
				case	FTM_NODE_TYPE_SNMP:
					strncpy(pNodeInfo->xOption.xSNMP.pURL, pArgv[i], FTM_URL_LEN);
					break;
				}
			}
			else if (strcasecmp(pColName[i], "OPT2") == 0)
			{
				switch (pNodeInfo->xType)
				{
				case	FTM_NODE_TYPE_SNMP:
					strncpy(pNodeInfo->xOption.xSNMP.pCommunity, pArgv[i], FTM_SNMP_COMMUNITY_LEN);
					break;
				}
			}
			else if (strcasecmp(pColName[i], "OPT3") == 0)
			{
				switch (pNodeInfo->xType)
				{
				case	FTM_NODE_TYPE_SNMP:
					strncpy(pNodeInfo->xOption.xSNMP.pMIB, pArgv[i], FTM_SNMP_MIB_LEN);
					break;
				}
			}
		}
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_NODE_getList
(
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

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "SELECT * FROM node_info");
	xRet = sqlite3_exec(_pSQLiteDB, pSQL, _FTDM_DBIF_NODE_getListCB, &xParams, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
FTM_RET	FTDM_DBIF_NODE_append
(
 	FTM_NODE_PTR	pNodeInfo
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_CHAR		pOpt0[256] = {0,};
	FTM_CHAR		pOpt1[256] = {0,};
	FTM_CHAR		pOpt2[256] = {0,};
	FTM_CHAR		pOpt3[256] = {0,};

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	if (pNodeInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	switch(pNodeInfo->xType)
	{
	case	FTM_NODE_TYPE_SNMP:	
		{	
			sprintf(pOpt0, "%lu", pNodeInfo->xOption.xSNMP.ulVersion);
			sprintf(pOpt1, "%s", pNodeInfo->xOption.xSNMP.pURL);
			sprintf(pOpt2, "%s", pNodeInfo->xOption.xSNMP.pCommunity);
			sprintf(pOpt3, "%s", pNodeInfo->xOption.xSNMP.pMIB);
		}
		break;
	}

	sprintf(pSQL, 
			"INSERT INTO node_info (DID,TYPE,LOC,INTERVAL,TIMEOUT,OPT0,OPT1,OPT2,OPT3) "\
			"VALUES (\'%s\',%lu,\'%s\',%lu,%lu,\'%s\',\'%s\',\'%s\',\'%s\')",
			pNodeInfo->pDID, 
			pNodeInfo->xType, 
			pNodeInfo->pLocation,
			pNodeInfo->ulInterval,
			pNodeInfo->ulTimeout,
			pOpt0,
			pOpt1,
			pOpt2,
			pOpt3);
	xRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
FTM_RET	FTDM_DBIF_NODE_del
(
	FTM_CHAR_PTR		pDID
)
{
	FTM_RET		xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	if (strlen(pDID) > FTM_DID_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	sprintf(pSQL, "DELETE FROM node_info WHERE DID == %s", pDID);
	xRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
static int _FTDM_DBIF_NODE_getCB(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTM_NODE_PTR pInfo = (FTM_NODE_PTR)pData;

	TRACE("%s : %s\n", pColName, pArgv[0]);
	
	if (nArgc != 0)
	{
		if (strcmp(pColName[0], "DID") == 0)
		{
			memcpy(pInfo->pDID, pArgv[0], 32);
		}
		else if (strcmp(pColName[0], "TYPE") == 0)
		{
			pInfo->xType = atoi(pArgv[0]);
		}
		else if (strcmp(pColName[0], "LOC") == 0)
		{
			strncpy(pInfo->pLocation, pArgv[0], FTM_LOCATION_LEN);
		}
		else if (strcmp(pColName[0], "INTERVAL") == 0)
		{
			pInfo->ulInterval = strtoul(pArgv[0], NULL, 10);
		}
		else if (strcmp(pColName[0], "TIMEOUT") == 0)
		{
			pInfo->ulTimeout= strtoul(pArgv[0], NULL, 10);
		}
		else if (strcmp(pColName[0], "OPT0") == 0)
		{
			if (pInfo->xType == FTM_NODE_TYPE_SNMP)
			{
				pInfo->xOption.xSNMP.ulVersion = strtoul(pArgv[0],0, 10);
			}
		}
		else if (strcmp(pColName[0], "OPT1") == 0)
		{
			if (pInfo->xType == FTM_NODE_TYPE_SNMP)
			{
				strncpy(pInfo->xOption.xSNMP.pURL, pArgv[0], FTM_URL_LEN);
			}
		}
		else if (strcmp(pColName[0], "OPT0") == 0)
		{
			if (pInfo->xType == FTM_NODE_TYPE_SNMP)
			{
				strncpy(pInfo->xOption.xSNMP.pCommunity, pArgv[0], FTM_SNMP_COMMUNITY_LEN);
			}
		}
		else if (strcmp(pColName[0], "OPT0") == 0)
		{
			if (pInfo->xType == FTM_NODE_TYPE_SNMP)
			{
				strncpy(pInfo->xOption.xSNMP.pMIB, pArgv[0], FTM_SNMP_MIB_LEN);
			}
		}
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_NODE_get
(
	FTM_CHAR_PTR		pDID, 
	FTM_NODE_PTR	pInfo
)
{
    int     xRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

    sprintf(strSQL, "SELECT * FROM node_info WHERE DID = '%s'", pDID);
    xRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_NODE_getCB, pInfo, &strErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTM_RET_ERROR;
    }

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_NODE_setURL
(
	FTM_CHAR_PTR		pDID, 
	FTM_CHAR_PTR		pURL
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_NODE_getURL
(
	FTM_CHAR_PTR		pDID, 
	FTM_CHAR_PTR		pBuff,
	FTM_ULONG			nBuffLen
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_NODE_setLocation
(
	FTM_CHAR_PTR		pDID, 
	FTM_CHAR_PTR		pLocation
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_NODE_getLocation
(
	FTM_CHAR_PTR		pDID, 
	FTM_CHAR_PTR		pBuff,
	FTM_ULONG			nBuffLen
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_initTable
(
	FTM_VOID
)
{
	FTM_CHAR_PTR	pTableName = "ep_info";
	FTM_BOOL		bExist = FTM_FALSE;

	if (_FTDM_DBIF_isExistTable(pTableName, &bExist) != FTM_RET_OK)
	{
		ERROR("_FTDM_DBIF_isExistTable(%s,bExist)\n", pTableName);  
		return	FTM_RET_DBIF_ERROR;	
	}

	if (bExist != FTM_TRUE)
	{
		if (_FTDM_DBIF_EP_createTable(pTableName) != FTM_RET_OK)
		{
			return	FTM_RET_DBIF_ERROR;	
		}
	}

	return	FTM_RET_OK;
}

FTM_BOOL FTDM_DBIF_EP_isTableExist
(
	FTM_VOID
)
{
	FTM_CHAR_PTR	pTableName = "ep_info";
	FTM_BOOL		bExist = FTM_FALSE;

	if (_FTDM_DBIF_isExistTable(pTableName, &bExist) != FTM_RET_OK)
	{
		ERROR("_FTDM_DBIF_isExistTable(%s,bExist)\n", pTableName);  
		return	FTM_FALSE;	
	}

	return	bExist;
}

FTM_RET	FTDM_DBIF_EP_append
(
 	FTM_EP_PTR		pEPInfo
)
{
	FTM_RET		xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	TRACE("EP[%08x] appended\n", pEPInfo->xEPID);
	sprintf(pSQL, 
			"INSERT INTO ep_info (EPID,DID,DEPID,TYPE,NAME,STATE,INTERVAL,TIMEOUT,UNIT,PID,PEPID) "\
			"VALUES (%lu, \"%s\", %lu, %lu, \"%s\", %d, %lu, %lu, \"%s\", \"%s\", %lu)",
			pEPInfo->xEPID, 
			pEPInfo->pDID, 
			pEPInfo->xDEPID, 
			pEPInfo->xType, 
			pEPInfo->pName, 
			pEPInfo->bEnable,
			pEPInfo->ulInterval, 
			pEPInfo->ulTimeout, 
			pEPInfo->pUnit, 
			pEPInfo->pPID,
			pEPInfo->xPEPID); 
	xRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_del
(
	FTM_EP_ID				xEPID
)
{
	FTM_RET		xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL,
			"DELETE FROM ep_info WHERE EPID == %lu", 
			xEPID);
	xRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
static int _FTDM_DBIF_EP_countCB(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTM_ULONG_PTR pnCount = (FTM_ULONG_PTR)pData;

	if (nArgc != 0)
	{
		*pnCount = atoi(pArgv[0]);
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_count
(
	FTM_ULONG_PTR		pulCount
)
{
    int     xRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

    sprintf(strSQL, "SELECT COUNT(*) FROM ep_info");
    xRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_EP_countCB, pulCount, &strErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTM_RET_ERROR;
    }

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

static int _FTDM_DBIF_EP_getListCB(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTDM_DBIF_CB_GET_EP_LIST_PARAMS_PTR pParams = (FTDM_DBIF_CB_GET_EP_LIST_PARAMS_PTR)pData;

	if (nArgc != 0)
	{
		FTM_INT	i;

		for(i = 0 ; i < nArgc ; i++)
		{
			if (strcmp(pColName[i], "EPID") == 0)
			{
				pParams->nCount++;
				pParams->pInfos[pParams->nCount-1].xEPID = atoi(pArgv[i]);
			}
			else if (strcmp(pColName[i], "TYPE") == 0)
			{
				pParams->pInfos[pParams->nCount-1].xType = atoi(pArgv[i]);
			}
			else if (strcmp(pColName[i], "NAME") == 0)
			{
				strncpy(pParams->pInfos[pParams->nCount-1].pName, pArgv[i], FTM_NAME_LEN);
			}
			else if (strcmp(pColName[i], "UNIT") == 0)
			{
				strncpy(pParams->pInfos[pParams->nCount-1].pUnit, pArgv[i], FTM_UNIT_LEN);
			}
			else if (strcmp(pColName[i], "STATE") == 0)
			{
				pParams->pInfos[pParams->nCount-1].bEnable = atoi(pArgv[i]);
			}
			else if (strcmp(pColName[i], "INTERVAL") == 0)
			{
				pParams->pInfos[pParams->nCount-1].ulInterval = atoi(pArgv[i]);
			}
			else if (strcmp(pColName[i], "TIMEOUT") == 0)
			{
				pParams->pInfos[pParams->nCount-1].ulTimeout = atoi(pArgv[i]);
			}
			else if (strcmp(pColName[i], "DID") == 0)
			{
				strncpy(pParams->pInfos[pParams->nCount-1].pDID, pArgv[i], FTM_DID_LEN);
			}
			else if (strcmp(pColName[i], "DEPID") == 0)
			{
				pParams->pInfos[pParams->nCount-1].xDEPID = atoi(pArgv[i]);
			}
			else if (strcmp(pColName[i], "PID") == 0)
			{
				strncpy(pParams->pInfos[pParams->nCount-1].pPID, pArgv[i], FTM_DID_LEN);
			}
			else if (strcmp(pColName[i], "PEPID") == 0)
			{
				pParams->pInfos[pParams->nCount-1].xPEPID = atoi(pArgv[i]);
			}
		}
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_getList
(
	FTM_EP_PTR		pInfos, 
	FTM_ULONG				nMaxCount,
	FTM_ULONG_PTR			pulCount
)
{
    int     xRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;
	FTDM_DBIF_CB_GET_EP_LIST_PARAMS xParams= 
	{
		.nMaxCount 	= nMaxCount,
		.nCount		= 0,
		.pInfos		= pInfos
	};

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

    sprintf(strSQL, "SELECT * FROM ep_info");
    xRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_EP_getListCB, &xParams, &strErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTM_RET_ERROR;
    }

	*pulCount = xParams.nCount;

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
static int _FTDM_DBIF_EP_getCB(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTM_EP_PTR pInfo = (FTM_EP_PTR)pData;

	if (nArgc != 0)
	{
		if (strcmp(pColName[0], "EPID") == 0)
		{
			pInfo->xEPID = atoi(pArgv[0]);
		}
		else if (strcmp(pColName[0], "TYPE") == 0)
		{
			pInfo->xType = atoi(pArgv[0]);
		}
		else if (strcmp(pColName[0], "NAME") == 0)
		{
			strncpy(pInfo->pName, pArgv[0], FTM_NAME_LEN);
		}
		else if (strcmp(pColName[0], "UNIT") == 0)
		{
			strncpy(pInfo->pUnit, pArgv[0], FTM_UNIT_LEN);
		}
		else if (strcmp(pColName[0], "STATE") == 0)
		{
			pInfo->bEnable = atoi(pArgv[0]);
		}
		else if (strcmp(pColName[0], "DID") == 0)
		{
			strncpy(pInfo->pDID, pArgv[0], FTM_DID_LEN);
		}
		else if (strcmp(pColName[0], "DEPID") == 0)
		{
			pInfo->xDEPID = atoi(pArgv[0]);
		}
		else if (strcmp(pColName[0], "PID") == 0)
		{
			strncpy(pInfo->pPID, pArgv[0], FTM_DID_LEN);
		}
		else if (strcmp(pColName[0], "PEPID") == 0)
		{
			pInfo->xPEPID = atoi(pArgv[0]);
		}
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_get
(
	FTM_EP_ID 				xEPID, 
 	FTM_EP_PTR		pInfo
)
{
    int     xRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

    sprintf(strSQL, "SELECT * FROM ep_info WHERE DID = %lu", xEPID);
    xRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_EP_getCB, pInfo, &strErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTM_RET_ERROR;
    }

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_setName
(
	FTM_EP_ID				xEPID,
	FTM_CHAR_PTR			pName,
	FTM_INT				nNameLen
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_getName
(
	FTM_EP_ID				xEPID,
	FTM_CHAR_PTR			pName,
	FTM_INT_PTR			pNameLen
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_setInterval
(
	FTM_EP_ID				xEPID,
	FTM_ULONG				ulInterval
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_getInterval
(
	FTM_EP_ID				xEPID,
	FTM_ULONG_PTR			pInterval
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_setUnit
(
	FTM_EP_ID				xEPID,
	FTM_CHAR_PTR			pUnit,
	FTM_INT				nUnit
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_getUnit
(
	FTM_EP_ID				xEPID,
	FTM_CHAR_PTR			pUnit,
	FTM_INT_PTR			pUnitLen
)
{
	return	FTM_RET_OK;
}


FTM_RET	FTDM_DBIF_EP_DATA_initTable
(
	FTM_EP_ID	xEPID
)
{
	FTM_RET			xRet;
	FTM_BOOL		bExist = FTM_FALSE;
	FTM_CHAR		pTableName[16];

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pTableName, "ep_%08lx", xEPID);

	xRet = _FTDM_DBIF_isExistTable(pTableName, &bExist);
	if (xRet != FTM_RET_OK)
	{
		ERROR("_FTDM_DBIF_isExistTable(%s,bExist)\n", pTableName);  
		return	xRet;	
	}

	if (bExist == FTM_FALSE)
	{ 
		FTM_RET		xRet;
		FTM_CHAR_PTR	pErrMsg = NULL;
		FTM_CHAR		pSQL[1024];

		sprintf(pSQL, "CREATE TABLE ep_%08lx (ID INT64,TIME INT,STATE INT,VALUE TEXT)", xEPID);

		xRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
		if (xRet != SQLITE_OK)
		{
			ERROR("SQL error : %s\n", pErrMsg);	
			sqlite3_free(pErrMsg);

			return	FTM_RET_ERROR;
		}

		TRACE("Table ep_%08lx creating has done.\n", xEPID);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_DATA_append
(
	FTM_EP_ID				xEPID,
	FTM_EP_DATA_PTR			pData
)
{
	int				xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	char			pSQL[1024];
	struct timeval	tv;

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	gettimeofday(&tv, NULL);

	switch (pData->xType)
	{
	case	FTM_EP_DATA_TYPE_INT:
		{
			if (FTDM_DBIF_EP_DATA_bIOTrace)
			{
				TRACE("INSERT INTO ep_%08lx VALUES (%llu, %lu, %lu, 'i%d')\n", 
						xEPID,
						tv.tv_sec * (long long)1000000 + tv.tv_usec, 
						pData->ulTime, 
						(FTM_ULONG)pData->xState,
						pData->xValue.nValue);
			}

			sprintf(pSQL, "INSERT INTO ep_%08lx VALUES (%llu, %lu, %lu, 'i%d')", 
					xEPID,
					tv.tv_sec * (long long)1000000 + tv.tv_usec, 
					pData->ulTime, 
					(FTM_ULONG)pData->xState,
					pData->xValue.nValue);
		}
		break;

	case	FTM_EP_DATA_TYPE_ULONG:
		{
			if (FTDM_DBIF_EP_DATA_bIOTrace)
			{
				TRACE("INSERT INTO ep_%08lx VALUES (%llu, %lu, %lu, 'u%lu')\n", 
						xEPID,
						tv.tv_sec * (long long)1000000 + tv.tv_usec, 
						pData->ulTime, 
						(FTM_ULONG)pData->xState,
						pData->xValue.ulValue);
			}
			sprintf(pSQL, "INSERT INTO ep_%08lx VALUES (%llu, %lu, %lu, 'u%lu')", 
					xEPID,
					tv.tv_sec * (long long)1000000 + tv.tv_usec, 
					pData->ulTime, 
					(FTM_ULONG)pData->xState,
					pData->xValue.ulValue);
		}
		break;

	case	FTM_EP_DATA_TYPE_FLOAT:
		{
			if (FTDM_DBIF_EP_DATA_bIOTrace)
			{
				TRACE("INSERT INTO ep_%08lx VALUES (%llu, %lu, %lu, 'f%8.3lf')\n", 
						xEPID, 
						tv.tv_sec * (long long)1000000 + tv.tv_usec, 
						pData->ulTime, 
						(FTM_ULONG)pData->xState,
						pData->xValue.fValue);
			}
			sprintf(pSQL, "INSERT INTO ep_%08lx VALUES (%llu, %lu, %lu, 'f%8.3lf')", 
					xEPID, 
					tv.tv_sec * (long long)1000000 + tv.tv_usec, 
					pData->ulTime, 
					(FTM_ULONG)pData->xState,
					pData->xValue.fValue);
		}
		break;

	default:
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xRet = FTDM_DBIF_EP_DATA_initTable(xEPID);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

/********************************************************
 *
 ********************************************************/
static int _FTDM_DBIF_EP_DATA_timeCB(void *pData, int nArgc, char **pArgv, char **pColName)
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
	FTM_EP_ID				xEPID,
	FTM_ULONG_PTR			pulBeginTime,
	FTM_ULONG_PTR			pulEndTime
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "SELECT TIME from ep_%08lx "\
				  " ORDER BY TIME DESC LIMIT 1", xEPID);
	xRet = sqlite3_exec(_pSQLiteDB, pSQL, _FTDM_DBIF_EP_DATA_timeCB, pulEndTime, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	sprintf(pSQL, "SELECT TIME from ep_%08lx "\
				  " ORDER BY TIME ASC LIMIT 1", xEPID);
	xRet = sqlite3_exec(_pSQLiteDB, pSQL, _FTDM_DBIF_EP_DATA_timeCB, pulBeginTime, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

/********************************************************
 *
 ********************************************************/
static int _FTDM_DBIF_EP_DATA_countCB(void *pData, int nArgc, char **pArgv, char **pColName)
{
	*((FTM_ULONG_PTR)pData) = strtoul(pArgv[0], 0, 10);

    return  FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_DATA_count
(
	FTM_EP_ID				xEPID,
	FTM_ULONG_PTR			pulCount
)
{
	FTM_RET		xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "SELECT COUNT(*) from ep_%08lx ", xEPID);
	xRet = sqlite3_exec(_pSQLiteDB, pSQL, _FTDM_DBIF_EP_DATA_countCB, pulCount, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}


FTM_RET	FTDM_DBIF_EP_DATA_CountWithTime
(
	FTM_EP_ID				xEPID,
	FTM_ULONG				xBeginTime,
	FTM_ULONG				xEndTime,
	FTM_ULONG_PTR			pulCount
)
{
	FTM_RET		xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_INT		nSQLLen = 0;
	FTM_BOOL		bConditionOn = FTM_FALSE;

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	nSQLLen = sprintf(pSQL, "SELECT COUNT(*) from ep_%08lx ", xEPID);
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


	xRet = sqlite3_exec(_pSQLiteDB, pSQL, _FTDM_DBIF_EP_DATA_countCB, pulCount, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
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

static int _FTDM_DBIF_EP_DATA_getCB(void *pData, int nArgc, char **pArgv, char **pColName)
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
						pParams->pEPData[pParams->nCount-1].xType = FTM_EP_DATA_TYPE_INT;
						pParams->pEPData[pParams->nCount-1].xValue.nValue = strtol(&pArgv[i][1], NULL, 10);
					}
					else if (pArgv[i][0] == 'u')
					{
						pParams->pEPData[pParams->nCount-1].xType = FTM_EP_DATA_TYPE_ULONG;
						pParams->pEPData[pParams->nCount-1].xValue.ulValue = strtoul(&pArgv[i][1], NULL, 10);
					}
					else if (pArgv[i][0] == 'f')
					{
						pParams->pEPData[pParams->nCount-1].xType = FTM_EP_DATA_TYPE_FLOAT;
						pParams->pEPData[pParams->nCount-1].xValue.fValue = strtod(&pArgv[i][1], NULL);
					}
				}
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_DATA_get
(
	FTM_EP_ID			xEPID,
	FTM_ULONG			nStartIndex,
	FTM_EP_DATA_PTR		pEPData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pulCount
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_INT			nSQLLen = 0;

	_FTDM_DBIF_CB_GET_EP_DATA_PARAMS	xParams;

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}


	if (nMaxCount == 0)
	{
		nMaxCount = 100;	
	}
	
	nSQLLen =  sprintf(pSQL,"SELECT * FROM ep_%08lx ", xEPID);
	nSQLLen += sprintf(&pSQL[nSQLLen], " ORDER BY TIME DESC LIMIT %lu OFFSET %lu", nMaxCount, nStartIndex);

	xParams.pEPData = pEPData;
	xParams.nMaxCount = nMaxCount;
	xParams.nCount = 0;

	xRet = sqlite3_exec(_pSQLiteDB, pSQL, _FTDM_DBIF_EP_DATA_getCB, &xParams, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	*pulCount = xParams.nCount;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_DATA_getWithTime
(
	FTM_EP_ID			xEPID,
	FTM_ULONG			xBeginTime,
	FTM_ULONG			xEndTime,
	FTM_EP_DATA_PTR		pEPData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pulCount
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_CHAR		pLimit[512];
	FTM_INT			nSQLLen = 0;
	FTM_INT			nLimitLen = 0;

	_FTDM_DBIF_CB_GET_EP_DATA_PARAMS	xParams;

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	if (nMaxCount == 0)
	{
		nMaxCount = 100;	
	}
	
	if (xBeginTime != 0)
	{
		if (nLimitLen != 0)
		{
			nLimitLen += sprintf(&pLimit[nLimitLen], " AND");
		}

		nLimitLen += sprintf(&pLimit[nLimitLen], " (TIME >= %lu)", xBeginTime);
	}

	if (xEndTime != 0)
	{
		if (nLimitLen != 0)
		{
			nLimitLen += sprintf(&pLimit[nLimitLen], " AND");
		}

		nLimitLen += sprintf(&pLimit[nLimitLen], " (TIME <= %lu)", xEndTime);
	}

	if (nLimitLen != 0)
	{
		nSQLLen =  sprintf(pSQL,"SELECT * FROM ep_%08lx WHERE %s", xEPID, pLimit);
	}
	else
	{
		nSQLLen =  sprintf(pSQL,"SELECT * FROM ep_%08lx", xEPID);
	}

	nSQLLen += sprintf(&pSQL[nSQLLen], " ORDER BY TIME DESC LIMIT %lu", nMaxCount);
	xParams.pEPData = pEPData;
	xParams.nMaxCount = nMaxCount;
	xParams.nCount = 0;
	TRACE("SQL : %s\n", pSQL);
	xRet = sqlite3_exec(_pSQLiteDB, pSQL, _FTDM_DBIF_EP_DATA_getCB, &xParams, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	*pulCount = xParams.nCount;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_DATA_del
(
	FTM_EP_ID				xEPID,
	FTM_ULONG				nIndex,
	FTM_ULONG				nCount
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_INT			nSQLLen = 0;

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	nSQLLen += sprintf(pSQL, "DELETE FROM ep_%08lx 	WHERE ROWID >= nIndex ", xEPID);

	xRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EP_DATA_delWithTime
(
	FTM_EP_ID				xEPID,
	FTM_ULONG				xBeginTime,
	FTM_ULONG				xEndTime
)
{
	FTM_RET			xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_INT			nSQLLen = 0;
	FTM_BOOL		bConditionOn = FTM_FALSE;

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	nSQLLen += sprintf(pSQL, "DELETE FROM ep_%08lx ", xEPID);
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


	MESSAGE("SQL : %s\n", pSQL);
	xRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}


/*************************************************************************
 *
 *************************************************************************/
FTM_RET	FTDM_DBIF_EP_DATA_countWithTime
(
	FTM_EP_ID			xEPID,
	FTM_ULONG			xBeginTime,
	FTM_ULONG			xEndTime,
	FTM_ULONG_PTR		pulCount
)
{
    int     xRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	if (xBeginTime == 0)
	{
		if (xEndTime == 0)
		{
    		sprintf(strSQL, 
					"SELECT COUNT(*) FROM ep_%08lx", xEPID);
		}
		else
		{
    		sprintf(strSQL, 
					"SELECT COUNT(*) FROM ep_%08lx WHERE (TIME <= %lu)", xEPID, xEndTime);
		}
	}
	else
	{
		if (xEndTime == 0)
		{
    		sprintf(strSQL, 
					"SELECT COUNT(*) FROM ep_%08lx WHERE (%lu <= TIME)", 
					xEPID, xBeginTime);
		}
		else
		{
    		sprintf(strSQL, 
					"SELECT COUNT(*) FROM ep_%08lx WHERE (%lu <= TIME) AND (TIME <= %lu)", 
					xEPID, xBeginTime, xEndTime);
		}
	
	}
    xRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_NODE_countCB, pulCount, &strErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTM_RET_ERROR;
    }

	return	FTM_RET_OK;
}

/*************************************************************************/
typedef struct
{
	FTM_BOOL		bExist;
	FTM_CHAR_PTR	pName;
}   _FTDM_DBIF_CB_EXIST_TABLE_PARAMS, *  _FTDM_DBIF_CB_EXIST_TABLE_PARAMS_PTR;

static int _FTDM_DBIF_CB_isExistTable(void *pData, int nArgc, char **pArgv, char **pColName)
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
	FTM_CHAR_PTR	pTableName, 
	FTM_BOOL_PTR 	pExist
)
{
    int     xRet;
    _FTDM_DBIF_CB_EXIST_TABLE_PARAMS xParams = { .bExist = FTM_FALSE, .pName = pTableName};
    FTM_CHAR_PTR	pSQL = "select name from sqlite_master where type='table' order by name";
    FTM_CHAR_PTR	pErrMsg = NULL;

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

    xRet = sqlite3_exec(_pSQLiteDB, pSQL, _FTDM_DBIF_CB_isExistTable, &xParams, &pErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", pErrMsg);
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

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "CREATE TABLE %s ("\
						"DID		TEXT PRIMARY KEY,"\
						"TYPE		INT,"\
						"LOC		TEXT,"\
						"INTERVAL	INT,"\
						"TIMEOUT	INT,"\
						"OPT0		TEXT,"\
						"OPT1		TEXT,"\
						"OPT2		TEXT,"\
						"OPT3		TEXT)", pTableName);

	xRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
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

static int _FTDM_DBIF_CB_isExist(void *pData, int nArgc, char **pArgv, char **pColName)
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

FTM_RET _FTDM_DBIF_isExistNode
(
	FTM_CHAR_PTR	pTableName, 
	FTM_CHAR_PTR 	pDID, 
	FTM_BOOL_PTR pExist
)
{
	_FTDM_DBIF_CB_IS_EXIST_PARAMS xParams = {.bExist = FTM_TRUE };
    int     xRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

    sprintf(strSQL, "SELECT COUNT(DID) FROM %s WHERE DID = '%s'", pTableName, pDID);
    xRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_CB_isExist, &xParams, &strErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTM_RET_ERROR;
    }

    *pExist = xParams.bExist;

    return  FTM_RET_OK;
}

/*******************************************************
 *
 *******************************************************/
FTM_RET	_FTDM_DBIF_EP_createTable
(
	FTM_CHAR_PTR	pTableName
)
{
	int	xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	char			pSQL[1024];

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "CREATE TABLE %s ("\
						"EPID	INTEGER PRIMARY KEY,"\
						"DID	TEXT,"\
						"DEPID	INTEGER,"\
						"TYPE	INT,"\
						"NAME	TEXT,"\
						"STATE	INT,"\
						"INTERVAL INT,"\
						"TIMEOUT INT,"\
						"UNIT	TEXT,"\
						"PID	TEXT,"\
						"PEPID	INTEGER)", pTableName);

	xRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}


FTM_RET FTDM_DBIF_setTrace
(
	FTM_BOOL			bTraceON
)
{
	FTDM_DBIF_EP_DATA_bIOTrace = bTraceON;

	return	FTM_RET_OK;
}

FTM_RET FTDM_DBIF_getTrace
(	
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
	FTM_VOID
)
{
	FTM_INT			xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_CHAR_PTR	pTableName = "event";
	FTM_BOOL		bExist = FTM_FALSE;

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	if (_FTDM_DBIF_isExistTable(pTableName, &bExist) != FTM_RET_OK)
	{
		ERROR("_FTDM_DBIF_isExistTable(%s,bExist)\n", pTableName);  
		return	FTM_RET_DBIF_ERROR;	
	}

	if (bExist != FTM_TRUE)
	{
		ERROR("%s is not exist\n", pTableName);
		sprintf(pSQL, "CREATE TABLE %s ("\
							"ID			TEXT PRIMARY KEY,"\
							"TYPE	INT,"\
							"EPID	INT,"\
							"VALUE		BLOB)", pTableName);

		xRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
		if (xRet != SQLITE_OK)
		{
			ERROR("SQL error : %s\n", pErrMsg);	
			sqlite3_free(pErrMsg);

			return	FTM_RET_DBIF_ERROR;	
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_TRIGGER_append
(
 	FTM_TRIGGER_PTR	pTrigger
)
{
	ASSERT(pTrigger != NULL);

	FTM_INT			nRC;
	sqlite3_stmt 	*pStmt;
	FTM_CHAR		pSQL[1024];
	
	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	sprintf(pSQL, "INSERT INTO event (ID,TYPE,EPID,VALUE) VALUES (%08lx,%u,%lu,?)", pTrigger->xID, pTrigger->xType, pTrigger->xEPID);

	do 
	{
		nRC = sqlite3_prepare(_pSQLiteDB, pSQL, -1, &pStmt, 0);
		if( nRC!=SQLITE_OK )
		{
			return FTM_RET_ERROR;
		}

		sqlite3_bind_blob(pStmt, 2, pTrigger, sizeof(FTM_TRIGGER), SQLITE_STATIC);

		nRC = sqlite3_step(pStmt);
		ASSERT( nRC != SQLITE_ROW);

		nRC = sqlite3_finalize(pStmt);
	}  while (nRC == SQLITE_SCHEMA);

	return FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
static int _FTDM_DBIF_TRIGGER_getCB(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTM_TRIGGER_PTR	pTrigger = (FTM_TRIGGER_PTR)pData;

	TRACE("%s : %s\n", pColName, pArgv[0]);
	
	if (nArgc != 0)
	{
		if (strcmp(pColName[0], "ID") == 0)
		{
			pTrigger->xID = strtoul(pArgv[0], 0, 16);
		}
		else if (strcmp(pColName[0], "TYPE") == 0)
		{
			pTrigger->xType = strtoul(pArgv[0], 0, 10);
		}
		else if (strcmp(pColName[0], "EPID") == 0)
		{
			pTrigger->xEPID = strtoul(pArgv[0], 0, 16);
		}
		else if (strcmp(pColName[0], "VALUE") == 0)
		{
			memcpy(&pTrigger->xParams, pArgv[0], sizeof(pTrigger->xParams));
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_TRIGGER_get
(
	FTM_TRIGGER_ID	xID,
 	FTM_TRIGGER_PTR	pTrigger
)
{
    FTM_INT			xRet;
    FTM_CHAR		strSQL[1024];
    FTM_CHAR_PTR	strErrMsg = NULL;

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

    sprintf(strSQL, "SELECT * FROM event WHERE ID = '%08lx'", xID);
    xRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_TRIGGER_getCB, pTrigger, &strErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTM_RET_ERROR;
    }

	return	FTM_RET_OK;
}

typedef struct
{
	FTM_ULONG		ulMaxCount;
	FTM_ULONG		ulCount;
	FTM_TRIGGER_PTR	pTriggers;
}	FTDM_DBIF_CB_GET_TRIGGER_LIST_PARAMS, _PTR_ FTDM_DBIF_CB_GET_TRIGGER_LIST_PARAMS_PTR;

static int _FTDM_DBIF_TRIGGER_getListCB(void *pData, int nArgc, char **pArgv, char **pColName)
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
				pTrigger->xID = strtoul(pArgv[i], 0, 16);
			}
			else if (strcasecmp(pColName[i], "TYPE") == 0)
			{
				pTrigger->xType = strtoul(pArgv[i], 0, 10);
			}
			else if (strcasecmp(pColName[i], "EPID") == 0)
			{
				pTrigger->xEPID = strtoul(pArgv[i], 0, 16);
			}
			else if (strcasecmp(pColName[i], "VALUE") == 0)
			{
				memcpy(&pTrigger->xParams, pArgv[i], sizeof(pTrigger->xParams));
			}
		}
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_TRIGGER_getList
(
	FTM_TRIGGER_PTR		pTriggers, 
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pulCount
)
{
    int     xRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;
	FTDM_DBIF_CB_GET_TRIGGER_LIST_PARAMS xParams= 
	{
		.ulMaxCount = nMaxCount,
		.ulCount	= 0,
		.pTriggers	= pTriggers
	};

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

    sprintf(strSQL, "SELECT * FROM event");
    xRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_TRIGGER_getListCB, &xParams, &strErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTM_RET_ERROR;
    }

	*pulCount = xParams.ulCount;

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
FTM_RET	FTDM_DBIF_ACTION_initTable
(
	FTM_VOID
)
{
	FTM_INT			xRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_CHAR_PTR	pTableName = "event";
	FTM_BOOL		bExist = FTM_FALSE;

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	if (_FTDM_DBIF_isExistTable(pTableName, &bExist) != FTM_RET_OK)
	{
		ERROR("%s table check error.\n", pTableName);  
		return	FTM_RET_DBIF_ERROR;	
	}

	if (bExist != FTM_TRUE)
	{
		ERROR("%s is not exist\n", pTableName);
		sprintf(pSQL, "CREATE TABLE %s ("\
							"ID		TEXT PRIMARY KEY,"\
							"TYPE	INT,"\
							"EPID	TEXT)", pTableName);

		xRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
		if (xRet != SQLITE_OK)
		{
			ERROR("SQL error : %s\n", pErrMsg);	
			sqlite3_free(pErrMsg);

			return	FTM_RET_DBIF_ERROR;	
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_ACTION_append
(
 	FTM_ACTION_PTR	pAction
)
{
	ASSERT(pAction != NULL);

	FTM_INT			xRet;
	FTM_CHAR		pSQL[1024];
	FTM_CHAR_PTR	pErrMsg = NULL;

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	switch(pAction->xType)
	{
	case	FTM_ACTION_TYPE_SET:
		{
			sprintf(pSQL, "INSERT INTO event (ID,TYPE,EPID) VALUES (%08lx,%u,%08lx)", pAction->xID, pAction->xType, pAction->xParams.xSet.xEPID);
		}
		break;
	}

	xRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (xRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
static int _FTDM_DBIF_ACTION_getCB(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTM_ACTION_PTR	pAction = (FTM_ACTION_PTR)pData;

	TRACE("%s : %s\n", pColName, pArgv[0]);
	
	if (nArgc != 0)
	{
		if (strcmp(pColName[0], "ID") == 0)
		{
			pAction->xID = strtoul(pArgv[0], 0, 16);
		}
		else if (strcmp(pColName[0], "TYPE") == 0)
		{
			pAction->xType = strtoul(pArgv[0], 0, 10);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_ACTION_get
(
	FTM_ACTION_ID	xID,
 	FTM_ACTION_PTR	pAction
)
{
    FTM_INT			xRet;
    FTM_CHAR		strSQL[1024];
    FTM_CHAR_PTR	strErrMsg = NULL;

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

    sprintf(strSQL, "SELECT * FROM event WHERE ID = '%08lx'", xID);
    xRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_ACTION_getCB, pAction, &strErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTM_RET_ERROR;
    }

	return	FTM_RET_OK;
}

typedef struct
{
	FTM_ULONG		ulMaxCount;
	FTM_ULONG		ulCount;
	FTM_ACTION_PTR	pActions;
}	FTDM_DBIF_CB_GET_ACTION_LIST_PARAMS, _PTR_ FTDM_DBIF_CB_GET_ACTION_LIST_PARAMS_PTR;

static int _FTDM_DBIF_ACTION_getListCB(void *pData, int nArgc, char **pArgv, char **pColName)
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
				pAction->xID = strtoul(pArgv[i], 0, 16);
			}
			else if (strcasecmp(pColName[i], "TYPE") == 0)
			{
				pAction->xType = strtoul(pArgv[i], 0, 10);
			}
		}
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_ACTION_getList
(
	FTM_ACTION_PTR		pActions, 
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pulCount
)
{
    int     xRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;
	FTDM_DBIF_CB_GET_ACTION_LIST_PARAMS xParams= 
	{
		.ulMaxCount = nMaxCount,
		.ulCount	= 0,
		.pActions	= pActions
	};

	if (_pSQLiteDB == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

    sprintf(strSQL, "SELECT * FROM event");
    xRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_ACTION_getListCB, &xParams, &strErrMsg);
    if (xRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTM_RET_ERROR;
    }

	*pulCount = xParams.ulCount;

	return	FTM_RET_OK;
}

