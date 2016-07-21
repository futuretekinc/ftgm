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
	FTM_ULONG	ulBegin, ulEnd;
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

	xRet = FTOM_CLIENT_EP_DATA_info(pClient, pEPID, &ulBegin, &ulEnd, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		MESSAGE("%16s : %s", "Begin", ctime((const time_t *)&ulBegin));
		MESSAGE("%16s : %s", "End", ctime((const time_t *)&ulEnd));
		MESSAGE("%16s : %lu\n", "Count", ulCount);
	}
		
	xRet = FTOM_CLIENT_EP_DATA_getLast(pClient, pEPID, &xData);
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

FTM_RET	FTOM_CLIENT_CMD_EP
(
	FTM_SHELL_PTR	pShell,
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_VOID_PTR 	pData
)
{
	FTM_RET		xRet;
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
			if (nArgc == 2)
			{
				FTOM_CLIENT_CMD_EPInfo(pClient, pArgv[1]);	
			}
			else
			{
				if (strcasecmp(pArgv[2], "start") == 0)
				{
					if (xInfo.bEnable)
					{
						MESSAGE("EP[%s] has been already started.\n", xInfo.pEPID);
					}
					else
					{
						xInfo.bEnable = FTM_TRUE;
						xRet = FTOM_CLIENT_EP_set(pClient, xInfo.pEPID, FTM_EP_FIELD_ENABLE, &xInfo);
					}
				}
				else if (strcasecmp(pArgv[2], "stop") == 0)
				{
					if (!xInfo.bEnable)
					{
						MESSAGE("EP[%s] did not start.\n", xInfo.pEPID);
					}
					else
					{
						xInfo.bEnable = FTM_FALSE;
						xRet = FTOM_CLIENT_EP_set(pClient, xInfo.pEPID, FTM_EP_FIELD_ENABLE, &xInfo);
					}
				}
				else if (strcasecmp(pArgv[2], "name") == 0)
				{
					if (nArgc != 4)
					{
						MESSAGE("Invalid arguments!\n");	
					}
					else
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
				else if (strcasecmp(pArgv[2], "add") == 0)
				{
					struct tm	xTM;
					FTM_EP_DATA	xData;
					FTM_CHAR	pTemp[128];
			
					if (nArgc != 6)
					{
						ERROR2(FTM_RET_INVALID_ARGUMENTS, "Invalid arguments[nArgc = %d]\n", nArgc);	
						return	FTM_RET_INVALID_ARGUMENTS;
					}
			
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
				else if (strcasecmp(pArgv[2], "del") == 0)
				{
					if (strcasecmp(pArgv[3], "all") == 0)
					{
					}
					else if (strcasecmp(pArgv[3], "-i") == 0)
					{
						FTM_ULONG	ulMaxCount = 100;
						FTM_ULONG	ulStartIndex;

						if (nArgc == 6)
						{
							ulMaxCount 	= strtoul(pArgv[5], NULL, 10);
							ulStartIndex = strtoul(pArgv[4], NULL, 10);
						}	
						else if (nArgc == 5)
						{
							ulStartIndex = strtoul(pArgv[4], NULL, 10);
						}
						else
						{
							MESSAGE("Invalid parameters\n");
							return	FTM_RET_INVALID_ARGUMENTS;
						}

						MESSAGE("EP[%s] data removed (%lu ~ %lu)\n", pEPID, ulStartIndex, ulStartIndex + ulMaxCount - 1);
					}
					else if (strcasecmp(pArgv[3], "-t") == 0)
					{
						FTM_TIME	xTime;
						FTM_ULONG	ulBeginTime;
						FTM_ULONG	ulEndTime;
						FTM_ULONG	ulDeletedCount = 0;

						if ((nArgc < 5) || (nArgc > 7))
						{
							MESSAGE("Invalid parameters\n");
							return	FTM_RET_INVALID_ARGUMENTS;
						}

						xRet = FTM_TIME_setString(&xTime, pArgv[4]);
						if (xRet != FTM_RET_OK)
						{
							MESSAGE("Invalid time!\n");
							return	FTM_RET_INVALID_ARGUMENTS;
						}

						FTM_TIME_toSecs(&xTime, &ulBeginTime);

						xRet = FTM_TIME_setString(&xTime, pArgv[5]);
						if (xRet != FTM_RET_OK)
						{
							MESSAGE("Invalid time!\n");
							return	FTM_RET_INVALID_ARGUMENTS;
						}

						FTM_TIME_toSecs(&xTime, &ulEndTime);

						xRet = FTOM_CLIENT_EP_DATA_delWithTime(pClient, pEPID, ulBeginTime, ulEndTime, &ulDeletedCount);
						if (xRet != FTM_RET_OK)
						{
							ERROR2(xRet, "Failed to remove data\n");
						}
					}
			
				}
				else if (strcasecmp(pArgv[2], "data") == 0)
				{
					FTM_EP_DATA_PTR	pEPData;

					if ((nArgc < 5) || (9 < nArgc))
					{
						MESSAGE("Invalid arguments!\n");
					}
					else
					{
						if (strcasecmp(pArgv[3], "-i") == 0)
						{
							FTM_ULONG		nStartIndex=0;
							FTM_ULONG		nMaxCount=50;
							FTM_ULONG		ulCount=0;

							switch(nArgc)
							{
							case	6:
								nMaxCount 	= strtol(pArgv[5], NULL, 10);
								nStartIndex = strtol(pArgv[4], NULL, 10);
								break;
								
							case	5:
								nMaxCount 	= strtol(pArgv[4], NULL, 10);
								break;
							
							default:
								MESSAGE("Invalid arguments!\n");
								return	FTM_RET_INVALID_ARGUMENTS;
							}
			
							pEPData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA) * nMaxCount);
							if (pEPData == NULL)
							{
								MESSAGE("System not enough memory!\n");
								return	FTM_RET_NOT_ENOUGH_MEMORY;		
							}
			
							xRet = FTOM_CLIENT_EP_DATA_getList(pClient, pEPID, nStartIndex, pEPData, nMaxCount, &ulCount);
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
						else if (strcasecmp(pArgv[3], "-t") == 0)
						{
							FTM_TIME	xBeginTime,xEndTime;
							FTM_ULONG	ulBegin, ulEnd;
							FTM_ULONG	ulMaxCount = 100;
							FTM_ULONG	ulCount;

							switch(nArgc)
							{
							case	7:
								{
									ulMaxCount = strtoul(pArgv[6], NULL, 10);

									if (strlen(pArgv[5]) != 14)
									{
										MESSAGE("Invalid time!\n");
										return	FTM_RET_INVALID_ARGUMENTS;
									}

									xRet = FTM_TIME_setString(&xEndTime, pArgv[5]);
									if (xRet != FTM_RET_OK)
									{
										MESSAGE("Invalid time!\n");
										return	FTM_RET_INVALID_ARGUMENTS;
									}
							
									FTM_TIME_toSecs(&xEndTime, &ulEnd);

									xRet = FTM_TIME_setString(&xBeginTime, pArgv[4]);
									if (xRet != FTM_RET_OK)
									{
										MESSAGE("Invalid time!\n");
										return	FTM_RET_INVALID_ARGUMENTS;
									}
									
									FTM_TIME_toSecs(&xBeginTime, &ulBegin);
								}	
								break;

							case	6:
								{
									if (strlen(pArgv[5]) == 14)
									{
										xRet = FTM_TIME_setString(&xEndTime, pArgv[5]);
										if (xRet != FTM_RET_OK)
										{
											MESSAGE("Invalid time!\n");
											return	FTM_RET_INVALID_ARGUMENTS;
										}
									
										FTM_TIME_toSecs(&xEndTime, &ulEnd);
									}
									else 
									{
										ulEnd 	= time(NULL);
										ulMaxCount = strtoul(pArgv[5], NULL, 10);
									}

									xRet = FTM_TIME_setString(&xBeginTime, pArgv[4]);
									if (xRet != FTM_RET_OK)
									{
										MESSAGE("Invalid time!\n");
										return	FTM_RET_INVALID_ARGUMENTS;
									}
									
									FTM_TIME_toSecs(&xBeginTime, &ulBegin);
								}	
								break;

							case	5:
								{
									xRet = FTM_TIME_setString(&xBeginTime, pArgv[4]);
									if (xRet != FTM_RET_OK)
									{
										MESSAGE("Invalid time!\n");
										return	FTM_RET_INVALID_ARGUMENTS;
									}
									
									FTM_TIME_toSecs(&xBeginTime, &ulBegin);

									ulEnd 	= time(NULL);
								}	
							
								break;
							
							default:
								MESSAGE("Invalid arguments!\n");
								return	FTM_RET_INVALID_ARGUMENTS;
							}
			
							pEPData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA) * ulMaxCount);
							if (pEPData == NULL)
							{
								MESSAGE("System not enough memory!\n");
								return	FTM_RET_NOT_ENOUGH_MEMORY;		
							}
	
							xRet = FTOM_CLIENT_EP_DATA_getListWithTime(pClient, pEPID, ulBegin, ulEnd, pEPData, ulMaxCount, &ulCount);
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
					}
				}
				else
				{
					MESSAGE("Invalid arguemtns.\n");
				}
			}
		}
	}

	return	FTM_RET_OK;
}

