#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>    
#include <string.h>    
#include <unistd.h>    
#define	__USE_XOPEN
#include <time.h>    
#include <sys/socket.h> 
#include <arpa/inet.h>
#include "libconfig.h"
#include "ftm_shell.h"
#include "ftdm_client.h"
#include "ftdm_client_cmds.h"
#include "ftdm_client_config.h"
#include "ftm_debug.h"

static FTM_RET	FTDMC_cmdConnect(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	FTDMC_cmdDisconnect(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	FTDMC_NODE_cmd(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	FTDMC_EP_cmd(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	FTDMC_EP_DATA_cmd(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	FTDMC_EVENT_cmd(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	FTDMC_DEBUG_cmd(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	FTDMC_cmdQuit(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);

extern FTDMC_CFG		xClientConfig;
extern FTDMC_SESSION	_xSession;
extern FTM_BOOL			_bQuit;

FTM_SHELL_CMD			FTDMC_pCmdList[] = 
{
	{	
		.pString	= "connect",
		.function	= FTDMC_cmdConnect,
		.pShortHelp = "Connnect to FTDM server.",
		.pHelp		= "<Server IP> <Service Port>\n"\
					  "\tConnect to FTDM server.\n"\
					  "PARAMETERS:\n"\
					  "\tServer IP    Target server IP address\n"\
					  "\t             (Default : 127.0.0.1)\n"\
					  "\tServer Port  FTDM Port"
	},
	{	
		.pString	= "disconnect",	
		.function	= FTDMC_cmdDisconnect,
		.pShortHelp = "Disconnect from FTDM server.",
		.pHelp		= "\n"\
					  "\tDisconnect to FTDM server.\n"
	},
	{	
		.pString	= "node",
		.function	= FTDMC_NODE_cmd,
		.pShortHelp = "Node management command set.",
		.pHelp		= "<COMMAND>\n"\
					  "\tNode management command set.\n"\
					  "COMMAND:\n"\
					  "\tadd    <DID> SNMP <URL> <COMMUNITY>\n"\
					  "\tdel    <DID>\n"\
					  "\tinfo   <DID>\n"\
					  "\tlist\n"\
					  "PARAMETERS:\n"\
					  "\tDID    Node Identifier Number.\n"\
					  "\t       Max 32 characters\n"\
					  "\tTYPE	Define node type\n"\
					  "\t       SNMP - SNMP supported node\n"\
					  "\t       MODBUS/TCP - MODBUS over TCP/IP supported node\n"\
					  "\t       MODBUS - MODBUS over Serial supported node\n"\
					  "\tURL    Node location\n"\
					  "\t       (Ex. : www.node.com or 192.168.1.100)\n"\
					  "\tCOMMUNITY SNMP communication community\n"\
					  "\t       (Ex. : public, private,...)"
	},
	{	
		.pString	= "ep",
		.function   = FTDMC_EP_cmd,
		.pShortHelp = "EndPoint management command set.",
		.pHelp      = "<COMMAND> ...\n"\
					  "\tEndPoint management.\n"\
					  "COMMANDS:\n"\
					  "\tadd    <EPID> <Type> [Name] [Unit] [Interval] [DID] [PID]\n"\
					  "\tdel    <EPID>\n"\
					  "\tlist\n"\
					  "PARAMETERS:\n"\
					  "\tEPID   EndPoint ID. 8Digit Hexa Number\n"\
					  "\t       (Ex. : 010A0003)\n"

	},
	{	
		.pString	= "data",
		.function	= FTDMC_EP_DATA_cmd,
		.pShortHelp	= "EndPoint management command set.",
		.pHelp		= "<COMMAND> ...\n"\
					  "\tEndPoint data management.\n"\
					  "COMMANDS:\n"\
					  "\tadd    EPID <Time> <Value>\n"\
					  "\tdel    -i EPID <Index> <Count>\n"\
					  "\tdel    -t EPID <Begin Time> [End Time | Count]\n"\
					  "\tcount  -i EPID <Index>\n"\
					  "\tcount  -t EPID <Begin Time> <End Time>\n"\
					  "\tget    -i EPID <Index> <Count>\n"\
					  "\tget    -t EPID <Begin Time> <End Time> <Count>\n"\
					  "OPTIONS:\n"\
					  "\t-i     Index mode\n"\
					  "\t-t     Time mode\n"\
					  "PARAMETERS:\n"\
					  "\tEPID   EndPoint ID. 8Digit Hexa Number\n"\
					  "\t       (Ex. : 010A0003)\n"\
					  "\tTime   Format => YYYY-MM-DD HH:MM:SS\n"\
					  "\t       (Ex. : 2015-01-05 13:11:15)\n"\
					  "\tIndex  Decimal Number\n"\
					  "\tCount  Element count"
		
	},
	{	.pString	= "event",
		.function	= FTDMC_EVENT_cmd,
		.pShortHelp = "Event management command set.",
		.pHelp		= "<COMMAND> ...\n"\
					 "\tEvent management\n"\
					 "COMMANDS:\n"\
					 "\tadd     EPID <Type> [<Condition 1> [<Condition 2>]]\n"\
					 "\tdel     EPID\n"\
					 "\tlist    Show event list"
	},
	{	.pString	= "debug",
		.function	= FTDMC_DEBUG_cmd,
		.pShortHelp = "",
		.pHelp		= ""
	},
	{	
		.pString	= "quit",
		.function	= FTDMC_cmdQuit,
		.pShortHelp = "Quit program.",
		.pHelp 		= "\n"\
					  "\tQuit program."
	}
};

FTM_ULONG		FTDMC_ulCmdCount = sizeof(FTDMC_pCmdList) / sizeof(FTM_SHELL_CMD);
FTM_CHAR_PTR	FTDMC_pPrompt = "FTDMC> ";

FTM_RET	FTDMC_cmdConnect(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	in_addr_t		xAddr	= inet_addr(xClientConfig.xNetwork.pServerIP);
	FTM_USHORT		nPort 	= xClientConfig.xNetwork.usPort;

	switch(nArgc)
	{
	case	3:
		{
			nPort = (FTM_USHORT)strtoul(pArgv[2], NULL, 10);
		}

	case	2:
		{
			xAddr = inet_addr(pArgv[1]);
			if (xAddr == 0)
			{
				MESSAGE("Invalid ip address\n");
				return	FTM_RET_INVALID_ARGUMENTS;
			}
		}
		break;

	case	1:
		{
		}
		break;
	default:
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}
	}

	if (FTDMC_connect(&_xSession, xAddr, nPort) != FTM_RET_OK)
	{
		perror("connect failed. Error");
		return	FTM_RET_ERROR;	
	}

	MESSAGE("Connected\n");

	return	FTM_RET_OK;
}

FTM_RET	FTDMC_cmdDisconnect(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{

	switch(nArgc)
	{
	case	1: 
		{
			FTDMC_disconnect(&_xSession);
		}
		break;

	default:
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDMC_NODE_cmd(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	FTM_RET	nRet;
	FTM_INT	i;
	FTM_NODE	xNodeInfo;
	FTM_CHAR	pDID[FTM_DID_LEN + 1];
	FTM_CHAR	pURL[FTM_URL_LEN + 1];
	FTM_CHAR	pLocation[FTM_LOCATION_LEN + 1];
	
	memset(pDID, 0, sizeof(pDID));
	memset(pURL, 0, sizeof(pURL));
	memset(pLocation, 0, sizeof(pLocation));

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	if (strcasecmp(pArgv[1], "add") == 0)
	{
		if (nArgc < 4)
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}

		memset(&xNodeInfo, 0, sizeof(FTM_NODE));
		if (strcasecmp(pArgv[3], "SNMP") == 0)
		{
			xNodeInfo.xType = FTM_NODE_TYPE_SNMP;	
		}
		else if (strcasecmp(pArgv[3], "MODBUS/TCP") == 0)
		{
			xNodeInfo.xType = FTM_NODE_TYPE_MODBUS_OVER_TCP;	
		}
		else if (strcasecmp(pArgv[3], "MODBUS") == 0)
		{
			xNodeInfo.xType = FTM_NODE_TYPE_MODBUS_OVER_SERIAL;	
		}
		else
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}

		if ((xNodeInfo.xType == FTM_NODE_TYPE_SNMP) && (nArgc == 6))
		{
			if ((strlen(pArgv[5]) > FTM_SNMP_COMMUNITY_LEN) ||
				(strlen(pArgv[4]) > FTM_URL_LEN) ||
				(strlen(pArgv[2]) > FTM_DID_LEN))
			{
				ERROR("Invalid DID length[< %d]\n", FTM_DID_LEN);
				return	FTM_RET_INVALID_ARGUMENTS;
			}

			for(i = 0 ; i < strlen(pArgv[5]) ; i++)
			{
				xNodeInfo.xOption.xSNMP.pCommunity[i] = toupper(pArgv[5][i]);	
			}


			for(i = 0 ; i < strlen(pArgv[4]) ; i++)
			{
				xNodeInfo.xOption.xSNMP.pURL[i] = toupper(pArgv[4][i]);	
			}

			for(i = 0 ; i < strlen(pArgv[2]) ; i++)
			{
				xNodeInfo.pDID[i] = toupper(pArgv[2][i]);	
			}

			nRet = FTDMC_NODE_append(&_xSession, &xNodeInfo);
			if (nRet != FTM_RET_OK)
			{
				ERROR("%s : ERROR - %lx\n", pArgv[0], nRet);
			}
			else
			{
				MESSAGE("%s : The node[%s] has been created successfully.\n", pArgv[0], pDID);
			}
		}
		else
		{
			ERROR("Invalid type or arguments [ Type = %08lx ]\n", xNodeInfo.xType);
			return	FTM_RET_INVALID_ARGUMENTS;
		}
	}
	else if (strcasecmp(pArgv[1], "del") == 0)
	{
		FTM_INT	i;

		if ((nArgc < 3) || (strlen(pArgv[2]) > FTM_DID_LEN))
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}

		memset(pDID, 0, sizeof(pDID));
		for(i = 0 ; i < strlen(pArgv[2]) ; i++)
		{
			pDID[i] = toupper(pArgv[2][i]);	
		}

		nRet = FTDMC_NODE_remove(&_xSession, pDID);
		if (nRet != FTM_RET_OK)
		{
			ERROR("%s : ERROR - %lu\n", pArgv[0], nRet);
		}
		else
		{
			TRACE("%s : The node[%s] has been destroied successfully.\n", pArgv[0], pDID);
		}
	}
	else if (strcasecmp(pArgv[1], "info") == 0)
	{
		FTM_INT			i;
		FTM_NODE	xInfo;

		if ((nArgc < 3) || (strlen(pArgv[2]) > FTM_DID_LEN))
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}

		memset(pDID, 0, sizeof(pDID));
		for(i = 0 ; i < strlen(pArgv[2]) ; i++)
		{
			pDID[i] = toupper(pArgv[2][i]);	
		}

		nRet = FTDMC_NODE_get(&_xSession, pDID, &xInfo);
		if (nRet != FTM_RET_OK)
		{
			ERROR("%s : ERROR - %lu\n", pArgv[0], nRet);
		}
		else
		{
			MESSAGE("NODE INFORMATION\n"\
					"%16s : %s\n"\
					"%16s : %lu\n"\
					"%16s : %s\n",
					"DID", xInfo.pDID,
					"TYPE", xInfo.xType,
					"LOCATION", xInfo.pLocation);
		}
	}
	else if (strcasecmp(pArgv[1], "list") == 0)
	{
		FTM_ULONG	i, nNodeCount = 0;

		nRet = FTDMC_NODE_count(&_xSession, &nNodeCount);
		if (nRet != FTM_RET_OK)
		{
			ERROR("%s : ERROR - %lu\n", pArgv[0], nRet);
		}

		MESSAGE("NODE COUNT : %d\n", nNodeCount);

		MESSAGE("%-16s %-16s %-16s %-8s %-8s %-16s %-16s %-16s %-16s\n", 
			"DID", "TYPE", "LOCATION", "INTERVAL", "TIMEOUT", "OPT0", "OPT1", "OPT2", "OPT3");

		for(i = 0 ; i < nNodeCount; i++)
		{
			FTM_NODE	xInfo;

			nRet = FTDMC_NODE_getAt(&_xSession, i, &xInfo);
			if (nRet == FTM_RET_OK)
			{
				MESSAGE("%-16s %-16s %-16s %8d ", 
					xInfo.pDID, 
					FTM_nodeTypeString(xInfo.xType), 
					xInfo.pLocation,
					xInfo.ulInterval);

				switch(xInfo.xType)
				{
				case	FTM_NODE_TYPE_SNMP:
					{
						MESSAGE("%-16d %-16s %-16s", 
							xInfo.xOption.xSNMP.ulVersion,
							xInfo.xOption.xSNMP.pURL,
							xInfo.xOption.xSNMP.pCommunity);
						
					}
					break;
				}

				MESSAGE("\n");
			}
		
		}
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTDMC_EP_cmd(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	FTM_RET		nRet;
	FTM_INT		i;
	FTM_CHAR		pPID[FTM_DID_LEN + 1];
	FTM_CHAR		pDID[FTM_DID_LEN + 1];
	FTM_CHAR		pName[FTM_NAME_LEN + 1];
	FTM_CHAR		pUnit[FTM_UNIT_LEN + 1];
	FTM_EP_ID		xEPID = 0;
	FTM_EP		xInfo;

	memset(pPID, 0, sizeof(pPID));
	memset(pDID, 0, sizeof(pDID));
	memset(pUnit, 0, sizeof(pUnit));
	memset(pName, 0, sizeof(pName));

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	
	if (strcmp(pArgv[1], "add") == 0)
	{
		memset(&xInfo, 0, sizeof(xInfo));
		switch(nArgc)
		{
		case	8:
			if (strlen(pArgv[7]) > FTM_DID_LEN)
			{
				return	FTM_RET_INVALID_ARGUMENTS;
			}

			for(i = 0 ; i < strlen(pArgv[7]) ; i++)
			{
				xInfo.pPID[i] = toupper(pArgv[7][i]);	
			}

		case	7:
			if (strlen(pArgv[6]) > FTM_DID_LEN)
			{
				return	FTM_RET_INVALID_ARGUMENTS;
			}

			for(i = 0 ; i < strlen(pArgv[6]) ; i++)
			{
				xInfo.pDID[i] = toupper(pArgv[6][i]);	
			}

		case	6:
			xInfo.ulInterval = atoi(pArgv[5]);
			if (xInfo.ulInterval < 0)
			{
				return	FTM_RET_INVALID_ARGUMENTS;
			}

		case	5:
			if (strlen(pArgv[4]) > FTM_UNIT_LEN)
			{
				return	FTM_RET_INVALID_ARGUMENTS;
			}

			for(i = 0 ; i < strlen(pArgv[4]) ; i++)
			{
				xInfo.pUnit[i] = toupper(pArgv[4][i]);	
			}

		case	4:
			if (strlen(pArgv[3]) > FTM_NAME_LEN)
			{
				return	FTM_RET_INVALID_ARGUMENTS;
			}

			for(i = 0 ; i < strlen(pArgv[3]) ; i++)
			{
				xInfo.pName[i] = toupper(pArgv[3][i]);	
			}

		case	3:
			xInfo.xEPID = strtoul(pArgv[2], NULL, 16);
			if (xEPID < 0)
			{
				return	FTM_RET_INVALID_ARGUMENTS;
			}

			xInfo.xType = (xEPID & FTM_EP_TYPE_MASK);

			nRet = FTDMC_EP_append(&_xSession, &xInfo);
			if (nRet != FTM_RET_OK)
			{
				ERROR("%s : ERROR - %lu\n", pArgv[0], nRet);
			}
			break;
		}

	}
	else if (strcmp(pArgv[1], "del") == 0)
	{
		if (nArgc != 3)
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}

		xEPID = strtoul(pArgv[2], 0, 16);
		nRet = FTDMC_EP_remove(&_xSession, xEPID);	
		if (nRet != FTM_RET_OK)
		{
			ERROR("%s : ERROR - %lu\n", pArgv[0], nRet);
		}
	}
	else if (strcmp(pArgv[1], "list") == 0)
	{
		FTM_ULONG	nCount;

		nRet = FTDMC_EP_count(&_xSession, 0, &nCount);
		if (nRet == FTM_RET_OK)
		{
			MESSAGE("%8s %16s %16s %16s %8s %16s %16s\n",
					"EPID", "TYPE", "NAME", "UNIT", "INTERVAL", "DID", "PID");

			for(i = 0 ; i< nCount ; i++)
			{
				FTM_EP	xInfo;

				nRet = FTDMC_EP_getAt(&_xSession, i, &xInfo);
				if (nRet == FTM_RET_OK)
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
			}
		}
		else
		{
			ERROR("%s : ERROR - %lu\n", pArgv[0], nRet);
		}
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDMC_EP_DATA_cmd(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	FTM_RET			nRet;
	FTM_INT			nOpt = 0;
	FTM_ULONG		nBeginTime = 0;
	FTM_ULONG		nEndTime = 0;
	FTM_EP_ID		xEPID;
	FTM_EP_DATA		xData;
	FTM_EP_DATA_PTR	pEPData;	
	FTM_ULONG		nStartIndex=0;
	FTM_ULONG		nMaxCount=50;
	FTM_ULONG		nCount=0;

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	if (strcmp(pArgv[1], "add") == 0)
	{
		struct tm	xTM;
		FTM_CHAR	pTemp[128];

		if (nArgc != 6)
		{
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

		nRet = FTDMC_EP_DATA_append(&_xSession, xEPID, &xData);
		if (nRet == FTM_RET_OK)
		{
			MESSAGE("EndPoint data appending done successfully!\n");	
		}
		else
		{
			ERROR("EndPoint data appending failed [ ERROR = %08lx ]\n", nRet);	
		}
	}
	else if (strcmp(pArgv[1], "del") == 0)
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

				nRet = FTDMC_EP_DATA_removeWithTime(&_xSession, xEPID, nBeginTime, nEndTime);
				if (nRet == FTM_RET_OK)
				{
					MESSAGE("EndPoint data deleted successfully!\n");	
				}
				else
				{
					ERROR("EndPoint data deleting failed [ ERROR = %08lx ]\n", nRet);	
				}
			}
			break;

		default:
			{
				return	FTM_RET_INVALID_ARGUMENTS;
			}
		}

	}
	else if (strcmp(pArgv[1], "count") == 0)
	{
		optind = 2;
		if ((nOpt = getopt(nArgc, pArgv, "a:i:t:")) == -1)
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}

		switch(toupper(nOpt))
		{
		case	'A':
		case	'I':
			{
				FTM_ULONG	ulCount;

				if (nArgc != 4) 
				{
					return	FTM_RET_INVALID_ARGUMENTS;
				}

				xEPID	= strtoul(pArgv[3], NULL, 16);

				nRet = FTDMC_EP_DATA_count(&_xSession, xEPID, &ulCount);
				if (nRet == FTM_RET_OK)
				{
					MESSAGE("      EPID : %08lx\n", xEPID);
					MESSAGE("DATA COUNT : %lu\n", ulCount);
				}

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
	else if (strcmp(pArgv[1], "get") == 0)
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

				pEPData = (FTM_EP_DATA_PTR)malloc(sizeof(FTM_EP_DATA) * nMaxCount);
				if (pEPData == NULL)
				{
					MESSAGE("System not enough memory!\n");
					return	FTM_RET_NOT_ENOUGH_MEMORY;		
				}

				nRet = FTDMC_EP_DATA_get(&_xSession, xEPID, nStartIndex,
						pEPData, nMaxCount, &nCount);
				TRACE("FTDMC_EP_DATA_get(hClient, %08lx, %d, %d, pEPData, %d, %d) = %08lx\n",
						xEPID, nBeginTime, nEndTime, nMaxCount, nCount, nRet);
				if (nRet == FTM_RET_OK)
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
					MESSAGE("EndPoint data loading failed [ ERROR = %08lx ]\n", nRet);	
				}

				free(pEPData);
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
	else if (strcmp(pArgv[1], "test_gen") == 0)
	{
		FTM_EP_ID	xEPID;
		FTM_EP_DATA	xData;
		FTM_INT		i, nDataGenCount;
		time_t		_startTime;
		time_t		_endTime;

		if (nArgc != 3)
		{
			MESSAGE("Invalid parameters\n");
			return	FTM_RET_INVALID_ARGUMENTS;
		}

		_startTime = mktime(&xClientConfig.xDiagnostic.xStartTM);
		_endTime = mktime(&xClientConfig.xDiagnostic.xEndTM);

		nDataGenCount = strtol(pArgv[2], NULL, 10);

		for(i = 0 ; i < nDataGenCount ; i++)
		{
			FTM_INT	nIndex = 0;
			FTM_ULONG	ulSize;

			FTM_LIST_count(&xClientConfig.xDiagnostic.xEPList, &ulSize);
			nIndex = rand() % ulSize;

			if (FTM_LIST_getAt(&xClientConfig.xDiagnostic.xEPList, nIndex, (FTM_VOID_PTR _PTR_)&xEPID) == FTM_RET_OK)
			{
				xData.xType = FTM_EP_DATA_TYPE_INT;
				xData.ulTime = _startTime + rand() % (_endTime - _startTime);
				xData.xValue.nValue = rand();

				FTDMC_EP_DATA_append(&_xSession, xEPID, &xData);
			}

			if ((i+1) % 100 == 0)
			{
				MESSAGE("\rTest generation completed : %4d", i);	
			}
		}
		MESSAGE("\rTest generation completed : %4d\n", nDataGenCount);	
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDMC_EVENT_cmd(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	FTM_RET	xRet;

	if ((nArgc == 1) || (strcasecmp(pArgv[1], "list") == 0))
	{
		FTM_ULONG	i, ulCount;

		xRet = FTDMC_EVENT_count(&_xSession, &ulCount);
		if (xRet != FTM_RET_OK)
		{
			return	xRet;
		}

		MESSAGE("\t%-8s %-8s %-16s %s\n", "ID", "EPID", "TYPE", "CONDITION");
		for(i = 0 ; i < ulCount ; i++)
		{
			FTM_EVENT	xEvent;
			xRet = FTDMC_EVENT_getAt(&_xSession, i, &xEvent);
			if (xRet == FTM_RET_OK)
			{
				FTM_CHAR	pBuff[1024];

				FTM_EVENT_conditionToString(&xEvent, pBuff, sizeof(pBuff) );
				MESSAGE("\t%08x %08x %-16s %s\n", xEvent.xID, xEvent.xEPID, FTM_EVENT_typeString(xEvent.xType), pBuff);
			}
		}
	}
	else if ((nArgc == 3) && (strcasecmp(pArgv[1], "del") == 0))
	{
		FTM_EVENT_ID	xEventID;

		xEventID 	= (FTM_EVENT_ID)strtoul(pArgv[2], 0, 10);
		xRet = FTDMC_EVENT_del(&_xSession, xEventID);
		if (xRet == FTM_RET_OK)
		{
			MESSAGE("The event deleted successfully.\n");	
		}
		else
		{
			MESSAGE("The event failed to unregistration.\n");	
		}
	}
	else if ((nArgc == 5) && (strcasecmp(pArgv[1], "add") == 0))
	{
		FTM_EVENT		xEvent;

		memset(&xEvent, 0, sizeof(xEvent));
		xEvent.xEPID 	= (FTM_EP_ID)strtoul(pArgv[2], 0, 10);
		
		if (strcasecmp(pArgv[3], "ge") == 0)
		{
			xEvent.xType = FTM_EVENT_TYPE_ABOVE;
			if ((pArgv[4][0] | 0x20) == 'f')
			{
				xEvent.xParams.xAbove.xValue.xType = FTM_EP_DATA_TYPE_FLOAT;
				xEvent.xParams.xAbove.xValue.xValue.fValue = strtod(&pArgv[4][1], 0);
			}
			else if ((pArgv[4][0] | 0x20) == 'i')
			{
		TRACE("3\n");
				xEvent.xParams.xAbove.xValue.xType = FTM_EP_DATA_TYPE_INT;
				xEvent.xParams.xAbove.xValue.xValue.nValue = strtol(&pArgv[4][1], 0, 10);
			}
			else if ((pArgv[4][0] | 0x20) == 'b')
			{
				xEvent.xParams.xAbove.xValue.xType = FTM_EP_DATA_TYPE_BOOL;
				xEvent.xParams.xAbove.xValue.xValue.bValue = (strtoul(&pArgv[4][1], 0, 10) != 0);
			}
			else
			{
				return	FTM_RET_INVALID_ARGUMENTS;	
			}
		}
		else if (strcasecmp(pArgv[3], "le") == 0)
		{
			xEvent.xType = FTM_EVENT_TYPE_BELOW;
			if ((pArgv[4][0] | 0x20) == 'f')
			{
				xEvent.xParams.xBelow.xValue.xType = FTM_EP_DATA_TYPE_FLOAT;
				xEvent.xParams.xBelow.xValue.xValue.fValue = strtod(&pArgv[4][1], 0);
			}
			else if ((pArgv[4][0] | 0x20) == 'i')
			{
				xEvent.xParams.xBelow.xValue.xType = FTM_EP_DATA_TYPE_INT;
				xEvent.xParams.xBelow.xValue.xValue.nValue = strtol(&pArgv[4][1], 0, 10);
			}
			else if ((pArgv[4][0] | 0x20) == 'b')
			{
				xEvent.xParams.xBelow.xValue.xType = FTM_EP_DATA_TYPE_BOOL;
				xEvent.xParams.xBelow.xValue.xValue.bValue = (strtoul(&pArgv[4][1], 0, 10) != 0);
			}
			else
			{
				return	FTM_RET_INVALID_ARGUMENTS;	
			}
		}
		else
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}

		xRet = FTDMC_EVENT_add(&_xSession, &xEvent);
		if (xRet == FTM_RET_OK)
		{
			MESSAGE("The event added successfully.\n");	
		}
		else
		{
			MESSAGE("The event failed to registration.\n");	
		
		}
	}
	else if ((nArgc == 6) && (strcasecmp(pArgv[1], "add") == 0))
	{
		FTM_EVENT		xEvent;

		memset(&xEvent, 0, sizeof(xEvent));
		xEvent.xEPID 	= (FTM_EP_ID)strtoul(pArgv[2], 0, 10);

		if (strcasecmp(pArgv[3], "include") == 0)
		{
			xEvent.xType = FTM_EVENT_TYPE_INCLUDE;
			if ((pArgv[4][0] | 0x20) == 'f')
			{
				xEvent.xParams.xInclude.xLower.xType = FTM_EP_DATA_TYPE_FLOAT;
				xEvent.xParams.xInclude.xLower.xValue.fValue = strtod(&pArgv[4][1], 0);
			}
			else if ((pArgv[4][0] | 0x20) == 'i')
			{
				xEvent.xParams.xInclude.xLower.xType = FTM_EP_DATA_TYPE_INT;
				xEvent.xParams.xInclude.xLower.xValue.nValue = strtol(&pArgv[4][1], 0, 10);
			}
			else if ((pArgv[4][0] | 0x20) == 'b')
			{
				xEvent.xParams.xInclude.xLower.xType = FTM_EP_DATA_TYPE_BOOL;
				xEvent.xParams.xInclude.xLower.xValue.bValue = (strtoul(&pArgv[4][1], 0, 10) != 0);
			}
			if ((pArgv[5][0] | 0x20) == 'f')
			{
				xEvent.xParams.xInclude.xUpper.xType = FTM_EP_DATA_TYPE_FLOAT;
				xEvent.xParams.xInclude.xUpper.xValue.fValue = strtod(&pArgv[5][1], 0);
			}
			else if ((pArgv[5][0] | 0x20) == 'i')
			{
				xEvent.xParams.xInclude.xUpper.xType = FTM_EP_DATA_TYPE_INT;
				xEvent.xParams.xInclude.xUpper.xValue.nValue = strtol(&pArgv[5][1], 0, 10);
			}
			else if ((pArgv[5][0] | 0x20) == 'b')
			{
				xEvent.xParams.xInclude.xUpper.xType = FTM_EP_DATA_TYPE_BOOL;
				xEvent.xParams.xInclude.xUpper.xValue.bValue= (strtoul(&pArgv[5][1], 0, 10) != 0);
			}
			else
			{
				return	FTM_RET_INVALID_ARGUMENTS;	
			}
		}
		else if (strcasecmp(pArgv[3], "except") == 0)
		{
			xEvent.xType = FTM_EVENT_TYPE_EXCEPT;
			if ((pArgv[4][0] | 0x20) == 'f')
			{
				xEvent.xParams.xExcept.xLower.xType = FTM_EP_DATA_TYPE_FLOAT;
				xEvent.xParams.xExcept.xLower.xValue.fValue = strtod(&pArgv[4][1], 0);
			}
			else if ((pArgv[4][0] | 0x20) == 'i')
			{
				xEvent.xParams.xExcept.xLower.xType = FTM_EP_DATA_TYPE_INT;
				xEvent.xParams.xExcept.xLower.xValue.nValue = strtol(&pArgv[4][1], 0, 10);
			}
			else if ((pArgv[4][0] | 0x20) == 'b')
			{
				xEvent.xParams.xExcept.xLower.xType = FTM_EP_DATA_TYPE_BOOL;
				xEvent.xParams.xExcept.xLower.xValue.bValue = (strtoul(&pArgv[4][1], 0, 10) != 0);
			}
			if ((pArgv[5][0] | 0x20) == 'f')
			{
				xEvent.xParams.xExcept.xUpper.xType = FTM_EP_DATA_TYPE_FLOAT;
				xEvent.xParams.xExcept.xUpper.xValue.fValue = strtod(&pArgv[5][1], 0);
			}
			else if ((pArgv[5][0] | 0x20) == 'i')
			{
				xEvent.xParams.xExcept.xUpper.xType = FTM_EP_DATA_TYPE_INT;
				xEvent.xParams.xExcept.xUpper.xValue.nValue = strtol(&pArgv[5][1], 0, 10);
			}
			else if ((pArgv[5][0] | 0x20) == 'b')
			{
				xEvent.xParams.xExcept.xUpper.xType = FTM_EP_DATA_TYPE_BOOL;
				xEvent.xParams.xExcept.xUpper.xValue.bValue = (strtoul(&pArgv[5][1], 0, 10) != 0);
			}
			else
			{
				return	FTM_RET_INVALID_ARGUMENTS;	
			}
		}
		else if (strcasecmp(pArgv[3], "and") == 0)
		{
			xEvent.xType = FTM_EVENT_TYPE_AND;
			xEvent.xParams.xAnd.xID1 = strtol(pArgv[4], 0, 10);
			xEvent.xParams.xAnd.xID2 = strtol(pArgv[5], 0, 10);
		}
		else if (strcasecmp(pArgv[3], "or") == 0)
		{
			xEvent.xType = FTM_EVENT_TYPE_OR;
			xEvent.xParams.xOr.xID1 = strtol(pArgv[4], 0, 10);
			xEvent.xParams.xOr.xID2 = strtol(pArgv[5], 0, 10);
		}
		else
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}

		xRet = FTDMC_EVENT_add(&_xSession, &xEvent);
		if (xRet == FTM_RET_OK)
		{
			MESSAGE("The event added successfully.\n");	
		}
		else
		{
			MESSAGE("The event failed to registration.\n");	
		
		}

		return	xRet;
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDMC_DEBUG_cmd(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	if (strcasecmp(pArgv[1], "mode") == 0)
	{
		if (nArgc == 2)
		{
			FTM_ULONG	ulLevel;

			FTM_PRINT_getLevel(&ulLevel);
			MESSAGE("DEBUG OUT MODE : %s\n", FTM_PRINT_levelString(ulLevel));
		}
		else if (nArgc == 3)
		{
			FTM_ULONG	ulLevel, ulNewLevel;

			ulNewLevel = strtoul(pArgv[2], NULL, 10);
		
			FTM_PRINT_getLevel(&ulLevel);
			FTM_PRINT_setLevel(ulLevel);

			MESSAGE("DEBUG OUT MODE : %s to %s\n",
				FTM_PRINT_levelString(ulLevel),
				FTM_PRINT_levelString(ulNewLevel));
		}
		else
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDMC_cmdQuit(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	if (nArgc != 1)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	
	FTDMC_disconnect(&_xSession);
	_bQuit = FTM_TRUE;


	return	FTM_RET_OK;
}

