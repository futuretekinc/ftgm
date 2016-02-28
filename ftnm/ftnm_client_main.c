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
#include "ftm_types.h"
#include "ftm_error.h"
#include "ftm_mem.h"
#include "ftnm_client.h"
#include "ftnm_client_config.h"
#include "ftm_debug.h"

#define		FTNMC_MAX_LINE	2048
#define		FTNMC_MAX_ARGS	16

typedef struct	_FTNMC_CMD
{
	FTM_CHAR_PTR	pString;
	FTM_RET			(*function)(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
	FTM_CHAR_PTR	pShortHelp;
	FTM_CHAR_PTR	pHelp;
}	FTNMC_CMD, _PTR_ FTNMC_CMD_PTR;

static FTM_RET	FTNMC_CMD_parseCmdLine
(
	FTM_CHAR_PTR 	pLine, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_INT 		nMaxArgs, 
	FTM_INT_PTR 	pArgc
);

static FTM_RET FTNMC_CMD_get
(
	FTM_CHAR_PTR 		pCmdString, 
	FTNMC_CMD_PTR _PTR_ ppCmd
);

static FTM_RET	FTNMC_notifyCallback(FTM_VOID_PTR pData);

static FTM_RET	FTNMC_CMD_connect(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	FTNMC_CMD_disconnect(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	FTNMC_CMD_NODE(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	FTNMC_CMD_EP(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	FTNMC_CMD_EP_DATA(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	FTNMC_CMD_debug(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	FTNMC_CMD_help(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	FTNMC_CMD_quit(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);


static FTNMC_SESSION_PTR		pCurrentSession = NULL;
FTM_CHAR_PTR 		_strPrompt = "FTNMC > ";
FTNMC_CMD			_cmds[] = 
{
	{	
		.pString	= "connect",
		.function	= FTNMC_CMD_connect,
		.pShortHelp = "Connnect to FTNM server.",
		.pHelp		= "<Server IP> <Service Port>\n"\
					  "\tConnect to FTNM server.\n"\
					  "PARAMETERS:\n"\
					  "\tServer IP    Target server IP address\n"\
					  "\t             (Default : 127.0.0.1)\n"\
					  "\tServer Port  FTNM Port"
	},
	{	
		.pString	= "disconnect",	
		.function	= FTNMC_CMD_disconnect,
		.pShortHelp = "Disconnect from FTNM server.",
		.pHelp		= "\n"\
					  "\tDisconnect to FTNM server.\n"
	},
	{	
		.pString	= "node",
		.function	= FTNMC_CMD_NODE,
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
		.function   = FTNMC_CMD_EP,
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
		.function	= FTNMC_CMD_EP_DATA,
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
	{	
		.pString	= "help",
		.function	= FTNMC_CMD_help,
		.pShortHelp = "Help command.",
		.pHelp	    = "<COMMAND>\n"\
					  "\tHelp command.\n"\
					  "PARAMETERS:\n"\
					  "\tCOMMAND    Target command for help."
	},
	{	
		.pString	= "?",
		.function	= FTNMC_CMD_help,
		.pShortHelp = "Help command.",
		.pHelp	    = "<COMMAND>\n"\
					  "\tHelp command.\n"\
					  "PARAMETERS:\n"\
					  "\tCOMMAND    Target command for help."
	},
	{	.pString	= "debug",
		.function	= FTNMC_CMD_debug,
		.pShortHelp = "",
		.pHelp		= ""
	},
	{	
		.pString	= "quit",
		.function	= FTNMC_CMD_quit,
		.pShortHelp = "Quit program.",
		.pHelp 		= "\n"\
					  "\tQuit program."
	},
	{	
		.pString	= NULL,
	}
};


static	FTNM_CFG_CLIENT	xClientConfig;
static	FTM_BOOL	_bQuit = FTM_FALSE;
extern	char *		program_invocation_short_name;
	FTM_CHAR		pCmdLine[FTNMC_MAX_LINE];

int main(int argc , char *argv[])
{
	FTM_INT			nArgc;
	FTM_CHAR_PTR	pArgv[FTNMC_MAX_ARGS];
	FTM_CHAR		pConfigFileName[FTM_FILE_NAME_LEN];

	sprintf(pConfigFileName, "%s.conf", program_invocation_short_name);

	FTM_MEM_init();
	/* load configuraton */
	FTNMC_initConfig(&xClientConfig);
	FTNMC_loadConfig(&xClientConfig, pConfigFileName);

	/* apply configuraton */
	FTNMC_init(&xClientConfig);

	while(!_bQuit)
	{
		printf("%s", _strPrompt);
		memset(pCmdLine, 0, sizeof(pCmdLine));
		fgets(pCmdLine, sizeof(pCmdLine), stdin);

		FTNMC_CMD_parseCmdLine(pCmdLine, pArgv, FTNMC_MAX_ARGS, &nArgc);

		if (nArgc != 0)
		{
			FTNMC_CMD_PTR 	pCmd;

			if (FTNMC_CMD_get(pArgv[0], &pCmd) == FTM_RET_OK)
			{
				if (pCmd->function(nArgc, pArgv) == FTM_RET_INVALID_ARGUMENTS)
				{
					MESSAGE("Usage : %s %s\n", pCmd->pString, pCmd->pHelp);
				}
			}
			else
			{
				FTM_CHAR_PTR pArgv[] = {"help"};
				MESSAGE("%s is invalid command.\n", pArgv[0]);
				FTNMC_CMD_help(1, pArgv);
			}
		}
	}

	FTNMC_final();

	FTNMC_finalConfig(&xClientConfig);
	FTM_MEM_final();

	return	0;
}

FTM_RET FTNMC_CMD_get(FTM_CHAR_PTR pCmdString, FTNMC_CMD_PTR _PTR_ ppCmd)
{
	FTNMC_CMD_PTR pCmd = _cmds;
	while(pCmd->pString != NULL)
	{
		if (strcmp(pCmd->pString, pCmdString) == 0)
		{
			*ppCmd = pCmd;
			return	FTM_RET_OK;
		}

		pCmd++;
	}

	return	FTM_RET_INVALID_COMMAND;
}

FTM_RET	FTNMC_CMD_parseCmdLine(FTM_CHAR_PTR pLine, FTM_CHAR_PTR pArgv[], FTM_INT nMaxArgs, FTM_INT_PTR pArgc)
{
	FTM_INT		nCount = 0;
	FTM_CHAR_PTR	pWord = NULL;
	FTM_CHAR_PTR	pSeperator = "\t \n\r";	

	pWord = strtok(pLine, pSeperator); 
	while((pWord != NULL) && (nCount < nMaxArgs))
	{
		pArgv[nCount++] = pWord;
		pWord = strtok(NULL, pSeperator);
	}

	*pArgc = nCount;

	return	FTM_RET_OK;
}

FTM_RET	FTNMC_CMD_connect(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	FTM_RET			xRet;
	in_addr_t		xAddr	= inet_addr(xClientConfig.xNetwork.pServerIP);
	struct	in_addr	xAddr2;
	FTM_USHORT		nPort 	= xClientConfig.xNetwork.usPort;

	inet_aton(xClientConfig.xNetwork.pServerIP, &xAddr2);
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

	xRet = FTNMC_SESSION_create(&pCurrentSession);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't not create session.[%08x]\n", xRet);
		return	xRet;	
	}

	TRACE("Try connection to %s:%d\n", inet_ntoa(xAddr2), nPort);
	if (FTNMC_SESSION_connect(pCurrentSession, xAddr, nPort) != FTM_RET_OK)
	{
		TRACE("connect failed. Error");
		return	FTM_RET_ERROR;	
	}

	FTNMC_setNotifyCallback(pCurrentSession, FTNMC_notifyCallback);
	MESSAGE("Connected\n");

	return	FTM_RET_OK;
}

FTM_RET	FTNMC_CMD_disconnect(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{

	if (pCurrentSession == NULL)
	{
		TRACE("Does not exist connected session!\n");
		return	FTM_RET_OK;	
	}

	switch(nArgc)
	{
	case	1: 
		{
			FTNMC_SESSION_disconnect(pCurrentSession);
			FTNMC_SESSION_destroy(pCurrentSession);
			pCurrentSession = NULL;
		}
		break;

	default:
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNMC_CMD_NODE(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
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

			nRet = FTNMC_NODE_create(pCurrentSession, &xNodeInfo);
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

		nRet = FTNMC_NODE_destroy(pCurrentSession, pDID);
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

		nRet = FTNMC_NODE_get(pCurrentSession, pDID, &xInfo);
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

		nRet = FTNMC_NODE_count(pCurrentSession, &nNodeCount);
		if (nRet != FTM_RET_OK)
		{
			ERROR("%s : ERROR - %lu\n", pArgv[0], nRet);
		}

		MESSAGE("NODE COUNT : %d\n", nNodeCount);

		MESSAGE("%-16s %-16s %-16s %-8s %-16s %-16s %-16s\n", 
			"DID", "TYPE", "LOCATION", "INTERVAL", "OPT0", "OPT1", "OPT2", "OPT3");

		for(i = 0 ; i < nNodeCount; i++)
		{
			FTM_NODE	xInfo;

			nRet = FTNMC_NODE_getAt(pCurrentSession, i, &xInfo);
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

FTM_RET	FTNMC_CMD_EP(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
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
			xInfo.ulInterval = (FTM_ULONG)strtoul(pArgv[5], 0, 10);
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

			nRet = FTNMC_EP_create(pCurrentSession, &xInfo);
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
		nRet = FTNMC_EP_destroy(pCurrentSession, xEPID);	
		if (nRet != FTM_RET_OK)
		{
			ERROR("%s : ERROR - %lu\n", pArgv[0], nRet);
		}
	}
	else if (strcmp(pArgv[1], "list") == 0)
	{
		FTM_ULONG	nCount;

		if (nArgc == 2)
		{
			nRet = FTNMC_EP_count(pCurrentSession, 0, &nCount);
			if (nRet == FTM_RET_OK)
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
					MESSAGE("%-8s %-8s %-16s %-8s %-8s %-16s %-16s %-8s\n",
						"EPID", "CLASS", "NAME", "UNIT", "INTERNAL", "DID", "PID", "VALUE");
					FTNMC_EP_getList(pCurrentSession, 0x00000000, pEPIDs, nCount, &nCount);
					for(i = 0 ; i< nCount ; i++)
					{
						FTM_EP	xInfo;

						nRet = FTNMC_EP_get(pCurrentSession, pEPIDs[i], &xInfo);
						if (nRet == FTM_RET_OK)
						{
							MESSAGE("%08lx %08x %-16s %-8s %8lu %-16s %-16s ",
									xInfo.xEPID,
									xInfo.xType,//"UNKNOWN",//FTM_getEPTypeString(xInfo.xType),
									xInfo.pName,
									xInfo.pUnit,
									xInfo.ulInterval,
									xInfo.pDID,
									xInfo.pPID);
						}

						nRet = FTNMC_EP_DATA_getLast(pCurrentSession, pEPIDs[i], &xData);
						if (nRet == FTM_RET_OK)
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
			FTM_EP_CLASS	xEPClass;
			FTM_EP_ID		pEPID[100];

			xEPClass = strtoul(pArgv[2], 0, 16);
			nRet = FTNMC_EP_getList(pCurrentSession, xEPClass, pEPID, 100, &nCount);
			if (nRet == FTM_RET_OK)
			{
				MESSAGE("%8s %16s %16s %16s %8s %16s %16s\n",
						"EPID", "TYPE", "NAME", "UNIT", "INTERVAL", "DID", "PID");

				for(i = 0 ; i< nCount ; i++)
				{
					FTM_EP	xInfo;

					nRet = FTNMC_EP_get(pCurrentSession, pEPID[i], &xInfo);
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
					else
					{
							ERROR("%s : ERROR - %lu\n", pArgv[0], nRet);
					}
				}
			}
		}
	}
	else if (strcmp(pArgv[1], "count") == 0)
	{
	
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNMC_CMD_EP_DATA(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
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
		ERROR("Invalid arguments[nArgc = %d]\n", nArgc);	
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	if (strcmp(pArgv[1], "add") == 0)
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

		nRet = FTNMC_EP_DATA_add(pCurrentSession, xEPID, &xData);
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
/*
				nRet = FTNMC_EP_DATA_delWithTime(pCurrentSession, xEPID, nBeginTime, nEndTime);
				if (nRet == FTM_RET_OK)
				{
					MESSAGE("EndPoint data deleted successfully!\n");	
				}
				else
				{
					ERROR("EndPoint data deleting failed [ ERROR = %08lx ]\n", nRet);	
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
	else if (strcmp(pArgv[1], "count") == 0)
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

				nRet = FTNMC_EP_DATA_info(pCurrentSession, xEPID, &ulBeginTime, &ulEndTime, &ulCount);
				if (nRet == FTM_RET_OK)
				{
					MESSAGE("      EPID : %08lx\n", xEPID);
					MESSAGE("DATA COUNT : %lu\n", ulCount);
					MESSAGE("BEGIN TIME : %s\n", ctime(&ulBeginTime));
					MESSAGE("  END TIME : %s\n", ctime(&ulEndTime));
				}
				else
				{
					TRACE("FTNMC_EP_DATA_count error [%08lx]\n", nRet);	
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

				pEPData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA) * nMaxCount);
				if (pEPData == NULL)
				{
					MESSAGE("System not enough memory!\n");
					return	FTM_RET_NOT_ENOUGH_MEMORY;		
				}

				nRet = FTNMC_EP_DATA_getList(pCurrentSession, 
						xEPID, 
						nStartIndex,
						pEPData, 
						nMaxCount, 
						&nCount);
				TRACE("FTNMC_getEPData(hClient, %08lx, %d, %d, pEPData, %d, %d) = %08lx\n",
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

FTM_RET	FTNMC_CMD_debug(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	if (strcasecmp(pArgv[1], "mode") == 0)
	{
		if (nArgc == 2)
		{
			FTM_ULONG	nMode;
			FTM_PRINT_getLevel(&nMode);
			switch(nMode)
			{
			case	0: MESSAGE("DEBUG OUT MODE : NONE\n"); break;
			case	1: MESSAGE("DEBUG OUT MODE : NORMAL\n"); break;
			case	2: MESSAGE("DEBUG OUT MODE : ALL\n"); break;
			}
		}
		else if (nArgc == 3)
		{
			FTM_ULONG	nMode, nNewMode;

			nNewMode = strtoul(pArgv[2], NULL, 10);
		
			FTM_PRINT_getLevel(&nMode);
			FTM_PRINT_setLevel(nNewMode);

			switch(nMode)
			{
			case	0: MESSAGE("DEBUG OUT MODE : NONE"); break;
			case	1: MESSAGE("DEBUG OUT MODE : NORMAL"); break;
			case	2: MESSAGE("DEBUG OUT MODE : ALL"); break;
			}

			switch(nNewMode)
			{
			case	0: MESSAGE(" to NONE\n"); break;
			case	1: MESSAGE(" to NORMAL\n"); break;
			case	2: MESSAGE(" to ALL\n"); break;
			}
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

FTM_RET	FTNMC_CMD_help(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{

	switch(nArgc)
	{
	case	1:
		{
			FTNMC_CMD_PTR	pCmd = _cmds; 
			while(pCmd->pString != NULL)
			{
				MESSAGE("%-16s    %s\n", pCmd->pString, pCmd->pShortHelp);
				pCmd++;	
			}
		}
		break;

	case	2:
		{
			FTNMC_CMD_PTR pCmd;

			if (FTNMC_CMD_get(pArgv[1], &pCmd) == FTM_RET_OK)
			{
				MESSAGE("Usage : %s %s\n", pArgv[1], pCmd->pHelp);
			}
		}
		break;

	default:
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTNMC_CMD_quit(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	if (nArgc != 1)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	
	FTNMC_SESSION_disconnect(pCurrentSession);
	FTNMC_SESSION_destroy(pCurrentSession);
	pCurrentSession = NULL;

	_bQuit = FTM_TRUE;


	return	FTM_RET_OK;
}

FTM_RET	FTNMC_notifyCallback(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);

	FTNM_MSG_PTR	pMsg = (FTNM_MSG_PTR)pData;

	switch(pMsg->xType)
	{
	case	FTNM_MSG_TYPE_EP_CHANGED:
		{
			switch(pMsg->xParams.xEPChanged.xData.xType)
			{
			case	FTM_EP_DATA_TYPE_INT:
				{
					TRACE("EP[%08x] : changed value = %d\n", pMsg->xParams.xEPChanged.xEPID, pMsg->xParams.xEPChanged.xData.xValue.nValue);
				}
				break;

			case	FTM_EP_DATA_TYPE_ULONG:
				{
					TRACE("EP[%08x] : changed value = %lu\n", pMsg->xParams.xEPChanged.xEPID, pMsg->xParams.xEPChanged.xData.xValue.ulValue);
				}
				break;

			case	FTM_EP_DATA_TYPE_FLOAT:
				{
					TRACE("EP[%08x] : changed value = %5.2f\n", pMsg->xParams.xEPChanged.xEPID, pMsg->xParams.xEPChanged.xData.xValue.fValue);
				}
				break;

			}

		}
		break;
	
	default:
		{
			ERROR("Not supported message[%08x]\n", pMsg->xType);
		}
	}
	
	return	FTM_RET_OK;

}
