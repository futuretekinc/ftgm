#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>    
#include <string.h>    
#include <unistd.h>    
#include <sys/socket.h> 
#include <arpa/inet.h>
#include "ftdm_client.h"
#include "ftm_debug.h"

#define		FTDMC_MAX_LINE	2048
#define		FTDMC_MAX_ARGS	16

typedef	enum	_FTDMC_SUB_CMD
{ 
	FTDMC_SUB_CMD_UNKNOWN = 0,
	FTDMC_SUB_CMD_HELP,
	FTDMC_SUB_CMD_SHORT_HELP,
	FTDMC_SUB_CMD_ADD,
	FTDMC_SUB_CMD_DEL,
	FTDMC_SUB_CMD_COUNT,
	FTDMC_SUB_CMD_LIST,
	FTDMC_SUB_CMD_GET,
	FTDMC_SUB_CMD_INFO,
	FTDMC_SUB_CMD_DATA,
	FTDMC_SUB_CMD_CONNECT,
	FTDMC_SUB_CMD_DISCONNECT,
	FTDMC_SUB_CMD_RUN

}	FTDMC_SUB_CMD, _PTR_ FTDMC_SUB_CMD_PTR;

static FTM_RET	_parseLine
(
	FTM_CHAR_PTR 	pLine, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_INT 		nMaxArgs, 
	FTM_INT_PTR 	pArgc
);

static FTM_RET	_connect(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	_disconnect(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	_node(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	_ep(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	_data(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	_help(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	_quit(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);

typedef struct	_FTDMC_CMD
{
	FTM_CHAR_PTR	pString;
	FTM_RET		(*function)(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
}	FTDMC_CMD, _PTR_ FTDMC_CMD_PTR;

FTDM_CLIENT_HANDLE	_hClient = 0;
FTM_CHAR_PTR 		_strPrompt = "FTDMC> ";
FTDMC_CMD			_cmds[] = 
{
	{	"connect",		_connect },
	{	"disconnect",	_disconnect},
	{	"node",		_node},
	{	"ep",			_ep},
	{	"data",			_data},
	{	"help",			_help},
	{	"?",			_help},
	{	"quit",			_quit},
	{	NULL,}
};

int main(int argc , char *argv[])
{
	FTM_INT		nArgc;
	FTM_CHAR_PTR	pArgv[FTDMC_MAX_ARGS];
	FTM_CHAR		pCmdLine[FTDMC_MAX_LINE];

	setPrintMode(2);

	while(1)
	{
		printf("%s", _strPrompt);
		memset(pCmdLine, 0, sizeof(pCmdLine));
		fgets(pCmdLine, sizeof(pCmdLine), stdin);

		_parseLine(pCmdLine, pArgv, FTDMC_MAX_ARGS, &nArgc);

		if (nArgc != 0)
		{
			FTDMC_CMD_PTR pCmd = _cmds;
			while(pCmd->pString != NULL)
			{
				if (strcmp(pCmd->pString, pArgv[0]) == 0)
				{
					if (pCmd->function(nArgc, pArgv) != FTM_RET_OK)
					{
						exit(0);
					}
					break;
				}

				pCmd++;
			}
		}
	}
}

FTM_RET	_parseLine(FTM_CHAR_PTR pLine, FTM_CHAR_PTR pArgv[], FTM_INT nMaxArgs, FTM_INT_PTR pArgc)
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

FTM_RET	_connect(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	in_addr_t		xAddr	= inet_addr("127.0.0.1");
	FTM_USHORT		nPort 	= 8888;
	FTDMC_SUB_CMD	xCmd  	= FTDMC_SUB_CMD_CONNECT;

	if (nArgc >= 2)
	{
		if (strcmp(pArgv[1], "help") == 0)
		{
			xCmd = FTDMC_SUB_CMD_HELP;
		}
		else if (strcmp(pArgv[1], "short_help") == 0)
		{
			xCmd = FTDMC_SUB_CMD_SHORT_HELP;
		}

		switch(nArgc)
		{
		case	3:
			{
				nPort = atoi(pArgv[2]);
			}

		case	2:
			{
				xAddr = inet_addr(pArgv[1]);
				if (xAddr == 0)
				{
					MESSAGE("Usage : %s [ip address] [port]\n", pArgv[0]);
					xCmd = FTDMC_SUB_CMD_SHORT_HELP;
				}
			}
		}
	}

	switch(xCmd)
	{
	case	FTDMC_SUB_CMD_CONNECT:
		{
			if (FTDMC_connect(xAddr, nPort, &_hClient) != FTM_RET_OK)
			{
				perror("connect failed. Error");
				return	FTM_RET_ERROR;	
			}
			MESSAGE("Connected\n");
		}
		break;
	

	case	FTDMC_SUB_CMD_SHORT_HELP:
		{
			MESSAGE("%-16s [IP] [Port]\n", pArgv[0]);
		}
		break;

	default:
		{
			MESSAGE("Usage : %s [IP] [Port]\n", pArgv[0]);
			MESSAGE("  %-16s Server IP\n", "IP");
			MESSAGE("  %-16s Service Port\n", "Port");
		}
		break;
	}

	return	FTM_RET_OK;
}

FTM_RET	_disconnect(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	FTDMC_SUB_CMD	xCmd;

	switch(nArgc)
	{
	case	1: xCmd = FTDMC_SUB_CMD_DISCONNECT; break;
	case	2:
		{
			if (strcmp(pArgv[1], "help") == 0)
			{
				xCmd = FTDMC_SUB_CMD_HELP;	
				break;
			}
			else if (strcmp(pArgv[1], "short_help") == 0)
			{
				xCmd = FTDMC_SUB_CMD_SHORT_HELP;	
				break;
			}
		}
	default:
		xCmd = FTDMC_SUB_CMD_UNKNOWN;	
	}

	switch(xCmd)
	{
	case	FTDMC_SUB_CMD_DISCONNECT:	
		{
			FTDMC_disconnect(_hClient);
		}
		break;

	case	FTDMC_SUB_CMD_SHORT_HELP:
		{
			MESSAGE("%-16s\n", pArgv[0]);	
		}
		break;

	default:	
		{
			MESSAGE("Usage : %s\n", pArgv[0]);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	_node(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	FTM_RET	nRet;
	FTM_INT	i;
	FTM_BOOL	bShowUsage = FTM_BOOL_FALSE;
	FTM_CHAR	pDID[FTM_DID_LEN + 1];
	FTM_CHAR	pURL[FTM_URL_LEN + 1];
	FTM_CHAR	pLocation[FTM_LOCATION_LEN + 1];
	FTDMC_SUB_CMD	nSubCommand = FTDMC_SUB_CMD_HELP;
	
	memset(pDID, 0, sizeof(pDID));
	memset(pURL, 0, sizeof(pURL));
	memset(pLocation, 0, sizeof(pLocation));

	if (nArgc < 2)
	{
		bShowUsage = FTM_BOOL_TRUE;	
	}
	else
	{
		if (strcasecmp(pArgv[1], "add") == 0)
		{
			nSubCommand = FTDMC_SUB_CMD_ADD;			
		}
		else if (strcasecmp(pArgv[1], "del") == 0)
		{
			nSubCommand = FTDMC_SUB_CMD_DEL;			
		}
		else if (strcasecmp(pArgv[1], "info") == 0)
		{
			nSubCommand = FTDMC_SUB_CMD_INFO;			
		}
		else if (strcasecmp(pArgv[1], "list") == 0)
		{
			nSubCommand = FTDMC_SUB_CMD_LIST;			
		}
		else
		{
			bShowUsage = FTM_BOOL_TRUE;	
		}
	
	}

	switch(nSubCommand)
	{
	case		FTDMC_SUB_CMD_ADD:
		{
			FTM_NODE_INFO	xNodeInfo;

			if (nArgc < 4)
			{
				ERROR("Invalid node Arguments [ nArgc = %d ]\n", nArgc);
				bShowUsage = FTM_BOOL_TRUE;	
				break;	
			}

			memset(&xNodeInfo, 0, sizeof(FTM_NODE_INFO));
			xNodeInfo.xType = strtol(pArgv[3], 0, 16);

			if (xNodeInfo.xType == FTM_NODE_TYPE_SNMP)
			{
				if (nArgc != 6)
				{
					ERROR("Invalid SNMP node Arguments [ nArgc = %d ]\n", nArgc);
					bShowUsage = FTM_BOOL_TRUE;	
					break;	
				}

				for(i = 0 ; i < strlen(pArgv[5]) ; i++)
				{
					xNodeInfo.xOption.xSNMP.pCommunity[i] = toupper(pArgv[5][i]);	
				}

		
				for(i = 0 ; i < strlen(pArgv[4]) ; i++)
				{
					xNodeInfo.xOption.xSNMP.pURL[i] = toupper(pArgv[4][i]);	
				}


				if (strlen(pArgv[2]) > FTM_DID_LEN)
				{
					ERROR("Invalid DID length[< %d]\n", FTM_DID_LEN);
					bShowUsage = FTM_BOOL_TRUE;
					break;
				}

				for(i = 0 ; i < strlen(pArgv[2]) ; i++)
				{
					xNodeInfo.pDID[i] = toupper(pArgv[2][i]);	
				}
			}
			else
			{
				ERROR("Invalid Node Type [ Type = %08lx ]\n", xNodeInfo.xType);

				bShowUsage = FTM_BOOL_TRUE;	
				break;	
			}

			nRet = FTDMC_createNode(_hClient, &xNodeInfo);
			if (nRet != FTM_RET_OK)
			{
				ERROR("%s : ERROR - %lx\n", pArgv[0], nRet);
			}
			else
			{
				MESSAGE("%s : The node[%s] has been created successfully.\n", 
					pArgv[0], pDID);
			}
		}
		break;

	case	FTDMC_SUB_CMD_DEL:
	case	FTDMC_SUB_CMD_INFO:
		{
			FTM_INT	i;

			if (strlen(pArgv[2]) > FTM_DID_LEN)
			{
				ERROR("%s : ERROR - Invalid argument\n", pArgv[0]);	
				bShowUsage = FTM_BOOL_TRUE;
				break;
			}

			memset(pDID, 0, sizeof(pDID));
			for(i = 0 ; i < strlen(pArgv[2]) ; i++)
			{
				pDID[i] = toupper(pArgv[2][i]);	
			}

			if (strcasecmp(pArgv[1], "del") == 0)
			{
				nRet = FTDMC_destroyNode(_hClient, pDID);
				if (nRet != FTM_RET_OK)
				{
					ERROR("%s : ERROR - %lu\n", pArgv[0], nRet);
				}
				else
				{
					TRACE("%s : The node[%s] has been destroied successfully.\n", 
						pArgv[0], pDID);
				}
			}
			else if (strcasecmp(pArgv[1], "info") == 0)
			{
				FTM_NODE_INFO	xInfo;

				nRet = FTDMC_getNodeInfo(_hClient, pDID, &xInfo);
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
		}
		break;

	case	FTDMC_SUB_CMD_LIST:
		{
			if (strcasecmp(pArgv[1], "list") == 0)
			{
				FTM_ULONG	i, nNodeCount = 0;

				nRet = FTDMC_getNodeCount(_hClient, &nNodeCount);
				if (nRet != FTM_RET_OK)
				{
					ERROR("%s : ERROR - %lu\n", pArgv[0], nRet);
				}

				MESSAGE("%16s %16s %16s %16s %16s %16s %16s\n", 
					"DID", "TYPE", "LOCATION", "OPT0", "OPT1", "OPT2", "OPT3");

				for(i = 0 ; i < nNodeCount; i++)
				{
					FTM_NODE_INFO	xInfo;

					nRet = FTDMC_getNodeInfoByIndex(_hClient, i, &xInfo);
					if (nRet == FTM_RET_OK)
					{
						MESSAGE("%16s %16s %16s ", 
							xInfo.pDID, 
							FTM_nodeTypeString(xInfo.xType), 
							xInfo.pLocation);

						switch(xInfo.xType)
						{
						case	FTM_NODE_TYPE_SNMP:
							{
								MESSAGE("%16d %16s %16s", 
									xInfo.xOption.xSNMP.nVersion,
									xInfo.xOption.xSNMP.pURL,
									xInfo.xOption.xSNMP.pCommunity);
								
							}
							break;
						}

						MESSAGE("\n");
					}
				
				}
			}
		}
		break;

	default:
		break;
	}

	if (bShowUsage)
	{
		MESSAGE("Usage : %s <cmd> <DID> ...\n", pArgv[0]);
		MESSAGE("COMMAND:\n");
		MESSAGE("\tadd <DID> <TYPE>\n");
		MESSAGE("    cmd - add, del, info\n");
		MESSAGE("    DID - string for Node ID [Max %d] \n", FTM_DID_LEN);
	}

	return	FTM_RET_OK;
}

FTM_RET	_ep(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	FTM_RET		nRet;
	FTM_INT		i;
	FTM_BOOL		bShowUsage = FTM_BOOL_FALSE;
	FTM_CHAR		pPID[FTM_DID_LEN + 1];
	FTM_CHAR		pDID[FTM_DID_LEN + 1];
	FTM_CHAR		pName[FTM_NAME_LEN + 1];
	FTM_CHAR		pUnit[FTM_UNIT_LEN + 1];
	FTM_ULONG		nInterval = 0;
	FTM_EP_TYPE		nType = 0;
	FTM_EPID		xEPID = 0;

	memset(pPID, 0, sizeof(pPID));
	memset(pDID, 0, sizeof(pDID));
	memset(pUnit, 0, sizeof(pUnit));
	memset(pName, 0, sizeof(pName));

	if (nArgc < 2)
	{
		bShowUsage = FTM_BOOL_TRUE;	
	}
	else
	{
		FTM_ULONG	nSubCommand = 0;

		if (strcmp(pArgv[1], "help") == 0)
		{
			nSubCommand = FTDMC_SUB_CMD_HELP;			
		}
		else if (strcmp(pArgv[1], "add") == 0)
		{
			nSubCommand = FTDMC_SUB_CMD_ADD;			
		}
		else if (strcmp(pArgv[1], "del") == 0)
		{
			nSubCommand = FTDMC_SUB_CMD_DEL;
		}
		else if (strcmp(pArgv[1], "list") == 0)
		{
			nSubCommand = FTDMC_SUB_CMD_LIST;			
		}
		else if (strcmp(pArgv[1], "data") == 0)
		{
			nSubCommand = FTDMC_SUB_CMD_DATA;			
		}

		switch(nSubCommand)
		{
		case	FTDMC_SUB_CMD_ADD:
			{
				switch(nArgc)
				{
				case	9:
					if (strlen(pArgv[8]) > FTM_DID_LEN)
					{
						bShowUsage = FTM_BOOL_TRUE;
						break;
					}

					for(i = 0 ; i < strlen(pArgv[8]) ; i++)
					{
						pPID[i] = toupper(pArgv[8][i]);	
					}

				case	8:
					if (strlen(pArgv[7]) > FTM_DID_LEN)
					{
						bShowUsage = FTM_BOOL_TRUE;
						break;
					}

					for(i = 0 ; i < strlen(pArgv[7]) ; i++)
					{
						pDID[i] = toupper(pArgv[7][i]);	
					}

				case	7:
					nInterval = atoi(pArgv[6]);

				case	6:
					if (strlen(pArgv[5]) > FTM_UNIT_LEN)
					{
						bShowUsage = FTM_BOOL_TRUE;
						break;
					}

					for(i = 0 ; i < strlen(pArgv[5]) ; i++)
					{
						pUnit[i] = toupper(pArgv[5][i]);	
					}

				case	5:
					if (strlen(pArgv[4]) > FTM_NAME_LEN)
					{
						bShowUsage = FTM_BOOL_TRUE;
						break;
					}

					for(i = 0 ; i < strlen(pArgv[4]) ; i++)
					{
						pName[i] = toupper(pArgv[4][i]);	
					}

				case	4:
					nType = atoi(pArgv[3]);

				case	3:
					xEPID = atoi(pArgv[2]);

				case	2:
					{
						FTM_EP_INFO	xInfo;

						xInfo.xEPID 	= xEPID;
						xInfo.xType 	= nType;
						xInfo.nInterval = nInterval;
						strcpy(xInfo.pName, pName);
						strcpy(xInfo.pUnit, pUnit);
						strcpy(xInfo.pPID, pPID);
						strcpy(xInfo.pDID, pDID);

						nRet = FTDMC_createEP(_hClient, &xInfo);
						if (nRet != FTM_RET_OK)
						{
							ERROR("%s : ERROR - %lu\n", pArgv[0], nRet);
						}
					}
					break;

				default:
					bShowUsage = FTM_BOOL_TRUE;
					break;
				}
				break;

			case	FTDMC_SUB_CMD_DEL:
				{
					if (nArgc != 3)
					{
						bShowUsage = FTM_BOOL_TRUE;
						break;
					}

					xEPID = atoi(pArgv[2]);
					nRet = FTDMC_destroyEP(_hClient, xEPID);	
					if (nRet != FTM_RET_OK)
					{
						ERROR("%s : ERROR - %lu\n", pArgv[0], nRet);
					}
				}
				break;

			case	FTDMC_SUB_CMD_LIST:
				{
					FTM_ULONG	nCount;

					nRet = FTDMC_getEPCount(_hClient, &nCount);
					if (nRet != FTM_RET_OK)
					{
						ERROR("%s : ERROR - %lu\n", pArgv[0], nRet);
						break;
					}

					MESSAGE("%8s %8s %16s %16s %8s %16s %16s\n",
						"EPID", "TYPE", "NAME", "UNIT", "INTERVAL", "DID", "PID");

					for(i = 0 ; i< nCount ; i++)
					{
						FTM_EP_INFO	xInfo;

						nRet = FTDMC_getEPInfoByIndex(_hClient, i, &xInfo);
						if (nRet == FTM_RET_OK)
						{
							MESSAGE("%08lx %08lx %16s %16s %8lu %16s %16s\n",
									xInfo.xEPID,
									xInfo.xType,
									xInfo.pName,
									xInfo.pUnit,
									xInfo.nInterval,
									xInfo.pDID,
									xInfo.pPID);
						}
					}
				}
				break;

			default:
				bShowUsage = FTM_BOOL_TRUE;
			}
		}

	}

	if (bShowUsage)
	{
		MESSAGE("Usage : %s <COMMAND> ...\n"\
				"\tEndPoint management.\n"\
				"COMMANDS:\n"\
				"\tadd <EPID> <Type> [Name] [Unit] [Interval] [DID] [PID]\n"\
				"\tdel <EPID>\n"\
				"\tlist\n", pArgv[0]);
	}

	return	FTM_RET_OK;
}

FTM_RET	_data(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	FTM_RET		nRet;
	FTDMC_SUB_CMD	xCmd = FTDMC_SUB_CMD_HELP;

	if (nArgc >= 2)
	{
		if (strcmp(pArgv[1], "add") == 0)
		{
			if (nArgc == 5)
			{
				xCmd =FTDMC_SUB_CMD_ADD;
			}
		}
		else if (strcmp(pArgv[1], "del") == 0)
		{
			xCmd =FTDMC_SUB_CMD_DEL;
		}
		else if (strcmp(pArgv[1], "count") == 0)
		{
			xCmd = FTDMC_SUB_CMD_COUNT;
		}
		else if (strcmp(pArgv[1], "get") == 0)
		{
			xCmd = FTDMC_SUB_CMD_GET;
		}
		else if (strcmp(pArgv[1], "short_help") == 0)
		{
			xCmd = FTDMC_SUB_CMD_SHORT_HELP;
		}
		else
		{
			xCmd = FTDMC_SUB_CMD_HELP;
		}
	}

	switch(xCmd)
	{
	case	FTDMC_SUB_CMD_ADD:
		{
			FTM_EPID	xEPID = atoi(pArgv[2]);
			FTM_ULONG	nTime = atoi(pArgv[3]);
			FTM_ULONG	nValue= atoi(pArgv[4]);

			nRet = FTDMC_appendEPData(_hClient, xEPID, nTime, nValue);

			if (nRet == FTM_RET_OK)
			{
				MESSAGE("EndPoint data appending done successfully!\n");	
			}
			else
			{
				ERROR("EndPoint data appending failed [ ERROR = %08lx ]\n", nRet);	

			}
		}
		break;
	case	FTDMC_SUB_CMD_DEL:
		{
			FTM_INT		opt = 0;
			FTM_ULONG		nMaxCount=0;
			FTM_ULONG		nBeginTime = 0;
			FTM_ULONG		nEndTime = 0;
			FTM_EPID		pEPIDs[32];
			FTM_ULONG		nEPIDCount=0;

			optind = 2;
			while((opt = getopt(nArgc, pArgv, "c: b: e:")) != -1)
			{
				switch(opt)
				{
					case	'c':
						nMaxCount = atoi(optarg);
						break;

					case	'b':
						nBeginTime = atoi(optarg);
						break;

					case	'e':
						nEndTime = atoi(optarg);
				}
			}

			while((nEPIDCount < 32) && (optind < nArgc))
			{
				pEPIDs[nEPIDCount++] = atoi(pArgv[optind++]);	
			}

			nRet = FTDMC_removeEPData(_hClient, pEPIDs, nEPIDCount, nBeginTime, nEndTime, nMaxCount);
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

	case	FTDMC_SUB_CMD_GET:
		{
			FTM_INT		opt = 0;
			FTM_ULONG		nBeginTime = 0;
			FTM_ULONG		nEndTime = 0;
			static FTM_EPID		pEPIDs[32];
			FTM_ULONG		nEPIDCount=0;
			FTM_EP_DATA_PTR	pEPData;	
			FTM_ULONG		nMaxCount=100;
			FTM_ULONG		nCount=0;

			optind = 2;
			while((opt = getopt(nArgc, pArgv, "c:b:e:")) != -1)
			{
				switch(opt)
				{
					case	'c':
						{
							FTM_ULONG	nNewMax = 	atoi(optarg);

							if (nNewMax < nMaxCount)
							{
								nMaxCount = nNewMax;	
							}
						}
						break;

					case	'b':
						nBeginTime = atoi(optarg);
						break;

					case	'e':
						nEndTime = atoi(optarg);
				}
			}


			while((nEPIDCount < 32) && (optind < nArgc))
			{
				pEPIDs[nEPIDCount++] = atoi(pArgv[optind++]);
			}

			pEPData = (FTM_EP_DATA_PTR)malloc(sizeof(FTM_EP_DATA) * nMaxCount);
			if (pEPData == NULL)
			{
				MESSAGE("System not enough memory!\n");
				return	FTM_RET_NOT_ENOUGH_MEMORY;		
			}

			nRet = FTDMC_getEPData(_hClient, 
					pEPIDs, nEPIDCount, 
					nBeginTime, nEndTime, 
					pEPData, nMaxCount, &nCount);
			if (nRet == FTM_RET_OK)
			{
				FTM_INT	i;

				for(i = 0 ; i < nCount ; i++)
				{
					MESSAGE("%08lx, %d, %d\n", pEPData[i].xEPID, pEPData[i].nTime, pEPData[i].nValue);	
				}
				MESSAGE("EndPoint data load completed!\n");	
			}
			else
			{
				MESSAGE("EndPoint data loading failed [ ERROR = %08lx ]\n", nRet);	
			}

			free(pEPData);
		}
		break;

	case	FTDMC_SUB_CMD_SHORT_HELP:
		{
			MESSAGE("%-16s <COMMAND>\n", pArgv[0]);
		}
		break;

	case	FTDMC_SUB_CMD_HELP:
	default:
		{
			MESSAGE("Usage : %s <COMMAND> ...\n"\
					"\tEndPoint data management.\n"\
					"COMMANDS:\n"\
					"\tadd <EPID> <Time> <Value>\n"\
					"\tdel [-c MaxCount] [-b BeginTime] [-e EndTime] [EPID ...]\n"\
					"\tcount [-b BeginTime] [-e EndTime] [EPID ...]\n"\
					"\tget [-b BeginTime] [-e EndTime] [EPID ...]\n", pArgv[0]);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	_help(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{

	switch(nArgc)
	{
		case	1:
			{
				FTDMC_CMD_PTR	pCmd = _cmds; 
				FTM_CHAR_PTR	pArgv[2] = { NULL, "short_help"};

				while(pCmd->pString != NULL)
				{
					pArgv[0] = pCmd->pString;
					pCmd->function(2, pArgv);
					pCmd++;	
				}
			}
			break;

		case	2:
			{
				if (strcmp(pArgv[1], "short_help") == 0)
				{
					MESSAGE("%s - Help\n", pArgv[0]) ;
				}
				else if (strcmp(pArgv[1], "help") == 0)
				{
					MESSAGE("%s - Help\n", pArgv[0]);
			}
			else
			{
				FTDMC_CMD_PTR	pCmd = _cmds; 
				FTM_CHAR_PTR	pArgv[2] = { NULL, "help"};

				while(pCmd->pString != NULL)
				{
					if (strcmp(pArgv[1], pCmd->pString) == 0)
					{
						pArgv[0] = pCmd->pString;
						pCmd->function(2, pArgv);
						break;
					}
					pCmd++;	
				}

			}

		}
	};

	return	FTM_RET_OK;
}

FTM_RET	_quit(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	switch (nArgc)
	{
	case	1:
		{
			FTDMC_disconnect(_hClient);
		}
		break;

	default:
		{
			if (strcmp(pArgv[1], "short_help") == 0)
			{
				MESSAGE("%-16s Exit the program.\n", pArgv[0]);	
			}
			else 
			{
				MESSAGE("Usage : %s\n"\
						"\tExit the program.\n", pArgv[0]);
			}
		}
	}	

	exit(1);

	return	FTM_RET_OK;
}

