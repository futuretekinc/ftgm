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
#include "ftom_client.h"

FTM_RET	FTOM_CLIENT_CMD_EPList
(
	FTOM_CLIENT_PTR	pClient,
	FTM_EP_TYPE		xType

)
{
	FTM_RET		xRet;
	FTM_ULONG	ulCount;

	xRet = FTOM_CLIENT_EP_count(pClient, xType, NULL, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		FTM_CHAR		pEPIDs[50][FTM_EPID_LEN+1];
		FTM_ULONG		ulStart = 0;

		MESSAGE("\n# EP Information\n");
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
					MESSAGE("%16s ", "");
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

FTM_RET	FTOM_CLIENT_CMD_EPInfo
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pEPID
)
{
	ASSERT(pClient != NULL);
	ASSERT(pEPID != NULL);

	FTM_RET	xRet;
	FTM_EP	xEPInfo;
	FTM_EP_DATA	xData;
	FTM_BOOL	bRun;
	FTM_ULONG	ulCount;
	FTM_CHAR	pValue[32];
			
	xRet = FTOM_CLIENT_EP_get(pClient, pEPID, &xEPInfo);	
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("EP[%s] not found!\n", pEPID);
		return	FTM_RET_OK;
	}

	MESSAGE("EP Information\n");
	FTM_EP_print(&xEPInfo);

	xRet = FTOM_CLIENT_EP_isRun(pClient, pEPID, &bRun);
	if (xRet == FTM_RET_OK)
	{
		MESSAGE("%16s : %s\n", "Status", (bRun)?"Run":"Stop");
	}

	xRet = FTOM_CLIENT_EP_DATA_count(pClient, pEPID, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		MESSAGE("%16s : %lu\n", "COUNT", ulCount);
	}
		
	xRet = FTOM_CLIENT_EP_DATA_getLast(pClient, pEPID, &xData);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_EP_DATA_snprint(pValue, sizeof(pValue), &xData);	
		if (xRet == FTM_RET_OK)
		{
			MESSAGE("%16s : %s\n", "VALUE", pValue);
		}
		else
		{
			MESSAGE("%16s : INVALID\n", "VALUE");
		}
	}
	else
	{
		MESSAGE("%16s : NOT EXISTS\n", "VALUE");
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
	FTM_INT		i;
	FTM_CHAR	pPID[FTM_DID_LEN + 1];
	FTM_CHAR	pDID[FTM_DID_LEN + 1];
	FTM_EP		xInfo;
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;

	memset(pPID, 0, sizeof(pPID));
	memset(pDID, 0, sizeof(pDID));

	if (nArgc < 1)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	if (nArgc == 1)
	{
		FTOM_CLIENT_CMD_EPList(pClient, 0);
	}
	else if (strcmp(pArgv[1], "add") == 0)
	{
		FTM_INT	i, nOpt;


		FTM_EP_setDefault(&xInfo);

		optind = 2;
		if ((nOpt = getopt(nArgc, pArgv, "n:u:i:t:")) != -1)
		{
			switch(nOpt)
			{
			case	'n':	
				{
					strncpy(xInfo.pName, optarg, sizeof(xInfo.pName) - 1);
				}
				break;

			case	'u':
				{
					strncpy(xInfo.pUnit, optarg, sizeof(xInfo.pUnit) - 1);
				}
				break;

			case	'i':
				{
					xInfo.ulUpdateInterval = strtoul(optarg, NULL, 10);
				}
				break;

			case	'T':
				{
					if (strcasecmp(optarg, "temperature") == 0)
					{
						xInfo.xType = FTM_EP_TYPE_TEMPERATURE;	
					}
					else if (strcasecmp(optarg, "humidity") == 0)
					{
						xInfo.xType = FTM_EP_TYPE_HUMIDITY;	
					}
				}
				break;

			case	't':
				{
					xInfo.ulTimeout = strtoul(optarg, NULL, 10);
				}
				break;

			default:
				{
					return	FTM_RET_INVALID_ARGUMENTS;
				}

			}
		}

		if (nArgc < 4)
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}

		strncpy(xInfo.pEPID, pArgv[2], FTM_EPID_LEN);

		for(i = 0 ; i < sizeof(xInfo.pDID) - 1 && i < strlen(pArgv[3]); i ++)
		{
			xInfo.pDID[i] = toupper(pArgv[3][i]);
		}

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

	}
	else if (strcmp(pArgv[1], "del") == 0)
	{
		FTM_CHAR	pEPID[FTM_EPID_LEN+1];

		if (nArgc != 3)
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}

		strncpy(pEPID, pArgv[2], FTM_EPID_LEN);
		xRet = FTOM_CLIENT_EP_destroy(pClient, pEPID);	
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to destroy EP[%s]\n", pArgv[2]);
		}
	}
	else if (strcmp(pArgv[1], "list") == 0)
	{
		FTM_ULONG	ulCount;

		if (nArgc == 2)
		{
			FTOM_CLIENT_CMD_EPList(pClient, 0);
		}
		else if (nArgc == 3)
		{
			FTM_EP_TYPE	xType;

			xType = strtoul(pArgv[2], 0, 16);

			xRet = FTOM_CLIENT_EP_count(pClient, xType, NULL, &ulCount);
			if (xRet != FTM_RET_OK)
			{
				MESSAGE("Failed to get EP Count!\n");
			}
			else
			{
				FTOM_CLIENT_CMD_EPList(pClient, xType);
			}
		}
	}
	else if (strcasecmp(pArgv[1], "info") == 0)
	{
		FTOM_CLIENT_CMD_EPInfo(pClient, pArgv[2]);
	}
	else if (strcmp(pArgv[1], "count") == 0)
	{
		FTM_ULONG	ulCount;

		xRet = FTOM_CLIENT_EP_count(pClient, 0, NULL, &ulCount);
		if (xRet != FTM_RET_OK)
		{
			MESSAGE("%s : ERROR - %lu\n", pArgv[0], xRet);
		}
		else
		{
			MESSAGE("EP Count : %lu\n", ulCount);	
		}
	
	}
	else 
	{
		FTM_CHAR	pEPID[FTM_EPID_LEN+1];
		FTM_EP			xInfo;

		strncpy(pEPID, pArgv[1], FTM_EPID_LEN);
		xRet = FTOM_CLIENT_EP_get(pClient, pEPID, &xInfo);
		if (xRet != FTM_RET_OK)
		{
			MESSAGE("EP[%s] not exists.\n", pEPID);
		}
		else	
		{
			switch(nArgc)
			{
			case	2:
				{
					FTOM_CLIENT_CMD_EPInfo(pClient, pArgv[1]);	
				}
				break;
	
			case	3:
				{
					if (strcasecmp(pArgv[2], "start") == 0)
					{
						if (xInfo.bEnable)
						{
							MESSAGE("EP[%s] has been already started.\n", xInfo.pEPID);
							break;
						}

						xInfo.bEnable = FTM_TRUE;
						xRet = FTOM_CLIENT_EP_set(pClient, xInfo.pEPID, FTM_EP_FIELD_ENABLE, &xInfo);
					}
					else if (strcasecmp(pArgv[2], "stop") == 0)
					{
						if (!xInfo.bEnable)
						{
							MESSAGE("EP[%s] did not start.\n", xInfo.pEPID);
							break;
						}
						xInfo.bEnable = FTM_FALSE;
						xRet = FTOM_CLIENT_EP_set(pClient, xInfo.pEPID, FTM_EP_FIELD_ENABLE, &xInfo);
					}

				}
				break;

			case	4:
				{
					if (strcasecmp(pArgv[2], "name") == 0)
					{
						if (strlen(pArgv[3]) > FTM_NAME_LEN)
						{
							MESSAGE("Name is too long.\n");
						}
						else
						{
							strcpy(xInfo.pName, pArgv[3]);	
						}

						xRet = FTOM_CLIENT_EP_set(pClient, xInfo.pEPID, FTM_EP_FIELD_NAME, &xInfo);
					}
				}
				break;

			case	5:
				{
					if (strcasecmp(pArgv[2], "data") == 0)
					{
						FTM_ULONG		ulStartIndex;
						FTM_ULONG		ulCount;
						FTM_EP_DATA_PTR	pData;
				
						ulStartIndex = strtoul(pArgv[3], 0, 10);
						if (nArgc == 5)
						{
							ulCount = strtoul(pArgv[4], 0, 10);
						}
						else
						{
							ulCount = 10;
						}
				
						pData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA) * ulCount);
						if (pData == NULL)
						{
							MESSAGE("Not enough memory.\n");	
						}
						else
						{
							xRet = FTOM_CLIENT_EP_DATA_getList(pClient, pEPID, ulStartIndex, pData, ulCount, &ulCount);
							if (xRet != FTM_RET_OK)
							{
								MESSAGE("%s : ERROR - %lu\n", pArgv[0], xRet);
							}
							else
							{
								FTM_CHAR	pBuff[64];
				
								for(i = 0 ; i < ulCount ; i++)
								{
									FTM_EP_DATA_snprint(pBuff, sizeof(pBuff), &pData[i]);		
									MESSAGE("%3lu : %s\n", ulStartIndex + i, pBuff);
								}
							}
						}
				
						FTM_MEM_free(pData);
					}
				}
				break;

			default:
				MESSAGE("Invalid arguemtns.\n");
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_CMD_EP_DATA(FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData)
{
	FTM_RET			xRet;
	FTM_INT			nOpt = 0;
	FTM_ULONG		nBeginTime = 0;
	FTM_ULONG		nEndTime = 0;
	FTM_CHAR		pEPID[FTM_EPID_LEN+1];
	FTM_EP_DATA		xData;
	FTM_EP_DATA_PTR	pEPData;	
	FTM_ULONG		nStartIndex=0;
	FTM_ULONG		nMaxCount=50;
	FTM_ULONG		ulCount=0;
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;

	if (nArgc < 2)
	{
		ERROR2(FTM_RET_INVALID_ARGUMENTS, "Invalid arguments[nArgc = %d]\n", nArgc);	
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	if (strcasecmp(pArgv[1], "add") == 0)
	{
		struct tm	xTM;
		FTM_CHAR	pTemp[128];

		if (nArgc != 6)
		{
			ERROR2(FTM_RET_INVALID_ARGUMENTS, "Invalid arguments[nArgc = %d]\n", nArgc);	
			return	FTM_RET_INVALID_ARGUMENTS;
		}

		strncpy(pEPID, pArgv[2], FTM_EPID_LEN);

		switch(toupper(pArgv[5][0]))
		{
		case	'I':	
			{
				FTM_EP_DATA_init(&xData, FTM_VALUE_TYPE_INT, &pArgv[5][1]);
			}

			break;

		case	'F':	
			{
				FTM_EP_DATA_init(&xData, FTM_VALUE_TYPE_FLOAT, &pArgv[5][1]);
			} 
			break;

		case	'U':	
			{	
				FTM_EP_DATA_init(&xData, FTM_VALUE_TYPE_ULONG, &pArgv[5][1]);
			}	
			break;

		case	'0': case	'1': case	'2': case	'3':	
		case	'4': case	'5': case	'6': case	'7':	
		case	'8':	
		case	'9':	
			{
				FTM_EP_DATA_init(&xData, FTM_VALUE_TYPE_ULONG, pArgv[5]);
			}
			break;

		default:	
			{
				MESSAGE("Invalid arguments!\n");	
				return	FTM_RET_INVALID_ARGUMENTS;
			}
		}

		snprintf(pTemp, sizeof(pTemp), "%s %s", pArgv[3], pArgv[4]);
		strptime(pTemp, "%Y-%m-%d %H:%M:%S", &xTM);
		xData.ulTime	= (FTM_ULONG)mktime(&xTM);

		xRet = FTOM_CLIENT_EP_DATA_add(pClient, pEPID, &xData);
		if (xRet == FTM_RET_OK)
		{
			MESSAGE("EndPoint data appending done successfully!\n");	
		}
		else
		{
			ERROR2(xRet, "EndPoint data appending failed.\n");	
		}
	}
	else if (strcasecmp(pArgv[1], "del") == 0)
	{
		optind = 2;
		if ((nOpt = getopt(nArgc, pArgv, "a:i:t:")) == -1)
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}

		switch(toupper(nOpt))
		{
		case	'A':
			{
				if (nArgc == 4)
				{
					strncpy(pEPID, pArgv[3], FTM_EPID_LEN);
				}	
				else
				{
					MESSAGE("Invalid parameters\n");
					return	FTM_RET_INVALID_ARGUMENTS;
				}
			}
			break;

		case	'I':
			{
				if (nArgc == 6)
				{
					nMaxCount 	= strtol(pArgv[5], NULL, 10);
					nStartIndex = strtol(pArgv[4], NULL, 10);
					strncpy(pEPID, pArgv[3], FTM_EPID_LEN);
				}	
				else if (nArgc == 5)
				{
					nStartIndex = strtol(pArgv[4], NULL, 10);
					strncpy(pEPID, pArgv[3], FTM_EPID_LEN);
				}
				else
				{
					MESSAGE("Invalid parameters\n");
					return	FTM_RET_INVALID_ARGUMENTS;
				}
			}
			break;

		case	'T':
			{
				struct tm	xTMBegin;
				struct tm	xTMEnd;
				FTM_CHAR	pTemp[128];

				if (nArgc == 8)
				{
					strncpy(pEPID, pArgv[3], FTM_EPID_LEN);

					snprintf(pTemp, sizeof(pTemp), "%s %s", pArgv[4], pArgv[5]);
					strptime(pTemp, "%Y-%m-%d %H:%M:%S", &xTMBegin);
					nBeginTime = (FTM_ULONG)mktime(&xTMBegin);

					snprintf(pTemp, sizeof(pTemp), "%s %s", pArgv[6], pArgv[7]);
					strptime(pTemp, "%Y-%m-%d %H:%M:%S", &xTMEnd);
					nEndTime = (FTM_ULONG)mktime(&xTMEnd);
				}
				else if (nArgc == 7)
				{
					strncpy(pEPID, pArgv[3], FTM_EPID_LEN);

					snprintf(pTemp, sizeof(pTemp), "%s %s", pArgv[4], pArgv[5]);
					strptime(pTemp, "%Y-%m-%d %H:%M:%S", &xTMBegin);
					nBeginTime = (FTM_ULONG)mktime(&xTMBegin);

					nMaxCount 	= strtol(pArgv[6], NULL, 10);
				}
				else if (nArgc == 6)
				{
					strncpy(pEPID, pArgv[3], FTM_EPID_LEN);

					snprintf(pTemp, sizeof(pTemp), "%s %s", pArgv[4], pArgv[5]);
					strptime(pTemp, "%Y-%m-%d %H:%M:%S", &xTMBegin);
					nBeginTime = (FTM_ULONG)mktime(&xTMBegin);
				}
				else
				{
					MESSAGE("Invalid parameters\n");
					return	FTM_RET_INVALID_ARGUMENTS;
				}
/*
				xRet = FTOM_CLIENT_EP_DATA_delWithTime(pClient, pEPID, nBeginTime, nEndTime);
				if (xRet == FTM_RET_OK)
				{
					MESSAGE("EndPoint data deleted successfully!\n");	
				}
				else
				{
					ERROR2(xRet, "EndPoint data deleting failed.\n");	
				}
*/
			}
			break;

		default:
			{
				return	FTM_RET_INVALID_ARGUMENTS;
			}
		}

	}
	else if (strcasecmp(pArgv[1], "count") == 0)
	{
		optind = 2;
		if ((nOpt = getopt(nArgc, pArgv, "a:i:t:")) == -1)
		{
			MESSAGE("Invalid arguments\n");	
			return	FTM_RET_INVALID_ARGUMENTS;
		}

		switch(toupper(nOpt))
		{
		case	'A':
		case	'I':
			{
#if 0
				FTM_ULONG	ulBeginTime;
				FTM_ULONG	ulEndTime;
				FTM_ULONG	ulCount;

				if (nArgc != 4) 
				{
					MESSAGE("Invalid arguments\n");	
					return	FTM_RET_INVALID_ARGUMENTS;
				}
				
				strncpy(pEPID, pArgv[3], FTM_EPID_LEN);

				xRet = FTOM_CLIENT_EP_DATA_info(pClient, pEPID, &ulBeginTime, &ulEndTime, &ulCount);
				if (xRet == FTM_RET_OK)
				{
					MESSAGE("      EPID : %s\n", pEPID);
					MESSAGE("DATA COUNT : %lu\n", ulCount);
					MESSAGE("BEGIN TIME : %s\n", ctime(&ulBeginTime));
					MESSAGE("  END TIME : %s\n", ctime(&ulEndTime));
				}
				else
				{
					TRACE("FTOM_CLIENT_EP_DATA_count error [%08lx]\n", xRet);	
				}
#endif

			}
			break;

		case	'T':
			{
				if (nArgc != 8)
				{
					return	FTM_RET_INVALID_ARGUMENTS;
				}

				strncpy(pEPID, pArgv[3], FTM_EPID_LEN);
			}
			break;

		default:
			return	FTM_RET_INVALID_ARGUMENTS;
		}
	}
	else if (strcasecmp(pArgv[1], "get") == 0)
	{
		optind = 2;
		if ((nOpt = getopt(nArgc, pArgv, "i:t:")) == -1)
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}

		switch(toupper(nOpt))
		{
		case	'I':
			{
				switch(nArgc)
				{
				case	6:
					nMaxCount 	= strtol(pArgv[5], NULL, 10);
					nStartIndex = strtol(pArgv[4], NULL, 10);
					break;
					
				case	5:
					nMaxCount 	= strtol(pArgv[4], NULL, 10);
					break;
				
				case	4:
					break;

				default:
					return	FTM_RET_INVALID_ARGUMENTS;
				}

				strncpy(pEPID, pArgv[3], FTM_EPID_LEN);

				pEPData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA) * nMaxCount);
				if (pEPData == NULL)
				{
					MESSAGE("System not enough memory!\n");
					return	FTM_RET_NOT_ENOUGH_MEMORY;		
				}

				xRet = FTOM_CLIENT_EP_DATA_getList(pClient, 
						pEPID, 
						nStartIndex,
						pEPData, 
						nMaxCount, 
						&ulCount);
				TRACE("FTOM_CLIENT_getEPData(hClient, %s, %d, %d, pEPData, %d, %d) = %08lx\n",
						pEPID, nBeginTime, nEndTime, nMaxCount, ulCount, xRet);
				if (xRet == FTM_RET_OK)
				{
					FTM_INT	i;

					MESSAGE("%8s %32s %8s\n", "INDEX", "DATE", "VALUE");	
					for(i = 0 ; i < ulCount ; i++)
					{
						FTM_CHAR	pTime[64];
						time_t		xTime = pEPData[i].ulTime;

						strftime(pTime, sizeof(pTime), "%Y-%m-%d %H:%M:%S", gmtime(&xTime));
						MESSAGE("%8lu %32s %8s\n", nStartIndex + i, pTime, FTM_VALUE_print(&pEPData[i].xValue));	
					}
				}
				else
				{
					MESSAGE("EndPoint data loading failed [ ERROR = %08lx ]\n", xRet);	
				}

				FTM_MEM_free(pEPData);
			}
			break;

		case	'T':
			{
				struct tm	xTMBegin;
				struct tm	xTMEnd;
				FTM_CHAR	pTemp[128];

				if (nArgc == optind + 4)
				{
					strncpy(pEPID, pArgv[optind++], FTM_EPID_LEN);

					snprintf(pTemp, sizeof(pTemp), "%s %s", pArgv[optind], pArgv[optind+1]);
					optind+= 2;
					strptime(pTemp, "%Y-%m-%d %H:%M:%S", &xTMBegin);
					nBeginTime = (FTM_ULONG)mktime(&xTMBegin);
				}
				else if (nArgc == optind + 6)
				{
					strncpy(pEPID, pArgv[optind++], FTM_EPID_LEN);

					snprintf(pTemp, sizeof(pTemp), "%s %s", pArgv[optind], pArgv[optind+1]);
					optind+= 2;
					strptime(pTemp, "%Y-%m-%d %H:%M:%S", &xTMBegin);
					nBeginTime = (FTM_ULONG)mktime(&xTMBegin);

					snprintf(pTemp, sizeof(pTemp), "%s %s", pArgv[optind], pArgv[optind+1]);
					optind+= 2;
					strptime(pTemp, "%Y-%m-%d %H:%M:%S", &xTMEnd);
					nEndTime = (FTM_ULONG)mktime(&xTMEnd);
				}
				else
				{
					MESSAGE("Invalid parameters\n");
					return	FTM_RET_INVALID_ARGUMENTS;

				}
			}
			break;
		}
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	return	FTM_RET_OK;
}

