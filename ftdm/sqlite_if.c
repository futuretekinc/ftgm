#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "ftgm_type.h"
#include "sqlite_if.h"
#include "debug.h"

FTGM_RET _FTDM_BDIF_isExistTable
(
	FTGM_STRING strTableName, FTGM_BOOL_PTR pExist
);

FTGM_RET _FTDM_BDIF_createDeviceInfoTable
(
	FTGM_STRING 	strTableName
);

FTGM_RET _FTDM_DBIF_isExistDevice
(	
	FTGM_STRING 	strTableName, 
	FTGM_DEVICE_ID 	xDID, 
	FTGM_BOOL_PTR 	pExist
);

static FTGM_STRING	_strDefaultDBName = "./ftdm.db";

static sqlite3		*_pSQLiteDB= 0;

FTGM_RET	FTDM_DBIF_init
(
	void
)
{
	int	nRet;

	if (_pSQLiteDB)
	{
		return	FTGM_RET_ERROR_ALREADY_BEEN_COMPLETED;
	}

	nRet = sqlite3_open(_strDefaultDBName, &_pSQLiteDB);
	if ( nRet )
	{
		ERROR("SQL error : %s\n", sqlite3_errmsg(_pSQLiteDB));

		return	(FTGM_RET_ERROR_DB | nRet); 	
	}

	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_DBIF_final
(
	void
)
{
	if (_pSQLiteDB)
	{
		sqlite3_close(_pSQLiteDB);
		_pSQLiteDB = NULL;
	}

	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_DBIF_initDeviceInfoTable
(
	void
)
{
	FTGM_STRING	strTableName = "device_info";
	FTGM_BOOL	bExist = FTGM_BOOL_FALSE;

	if (_FTDM_BDIF_isExistTable(strTableName, &bExist) != FTGM_RET_OK)
	{
		ERROR("_FTDM_BDIF_isExistTable(%s,bExist)\n", strTableName);  
		return	FTGM_RET_ERROR;	
	}

	if (bExist != FTGM_BOOL_TRUE)
	{
		ERROR("%s is not exist\n", strTableName);
		if (_FTDM_BDIF_createDeviceInfoTable(strTableName) != FTGM_RET_OK)
		{
			return	FTGM_RET_ERROR;	
		}
	}

	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_DBIF_insertDeviceInfo
(
	FTGM_DEVICE_ID 		xDID, 
	FTGM_DEVICE_TYPE 	xType,
	FTGM_STRING			strURL,
	FTGM_STRING			strLOC
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_DBIF_removeDeviceInfo
(
	FTGM_DEVICE_ID		xDID
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_DBIF_getDeviceInfo
(
	FTGM_DEVICE_ID			xDID,
	FTGM_DEVICE_TYPE_PTR	pType,
	FTGM_STRING				strURL,
	FTGM_ULONG_PTR			pURLLen,
	FTGM_STRING				strLoc,
	FTGM_ULONG_PTR			pLocLen
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_DBIF_setDeviceURL
(
	FTGM_DEVICE_ID			xDID,
	FTGM_STRING				strURL
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_DBIF_getDeviceURL
(
	FTGM_DEVICE_ID			xDID,
	FTGM_STRING				strURL,
	FTGM_ULONG_PTR			pURLLen
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_DBIF_setDeviceLocation
(
	FTGM_DEVICE_ID			xDID,
	FTGM_STRING				strLoc
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_DBIF_getDeviceLocation
(
	FTGM_DEVICE_ID			xDID,
	FTGM_STRING				strLoc,
	FTGM_ULONG_PTR			pLocLen
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_DBIF_initEndPointInfoTable
(
	void
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_DBIF_insertEndPointInfo
(
	FTGM_EP_ID 				xEPID, 
	FTGM_EP_TYPE 			xType,
	FTGM_DEVICE_ID			xDID,
	FTGM_STRING				strName,
	FTGM_ULONG				nInterval,
	FTGM_STRING				strUnit,
	FTGM_DEVICE_ID			xParentID
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_DBIF_removeEndPointInfo
(
	FTGM_EP_ID				xEPID
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_DBIF_getEndPointInfo
(
	FTGM_EP_ID 				xEPID, 
	FTGM_EP_TYPE_PTR		pType,
	FTGM_DEVICE_ID			xDID,
	FTGM_STRING				strName,
	FTGM_ULONG_PTR			pNameLen,
	FTGM_ULONG_PTR			pInterval,
	FTGM_STRING				strUnit,
	FTGM_ULONG_PTR			pUnit,
	FTGM_DEVICE_ID			xParentID
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_DBIF_setEndPointName
(
	FTGM_EP_ID				xEPID,
	FTGM_STRING				strName
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_DBIF_getEndPointName
(
	FTGM_EP_ID				xEPID,
	FTGM_STRING				strName,
	FTGM_ULONG_PTR			pNameLen
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_DBIF_setEndPointInterval
(
	FTGM_EP_ID				xEPID,
	FTGM_ULONG				nInterval
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_DBIF_getEndPointInterval
(
	FTGM_EP_ID				xEPID,
	FTGM_ULONG_PTR			pInterval
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_DBIF_setEndPointUnit
(
	FTGM_EP_ID				xEPID,
	FTGM_STRING				strUnit
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_DBIF_getEndPointUnit
(
	FTGM_EP_ID				xEPID,
	FTGM_STRING				strUnit,
	FTGM_ULONG_PTR			pUnitLen
)
{
	return	FTGM_RET_OK;
}


FTGM_RET	FTDM_DBIF_initEndPointLogTable
(
	void
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_DBIF_appendEndPointLog
(
	FTGM_ULONG				xTime,
	FTGM_EP_ID				xEPID,
	FTGM_STRING				strValue
)
{
	return	FTGM_RET_OK;
}

FTGM_RET	FTDM_DBIF_EndPointLogCount
(
	FTGM_ULONG				xBeginTime,
	FTGM_ULONG				xEndTime,
	FTGM_EP_ID_PTR			pEPID,
	FTGM_ULONG				nEPID,
	FTGM_ULONG_PTR			pCount
)
{
	return	FTGM_RET_OK;
}

/*************************************************************************/
typedef struct
{
	FTGM_BOOL		bExist;
	FTGM_STRING		strName;
}   _FTDM_DBIF_CB_EXIST_TABLE_PARAMS, *  _FTDM_DBIF_CB_EXIST_TABLE_PARAMS_PTR;

static int _FTDM_DBIF_CB_isExistTable(void *pData, int nArgc, char **pArgv, char **pColName)
{
    int i;
  	_FTDM_DBIF_CB_EXIST_TABLE_PARAMS_PTR pParams = (_FTDM_DBIF_CB_EXIST_TABLE_PARAMS_PTR)pData;

    if (nArgc != 0)
    {
        for(i = 0 ; i < nArgc ; i++)
        {
            if (strcmp(pParams->strName, pArgv[i]) == 0)
            {
                pParams->bExist = FTGM_BOOL_TRUE;
                break;
            }
        }

    }

    return  0;
}

FTGM_RET _FTDM_BDIF_isExistTable
(
	FTGM_STRING 	strTableName, 
	FTGM_BOOL_PTR 	pExist
)
{
    int     nRet;
    _FTDM_DBIF_CB_EXIST_TABLE_PARAMS xParams = { .bExist = FTGM_BOOL_FALSE, .strName = strTableName};
    FTGM_STRING	strSQL = "select name from sqlite_master where type='table' order by name";
    FTGM_STRING	strErrMsg = NULL;

    nRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_CB_isExistTable, &xParams, &strErrMsg);
    if (nRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

        return  FTGM_RET_ERROR;
    }

    *pExist = xParams.bExist;

    return  FTGM_RET_OK;
}

FTGM_RET	_FTDM_BDIF_createDeviceInfoTable
(
	FTGM_STRING 	strTableName
)
{
	int	nRet;
	FTGM_STRING	strErrMsg = NULL;
	char		strSQL[1024];

	sprintf(strSQL, "CREATE TABLE %s ("\
						"DID	TEXT PRIMARY KEY,"\
						"TYPE	TEXT,"\
						"URL	TEXT,"\
						"LOC	TEXT)", strTableName);

	nRet = sqlite3_exec(_pSQLiteDB, strSQL, NULL, 0, &strErrMsg);
	if (nRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", strErrMsg);	
		sqlite3_free(strErrMsg);

		return	FTGM_RET_ERROR;
	}

	return	FTGM_RET_OK;
}

/*************************************************************************/
typedef struct
{
	FTGM_BOOL	bExist;
}   _FTDM_DBIF_CB_IS_EXIST_PARAMS, * _FTDM_DBIF_CB_IS_EXIST_PARAMS_PTR;

static int _FTDM_DBIF_CB_isExist(void *pData, int nArgc, char **pArgv, char **pColName)
{
    _FTDM_DBIF_CB_IS_EXIST_PARAMS_PTR pParams = (_FTDM_DBIF_CB_IS_EXIST_PARAMS_PTR)pData;

	if (nArgc != 0)
	{
		if (atoi(pArgv[0]) != 0)
		{
			pParams->bExist = FTGM_BOOL_TRUE;
		}
    }

    return  FTGM_RET_OK;
}

FTGM_RET _FTDM_DBIF_isExistDevice(FTGM_STRING strTableName, FTGM_DEVICE_ID xDID, FTGM_BOOL_PTR pExist)
{
	_FTDM_DBIF_CB_IS_EXIST_PARAMS xParams = {.bExist = FTGM_BOOL_TRUE };
    int     nRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;

    sprintf(strSQL, "SELECT COUNT(DID) FROM %s WHERE DID = '%s'", strTableName, xDID);
    nRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_CB_isExist, &xParams, &strErrMsg);
    if (nRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTGM_RET_ERROR;
    }

    *pExist = xParams.bExist;

    return  FTGM_RET_OK;
}

