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
#include "ftom_client_config.h"

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

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	
	if (strcmp(pArgv[1], "add") == 0)
	{
		FTM_INT	i, nOpt;


		FTM_EP_setDefault(&xInfo);

		optind = 2;
		if ((nOpt = getopt(nArgc, pArgv, "n:u:i:t:")) != -1)
		{
			switch(toupper(nOpt))
			{
			case	'N':	
				{
					strncpy(xInfo.pName, optarg, sizeof(xInfo.pName) - 1);
				}
				break;

			case	'U':
				{
					strncpy(xInfo.pUnit, optarg, sizeof(xInfo.pUnit) - 1);
				}
				break;

			case	'I':
				{
					xInfo.ulInterval = strtoul(optarg, NULL, 10);
				}
				break;

			case	'T':
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

		xInfo.xEPID = strtoul(pArgv[2], NULL, 16);
		xInfo.xType = (xInfo.xEPID & FTM_EP_TYPE_MASK);

		for(i = 0 ; i < sizeof(xInfo.pDID) - 1 && i < strlen(pArgv[3]); i ++)
		{
			xInfo.pDID[i] = toupper(pArgv[3][i]);
		}

		if (xInfo.pPID[0] == 0)
		{
			memcpy(xInfo.pPID, xInfo.pDID, sizeof(xInfo.pPID));	
		}

		xRet = FTM_EP_isValid(&xInfo);
		if (xRet != FTM_RET_OK)
		{
			MESSAGE("Invalid node information!\n");
			return	xRet;
		}

		xRet = FTOM_CLIENT_EP_create(pClient, &xInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR("%s : ERROR - %lu\n", pArgv[0], xRet);
		}

	}
	else if (strcmp(pArgv[1], "del") == 0)
	{
		FTM_EP_ID	xEPID;

		if (nArgc != 3)
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}

		xEPID = strtoul(pArgv[2], 0, 16);
		xRet = FTOM_CLIENT_EP_destroy(pClient, xEPID);	
		if (xRet != FTM_RET_OK)
		{
			ERROR("%s : ERROR - %lu\n", pArgv[0], xRet);
		}
	}
	else if (strcmp(pArgv[1], "list") == 0)
	{
		FTM_ULONG	nCount;

		if (nArgc == 2)
		{
			xRet = FTOM_CLIENT_EP_count(pClient, 0, &nCount);
			if (xRet == FTM_RET_OK)
			{
				FTM_EP_ID_PTR	pEPIDs;
				FTM_EP_DATA		xData;

				MESSAGE("EP COUNT : %d\n", nCount);
				pEPIDs = (FTM_EP_ID_PTR)FTM_MEM_calloc(nCount, sizeof(FTM_EP_ID));
				if (pEPIDs == NULL)
				{
					ERROR("Not enough memory.\n");	
				}
				else
				{
					MESSAGE("%8s %16s %16s %8s %8s %16s %16s %8s\n",
						"EPID", "CLASS", "NAME", "UNIT", "INTERNAL", "DID", "PID", "VALUE");
					FTOM_CLIENT_EP_getList(pClient, 0x00000000, pEPIDs, nCount, &nCount);
					for(i = 0 ; i< nCount ; i++)
					{
						FTM_EP	xInfo;

						xRet = FTOM_CLIENT_EP_get(pClient, pEPIDs[i], &xInfo);
						if (xRet == FTM_RET_OK)
						{
							MESSAGE("%08lx %16s %16s %8s %8lu %16s %16s ",
									xInfo.xEPID,
									FTM_EP_typeString(xInfo.xType),
									xInfo.pName,
									xInfo.pUnit,
									xInfo.ulInterval,
									xInfo.pDID,
									xInfo.pPID);
						}

						xRet = FTOM_CLIENT_EP_DATA_getLast(pClient, pEPIDs[i], &xData);
						if (xRet == FTM_RET_OK)
						{
							switch(xData.xType)
							{
							case	FTM_EP_DATA_TYPE_INT:
								{
									MESSAGE("%5d", xData.xValue.nValue);
								}
								break;

							case	FTM_EP_DATA_TYPE_ULONG:
								{
									MESSAGE("%5lu", xData.xValue.ulValue);
								}
								break;

							case	FTM_EP_DATA_TYPE_FLOAT:
								{
									MESSAGE("%5.2f", xData.xValue.fValue);
								}
								break;

							}

						
						}
						MESSAGE("\n");
					}

					FTM_MEM_free(pEPIDs);
				}
			}
		}
		else if (nArgc == 3)
		{
			FTM_EP_TYPE	xEPClass;
			FTM_EP_ID		pEPID[100];

			xEPClass = strtoul(pArgv[2], 0, 16);
			xRet = FTOM_CLIENT_EP_getList(pClient, xEPClass, pEPID, 100, &nCount);
			if (xRet == FTM_RET_OK)
			{
				MESSAGE("%8s %16s %16s %16s %8s %16s %16s\n",
						"EPID", "TYPE", "NAME", "UNIT", "INTERVAL", "DID", "PID");

				for(i = 0 ; i< nCount ; i++)
				{
					FTM_EP	xInfo;

					xRet = FTOM_CLIENT_EP_get(pClient, pEPID[i], &xInfo);
					if (xRet == FTM_RET_OK)
					{
						MESSAGE("%08lx %16s %16s %16s %8lu %16s %16s\n",
								xInfo.xEPID,
								FTM_getEPTypeString(xInfo.xType),
								xInfo.pName,
								xInfo.pUnit,
								xInfo.ulInterval,
								xInfo.pDID,
								xInfo.pPID);
					}
					else
					{
						ERROR("%s : ERROR - %lu\n", pArgv[0], xRet);
					}
				}
			}
		}
	}
	else if (strcasecmp(pArgv[1], "info") == 0)
	{
		FTM_EP_ID		xEPID;
		FTM_EP			xEPInfo;

		xEPID = strtoul(pArgv[2], 0, 16);
		xRet = FTOM_CLIENT_EP_get(pClient, xEPID, &xEPInfo);	
		if (xRet == FTM_RET_OK)
		{
			MESSAGE("EP Information\n");
			MESSAGE("%16s : %08x\n","ID",			xEPInfo.xEPID);
			MESSAGE("%16s : %s\n",	"Type",			FTM_EP_typeString(xEPInfo.xEPID & FTM_EP_TYPE_MASK));
			MESSAGE("%16s : %s\n",	"Name",			xEPInfo.pName);
			MESSAGE("%16s : %s\n",	"Unit",			xEPInfo.pUnit);
			MESSAGE("%16s : %s\n",	"State",		(xEPInfo.bEnable)?"Enable":"Disable");
			MESSAGE("%16s : %d\n",	"Timeout",		xEPInfo.ulTimeout);
			MESSAGE("%16s : %d\n",	"Interval",		xEPInfo.ulInterval);	
			MESSAGE("%16s : %d\n",	"Report Cycle",	xEPInfo.ulCycle);
			MESSAGE("%16s : %s\n",	"DID",			xEPInfo.pDID);
			MESSAGE("%16s : %08x\n","DEPID",		xEPInfo.xDEPID);
			MESSAGE("%16s : %s\n",	"PID",			xEPInfo.pPID);
			MESSAGE("%16s : %08x\n","PEPID",		xEPInfo.xPEPID);
			if (xEPInfo.xLimit.xType == FTM_EP_LIMIT_TYPE_TIME)
			{
				MESSAGE("%16s : %s(%d ~ %d)\n",		"Limit","Time", xEPInfo.xLimit.xParams.xTime.ulStart, xEPInfo.xLimit.xParams.xTime.ulEnd);
			}
			else
			{
				MESSAGE("%16s : %s(%d)\n",			"Limit","Count", xEPInfo.xLimit.xParams.ulCount);
			}
		}
	
	}
	else if (strcmp(pArgv[1], "count") == 0)
	{
		FTM_ULONG	ulCount;

		xRet = FTOM_CLIENT_EP_count(pClient, 0, &ulCount);
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
		FTM_EP_ID		xEPID;
		FTM_EP			xInfo;

		xEPID = strtoul(pArgv[1], 0, 16);
		xRet = FTOM_CLIENT_EP_get(pClient, xEPID, &xInfo);
		if (xRet != FTM_RET_OK)
		{
			MESSAGE("EP[%08x] not exists.\n", xEPID);
		}
		else	
		{
			switch(nArgc)
			{
			case	2:
				{
					FTM_EP_DATA	xData;
					FTM_ULONG	ulCount;
					FTM_CHAR	pValue[32];
			
					MESSAGE("%-8s : %08lx\n", 	"EPID", 	xInfo.xEPID);
					MESSAGE("%-8s : %s\n",		"TYPE",		FTM_EP_typeString(xInfo.xType));
					MESSAGE("%-8s : %s\n",		"NAME", 	xInfo.pName);
					MESSAGE("%-8s : %s\n",		"UNIT", 	xInfo.pUnit);
					MESSAGE("%-8s : %d\n",		"INTERVAL", xInfo.ulInterval);
					MESSAGE("%-8s : %s\n",		"DID", 		xInfo.pDID);
					MESSAGE("%-8s : %s\n",		"PID", 		xInfo.pPID);
			
					xRet = FTOM_CLIENT_EP_DATA_count(pClient, xEPID, &ulCount);
					if (xRet == FTM_RET_OK)
					{
						MESSAGE("%-8s : %d\n", "COUNT", ulCount);
					}
		
					xRet = FTOM_CLIENT_EP_DATA_getLast(pClient, xEPID, &xData);
					if (xRet == FTM_RET_OK)
					{
						xRet = FTM_EP_DATA_snprint(pValue, sizeof(pValue), &xData);	
						if (xRet == FTM_RET_OK)
						{
							MESSAGE("%-8s : %s\n", "VALUE", pValue);
						}
						else
						{
							MESSAGE("%-8s : INVALID\n", "VALUE");
						}
					}
					else
					{
						MESSAGE("%-8s : NOT EXISTS\n", "VALUE");
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

						xRet = FTOM_CLIENT_EP_set(pClient, &xInfo);
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
							xRet = FTOM_CLIENT_EP_DATA_getList(pClient, xEPID, ulStartIndex, pData, ulCount, &ulCount);
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
									MESSAGE("%3d : %s\n", ulStartIndex + i, pBuff);
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
	FTM_EP_ID		xEPID;
	FTM_EP_DATA		xData;
	FTM_EP_DATA_PTR	pEPData;	
	FTM_ULONG		nStartIndex=0;
	FTM_ULONG		nMaxCount=50;
	FTM_ULONG		nCount=0;
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;

	if (nArgc < 2)
	{
		ERROR("Invalid arguments[nArgc = %d]\n", nArgc);	
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	if (strcasecmp(pArgv[1], "add") == 0)
	{
		struct tm	xTM;
		FTM_CHAR	pTemp[128];

		if (nArgc != 6)
		{
			ERROR("Invalid arguments[nArgc = %d]\n", nArgc);	
			return	FTM_RET_INVALID_ARGUMENTS;
		}

		xEPID = strtoul(pArgv[2], NULL, 16);

		snprintf(pTemp, sizeof(pTemp), "%s %s", pArgv[3], pArgv[4]);
		strptime(pTemp, "%Y-%m-%d %H:%M:%S", &xTM);
		xData.ulTime	= (FTM_ULONG)mktime(&xTM);

		switch(toupper(pArgv[5][0]))
		{
		case	'I':	
			{
				xData.xType = FTM_EP_DATA_TYPE_INT;
				xData.xValue.nValue = strtol(&pArgv[5][1], NULL, 10); 
			}

			break;

		case	'F':	
			{
				xData.xType = FTM_EP_DATA_TYPE_FLOAT;
				xData.xValue.fValue = strtod(&pArgv[5][1], NULL); 
			} 
			break;

		case	'U':	
			{	
				xData.xType = FTM_EP_DATA_TYPE_ULONG;
				xData.xValue.ulValue = strtoul(&pArgv[5][1], NULL, 10); 
			}	
			break;

		case	'0': case	'1': case	'2': case	'3':	
		case	'4': case	'5': case	'6': case	'7':	
		case	'8':	
		case	'9':	
			{
				xData.xType = FTM_EP_DATA_TYPE_INT;
				xData.xValue.nValue = strtol(pArgv[4], NULL, 10); 
			}
			break;

		default:	
			{
				MESSAGE("Invalid arguments!\n");	
				return	FTM_RET_INVALID_ARGUMENTS;
			}
		}

		xRet = FTOM_CLIENT_EP_DATA_add(pClient, xEPID, &xData);
		if (xRet == FTM_RET_OK)
		{
			MESSAGE("EndPoint data appending done successfully!\n");	
		}
		else
		{
			ERROR("EndPoint data appending failed [ ERROR = %08lx ]\n", xRet);	
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
					xEPID	= strtol(pArgv[3], NULL, 16);
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
					xEPID 		= strtol(pArgv[3], NULL, 16);
				}	
				else if (nArgc == 5)
				{
					nStartIndex = strtol(pArgv[4], NULL, 10);
					xEPID 		= strtol(pArgv[3], NULL, 16);
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
					xEPID 		= strtol(pArgv[3], NULL, 16);

					snprintf(pTemp, sizeof(pTemp), "%s %s", pArgv[4], pArgv[5]);
					strptime(pTemp, "%Y-%m-%d %H:%M:%S", &xTMBegin);
					nBeginTime = (FTM_ULONG)mktime(&xTMBegin);

					snprintf(pTemp, sizeof(pTemp), "%s %s", pArgv[6], pArgv[7]);
					strptime(pTemp, "%Y-%m-%d %H:%M:%S", &xTMEnd);
					nEndTime = (FTM_ULONG)mktime(&xTMEnd);
				}
				else if (nArgc == 7)
				{
					xEPID 		= strtol(pArgv[3], NULL, 16);

					snprintf(pTemp, sizeof(pTemp), "%s %s", pArgv[4], pArgv[5]);
					strptime(pTemp, "%Y-%m-%d %H:%M:%S", &xTMBegin);
					nBeginTime = (FTM_ULONG)mktime(&xTMBegin);

					nMaxCount 	= strtol(pArgv[6], NULL, 10);
				}
				else if (nArgc == 6)
				{
					xEPID 		= strtol(pArgv[3], NULL, 16);

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
				xRet = FTOM_CLIENT_EP_DATA_delWithTime(pClient, xEPID, nBeginTime, nEndTime);
				if (xRet == FTM_RET_OK)
				{
					MESSAGE("EndPoint data deleted successfully!\n");	
				}
				else
				{
					ERROR("EndPoint data deleting failed [ ERROR = %08lx ]\n", xRet);	
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

				xEPID	= strtoul(pArgv[3], NULL, 16);

				xRet = FTOM_CLIENT_EP_DATA_info(pClient, xEPID, &ulBeginTime, &ulEndTime, &ulCount);
				if (xRet == FTM_RET_OK)
				{
					MESSAGE("      EPID : %08lx\n", xEPID);
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

				xEPID	= strtoul(pArgv[3], NULL, 16);
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

				xEPID 		= strtol(pArgv[3], NULL, 16);

				pEPData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA) * nMaxCount);
				if (pEPData == NULL)
				{
					MESSAGE("System not enough memory!\n");
					return	FTM_RET_NOT_ENOUGH_MEMORY;		
				}

				xRet = FTOM_CLIENT_EP_DATA_getList(pClient, 
						xEPID, 
						nStartIndex,
						pEPData, 
						nMaxCount, 
						&nCount);
				TRACE("FTOM_CLIENT_getEPData(hClient, %08lx, %d, %d, pEPData, %d, %d) = %08lx\n",
						xEPID, nBeginTime, nEndTime, nMaxCount, nCount, xRet);
				if (xRet == FTM_RET_OK)
				{
					FTM_INT	i;

					MESSAGE("%8s %32s %8s\n", "INDEX", "DATE", "VALUE");	
					for(i = 0 ; i < nCount ; i++)
					{
						FTM_CHAR	pTime[64];
						time_t		xTime = pEPData[i].ulTime;

						strftime(pTime, sizeof(pTime), "%Y-%m-%d %H:%M:%S", gmtime(&xTime));
						switch(pEPData[i].xType)
						{
						case	FTM_EP_DATA_TYPE_ULONG:
							{
								MESSAGE("%8d %32s %8lu\n", 
										nStartIndex + i, pTime, pEPData[i].xValue.ulValue);	
							}
							break;

						case	FTM_EP_DATA_TYPE_FLOAT:
							{
								MESSAGE("%8d %32s %8.3lf\n", 
										nStartIndex + i, pTime, pEPData[i].xValue.fValue);	
							}
							break;
						case	FTM_EP_DATA_TYPE_INT:
						default:
							{
								MESSAGE("%8d %32s %8d\n", 
										nStartIndex + i, pTime, pEPData[i].xValue.nValue);	
							}
							break;
						}
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
					xEPID 		= strtol(pArgv[optind++], NULL, 16);

					snprintf(pTemp, sizeof(pTemp), "%s %s", pArgv[optind], pArgv[optind+1]);
					optind+= 2;
					strptime(pTemp, "%Y-%m-%d %H:%M:%S", &xTMBegin);
					nBeginTime = (FTM_ULONG)mktime(&xTMBegin);
				}
				else if (nArgc == optind + 6)
				{
					xEPID 		= strtol(pArgv[optind++], NULL, 16);

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

