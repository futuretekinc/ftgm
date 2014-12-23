#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "ftdm.h"
#include "sqlite_if.h"

FTDM_RET _FTDM_BDIF_isExistTable
(
	FTDM_CHAR_PTR	pTableName, 
	FTDM_BOOL_PTR 	pExist
);

FTDM_RET _FTDM_BDIF_createDeviceInfoTable
(
	FTDM_CHAR_PTR	pTableName
);

FTDM_RET _FTDM_DBIF_isExistDevice
(	
	FTDM_CHAR_PTR	pTableName, 
	FTDM_CHAR_PTR	pDID,
	FTDM_BOOL_PTR 	pExist
);

FTDM_RET _FTDM_BDIF_createEPInfoTable
(
	FTDM_CHAR_PTR	pTableName
);

static FTDM_CHAR_PTR	_strDefaultDBName = "./ftdm.db";

static sqlite3		*_pSQLiteDB= 0;

FTDM_RET	FTDM_DBIF_init
(
	void
)
{
	int	nRet;

	if (_pSQLiteDB)
	{
		return	FTDM_RET_DBIF_ALREADY_INITIALIZED;
	}

	nRet = sqlite3_open(_strDefaultDBName, &_pSQLiteDB);
	if ( nRet )
	{
		ERROR("SQL error : %s\n", sqlite3_errmsg(_pSQLiteDB));

		return	(FTDM_RET_DBIF_DB_ERROR | nRet); 	
	}

	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_final
(
	void
)
{
	if (_pSQLiteDB)
	{
		sqlite3_close(_pSQLiteDB);
		_pSQLiteDB = NULL;
	}

	return	FTDM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
FTDM_RET	FTDM_DBIF_initDeviceInfoTable
(
	void
)
{
	FTDM_CHAR_PTR	pTableName = "device_info";
	FTDM_BOOL		bExist = FTDM_BOOL_FALSE;

	if (_FTDM_BDIF_isExistTable(pTableName, &bExist) != FTDM_RET_OK)
	{
		ERROR("_FTDM_BDIF_isExistTable(%s,bExist)\n", pTableName);  
		return	FTDM_RET_DBIF_ERROR;	
	}

	if (bExist != FTDM_BOOL_TRUE)
	{
		ERROR("%s is not exist\n", pTableName);
		if (_FTDM_BDIF_createDeviceInfoTable(pTableName) != FTDM_RET_OK)
		{
			return	FTDM_RET_DBIF_ERROR;	
		}
	}

	return	FTDM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
static int _FTDM_DBIF_CB_getDeviceCount(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTDM_ULONG_PTR pnCount = (FTDM_ULONG_PTR)pData;

	if (nArgc != 0)
	{
		*pnCount = atoi(pArgv[0]);
	}
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_getDeviceCount
(
	FTDM_ULONG_PTR		pCount
)
{
    int     nRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;

    sprintf(strSQL, "SELECT COUNT(*) FROM device_info");
    nRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_CB_getDeviceCount, pCount, &strErrMsg);
    if (nRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTDM_RET_ERROR;
    }

	return	FTDM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
typedef struct
{
	FTDM_ULONG				nMaxCount;
	FTDM_ULONG				nCount;
	FTDM_DEVICE_INFO_PTR	pInfos;
}	FTDM_DBIF_CB_GET_DEVICE_LIST_PARAMS, _PTR_ FTDM_DBIF_CB_GET_DEVICE_LIST_PARAMS_PTR;

static int _FTDM_DBIF_CB_getDeviceList(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTDM_DBIF_CB_GET_DEVICE_LIST_PARAMS_PTR pParams = (FTDM_DBIF_CB_GET_DEVICE_LIST_PARAMS_PTR)pData;

	if (nArgc != 0)
	{
		FTDM_INT	i;

		for(i = 0 ; i < nArgc ; i++)
		{
			if (strcmp(pColName[i], "DID") == 0)
			{
				pParams->nCount++;
				strncpy(pParams->pInfos[pParams->nCount-1].pDID, pArgv[i], 32);
			}
			else if (strcmp(pColName[i], "TYPE") == 0)
			{
				pParams->pInfos[pParams->nCount-1].xType = atoi(pArgv[i]);
			}
			else if (strcmp(pColName[i], "URL") == 0)
			{
				strncpy(pParams->pInfos[pParams->nCount-1].pURL, pArgv[i], FTDM_DEVICE_URL_LEN);
			}
			else if (strcmp(pColName[i], "LOC") == 0)
			{
				strncpy(pParams->pInfos[pParams->nCount-1].pLocation, pArgv[i], FTDM_DEVICE_LOCATION_LEN);
			}
		}
	}
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_getDeviceList
(
	FTDM_DEVICE_INFO_PTR	pInfos, 
	FTDM_ULONG				nMaxCount,
	FTDM_ULONG_PTR			pCount
)
{
    int     nRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;
	FTDM_DBIF_CB_GET_DEVICE_LIST_PARAMS xParams= 
	{
		.nMaxCount 	= nMaxCount,
		.nCount		= 0,
		.pInfos		= pInfos
	};

    sprintf(strSQL, "SELECT * FROM device_info");
    nRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_CB_getDeviceList, &xParams, &strErrMsg);
    if (nRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTDM_RET_ERROR;
    }

	*pCount = xParams.nCount;

	return	FTDM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
FTDM_RET	FTDM_DBIF_insertDeviceInfo
(
 	FTDM_DEVICE_INFO_PTR	pInfo
)
{
	FTDM_RET		nRet;
	FTDM_CHAR_PTR	pErrMsg = NULL;
	FTDM_CHAR		pSQL[1024];
	
	sprintf(pSQL, 
			"INSERT INTO device_info VALUES (\"%s\", %08lx, \"%s\", \"%s\")", 
			pInfo->pDID, 
			pInfo->xType, 
			pInfo->pURL, 
			pInfo->pLocation);
	nRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (nRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTDM_RET_ERROR;
	}

	return	FTDM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
FTDM_RET	FTDM_DBIF_removeDeviceInfo
(
	FTDM_CHAR_PTR		pDID
)
{
	FTDM_RET		nRet;
	FTDM_CHAR_PTR	pErrMsg = NULL;
	FTDM_CHAR		pSQL[1024];

	if (strlen(pDID) > FTDM_DEVICE_ID_LEN)
	{
		return	FTDM_RET_INVALID_ARGUMENTS;	
	}

	sprintf(pSQL, "DELETE FROM device_info WHERE DID == %s", pDID);
	nRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (nRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTDM_RET_ERROR;
	}

	return	FTDM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
static int _FTDM_DBIF_CB_getDeviceInfo(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTDM_DEVICE_INFO_PTR pInfo = (FTDM_DEVICE_INFO_PTR)pData;

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
		if (strcmp(pColName[0], "URL") == 0)
		{
			strncpy(pInfo->pURL, pArgv[0], FTDM_DEVICE_URL_LEN);
		}
		if (strcmp(pColName[0], "LOC") == 0)
		{
			strncpy(pInfo->pLocation, pArgv[0], FTDM_DEVICE_LOCATION_LEN);
		}
	}
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_getDeviceInfo
(
	FTDM_CHAR_PTR			pDID, 
	FTDM_DEVICE_INFO_PTR	pInfo
)
{
    int     nRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;

    sprintf(strSQL, "SELECT * FROM device_info WHERE DID = '%s'", pDID);
    nRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_CB_getDeviceInfo, pInfo, &strErrMsg);
    if (nRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTDM_RET_ERROR;
    }

	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_setDeviceURL
(
	FTDM_CHAR_PTR		pDID, 
	FTDM_CHAR_PTR		pURL
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_getDeviceURL
(
	FTDM_CHAR_PTR		pDID, 
	FTDM_CHAR_PTR		pBuff,
	FTDM_ULONG			nBuffLen
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_setDeviceLocation
(
	FTDM_CHAR_PTR		pDID, 
	FTDM_CHAR_PTR		pLocation
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_getDeviceLocation
(
	FTDM_CHAR_PTR		pDID, 
	FTDM_CHAR_PTR		pBuff,
	FTDM_ULONG			nBuffLen
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_initEPInfoTable
(
	void
)
{
	FTDM_CHAR_PTR	pTableName = "ep_info";
	FTDM_BOOL		bExist = FTDM_BOOL_FALSE;

	if (_FTDM_BDIF_isExistTable(pTableName, &bExist) != FTDM_RET_OK)
	{
		ERROR("_FTDM_BDIF_isExistTable(%s,bExist)\n", pTableName);  
		return	FTDM_RET_DBIF_ERROR;	
	}

	if (bExist != FTDM_BOOL_TRUE)
	{
		if (_FTDM_BDIF_createEPInfoTable(pTableName) != FTDM_RET_OK)
		{
			return	FTDM_RET_DBIF_ERROR;	
		}
	}

	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_insertEPInfo
(
 	FTDM_EP_INFO_PTR		pEPInfo
)
{
	FTDM_RET		nRet;
	FTDM_CHAR_PTR	pErrMsg = NULL;
	FTDM_CHAR		pSQL[1024];

	sprintf(pSQL, 
			"INSERT INTO ep_info (EPID,DID,TYPE,NAME,INTERVAL,UNIT,PID) "\
			"VALUES (%lu, \"%s\", %lu, \"%s\", %lu, \"%s\", \"%s\")",
			pEPInfo->xEPID, 
			pEPInfo->pDID, 
			pEPInfo->xType, 
			pEPInfo->pName, 
			pEPInfo->nInterval, 
			pEPInfo->pUnit, 
			pEPInfo->pPID);
	nRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (nRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTDM_RET_ERROR;
	}

	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_removeEPInfo
(
	FTDM_EP_ID				xEPID
)
{
	FTDM_RET		nRet;
	FTDM_CHAR_PTR	pErrMsg = NULL;
	FTDM_CHAR		pSQL[1024];

	sprintf(pSQL,
			"DELETE FROM ep_info WHERE EPID == %lu", 
			xEPID);
	nRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (nRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTDM_RET_ERROR;
	}

	return	FTDM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
static int _FTDM_DBIF_CB_getEPCount(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTDM_ULONG_PTR pnCount = (FTDM_ULONG_PTR)pData;

	if (nArgc != 0)
	{
		*pnCount = atoi(pArgv[0]);
	}
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_getEPCount
(
	FTDM_ULONG_PTR		pCount
)
{
    int     nRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;

    sprintf(strSQL, "SELECT COUNT(*) FROM ep_info");
    nRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_CB_getEPCount, pCount, &strErrMsg);
    if (nRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTDM_RET_ERROR;
    }

	return	FTDM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
typedef struct
{
	FTDM_ULONG			nMaxCount;
	FTDM_ULONG			nCount;
	FTDM_EP_INFO_PTR	pInfos;
}	FTDM_DBIF_CB_GET_EP_LIST_PARAMS, _PTR_ FTDM_DBIF_CB_GET_EP_LIST_PARAMS_PTR;

static int _FTDM_DBIF_CB_getEPList(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTDM_DBIF_CB_GET_EP_LIST_PARAMS_PTR pParams = (FTDM_DBIF_CB_GET_EP_LIST_PARAMS_PTR)pData;

	if (nArgc != 0)
	{
		FTDM_INT	i;

		for(i = 0 ; i < nArgc ; i++)
		{
			printf("%s : %s\n", pColName[i], pArgv[i]);
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
				strncpy(pParams->pInfos[pParams->nCount-1].pName, pArgv[i], FTDM_EP_NAME_LEN);
			}
			else if (strcmp(pColName[i], "UNIT") == 0)
			{
				strncpy(pParams->pInfos[pParams->nCount-1].pUnit, pArgv[i], FTDM_EP_UNIT_LEN);
			}
			else if (strcmp(pColName[i], "INTERVAL") == 0)
			{
				pParams->pInfos[pParams->nCount-1].nInterval = atoi(pArgv[i]);
			}
			else if (strcmp(pColName[i], "DID") == 0)
			{
				strncpy(pParams->pInfos[pParams->nCount-1].pDID, pArgv[i], FTDM_DEVICE_ID_LEN);
			}
			else if (strcmp(pColName[i], "PID") == 0)
			{
				strncpy(pParams->pInfos[pParams->nCount-1].pPID, pArgv[i], FTDM_DEVICE_ID_LEN);
			}
		}
	}
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_getEPList
(
	FTDM_EP_INFO_PTR		pInfos, 
	FTDM_ULONG				nMaxCount,
	FTDM_ULONG_PTR			pCount
)
{
    int     nRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;
	FTDM_DBIF_CB_GET_EP_LIST_PARAMS xParams= 
	{
		.nMaxCount 	= nMaxCount,
		.nCount		= 0,
		.pInfos		= pInfos
	};

    sprintf(strSQL, "SELECT * FROM ep_info");
    nRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_CB_getEPList, &xParams, &strErrMsg);
    if (nRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTDM_RET_ERROR;
    }

	*pCount = xParams.nCount;

	return	FTDM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
static int _FTDM_DBIF_CB_getEPInfo(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTDM_EP_INFO_PTR pInfo = (FTDM_EP_INFO_PTR)pData;

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
		if (strcmp(pColName[0], "NAME") == 0)
		{
			strncpy(pInfo->pName, pArgv[0], FTDM_EP_NAME_LEN);
		}
		if (strcmp(pColName[0], "UNIT") == 0)
		{
			strncpy(pInfo->pUnit, pArgv[0], FTDM_EP_UNIT_LEN);
		}
		if (strcmp(pColName[0], "DID") == 0)
		{
			strncpy(pInfo->pDID, pArgv[0], FTDM_DEVICE_ID_LEN);
		}
		if (strcmp(pColName[0], "PID") == 0)
		{
			strncpy(pInfo->pPID, pArgv[0], FTDM_DEVICE_ID_LEN);
		}
	}
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_getEPInfo
(
	FTDM_EP_ID 				xEPID, 
 	FTDM_EP_INFO_PTR		pInfo
)
{
    int     nRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;

    sprintf(strSQL, "SELECT * FROM ep_info WHERE DID = %lu", xEPID);
    nRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_CB_getEPInfo, pInfo, &strErrMsg);
    if (nRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTDM_RET_ERROR;
    }

	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_setEPName
(
	FTDM_EP_ID				xEPID,
	FTDM_CHAR_PTR			pName,
	FTDM_INT				nNameLen
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_getEPName
(
	FTDM_EP_ID				xEPID,
	FTDM_CHAR_PTR			pName,
	FTDM_INT_PTR			pNameLen
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_setEPInterval
(
	FTDM_EP_ID				xEPID,
	FTDM_ULONG				nInterval
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_getEPInterval
(
	FTDM_EP_ID				xEPID,
	FTDM_ULONG_PTR			pInterval
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_setEPUnit
(
	FTDM_EP_ID				xEPID,
	FTDM_CHAR_PTR			pUnit,
	FTDM_INT				nUnit
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_getEPUnit
(
	FTDM_EP_ID				xEPID,
	FTDM_CHAR_PTR			pUnit,
	FTDM_INT_PTR			pUnitLen
)
{
	return	FTDM_RET_OK;
}


FTDM_RET	FTDM_DBIF_initEPDataTable
(
	void
)
{
	FTDM_CHAR_PTR	pTableName = "ep_info";
	FTDM_BOOL		bExist = FTDM_BOOL_FALSE;

	if (_FTDM_BDIF_isExistTable(pTableName, &bExist) != FTDM_RET_OK)
	{
		ERROR("_FTDM_BDIF_isExistTable(%s,bExist)\n", pTableName);  
		return	FTDM_RET_DBIF_ERROR;	
	}

	if (bExist == FTDM_BOOL_FALSE)
	{ 
		FTDM_RET		nRet;
		FTDM_CHAR_PTR	pErrMsg = NULL;
		FTDM_CHAR		pSQL[1024];

		sprintf(pSQL, "CREATE TABLE ep_data (TIME	INT,EPID INT,VALUE	INT)");

		nRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
		if (nRet != SQLITE_OK)
		{
			ERROR("SQL error : %s\n", pErrMsg);	
			sqlite3_free(pErrMsg);

			return	FTDM_RET_ERROR;
		}
	}

	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_appendEPData
(
	FTDM_ULONG				xTime,
	FTDM_EP_ID				xEPID,
	FTDM_ULONG				nValue
)
{
	int				nRet;
	FTDM_CHAR_PTR	pErrMsg = NULL;
	char			pSQL[1024];

	sprintf(pSQL, "INSERT INTO ep_data VALUES (%lu,%lu,%lu)", xTime, xEPID, nValue);
	nRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (nRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTDM_RET_ERROR;
	}

	return	FTDM_RET_OK;
}

/********************************************************
 *
 ********************************************************/
typedef struct 
{
	FTDM_ULONG	nCount;
}	_FTDM_DBIF_CB_EP_DATA_COUNT_PARAMS, _PTR_ _FTDM_DBIF_CB_EP_DATA_COUNT_PARAMS_PTR;

static int _FTDM_DBIF_CB_EPDataCount(void *pData, int nArgc, char **pArgv, char **pColName)
{
     _FTDM_DBIF_CB_EP_DATA_COUNT_PARAMS_PTR pParams = (_FTDM_DBIF_CB_EP_DATA_COUNT_PARAMS_PTR)pData;

	if (nArgc != 0)
	{
		pParams->nCount = atoi(pArgv[0]);
    }

    return  FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_EPDataCount
(
	FTDM_EP_ID_PTR			pEPID,
	FTDM_ULONG				nEPID,
	FTDM_ULONG				xBeginTime,
	FTDM_ULONG				xEndTime,
	FTDM_ULONG_PTR			pCount
)
{
	FTDM_RET		nRet;
	FTDM_CHAR_PTR	pErrMsg = NULL;
	FTDM_CHAR		pSQL[1024];
	FTDM_INT		nSQLLen = 0;
	FTDM_BOOL		bConditionOn = FTDM_BOOL_FALSE;
	_FTDM_DBIF_CB_EP_DATA_COUNT_PARAMS	xParams;

	nSQLLen = sprintf(pSQL, "SELECT COUNT(*) from ep_data ");
	if (nEPID != 0)
	{
		FTDM_INT	i;

		bConditionOn = FTDM_BOOL_TRUE;

		nSQLLen += sprintf(&pSQL[nSQLLen], "WHERE (");
		for( i = 0 ; i < nEPID ; i++)
		{
			if (i == 0)
			{
				nSQLLen += sprintf(&pSQL[nSQLLen], "EPID == %lu ", pEPID[i]);
			}
			else
			{
				nSQLLen += sprintf(&pSQL[nSQLLen], "OR EPID == %lu ", pEPID[i]);
			}
		}
		nSQLLen += sprintf(&pSQL[nSQLLen], ") ");
	}

	if (xBeginTime != 0)
	{
		if (bConditionOn == FTDM_BOOL_FALSE)
		{
			bConditionOn = FTDM_BOOL_TRUE;
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
		if (bConditionOn == FTDM_BOOL_FALSE)
		{
			bConditionOn = FTDM_BOOL_TRUE;
			nSQLLen += sprintf(&pSQL[nSQLLen], "WHERE ");
		}
		else
		{
			nSQLLen += sprintf(&pSQL[nSQLLen], "AND ");
		}
		nSQLLen += sprintf(&pSQL[nSQLLen], "TIME <= %lu ", xEndTime);
	}


	nRet = sqlite3_exec(_pSQLiteDB, pSQL, _FTDM_DBIF_CB_EPDataCount, &xParams, &pErrMsg);
	if (nRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTDM_RET_ERROR;
	}

	*pCount = xParams.nCount;

	return	FTDM_RET_OK;
}

/***********************************************************
 *
 ***********************************************************/
typedef struct 
{
	FTDM_EP_DATA_PTR	pEPData;
	FTDM_INT			nMaxCount;
	FTDM_INT			nCount;
}	_FTDM_DBIF_CB_GET_EP_DATA_PARAMS, _PTR_ _FTDM_DBIF_CB_GET_EP_DATA_PARAMS_PTR;

static int _FTDM_DBIF_CB_getEPData(void *pData, int nArgc, char **pArgv, char **pColName)
{
	_FTDM_DBIF_CB_GET_EP_DATA_PARAMS_PTR pParams = (_FTDM_DBIF_CB_GET_EP_DATA_PARAMS_PTR)pData;

	if (nArgc != 0)
	{
		FTDM_INT	i;
		for(i = 0 ; i < nArgc ; i++)
		{
			printf("%s : %s\n", pColName[i], pArgv[i]);
			if (strcmp(pColName[i],"TIME") == 0)
			{
				pParams->nCount++;
				if (pParams->nCount <= pParams->nMaxCount)
				{
					pParams->pEPData[pParams->nCount-1].nTime = atoi(pArgv[i]);
				}
			}
			else if (strcmp(pColName[i], "EPID") == 0)
			{
				if (pParams->nCount <= pParams->nMaxCount)
				{
					pParams->pEPData[pParams->nCount-1].xEPID = atoi(pArgv[i]);
				}
			}
			else if (strcmp(pColName[i], "VALUE") == 0)
			{
				if (pParams->nCount <= pParams->nMaxCount)
				{
					pParams->pEPData[pParams->nCount-1].nValue = atoi(pArgv[i]);
				}
			}
		}
	}

	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_getEPData
(
	FTDM_EP_ID_PTR			pEPID,
	FTDM_ULONG				nEPIDCount,
	FTDM_ULONG				xBeginTime,
	FTDM_ULONG				xEndTime,
	FTDM_EP_DATA_PTR		pEPData,
	FTDM_ULONG				nMaxCount,
	FTDM_ULONG_PTR			pCount
)
{
	FTDM_RET		nRet;
	FTDM_CHAR_PTR	pErrMsg = NULL;
	FTDM_CHAR		pSQL[1024];
	FTDM_INT		nSQLLen = 0;
	FTDM_BOOL		bConditionOn = FTDM_BOOL_FALSE;
	_FTDM_DBIF_CB_GET_EP_DATA_PARAMS	xParams;

	nSQLLen += sprintf(pSQL, "SELECT * FROM ep_data ");
	if (nEPIDCount != 0)
	{
		FTDM_INT	i;

		bConditionOn = FTDM_BOOL_TRUE;

		nSQLLen += sprintf(&pSQL[nSQLLen], "WHERE (");
		for( i = 0 ; i < nEPIDCount ; i++)
		{
			if (i == 0)
			{
				nSQLLen += sprintf(&pSQL[nSQLLen], "EPID == 1 ", pEPID[i]);
			}
			else
			{
				nSQLLen += sprintf(&pSQL[nSQLLen], "OR EPID == %lu ", pEPID[i]);
			}
		}
		nSQLLen += sprintf(&pSQL[nSQLLen], ") ");
	}

	if (xBeginTime != 0)
	{
		if (bConditionOn == FTDM_BOOL_FALSE)
		{
			bConditionOn = FTDM_BOOL_TRUE;
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
		if (bConditionOn == FTDM_BOOL_FALSE)
		{
			bConditionOn = FTDM_BOOL_TRUE;
			nSQLLen += sprintf(&pSQL[nSQLLen], "WHERE ");
		}
		else
		{
			nSQLLen += sprintf(&pSQL[nSQLLen], "AND ");
		}
		nSQLLen += sprintf(&pSQL[nSQLLen], "TIME <= %lu ", xEndTime);
	}

	printf("%s\n", pSQL);
	xParams.pEPData = pEPData;
	xParams.nMaxCount = nMaxCount;
	xParams.nCount = 0;
	nRet = sqlite3_exec(_pSQLiteDB, pSQL, _FTDM_DBIF_CB_getEPData, &xParams, &pErrMsg);
	if (nRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTDM_RET_ERROR;
	}

	*pCount = xParams.nCount;

	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_removeEPData
(
	FTDM_EP_ID_PTR			pEPID,
	FTDM_ULONG				nEPIDCount,
	FTDM_ULONG				xBeginTime,
	FTDM_ULONG				xEndTime,
	FTDM_ULONG				nCount
)
{
	FTDM_RET		nRet;
	FTDM_CHAR_PTR	pErrMsg = NULL;
	FTDM_CHAR		pSQL[1024];
	FTDM_INT		nSQLLen = 0;
	FTDM_BOOL		bConditionOn = FTDM_BOOL_FALSE;

	nSQLLen += sprintf(pSQL, "DELETE FROM ep_data ");
	if (nEPIDCount != 0)
	{
		FTDM_INT	i;

		bConditionOn = FTDM_BOOL_TRUE;

		nSQLLen += sprintf(&pSQL[nSQLLen], "WHERE (");
		for( i = 0 ; i < nEPIDCount ; i++)
		{
			if (i == 0)
			{
				nSQLLen += sprintf(&pSQL[nSQLLen], "EPID == %lu ", pEPID[i]);
			}
			else
			{
				nSQLLen += sprintf(&pSQL[nSQLLen], "OR EPID == %lu ", pEPID[i]);
			}
		}
		nSQLLen += sprintf(&pSQL[nSQLLen], ") ");
	}

	if (xBeginTime != 0)
	{
		if (bConditionOn == FTDM_BOOL_FALSE)
		{
			bConditionOn = FTDM_BOOL_TRUE;
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
		if (bConditionOn == FTDM_BOOL_FALSE)
		{
			bConditionOn = FTDM_BOOL_TRUE;
			nSQLLen += sprintf(&pSQL[nSQLLen], "WHERE ");
		}
		else
		{
			nSQLLen += sprintf(&pSQL[nSQLLen], "AND ");
		}
		nSQLLen += sprintf(&pSQL[nSQLLen], "TIME <= %lu ", xEndTime);
	}


	nRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (nRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTDM_RET_ERROR;
	}

	return	FTDM_RET_OK;
}

/*************************************************************************/
typedef struct
{
	FTDM_BOOL		bExist;
	FTDM_CHAR_PTR	pName;
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
                pParams->bExist = FTDM_BOOL_TRUE;
                break;
            }
        }

    }

    return  0;
}

FTDM_RET _FTDM_BDIF_isExistTable
(
	FTDM_CHAR_PTR	pTableName, 
	FTDM_BOOL_PTR 	pExist
)
{
    int     nRet;
    _FTDM_DBIF_CB_EXIST_TABLE_PARAMS xParams = { .bExist = FTDM_BOOL_FALSE, .pName = pTableName};
    FTDM_CHAR_PTR	pSQL = "select name from sqlite_master where type='table' order by name";
    FTDM_CHAR_PTR	pErrMsg = NULL;

    nRet = sqlite3_exec(_pSQLiteDB, pSQL, _FTDM_DBIF_CB_isExistTable, &xParams, &pErrMsg);
    if (nRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", pErrMsg);
        sqlite3_free(pErrMsg);

        return  FTDM_RET_ERROR;
    }

    *pExist = xParams.bExist;

    return  FTDM_RET_OK;
}

FTDM_RET	_FTDM_BDIF_createDeviceInfoTable
(
	FTDM_CHAR_PTR	pTableName
)
{
	int	nRet;
	FTDM_CHAR_PTR	pErrMsg = NULL;
	char			pSQL[1024];

	sprintf(pSQL, "CREATE TABLE %s ("\
						"DID	TEXT PRIMARY KEY,"\
						"TYPE	INT,"\
						"URL	TEXT,"\
						"LOC	TEXT)", pTableName);

	nRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (nRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTDM_RET_ERROR;
	}

	return	FTDM_RET_OK;
}

/*************************************************************************/
typedef struct
{
	FTDM_BOOL	bExist;
}   _FTDM_DBIF_CB_IS_EXIST_PARAMS, * _FTDM_DBIF_CB_IS_EXIST_PARAMS_PTR;

static int _FTDM_DBIF_CB_isExist(void *pData, int nArgc, char **pArgv, char **pColName)
{
    _FTDM_DBIF_CB_IS_EXIST_PARAMS_PTR pParams = (_FTDM_DBIF_CB_IS_EXIST_PARAMS_PTR)pData;

	if (nArgc != 0)
	{
		if (atoi(pArgv[0]) != 0)
		{
			pParams->bExist = FTDM_BOOL_TRUE;
		}
    }

    return  FTDM_RET_OK;
}

FTDM_RET _FTDM_DBIF_isExistDevice
(
	FTDM_CHAR_PTR	pTableName, 
	FTDM_CHAR_PTR 	pDID, 
	FTDM_BOOL_PTR pExist
)
{
	_FTDM_DBIF_CB_IS_EXIST_PARAMS xParams = {.bExist = FTDM_BOOL_TRUE };
    int     nRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;

    sprintf(strSQL, "SELECT COUNT(DID) FROM %s WHERE DID = '%s'", pTableName, pDID);
    nRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_CB_isExist, &xParams, &strErrMsg);
    if (nRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTDM_RET_ERROR;
    }

    *pExist = xParams.bExist;

    return  FTDM_RET_OK;
}

/*******************************************************
 *
 *******************************************************/
FTDM_RET	_FTDM_BDIF_createEPInfoTable
(
	FTDM_CHAR_PTR	pTableName
)
{
	int	nRet;
	FTDM_CHAR_PTR	pErrMsg = NULL;
	char			pSQL[1024];

	sprintf(pSQL, "CREATE TABLE %s ("\
						"EPID	INTEGER PRIMARY KEY,"\
						"DID	TEXT,"\
						"TYPE	INT,"\
						"NAME	TEXT,"\
						"STATUS	INT,"\
						"INTERVAL INT,"\
						"UNIT	TEXT,"\
						"PID	TEXT)", pTableName);

	nRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (nRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTDM_RET_ERROR;
	}

	return	FTDM_RET_OK;
}


