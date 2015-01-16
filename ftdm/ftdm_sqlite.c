#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sqlite3.h>
#include <sys/time.h>
#include "ftdm.h"
#include "ftdm_sqlite.h"


FTM_RET _FTDM_BDIF_isExistTable
(
	FTM_CHAR_PTR	pTableName, 
	FTM_BOOL_PTR 	pExist
);

FTM_RET _FTDM_BDIF_createNodeInfoTable
(
	FTM_CHAR_PTR	pTableName
);

FTM_RET _FTDM_DBIF_isExistNode
(	
	FTM_CHAR_PTR	pTableName, 
	FTM_CHAR_PTR	pDID,
	FTM_BOOL_PTR 	pExist
);

FTM_RET _FTDM_BDIF_createEPInfoTable
(
	FTM_CHAR_PTR	pTableName
);

static FTM_CHAR_PTR	_strDefaultDBName = "./ftdm.db";

static sqlite3		*_pSQLiteDB= 0;

FTM_RET	FTDM_DBIF_init
(
	FTDM_CFG_DB_PTR	pConfig
)
{
	int	nRet;

	if (_pSQLiteDB)
	{
		return	FTM_RET_DBIF_ALREADY_INITIALIZED;
	}

	if ((pConfig != NULL) && (pConfig->pFileName != NULL))
	{
		_strDefaultDBName = pConfig->pFileName;
	}

	nRet = sqlite3_open(_strDefaultDBName, &_pSQLiteDB);
	if ( nRet )
	{
		ERROR("SQL error : %s\n", sqlite3_errmsg(_pSQLiteDB));

		return	(FTM_RET_DBIF_DB_ERROR | nRet); 	
	}

	nRet = FTDM_DBIF_initNodeInfoTable();
	if (nRet != FTM_RET_OK)
	{
		sqlite3_close(_pSQLiteDB);
		return	nRet;	
	}

	nRet = FTDM_DBIF_initEPInfoTable();
	if (nRet != FTM_RET_OK)
	{
		sqlite3_close(_pSQLiteDB);
		return	nRet;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_final
(
	void
)
{
	if (_pSQLiteDB)
	{
		sqlite3_close(_pSQLiteDB);
		_pSQLiteDB = NULL;
	}

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
FTM_RET	FTDM_DBIF_initNodeInfoTable
(
	void
)
{
	FTM_CHAR_PTR	pTableName = "node_info";
	FTM_BOOL		bExist = FTM_BOOL_FALSE;

	if (_FTDM_BDIF_isExistTable(pTableName, &bExist) != FTM_RET_OK)
	{
		ERROR("_FTDM_BDIF_isExistTable(%s,bExist)\n", pTableName);  
		return	FTM_RET_DBIF_ERROR;	
	}

	if (bExist != FTM_BOOL_TRUE)
	{
		ERROR("%s is not exist\n", pTableName);
		if (_FTDM_BDIF_createNodeInfoTable(pTableName) != FTM_RET_OK)
		{
			return	FTM_RET_DBIF_ERROR;	
		}
	}

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
static int _FTDM_DBIF_CB_getNodeCount(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTM_ULONG_PTR pnCount = (FTM_ULONG_PTR)pData;

	if (nArgc != 0)
	{
		*pnCount = atoi(pArgv[0]);
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_getNodeCount
(
	FTM_ULONG_PTR		pCount
)
{
    int     nRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;

    sprintf(strSQL, "SELECT COUNT(*) FROM node_info");
    nRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_CB_getNodeCount, pCount, &strErrMsg);
    if (nRet != SQLITE_OK)
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
	FTM_ULONG_PTR		pCount;
	FTM_NODE_INFO_PTR	pInfos;
}	FTDM_DBIF_CB_GET_DEVICE_LIST_PARAMS, _PTR_ FTDM_DBIF_CB_GET_DEVICE_LIST_PARAMS_PTR;

static int _FTDM_DBIF_CB_getNodeList(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTDM_DBIF_CB_GET_DEVICE_LIST_PARAMS_PTR pParams = (FTDM_DBIF_CB_GET_DEVICE_LIST_PARAMS_PTR)pData;

	if (nArgc != 0)
	{
		FTM_INT	i;
		FTM_NODE_INFO_PTR	pNodeInfo = &pParams->pInfos[(*pParams->pCount)++];

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
			else if (strcasecmp(pColName[i], "OPT0") == 0)
			{
				switch (pNodeInfo->xType)
				{
				case	FTM_NODE_TYPE_SNMP:
					pNodeInfo->xOption.xSNMP.nVersion = strtoul(pArgv[i], 0, 10);
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

FTM_RET	FTDM_DBIF_getNodeList
(
	FTM_NODE_INFO_PTR	pInfos, 
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pCount
)
{
	FTM_RET			nRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[2048];
	FTDM_DBIF_CB_GET_DEVICE_LIST_PARAMS	xParams = 
	{
		.nMaxCount  = nMaxCount,
		.pCount 	= pCount,
		.pInfos		= pInfos
	};	

	sprintf(pSQL, "SELECT * FROM node_info");
	nRet = sqlite3_exec(_pSQLiteDB, pSQL, _FTDM_DBIF_CB_getNodeList, &xParams, &pErrMsg);
	if (nRet != SQLITE_OK)
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
FTM_RET	FTDM_DBIF_insertNodeInfo
(
 	FTM_NODE_INFO_PTR	pNodeInfo
)
{
	FTM_RET			nRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_CHAR		pOpt0[256] = {0,};
	FTM_CHAR		pOpt1[256] = {0,};
	FTM_CHAR		pOpt2[256] = {0,};
	FTM_CHAR		pOpt3[256] = {0,};

	if (pNodeInfo == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	switch(pNodeInfo->xType)
	{
	case	FTM_NODE_TYPE_SNMP:	
		{	
			sprintf(pOpt0, "%lu", pNodeInfo->xOption.xSNMP.nVersion);
			sprintf(pOpt1, "%s", pNodeInfo->xOption.xSNMP.pURL);
			sprintf(pOpt2, "%s", pNodeInfo->xOption.xSNMP.pCommunity);
			sprintf(pOpt3, "%s", pNodeInfo->xOption.xSNMP.pMIB);
		}
		break;
	}

	sprintf(pSQL, 
			"INSERT INTO node_info (DID,TYPE,LOC,INTERVAL,OPT0,OPT1,OPT2,OPT3) "\
			"VALUES (\'%s\',%lu,\'%s\',%lu,\'%s\',\'%s\',\'%s\',\'%s\')",
			pNodeInfo->pDID, 
			pNodeInfo->xType, 
			pNodeInfo->pLocation,
			pNodeInfo->ulInterval,
			pOpt0,
			pOpt1,
			pOpt2,
			pOpt3);
	nRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (nRet != SQLITE_OK)
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
FTM_RET	FTDM_DBIF_delNodeInfo
(
	FTM_CHAR_PTR		pDID
)
{
	FTM_RET		nRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];

	if (strlen(pDID) > FTM_DID_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	sprintf(pSQL, "DELETE FROM node_info WHERE DID == %s", pDID);
	nRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (nRet != SQLITE_OK)
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
static int _FTDM_DBIF_CB_getNodeInfo(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTM_NODE_INFO_PTR pInfo = (FTM_NODE_INFO_PTR)pData;

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
		else if (strcmp(pColName[0], "OPT0") == 0)
		{
		}
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_getNodeInfo
(
	FTM_CHAR_PTR		pDID, 
	FTM_NODE_INFO_PTR	pInfo
)
{
    int     nRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;

    sprintf(strSQL, "SELECT * FROM node_info WHERE DID = '%s'", pDID);
    nRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_CB_getNodeInfo, pInfo, &strErrMsg);
    if (nRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTM_RET_ERROR;
    }

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_setNodeURL
(
	FTM_CHAR_PTR		pDID, 
	FTM_CHAR_PTR		pURL
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_getNodeURL
(
	FTM_CHAR_PTR		pDID, 
	FTM_CHAR_PTR		pBuff,
	FTM_ULONG			nBuffLen
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_setNodeLocation
(
	FTM_CHAR_PTR		pDID, 
	FTM_CHAR_PTR		pLocation
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_getNodeLocation
(
	FTM_CHAR_PTR		pDID, 
	FTM_CHAR_PTR		pBuff,
	FTM_ULONG			nBuffLen
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_initEPInfoTable
(
	void
)
{
	FTM_CHAR_PTR	pTableName = "ep_info";
	FTM_BOOL		bExist = FTM_BOOL_FALSE;

	if (_FTDM_BDIF_isExistTable(pTableName, &bExist) != FTM_RET_OK)
	{
		ERROR("_FTDM_BDIF_isExistTable(%s,bExist)\n", pTableName);  
		return	FTM_RET_DBIF_ERROR;	
	}

	if (bExist != FTM_BOOL_TRUE)
	{
		if (_FTDM_BDIF_createEPInfoTable(pTableName) != FTM_RET_OK)
		{
			return	FTM_RET_DBIF_ERROR;	
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_insertEPInfo
(
 	FTM_EP_INFO_PTR		pEPInfo
)
{
	FTM_RET		nRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];

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

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_delEPInfo
(
	FTM_EPID				xEPID
)
{
	FTM_RET		nRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];

	sprintf(pSQL,
			"DELETE FROM ep_info WHERE EPID == %lu", 
			xEPID);
	nRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (nRet != SQLITE_OK)
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
static int _FTDM_DBIF_CB_getEPCount(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTM_ULONG_PTR pnCount = (FTM_ULONG_PTR)pData;

	if (nArgc != 0)
	{
		*pnCount = atoi(pArgv[0]);
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_getEPCount
(
	FTM_ULONG_PTR		pCount
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
	FTM_EP_INFO_PTR	pInfos;
}	FTDM_DBIF_CB_GET_EP_LIST_PARAMS, _PTR_ FTDM_DBIF_CB_GET_EP_LIST_PARAMS_PTR;

static int _FTDM_DBIF_CB_getEPList(void *pData, int nArgc, char **pArgv, char **pColName)
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
			else if (strcmp(pColName[i], "INTERVAL") == 0)
			{
				pParams->pInfos[pParams->nCount-1].nInterval = atoi(pArgv[i]);
			}
			else if (strcmp(pColName[i], "DID") == 0)
			{
				strncpy(pParams->pInfos[pParams->nCount-1].pDID, pArgv[i], FTM_DID_LEN);
			}
			else if (strcmp(pColName[i], "PID") == 0)
			{
				strncpy(pParams->pInfos[pParams->nCount-1].pPID, pArgv[i], FTM_DID_LEN);
			}
		}
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_getEPList
(
	FTM_EP_INFO_PTR		pInfos, 
	FTM_ULONG				nMaxCount,
	FTM_ULONG_PTR			pCount
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

    	return  FTM_RET_ERROR;
    }

	*pCount = xParams.nCount;

	return	FTM_RET_OK;
}

/***************************************************************
 *
 ***************************************************************/
static int _FTDM_DBIF_CB_getEPInfo(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTM_EP_INFO_PTR pInfo = (FTM_EP_INFO_PTR)pData;

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
			strncpy(pInfo->pName, pArgv[0], FTM_NAME_LEN);
		}
		if (strcmp(pColName[0], "UNIT") == 0)
		{
			strncpy(pInfo->pUnit, pArgv[0], FTM_UNIT_LEN);
		}
		if (strcmp(pColName[0], "DID") == 0)
		{
			strncpy(pInfo->pDID, pArgv[0], FTM_DID_LEN);
		}
		if (strcmp(pColName[0], "PID") == 0)
		{
			strncpy(pInfo->pPID, pArgv[0], FTM_DID_LEN);
		}
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_getEPInfo
(
	FTM_EPID 				xEPID, 
 	FTM_EP_INFO_PTR		pInfo
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

    	return  FTM_RET_ERROR;
    }

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_setEPName
(
	FTM_EPID				xEPID,
	FTM_CHAR_PTR			pName,
	FTM_INT				nNameLen
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_getEPName
(
	FTM_EPID				xEPID,
	FTM_CHAR_PTR			pName,
	FTM_INT_PTR			pNameLen
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_setEPInterval
(
	FTM_EPID				xEPID,
	FTM_ULONG				nInterval
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_getEPInterval
(
	FTM_EPID				xEPID,
	FTM_ULONG_PTR			pInterval
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_setEPUnit
(
	FTM_EPID				xEPID,
	FTM_CHAR_PTR			pUnit,
	FTM_INT				nUnit
)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_getEPUnit
(
	FTM_EPID				xEPID,
	FTM_CHAR_PTR			pUnit,
	FTM_INT_PTR			pUnitLen
)
{
	return	FTM_RET_OK;
}


FTM_RET	FTDM_DBIF_initEPDataTable
(
	FTM_EPID	xEPID
)
{
	FTM_RET			nRet;
	FTM_BOOL		bExist = FTM_BOOL_FALSE;
	FTM_CHAR		pTableName[16];

	sprintf(pTableName, "ep_%08lx", xEPID);

	nRet = _FTDM_BDIF_isExistTable(pTableName, &bExist);
	if (nRet != FTM_RET_OK)
	{
		ERROR("_FTDM_BDIF_isExistTable(%s,bExist)\n", pTableName);  
		return	nRet;	
	}

	if (bExist == FTM_BOOL_FALSE)
	{ 
		FTM_RET		nRet;
		FTM_CHAR_PTR	pErrMsg = NULL;
		FTM_CHAR		pSQL[1024];

		sprintf(pSQL, "CREATE TABLE ep_%08lx (ID INT64,TIME INT,VALUE TEXT)", xEPID);

		nRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
		if (nRet != SQLITE_OK)
		{
			ERROR("SQL error : %s\n", pErrMsg);	
			sqlite3_free(pErrMsg);

			return	FTM_RET_ERROR;
		}

		TRACE("Table ep_%08lx creating has done.\n", xEPID);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_addEPData
(
	FTM_EPID				xEPID,
	FTM_EP_DATA_PTR			pData
)
{
	int				nRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	char			pSQL[1024];
	struct timeval	tv;

	gettimeofday(&tv, NULL);

	switch (pData->xType)
	{
	case	FTM_EP_DATA_TYPE_INT:
		{
			sprintf(pSQL, "INSERT INTO ep_%08lx VALUES (%llu, %lu, 'i%ld')", 
					xEPID,
					tv.tv_sec * (long long)1000000 + tv.tv_usec, 
					pData->ulTime, 
					pData->xValue.nValue);
		}
		break;

	case	FTM_EP_DATA_TYPE_ULONG:
		{
			sprintf(pSQL, "INSERT INTO ep_%08lx VALUES (%llu, %lu, 'u%lu')", 
					xEPID,
					tv.tv_sec * (long long)1000000 + tv.tv_usec, 
					pData->ulTime, 
					pData->xValue.ulValue);
		}
		break;

	case	FTM_EP_DATA_TYPE_FLOAT:
		{
			sprintf(pSQL, "INSERT INTO ep_%08lx VALUES (%llu, %lu, 'f%8.3lf')", 
					xEPID, 
					tv.tv_sec * (long long)1000000 + tv.tv_usec, 
					pData->ulTime, 
					pData->xValue.fValue);
		}
		break;

	default:
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	nRet = FTDM_DBIF_initEPDataTable(xEPID);
	if (nRet != FTM_RET_OK)
	{
		return	nRet;	
	}

	nRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (nRet != SQLITE_OK)
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
typedef struct 
{
	FTM_ULONG	nCount;
}	_FTDM_DBIF_CB_EP_DATA_COUNT_PARAMS, _PTR_ _FTDM_DBIF_CB_EP_DATA_COUNT_PARAMS_PTR;

static int _FTDM_DBIF_CB_EPDataCount(void *pData, int nArgc, char **pArgv, char **pColName)
{
     _FTDM_DBIF_CB_EP_DATA_COUNT_PARAMS_PTR pParams = (_FTDM_DBIF_CB_EP_DATA_COUNT_PARAMS_PTR)pData;

	if (nArgc != 0)
	{
		pParams->nCount = atoi(pArgv[0]);
    }

    return  FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_EPDataCount
(
	FTM_EPID				xEPID,
	FTM_ULONG				xBeginTime,
	FTM_ULONG				xEndTime,
	FTM_ULONG_PTR			pCount
)
{
	FTM_RET		nRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_INT		nSQLLen = 0;
	FTM_BOOL		bConditionOn = FTM_BOOL_FALSE;
	_FTDM_DBIF_CB_EP_DATA_COUNT_PARAMS	xParams;

	nSQLLen = sprintf(pSQL, "SELECT COUNT(*) from ep_%08lx ", xEPID);
	if (xBeginTime != 0)
	{
		if (bConditionOn == FTM_BOOL_FALSE)
		{
			bConditionOn = FTM_BOOL_TRUE;
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
		if (bConditionOn == FTM_BOOL_FALSE)
		{
			bConditionOn = FTM_BOOL_TRUE;
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

		return	FTM_RET_ERROR;
	}

	*pCount = xParams.nCount;

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

static int _FTDM_DBIF_CB_getEPData(void *pData, int nArgc, char **pArgv, char **pColName)
{
	_FTDM_DBIF_CB_GET_EP_DATA_PARAMS_PTR pParams = (_FTDM_DBIF_CB_GET_EP_DATA_PARAMS_PTR)pData;

	if ((nArgc != 0) && (pParams->nCount < pParams->nMaxCount))
	{
		FTM_INT	i;
		
		pParams->nCount++;
		for(i = 0 ; i < nArgc ; i++)
		{
			TRACE("%s : %s\n", pColName[i], pArgv[i]);

			if (strcmp(pColName[i],"TIME") == 0)
			{
				if (pParams->nCount <= pParams->nMaxCount)
				{
					pParams->pEPData[pParams->nCount-1].ulTime = strtoul(pArgv[i], 0, 10);
				}
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

FTM_RET	FTDM_DBIF_getEPData
(
	FTM_EPID			xEPID,
	FTM_ULONG			nStartIndex,
	FTM_EP_DATA_PTR		pEPData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pCount
)
{
	FTM_RET			nRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_INT			nSQLLen = 0;

	_FTDM_DBIF_CB_GET_EP_DATA_PARAMS	xParams;

	if (nMaxCount == 0)
	{
		nMaxCount = 100;	
	}
	
	nSQLLen =  sprintf(pSQL,"SELECT * FROM ep_%08lx ", xEPID);
	nSQLLen += sprintf(&pSQL[nSQLLen], " ORDER BY TIME DESC LIMIT %lu OFFSET %lu", nMaxCount, nStartIndex);

	xParams.pEPData = pEPData;
	xParams.nMaxCount = nMaxCount;
	xParams.nCount = 0;
	TRACE("SQL : %s\n", pSQL);
	nRet = sqlite3_exec(_pSQLiteDB, pSQL, _FTDM_DBIF_CB_getEPData, &xParams, &pErrMsg);
	if (nRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	*pCount = xParams.nCount;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_getEPDataWithTime
(
	FTM_EPID			xEPID,
	FTM_ULONG			xBeginTime,
	FTM_ULONG			xEndTime,
	FTM_EP_DATA_PTR		pEPData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pCount
)
{
	FTM_RET			nRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_CHAR		pLimit[512];
	FTM_INT			nSQLLen = 0;
	FTM_INT			nLimitLen = 0;

	_FTDM_DBIF_CB_GET_EP_DATA_PARAMS	xParams;

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
	nRet = sqlite3_exec(_pSQLiteDB, pSQL, _FTDM_DBIF_CB_getEPData, &xParams, &pErrMsg);
	if (nRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	*pCount = xParams.nCount;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_delEPData
(
	FTM_EPID				xEPID,
	FTM_ULONG				nIndex,
	FTM_ULONG				nCount
)
{
	FTM_RET			nRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_INT			nSQLLen = 0;

	nSQLLen += sprintf(pSQL, "DELETE FROM ep_%08lx 	WHERE ROWID >= nIndex ", xEPID);

	MESSAGE("SQL : %s\n", pSQL);
	nRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (nRet != SQLITE_OK)
	{
		ERROR("SQL error : %s\n", pErrMsg);	
		sqlite3_free(pErrMsg);

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_delEPDataWithTime
(
	FTM_EPID				xEPID,
	FTM_ULONG				xBeginTime,
	FTM_ULONG				xEndTime
)
{
	FTM_RET			nRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	FTM_CHAR		pSQL[1024];
	FTM_INT			nSQLLen = 0;
	FTM_BOOL		bConditionOn = FTM_BOOL_FALSE;

	nSQLLen += sprintf(pSQL, "DELETE FROM ep_%08lx ", xEPID);
	if (xBeginTime != 0)
	{
		if (bConditionOn == FTM_BOOL_FALSE)
		{
			bConditionOn = FTM_BOOL_TRUE;
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
		if (bConditionOn == FTM_BOOL_FALSE)
		{
			bConditionOn = FTM_BOOL_TRUE;
			nSQLLen += sprintf(&pSQL[nSQLLen], "WHERE ");
		}
		else
		{
			nSQLLen += sprintf(&pSQL[nSQLLen], "AND ");
		}
		nSQLLen += sprintf(&pSQL[nSQLLen], "TIME <= %lu ", xEndTime);
	}


	MESSAGE("SQL : %s\n", pSQL);
	nRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (nRet != SQLITE_OK)
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
static int _FTDM_DBIF_CB_getEPDataCount(void *pData, int nArgc, char **pArgv, char **pColName)
{
	FTM_ULONG_PTR pnCount = (FTM_ULONG_PTR)pData;

	if (nArgc != 0)
	{
		*pnCount = atoi(pArgv[0]);
	}
	return	FTM_RET_OK;
}

FTM_RET	FTDM_DBIF_getEPDataCount
(
	FTM_EPID			xEPID,
	FTM_ULONG_PTR		pCount
)
{
    int     nRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;

    sprintf(strSQL, "SELECT COUNT(*) FROM ep_%08lx", xEPID);
    nRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_CB_getEPDataCount, pCount, &strErrMsg);
    if (nRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", strErrMsg);
        sqlite3_free(strErrMsg);

    	return  FTM_RET_ERROR;
    }

	return	FTM_RET_OK;
}

/*************************************************************************
 *
 *************************************************************************/
FTM_RET	FTDM_DBIF_getEPDataCountWithTime
(
	FTM_EPID			xEPID,
	FTM_ULONG			xBeginTime,
	FTM_ULONG			xEndTime,
	FTM_ULONG_PTR		pCount
)
{
    int     nRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;

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
    nRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_CB_getNodeCount, pCount, &strErrMsg);
    if (nRet != SQLITE_OK)
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
                pParams->bExist = FTM_BOOL_TRUE;
                break;
            }
        }

    }

    return  0;
}

FTM_RET _FTDM_BDIF_isExistTable
(
	FTM_CHAR_PTR	pTableName, 
	FTM_BOOL_PTR 	pExist
)
{
    int     nRet;
    _FTDM_DBIF_CB_EXIST_TABLE_PARAMS xParams = { .bExist = FTM_BOOL_FALSE, .pName = pTableName};
    FTM_CHAR_PTR	pSQL = "select name from sqlite_master where type='table' order by name";
    FTM_CHAR_PTR	pErrMsg = NULL;

    nRet = sqlite3_exec(_pSQLiteDB, pSQL, _FTDM_DBIF_CB_isExistTable, &xParams, &pErrMsg);
    if (nRet != SQLITE_OK)
    {
        ERROR("SQL error : %s\n", pErrMsg);
        sqlite3_free(pErrMsg);

        return  FTM_RET_ERROR;
    }

    *pExist = xParams.bExist;

    return  FTM_RET_OK;
}

FTM_RET	_FTDM_BDIF_createNodeInfoTable
(
	FTM_CHAR_PTR	pTableName
)
{
	int	nRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
	char			pSQL[1024];

	sprintf(pSQL, "CREATE TABLE %s ("\
						"DID		TEXT PRIMARY KEY,"\
						"TYPE		INT,"\
						"LOC		TEXT,"\
						"INTERVAL	INT,"\
						"OPT0		TEXT,"\
						"OPT1		TEXT,"\
						"OPT2		TEXT,"\
						"OPT3		TEXT)", pTableName);

	nRet = sqlite3_exec(_pSQLiteDB, pSQL, NULL, 0, &pErrMsg);
	if (nRet != SQLITE_OK)
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
			pParams->bExist = FTM_BOOL_TRUE;
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
	_FTDM_DBIF_CB_IS_EXIST_PARAMS xParams = {.bExist = FTM_BOOL_TRUE };
    int     nRet;
    char    strSQL[1024];
    char    *strErrMsg = NULL;

    sprintf(strSQL, "SELECT COUNT(DID) FROM %s WHERE DID = '%s'", pTableName, pDID);
    nRet = sqlite3_exec(_pSQLiteDB, strSQL, _FTDM_DBIF_CB_isExist, &xParams, &strErrMsg);
    if (nRet != SQLITE_OK)
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
FTM_RET	_FTDM_BDIF_createEPInfoTable
(
	FTM_CHAR_PTR	pTableName
)
{
	int	nRet;
	FTM_CHAR_PTR	pErrMsg = NULL;
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

		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}


