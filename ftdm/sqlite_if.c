#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "ftdm_type.h"
#include "sqlite_if.h"
#include "debug.h"

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
	FTDM_BYTE_PTR	pDID,
	FTDM_BOOL_PTR 	pExist
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

FTDM_RET	FTDM_DBIF_insertDeviceInfo
(
	FTDM_BYTE_PTR		pDID, 
	FTDM_DEVICE_TYPE 	xType,
	FTDM_BYTE_PTR		pURL,
	FTDM_INT			nURLLen,
	FTDM_BYTE_PTR		pLocation,
	FTDM_INT			nLocationLen
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_removeDeviceInfo
(
	FTDM_BYTE_PTR		pDID
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_getDeviceInfo
(
	FTDM_BYTE_PTR			pDID, 
	FTDM_DEVICE_TYPE_PTR	pType,
	FTDM_CHAR_PTR			pURL,
	FTDM_INT_PTR			pURLLen,
	FTDM_CHAR_PTR			pLocation,
	FTDM_INT_PTR			pLocLen
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_setDeviceURL
(
	FTDM_BYTE_PTR		pDID, 
	FTDM_CHAR_PTR		pURL,
	FTDM_INT			nURLLen
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_getDeviceURL
(
	FTDM_BYTE_PTR		pDID, 
	FTDM_CHAR_PTR		pURL,
	FTDM_INT_PTR		pURLLen
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_setDeviceLocation
(
	FTDM_BYTE_PTR		pDID, 
	FTDM_CHAR_PTR		pLocation,
	FTDM_INT			nLocationLen
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_getDeviceLocation
(
	FTDM_BYTE_PTR		pDID, 
	FTDM_CHAR_PTR		pLocation,
	FTDM_INT_PTR		pLocLen
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_initEPInfoTable
(
	void
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_insertEPInfo
(
	FTDM_EP_ID 				xEPID, 
	FTDM_EP_TYPE 			xType,
	FTDM_BYTE_PTR			pDID,
	FTDM_CHAR_PTR			pName,
	FTDM_INT				nNameLen,
	FTDM_ULONG				nInterval,
	FTDM_CHAR_PTR			pUnit,
	FTDM_INT				nUnitLen,
	FTDM_BYTE_PTR			pParentID
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_removeEPInfo
(
	FTDM_EP_ID				xEPID
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_getEPInfo
(
	FTDM_EP_ID 				xEPID, 
	FTDM_EP_TYPE_PTR		pType,
	FTDM_BYTE_PTR			pDID,
	FTDM_CHAR_PTR			pName,
	FTDM_INT_PTR			pNameLen,
	FTDM_ULONG_PTR			pInterval,
	FTDM_CHAR_PTR			pUnit,
	FTDM_INT_PTR			pUnitLen,
	FTDM_BYTE_PTR			xParentID
)
{
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


FTDM_RET	FTDM_DBIF_initEPLogTable
(
	void
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_appendEPLog
(
	FTDM_ULONG				xTime,
	FTDM_EP_ID				xEPID,
	FTDM_ULONG				nValue
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_DBIF_EPLogCount
(
	FTDM_ULONG				xBeginTime,
	FTDM_ULONG				xEndTime,
	FTDM_EP_ID_PTR			pEPID,
	FTDM_ULONG				nEPID,
	FTDM_ULONG_PTR			pCount
)
{
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
						"TYPE	TEXT,"\
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
	FTDM_BYTE_PTR 	xDID, 
	FTDM_BOOL_PTR pExist
)
{
	_FTDM_DBIF_CB_IS_EXIST_PARAMS xParams = {.bExist = FTDM_BOOL_TRUE };
    int     nRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;

    sprintf(strSQL, "SELECT COUNT(DID) FROM %s WHERE DID = '%s'", pTableName, xDID);
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

