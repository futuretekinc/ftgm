#include <ctype.h>
#include <stdlib.h>    
#define	__USE_XOPEN
#include <stdio.h>
#include <string.h>    
#include <unistd.h>    
#define _XOPEN_SOURCE       /* See feature_test_macros(7) */
#include <time.h>    
#include <sys/socket.h> 
#include <arpa/inet.h>
#include "libconfig.h"
#include "ftm.h"
#include "ftm_opt.h"
#include "ftom_client.h"

FTM_RET	FTOM_CLIENT_CMD_printEPList
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)

{
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;
	FTM_RET		xRet;
	FTM_ULONG	ulCount;
	FTM_EP_TYPE	xType = 0;

	xRet = FTOM_CLIENT_EP_count(pClient, xType, NULL, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		FTM_CHAR		pEPIDs[50][FTM_EPID_LEN+1];
		FTM_ULONG		ulStart = 0;

		MESSAGE("\n[ EP INFORMATION ]\n");
		MESSAGE("%16s %16s %16s %16s %8s %8s %8s %8s %8s %24s\n", "EPID", "TYPE", "NAME", "DID", "STATE", "VALUE", "UNIT", "UPDATE", "REPORT", "TIME");
		while(ulCount > 0)
		{
			FTM_INT		i;
			FTM_ULONG	ulReadCount = 0;
			if (ulCount > 50)
			{
				FTOM_CLIENT_EP_getList(pClient, xType, NULL, ulStart, pEPIDs, 50, &ulReadCount);
			}
			else
			{
				FTOM_CLIENT_EP_getList(pClient, xType, NULL, ulStart, pEPIDs, ulCount, &ulReadCount);
			}

			if (ulReadCount == 0)
			{
				break;
			}

			for(i = 0 ; i< ulReadCount ; i++)
			{
				FTM_EP	xInfo;
	
				xRet = FTOM_CLIENT_EP_get(pClient, pEPIDs[i], &xInfo);
				if (xRet == FTM_RET_OK)
				{
					FTM_CHAR	pTimeString[64];
					FTM_EP_DATA	xData;
				
					xRet = FTOM_CLIENT_EP_DATA_getLast(pClient, pEPIDs[i], &xData);
					if (xRet == FTM_RET_OK)
					{
						ctime_r((time_t *)&xData.ulTime, pTimeString);
						if (strlen(pTimeString) != 0)
						{
							pTimeString[strlen(pTimeString) - 1] = '\0';
						}
					}
					else
					{
						memset(pTimeString, 0, sizeof(pTimeString));	
					}

					MESSAGE("%16s ", xInfo.pEPID);
					MESSAGE("%16s ", FTM_EP_typeString(xInfo.xType));
					MESSAGE("%16s ", xInfo.pName);
					MESSAGE("%16s ", xInfo.pDID);
					MESSAGE("%8s ", "Unknown");

					MESSAGE("%8s ", FTM_VALUE_print(&xData.xValue));
					MESSAGE("%8s ", xInfo.pUnit);
					MESSAGE("%8lu ", xInfo.ulUpdateInterval);
					MESSAGE("%8lu ", xInfo.ulReportInterval);
					MESSAGE("%24s\n", pTimeString);
				}
			}

			ulStart += ulReadCount;
			ulCount  -= ulReadCount;

		}
	}

	return	xRet;
}

FTM_RET	FTOM_CLIENT_CMD_printEPInfo
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);
	ASSERT(pArgv != NULL);

	FTM_RET		xRet;
	FTM_EP		xEPInfo;
	FTM_EP_DATA	xData;
	FTM_BOOL	bRun;
	FTM_ULONG	ulBegin, ulEnd;
	FTM_ULONG	ulCount;
	FTM_CHAR	pValue[32];
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xRet = FTOM_CLIENT_EP_get(pClient, pArgv[1], &xEPInfo);	
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("EP[%s] not found!\n", pArgv[1]);
		return	FTM_RET_OK;
	}

	MESSAGE("\n[ EP INFORMATION ]\n");
	FTM_EP_print(&xEPInfo);

	xRet = FTOM_CLIENT_EP_isRun(pClient, pArgv[1], &bRun);
	if (xRet == FTM_RET_OK)
	{
		MESSAGE("%16s : %s\n", "Status", (bRun)?"Run":"Stop");
	}

	xRet = FTOM_CLIENT_EP_DATA_info(pClient, pArgv[1], &ulBegin, &ulEnd, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		MESSAGE("%16s : %s", "Begin", ctime((const time_t *)&ulBegin));
		MESSAGE("%16s : %s", "End", ctime((const time_t *)&ulEnd));
		MESSAGE("%16s : %lu\n", "Count", ulCount);
	}
		
	xRet = FTOM_CLIENT_EP_DATA_getLast(pClient, pArgv[1], &xData);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_EP_DATA_snprint(pValue, sizeof(pValue), &xData);	
		if (xRet == FTM_RET_OK)
		{
			MESSAGE("%16s : %s\n", "Value", pValue);
		}
		else
		{
			MESSAGE("%16s : Invalid\n", "Value");
		}
	}
	else
	{
		MESSAGE("%16s : Not Exists\n", "Value");
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_CMD_EP_showDataList
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPID != NULL);

	FTM_RET	xRet;
	FTM_EP_DATA_PTR	pData = NULL;
	FTM_INT	i;

	if (ulCount == 0)
	{
		goto finish;
	}
	else if (ulCount > 1000)
	{
		MESSAGE("The count is too many![count <= 1000]\n"); 
		goto finish;
	}

	pData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA) * ulCount);
	if (pData == NULL)
	{
		MESSAGE("Not enough memory![size = %lu]\n", sizeof(FTM_EP_DATA) * ulCount);	
		goto finish;
	}

	xRet = FTOM_CLIENT_EP_DATA_getList(pClient, pEPID, ulIndex, pData, ulCount, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Failed to get EP[%s] datas.\n", pEPID);	
		goto finish;
	}

	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_CHAR	pTime[64];

		ctime_r((const time_t *)&pData[i].ulTime, pTime);
		pTime[strlen(pTime) - 1] = '\0';

		MESSAGE("%4d %16s %8s\n", i+1, pTime, FTM_VALUE_print(&pData[i].xValue));
	}
finish:
	if (pData != NULL)
	{
		FTM_MEM_free(pData);	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_CMD_addEP
(
	FTM_SHELL_PTR	pShell,
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_VOID_PTR 	pData
)
{
	ASSERT(pShell != NULL);
	ASSERT(pArgv != NULL);

	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;
	FTM_RET	xRet;
	FTM_INT	i;
	FTM_EP	xInfo;
	FTM_OPT	pOpts[8];
	FTM_ULONG	ulOpts;

	FTM_EP_setDefault(&xInfo);

	if (nArgc < 3)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xRet = FTM_getOptions(nArgc, pArgv, "n:u:i:T:t:", pOpts, 8, &ulOpts);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Invalid arguments!\n");
		return	xRet;
	}

	for(i = 0 ; i < ulOpts ; i++)
	{
		switch(pOpts[i].xOpt)
		{
		case	'n':	
			{
				strncpy(xInfo.pName, pOpts[i].pParam, sizeof(xInfo.pName) - 1);
			}
			break;

		case	'u':
			{
				strncpy(xInfo.pUnit, pOpts[i].pParam, sizeof(xInfo.pUnit) - 1);
			}
			break;

		case	'i':
			{
				xInfo.ulUpdateInterval = strtoul(pOpts[i].pParam, NULL, 10);
			}
			break;

		case	'T':
			{
				if (strcasecmp(pOpts[i].pParam, "temperature") == 0)
				{
					xInfo.xType = FTM_EP_TYPE_TEMPERATURE;	
				}
				else if (strcasecmp(pOpts[i].pParam, "humidity") == 0)
				{
					xInfo.xType = FTM_EP_TYPE_HUMIDITY;	
				}
			}
			break;

		case	't':
			{
				xInfo.ulTimeout = strtoul(pOpts[i].pParam, NULL, 10);
			}
			break;

		default:
			return	FTM_RET_INVALID_ARGUMENTS;
		}
	}

	strncpy(xInfo.pEPID, pArgv[1], FTM_EPID_LEN);
	strncpy(xInfo.pDID, pArgv[2], FTM_DID_LEN);

	xRet = FTM_EP_isValid(&xInfo);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Invalid node information!\n");
		return	xRet;
	}

	xRet = FTOM_CLIENT_EP_create(pClient, &xInfo, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create EP[%s]\n", pArgv[2]);
	}

	return	xRet;

}

FTM_RET	FTOM_CLIENT_CMD_delEP
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
	FTM_RET	xRet;
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;
	FTM_CHAR	pEPID[FTM_EPID_LEN+1];

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	strncpy(pEPID, pArgv[1], FTM_EPID_LEN);
	xRet = FTOM_CLIENT_EP_destroy(pClient, pEPID);	
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to destroy EP[%s]\n", pEPID);
	}

	return	xRet;
}

FTM_RET	FTOM_CLIENT_CMD_printEPCount
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
	FTM_RET		xRet;
	FTM_ULONG	ulCount;
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;

	xRet = FTOM_CLIENT_EP_count(pClient, 0, NULL, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Failed to get EP count\n");
	}
	else
	{
		MESSAGE("EP Count : %lu\n", ulCount);	
	}

	return	xRet;
}

FTM_RET	FTOM_CLIENT_CMD_startEP
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
	FTM_RET		xRet;
	FTM_INT		i;
	FTM_EP		xInfo;
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xInfo.bEnable = FTM_TRUE;

	for(i = 1 ; i < nArgc ; i++)
	{
		xRet = FTOM_CLIENT_EP_set(pClient, pArgv[i], FTM_EP_FIELD_ENABLE, &xInfo);
		if (xRet != FTM_RET_OK)
		{
			MESSAGE("Failed to activate EP[%s].\n", pArgv[i]);
		}
		else
		{
			MESSAGE("The EP[%s] is activated.\n", pArgv[i]);	
		}
	}

	return	FTM_RET_OK;
}
	
FTM_RET	FTOM_CLIENT_CMD_stopEP
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
	FTM_RET		xRet;
	FTM_INT		i;
	FTM_EP		xInfo;
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xInfo.bEnable = FTM_FALSE;

	for(i = 1 ; i < nArgc ; i++)
	{
		xRet = FTOM_CLIENT_EP_set(pClient, pArgv[i], FTM_EP_FIELD_ENABLE, &xInfo);
		if (xRet != FTM_RET_OK)
		{
			MESSAGE("Failed to stop EP[%s].\n", pArgv[i]);
		}
		else
		{
			MESSAGE("The EP[%s] is stopped.\n", pArgv[i]);	
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_CMD_setEP
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
	FTM_RET		xRet;
	FTM_INT		i;
	FTM_EP		xInfo;
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;
	FTM_CHAR_PTR	pEPID;
	FTM_OPT		pOpts[8];
	FTM_ULONG	ulOpts;
	FTM_ULONG	ulFields = 0;

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	pEPID = pArgv[1];

	FTM_EP_setDefault(&xInfo);

	xRet = FTM_getOptions(nArgc, pArgv, "n:u:i:t:", pOpts, 8, &ulOpts);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Invalid arguments!\n");
		return	xRet;
	}

	for(i = 0 ; i < ulOpts ; i++)
	{
		switch(pOpts[i].xOpt)
		{
		case	'n':	
			{
				if (strlen(pOpts[i].pParam) > FTM_NAME_LEN)
				{
					return	FTM_RET_INVALID_ARGUMENTS;
				}

				strncpy(xInfo.pName, pOpts[i].pParam, sizeof(xInfo.pName) - 1);
				ulFields |= FTM_EP_FIELD_NAME;
			}
			break;

		case	'u':
			{
				if (strlen(pOpts[i].pParam) > FTM_UNIT_LEN)
				{
					return	FTM_RET_INVALID_ARGUMENTS;
				}

				strncpy(xInfo.pUnit, pOpts[i].pParam, sizeof(xInfo.pUnit) - 1);
				ulFields |= FTM_EP_FIELD_UNIT;
			}
			break;

		case	'i':
			{
				xInfo.ulUpdateInterval = strtoul(pOpts[i].pParam, NULL, 10);
				ulFields |= FTM_EP_FIELD_INTERVAL;
			}
			break;

		case	't':
			{
				xInfo.ulTimeout = strtoul(pOpts[i].pParam, NULL, 10);
				ulFields |= FTM_EP_FIELD_TIMEOUT;
			}
			break;
		}
	}

	xRet = FTOM_CLIENT_EP_set(pClient, pEPID, ulFields, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Failed to set EP[%s].\n", pEPID);
	}
	else
	{
		MESSAGE("The EP[%s] setting changed.\n", pEPID);	
	}

	return	xRet;
}


FTM_RET	FTOM_CLIENT_CMD_EPData
(
	FTM_SHELL_PTR	pShell,
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_VOID_PTR 	pData
)
{
	FTM_RET			xRet;
	FTM_EP			xInfo;
	FTM_CHAR_PTR	pEPID;

	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;

	if (nArgc < 3)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	pEPID = pArgv[1];

	xRet = FTOM_CLIENT_EP_get(pClient, pEPID, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("The EP[%s] is not exists.\n", pEPID);
		return	xRet;
	}


	if (strcasecmp(pArgv[2], "del") == 0)
	{
		FTM_INT		i;
		FTM_ULONG	ulOpts;
		FTM_ULONG	ulIndex = 0;
		FTM_ULONG	ulCount = 0;
		FTM_ULONG	ulDeletedCount = 0;
		FTM_ULONG	ulBeginTime;
		FTM_ULONG	ulEndTime;
		FTM_TIME	xTime;
		FTM_OPT		pOpts[8];
	
		xRet = FTM_getOptions(nArgc, pArgv, "i:c:s:e:", pOpts, 8, &ulOpts);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Invalid arguments!\n");
			return	xRet;
		}
	
		for(i = 0 ; i < ulOpts ; i++)
		{
			switch(pOpts[i].xOpt)
			{
			case	'i':	
				{
					if ((ulBeginTime != 0) || (ulEndTime != 0))
					{
						return	FTM_RET_INVALID_ARGUMENTS;
					}

					ulIndex = strtoul(pOpts[i].pParam, 0, 10);
				}
				break;
	
			case	'c':	
				{
					if ((ulBeginTime != 0) || (ulEndTime != 0))
					{
						return	FTM_RET_INVALID_ARGUMENTS;
					}

					ulCount = strtoul(pOpts[i].pParam, 0, 10);
				}
				break;
	
			case	's':
				{
					if ((ulIndex != 0) || (ulCount != 0))
					{
						return	FTM_RET_INVALID_ARGUMENTS;
					}

					xRet = FTM_TIME_setString(&xTime, pOpts[i].pParam);
					if (xRet != FTM_RET_OK)
					{
						return	FTM_RET_INVALID_ARGUMENTS;	
					}
	
					FTM_TIME_toSecs(&xTime, &ulBeginTime);
				}
				break;
	
			case	'e':
				{
					if ((ulIndex != 0) || (ulCount != 0))
					{
						return	FTM_RET_INVALID_ARGUMENTS;
					}

					xRet = FTM_TIME_setString(&xTime, pOpts[i].pParam);
					if (xRet != FTM_RET_OK)
					{
						return	FTM_RET_INVALID_ARGUMENTS;	
					}
	
					FTM_TIME_toSecs(&xTime, &ulEndTime);
				}
				break;
	
	
			}
		}
	
		if ((ulIndex != 0) || (ulCount != 0))
		{
			xRet = FTOM_CLIENT_EP_DATA_del(pClient, pArgv[1], ulIndex, ulCount, &ulDeletedCount);
			if (xRet != FTM_RET_OK)
			{
				MESSAGE("Failed to remove data from EP[%s]\n", pArgv[1]);	
			}
		}
		else if ((ulBeginTime != 0) || (ulEndTime != 0))
		{
			xRet = FTOM_CLIENT_EP_DATA_delWithTime(pClient, pArgv[1], ulBeginTime, ulEndTime, &ulDeletedCount);
			if (xRet != FTM_RET_OK)
			{
				MESSAGE("Failed to remove data from EP[%s]\n", pArgv[1]);	
			}
		}
		else
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		
		}
	}
	else if (strcasecmp(pArgv[2], "get") == 0)
	{
		FTM_INT		i;
		FTM_OPT		pOpts[8];
		FTM_ULONG	ulMethod = 0;
		FTM_ULONG	ulOpts;
		FTM_ULONG	ulIndex = 0;
		FTM_ULONG	ulMaxCount = 100;
		FTM_ULONG	ulCount = 0;
		FTM_ULONG	ulStart = 0;
		FTM_ULONG	ulEnd = 0;

		xRet = FTM_getOptions(nArgc, pArgv, "m:i:c:s:e:", pOpts, 8, &ulOpts);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Invalid arguments!\n");
			return	xRet;
		}

		for(i = 0 ; i < ulOpts ; i++)
		{
			switch(pOpts[i].xOpt)
			{
			case	'm':
				{
					if (strcasecmp(pOpts[i].pParam, "index") == 0)
					{
						ulMethod = 1;	
					}
					else if (strcasecmp(pOpts[i].pParam, "time") == 0)
					{
						ulMethod = 2;	
					}
					else 
					{
						return	FTM_RET_INVALID_ARGUMENTS;	
					}
				}
				break;

			case	'i':
				{
					if (ulMethod != 1)
					{
						return	FTM_RET_INVALID_ARGUMENTS;
					}

					ulIndex = strtoul(pOpts[i].pParam, NULL, 10);
				}
				break;

			case	'c':
				{
					ulMaxCount = strtoul(pOpts[i].pParam, NULL, 10);
				}
				break;

			case	's':	
				{
					FTM_TIME	xTime;

					if (ulMethod != 2)
					{
						return	FTM_RET_INVALID_ARGUMENTS;
					}

					xRet = FTM_TIME_setString(&xTime, pOpts[i].pParam);
					if (xRet != FTM_RET_OK)
					{
						MESSAGE("Invalid time!\n");
						return	FTM_RET_INVALID_ARGUMENTS;
					}
				
					FTM_TIME_toSecs(&xTime, &ulStart);
				}
				break;

			case	'e':
				{
					FTM_TIME	xTime;

					if (ulMethod != 2)
					{
						return	FTM_RET_INVALID_ARGUMENTS;
					}

					xRet = FTM_TIME_setString(&xTime, pOpts[i].pParam);
					if (xRet != FTM_RET_OK)
					{
						MESSAGE("Invalid time!\n");
						return	FTM_RET_INVALID_ARGUMENTS;
					}
				
					FTM_TIME_toSecs(&xTime, &ulEnd);
				}
				break;
			}
		}

		
		switch(ulMethod)
		{
		case	2:
			{
				FTM_EP_DATA_PTR	pEPData;
	
				pEPData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA) * ulMaxCount);
				if (pEPData == NULL)
				{
					MESSAGE("System not enough memory!\n");
					return	FTM_RET_NOT_ENOUGH_MEMORY;		
				}
		
				xRet = FTOM_CLIENT_EP_DATA_getListWithTime(pClient, pEPID, ulStart, ulEnd, pEPData, ulMaxCount, &ulCount);
				if (xRet == FTM_RET_OK)
				{
					FTM_INT	i;
								
					MESSAGE("%8s %20s %8s\n", "INDEX", "DATE", "VALUE");	
					for(i = 0 ; i < ulCount ; i++)
					{
						FTM_TIME	xTime;
	
						FTM_TIME_setSeconds(&xTime, pEPData[i].ulTime);
						
						MESSAGE("%8d %20s %8s\n", i+1, FTM_TIME_printf(&xTime, NULL), FTM_VALUE_print(&pEPData[i].xValue));	
					}
				}
				else
				{
					MESSAGE("EndPoint data loading failed [ ERROR = %08lx ]\n", xRet);	
				}
				
				FTM_MEM_free(pEPData);
			}
			break;

		case	1:
			{
				FTM_EP_DATA_PTR	pEPData;
	
				pEPData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA) * ulMaxCount);
				if (pEPData == NULL)
				{
					MESSAGE("System not enough memory!\n");
					return	FTM_RET_NOT_ENOUGH_MEMORY;		
				}
				
				xRet = FTOM_CLIENT_EP_DATA_getList(pClient, pEPID, ulIndex, pEPData, ulMaxCount, &ulCount);
				if (xRet == FTM_RET_OK)
				{
					FTM_INT	i;
				
					MESSAGE("%8s %20s %8s\n", "INDEX", "DATE", "VALUE");	
					for(i = 0 ; i < ulCount ; i++)
					{
						MESSAGE("%8lu %20s %8s\n", ulIndex + i, FTM_TIME_printf2(pEPData[i].ulTime, NULL), FTM_VALUE_print(&pEPData[i].xValue));	
					}
				}
				else
				{
					MESSAGE("EndPoint data loading failed [ ERROR = %08lx ]\n", xRet);	
				}
				
				FTM_MEM_free(pEPData);
			}
			break;

		default:
			return	FTM_RET_INVALID_ARGUMENTS;
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_CMD_EP
(
	FTM_SHELL_PTR	pShell,
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_VOID_PTR 	pData
)
{
	FTM_RET		xRet;

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	if (strcmp(pArgv[1], "add") == 0)
	{
		xRet = FTOM_CLIENT_CMD_addEP(pShell, nArgc - 1, &pArgv[1], pData);
	}
	else if (strcmp(pArgv[1], "del") == 0)
	{
		xRet = FTOM_CLIENT_CMD_delEP(pShell, nArgc - 1, &pArgv[1], pData);
	}
	else if (strcmp(pArgv[1], "list") == 0)
	{
		xRet = FTOM_CLIENT_CMD_printEPList(pShell, nArgc - 1, &pArgv[1], pData);
	}
	else if (strcasecmp(pArgv[1], "info") == 0)
	{
		xRet = FTOM_CLIENT_CMD_printEPInfo(pShell, nArgc - 1, &pArgv[1], pData);
	}
	else if (strcasecmp(pArgv[1], "count") == 0)
	{
		xRet = FTOM_CLIENT_CMD_printEPCount(pShell, nArgc - 1, &pArgv[1], pData);
	}
	else if (strcasecmp(pArgv[1], "start") == 0)
	{
		xRet = FTOM_CLIENT_CMD_startEP(pShell, nArgc - 1, &pArgv[1], pData);	
	}
	else if (strcasecmp(pArgv[1], "stop") == 0)
	{
		xRet = FTOM_CLIENT_CMD_stopEP(pShell, nArgc - 1, &pArgv[1], pData);	
	}
	else if (strcasecmp(pArgv[1], "set") == 0)
	{
		xRet = FTOM_CLIENT_CMD_setEP(pShell, nArgc - 1, &pArgv[1], pData);
	}
	else if (strcasecmp(pArgv[1], "data") == 0)
	{
		xRet = FTOM_CLIENT_CMD_EPData(pShell, nArgc - 1, &pArgv[1], pData);
	}
	else
	{
		xRet = FTM_RET_INVALID_ARGUMENTS;	
	}

	return	xRet;
}

