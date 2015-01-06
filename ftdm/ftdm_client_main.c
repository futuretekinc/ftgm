#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>    
#include <string.h>    
#include <unistd.h>    
#include <time.h>    
#include <sys/socket.h> 
#include <arpa/inet.h>
#include "libconfig.h"
#include "ftdm_client.h"
#include "ftm_debug.h"
#include "simclist.h"

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
	FTDMC_SUB_CMD_TEST_GEN,
	FTDMC_SUB_CMD_RUN

}	FTDMC_SUB_CMD, _PTR_ FTDMC_SUB_CMD_PTR;

typedef struct	_FTDMC_CMD
{
	FTM_CHAR_PTR	pString;
	FTM_RET		(*function)(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
}	FTDMC_CMD, _PTR_ FTDMC_CMD_PTR;

static FTM_RET	_parseLine
(
	FTM_CHAR_PTR 	pLine, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_INT 		nMaxArgs, 
	FTM_INT_PTR 	pArgc
);

static FTM_RET FTDMC_getCmd
(
	FTM_CHAR_PTR 		pCmdString, 
	FTDMC_CMD_PTR _PTR_ ppCmd
);

static FTM_RET	FTDMC_init(FTM_CHAR_PTR pConfigFile);
static FTM_RET	FTDMC_final(void);
static FTM_RET	FTDMC_cmdConnect(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	FTDMC_cmdDisconnect(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	FTDMC_cmdNode(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	FTDMC_cmdEP(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	FTDMC_cmdEPData(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	FTDMC_cmdHelp(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
static FTM_RET	FTDMC_cmdQuit(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);


FTDM_CLIENT_HANDLE	_hClient = 0;
FTM_CHAR_PTR 		_strPrompt = "FTDMC > ";
FTDMC_CMD			_cmds[] = 
{
	{	"connect",		FTDMC_cmdConnect },
	{	"disconnect",	FTDMC_cmdDisconnect},
	{	"node",			FTDMC_cmdNode},
	{	"ep",			FTDMC_cmdEP},
	{	"data",			FTDMC_cmdEPData},
	{	"help",			FTDMC_cmdHelp},
	{	"?",			FTDMC_cmdHelp},
	{	"quit",			FTDMC_cmdQuit},
	{	NULL,}
};

static	FTM_BOOL	_bQuit = FTM_BOOL_FALSE;
static  list_t		_testEPList;
static  struct tm   _testEPStartTime;
static  struct tm   _testEPEndTime;

int main(int argc , char *argv[])
{
	FTM_INT			nArgc;
	FTM_CHAR_PTR	pArgv[FTDMC_MAX_ARGS];
	FTM_CHAR		pCmdLine[FTDMC_MAX_LINE];
	FTDMC_CMD_PTR 	pCmd;


	FTDMC_init("./ftdm_client.conf");

	while(!_bQuit)
	{
		printf("%s", _strPrompt);
		memset(pCmdLine, 0, sizeof(pCmdLine));
		fgets(pCmdLine, sizeof(pCmdLine), stdin);

		_parseLine(pCmdLine, pArgv, FTDMC_MAX_ARGS, &nArgc);

		if (nArgc != 0)
		{
			if (FTDMC_getCmd(pArgv[0], &pCmd) == FTM_RET_OK)
			{
				pCmd->function(nArgc, pArgv);
			}
			else
			{
				FTM_CHAR_PTR pArgv[] = {"help"};
				MESSAGE("%s is invalid command.\n", pArgv[0]);
				FTDMC_cmdHelp(1, pArgv);
			}
		}
	}

	FTDMC_final();

	return	0;
}

FTM_RET	FTDMC_init(FTM_CHAR_PTR pConfigFile)
{
	setPrintMode(2);
	FTM_initEPTypeString();

	if (pConfigFile != NULL)
	{
		config_t			xConfig;
		config_setting_t 	*pTypeStringSetting;
		config_setting_t 	*pEPTestSetting;
		FTM_INT				i;

		config_init(&xConfig);

		if (CONFIG_TRUE != config_read_file(&xConfig, pConfigFile))
		{
			ERROR("Configuration loading failed.[FILE = %s]\n", pConfigFile);
			return	FTM_RET_CONFIG_LOAD_FAILED;
		}

		pTypeStringSetting = config_lookup(&xConfig, "type_string");
		if (pTypeStringSetting)
		{
			for( i = 0 ; i < config_setting_length(pTypeStringSetting) ; i++)
			{
				config_setting_t	*pElement;

				pElement = config_setting_get_elem(pTypeStringSetting, i);
				if (pElement != NULL)
				{
					FTM_INT		 nType = config_setting_get_int_elem(pElement, 0);	
					FTM_CHAR_PTR pTypeString = (FTM_CHAR_PTR)config_setting_get_string_elem(pElement, 1);	

					if (pTypeString != NULL)
					{
						FTM_appendEPTypeString(nType, pTypeString);	
					}
				}
			}
		}
		
		pEPTestSetting = config_lookup(&xConfig, "ep_test");
		if (pEPTestSetting)
		{
			config_setting_t *pEPIDListSetting;
			config_setting_t *pStartTimeSetting;
			config_setting_t *pEndTimeSetting;

			pEPIDListSetting = config_setting_get_member(pEPTestSetting, "epid");
			if (pEPIDListSetting != 0)
			{
				list_init(&_testEPList);
				for( i = 0 ; i < config_setting_length(pEPIDListSetting) ; i++)
				{
					config_setting_t *pElement;
					
					pElement = config_setting_get_elem(pEPIDListSetting, i);	
					if (pElement != NULL)
					{
						FTM_INT	nEPID = config_setting_get_int(pElement);	
						list_append(&_testEPList, (void *)nEPID);
					}
				}
			}

			pStartTimeSetting = config_setting_get_member(pEPTestSetting, "start_time");
			if (pStartTimeSetting != 0)
			{
				strptime(config_setting_get_string(pStartTimeSetting), "%Y-%m-%d %H:%M:%S", &_testEPStartTime);
			}
		
			pEndTimeSetting = config_setting_get_member(pEPTestSetting, "end_time");
			if (pEndTimeSetting != 0)
			{
				strptime(config_setting_get_string(pEndTimeSetting), "%Y-%m-%d %H:%M:%S", &_testEPEndTime);
			}

			
		}
		

		config_destroy(&xConfig);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDMC_final(void)
{
	list_destroy(&_testEPList);
	FTM_finalEPTypeString();

	return	FTM_RET_OK;
}

FTM_RET FTDMC_getCmd(FTM_CHAR_PTR pCmdString, FTDMC_CMD_PTR _PTR_ ppCmd)
{
	FTDMC_CMD_PTR pCmd = _cmds;
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

FTM_RET	FTDMC_cmdConnect(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
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

FTM_RET	FTDMC_cmdDisconnect(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
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

FTM_RET	FTDMC_cmdNode(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	FTM_RET	nRet;
	FTM_INT	i;
	FTM_NODE_INFO	xNodeInfo;
	FTM_CHAR	pDID[FTM_DID_LEN + 1];
	FTM_CHAR	pURL[FTM_URL_LEN + 1];
	FTM_CHAR	pLocation[FTM_LOCATION_LEN + 1];
	FTDMC_SUB_CMD	nSubCommand = FTDMC_SUB_CMD_HELP;
	
	memset(pDID, 0, sizeof(pDID));
	memset(pURL, 0, sizeof(pURL));
	memset(pLocation, 0, sizeof(pLocation));

	if (nArgc >= 2)
	{
		if (strcasecmp(pArgv[1], "add") == 0)
		{

			if (nArgc < 4)
			{
				ERROR("Invalid node Arguments [ nArgc = %d ]\n", nArgc);
			}
			else
			{
				memset(&xNodeInfo, 0, sizeof(FTM_NODE_INFO));
				xNodeInfo.xType = strtol(pArgv[3], 0, 16);

				if ((xNodeInfo.xType == FTM_NODE_TYPE_SNMP) && (nArgc == 6))
				{
					if ((strlen(pArgv[5]) <= FTM_SNMP_COMMUNITY_LEN) &&
						(strlen(pArgv[4]) <= FTM_URL_LEN) &&
						(strlen(pArgv[2]) <= FTM_DID_LEN))
					{
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

						nSubCommand = FTDMC_SUB_CMD_ADD;			
					}
					else
					{
						ERROR("Invalid DID length[< %d]\n", FTM_DID_LEN);
					}

				}
				else
				{
					ERROR("Invalid type or arguments [ Type = %08lx ]\n", xNodeInfo.xType);
				}
			}

		}
		else if (strcasecmp(pArgv[1], "del") == 0)
		{
			FTM_INT	i;

			if (strlen(pArgv[2]) > FTM_DID_LEN)
			{
				ERROR("%s : ERROR - Invalid argument\n", pArgv[0]);	
			}
			else
			{
				memset(pDID, 0, sizeof(pDID));
				for(i = 0 ; i < strlen(pArgv[2]) ; i++)
				{
					pDID[i] = toupper(pArgv[2][i]);	
				}

				nSubCommand = FTDMC_SUB_CMD_DEL;			
			}
		}
		else if (strcasecmp(pArgv[1], "info") == 0)
		{
			FTM_INT	i;

			if (strlen(pArgv[2]) > FTM_DID_LEN)
			{
				ERROR("%s : ERROR - Invalid argument\n", pArgv[0]);	
			}
			else
			{
				memset(pDID, 0, sizeof(pDID));
				for(i = 0 ; i < strlen(pArgv[2]) ; i++)
				{
					pDID[i] = toupper(pArgv[2][i]);	
				}

				nSubCommand = FTDMC_SUB_CMD_INFO;			
			}
		}
		else if (strcasecmp(pArgv[1], "list") == 0)
		{
			nSubCommand = FTDMC_SUB_CMD_LIST;			
		}
		else if (strcasecmp(pArgv[1], "short_help") == 0)
		{
			nSubCommand = FTDMC_SUB_CMD_SHORT_HELP;			
		}
	
	}

	switch(nSubCommand)
	{
	case		FTDMC_SUB_CMD_ADD:
		{
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
		break;

	case	FTDMC_SUB_CMD_INFO:
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
		break;

	case	FTDMC_SUB_CMD_LIST:
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
		break;

	case	FTDMC_SUB_CMD_SHORT_HELP:
		{
			MESSAGE("%-16s <add|del|info> <DID> <type> ...\n", pArgv[0]);
		}
		break;

	case	FTDMC_SUB_CMD_HELP:
	default:
		{
			MESSAGE("Usage : %s <cmd> <DID> ...\n", pArgv[0]);
			MESSAGE("COMMAND:\n");
			MESSAGE("\tadd <DID> <TYPE>\n");
			MESSAGE("    cmd - add, del, info\n");
			MESSAGE("    DID - string for Node ID [Max %d] \n", FTM_DID_LEN);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDMC_cmdEP(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	FTM_RET		nRet;
	FTM_INT		i;
	FTM_CHAR		pPID[FTM_DID_LEN + 1];
	FTM_CHAR		pDID[FTM_DID_LEN + 1];
	FTM_CHAR		pName[FTM_NAME_LEN + 1];
	FTM_CHAR		pUnit[FTM_UNIT_LEN + 1];
	FTM_ULONG		nInterval = 0;
	FTM_EP_TYPE		xType = 0;
	FTM_EPID		xEPID = 0;
	FTM_ULONG		nSubCommand = FTDMC_SUB_CMD_HELP;

	memset(pPID, 0, sizeof(pPID));
	memset(pDID, 0, sizeof(pDID));
	memset(pUnit, 0, sizeof(pUnit));
	memset(pName, 0, sizeof(pName));

	if (nArgc >= 2)
	{
		if (strcmp(pArgv[1], "add") == 0)
		{
			switch(nArgc)
			{
			case	8:
				if (strlen(pArgv[7]) > FTM_DID_LEN)
				{
					break;
				}

				for(i = 0 ; i < strlen(pArgv[7]) ; i++)
				{
					pPID[i] = toupper(pArgv[7][i]);	
				}

			case	7:
				if (strlen(pArgv[6]) > FTM_DID_LEN)
				{
					break;
				}

				for(i = 0 ; i < strlen(pArgv[6]) ; i++)
				{
					pDID[i] = toupper(pArgv[6][i]);	
				}

			case	6:
				nInterval = atoi(pArgv[5]);
				if (nInterval < 0)
				{
					break;	
				}

			case	5:
				if (strlen(pArgv[4]) > FTM_UNIT_LEN)
				{
					break;
				}

				for(i = 0 ; i < strlen(pArgv[4]) ; i++)
				{
					pUnit[i] = toupper(pArgv[4][i]);	
				}

			case	4:
				if (strlen(pArgv[3]) > FTM_NAME_LEN)
				{
					break;
				}

				for(i = 0 ; i < strlen(pArgv[3]) ; i++)
				{
					pName[i] = toupper(pArgv[3][i]);	
				}

			case	3:
				xEPID = strtoul(pArgv[2], NULL, 16);
				if (xEPID < 0)
				{
					break;	
				}

				xType = (xEPID & FTM_EP_TYPE_MASK);
				nSubCommand = FTDMC_SUB_CMD_ADD;			
				break;
			}

		}
		else if (strcmp(pArgv[1], "del") == 0)
		{
			if (nArgc == 3)
			{
				xEPID = atoi(pArgv[2]);
				if (xEPID > 0)
				{
					nSubCommand = FTDMC_SUB_CMD_DEL;
				}
			}
		}
		else if (strcmp(pArgv[1], "list") == 0)
		{
			nSubCommand = FTDMC_SUB_CMD_LIST;			
		}
		else if (strcmp(pArgv[1], "data") == 0)
		{
			nSubCommand = FTDMC_SUB_CMD_DATA;			
		}
		else if (strcmp(pArgv[1], "short_help") == 0)
		{
			nSubCommand = FTDMC_SUB_CMD_SHORT_HELP;			
		}
		else
		{
			nSubCommand = FTDMC_SUB_CMD_HELP;			
		}

		switch(nSubCommand)
		{
		case	FTDMC_SUB_CMD_ADD:
			{
				FTM_EP_INFO	xInfo;

				xInfo.xEPID 	= xEPID;
				xInfo.xType 	= xType;
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

		case	FTDMC_SUB_CMD_DEL:
			{
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

				MESSAGE("%8s %16s %16s %16s %8s %16s %16s\n",
					"EPID", "TYPE", "NAME", "UNIT", "INTERVAL", "DID", "PID");

				for(i = 0 ; i< nCount ; i++)
				{
					FTM_EP_INFO	xInfo;

					nRet = FTDMC_getEPInfoByIndex(_hClient, i, &xInfo);
					if (nRet == FTM_RET_OK)
					{
						MESSAGE("%08lx %16s %16s %16s %8lu %16s %16s\n",
								xInfo.xEPID,
								FTM_getEPTypeString(xInfo.xType),
								xInfo.pName,
								xInfo.pUnit,
								xInfo.nInterval,
								xInfo.pDID,
								xInfo.pPID);
					}
				}
			}
			break;

		case	FTDMC_SUB_CMD_TEST_GEN:
			{
				FTM_EP_INFO	xInfo;

				xInfo.xEPID 	= xEPID;
				xInfo.xType 	= xType;
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

		case	FTDMC_SUB_CMD_SHORT_HELP:
			{
				MESSAGE("%-16s <add|del|list> [EPID] [TYPE] ...\n", pArgv[0]);
			}
			break;

		case	FTDMC_SUB_CMD_HELP:
		default:
			{
				MESSAGE("Usage : %s <COMMAND> ...\n"\
						"\tEndPoint management.\n"\
						"COMMANDS:\n"\
						"\tadd <EPID> <Type> [Name] [Unit] [Interval] [DID] [PID]\n"\
						"\tdel <EPID>\n"\
						"\tlist\n", pArgv[0]);
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDMC_cmdEPData(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	FTM_RET			nRet;
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
		else if (strcmp(pArgv[1], "test_gen") == 0)
		{
			if (nArgc == 3)
			{
				xCmd = FTDMC_SUB_CMD_TEST_GEN;			
			}
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
			FTM_EP_DATA	xData;

			xData.xEPID = strtoul(pArgv[2], NULL, 16);
			xData.nTime	= strtoul(pArgv[3], NULL, 10);
			switch(toupper(pArgv[4][0]))
			{
			case	'I':	
				{
					xData.xType = FTM_EP_DATA_TYPE_INT;
					xData.xValue.nValue = strtol(&pArgv[4][1], NULL, 10); 
				}
				break;

			case	'F':	
				{
					xData.xType = FTM_EP_DATA_TYPE_FLOAT;
					xData.xValue.fValue = strtod(&pArgv[4][1], NULL); 
				} 
				break;

			case	'U':	
				{	
					xData.xType = FTM_EP_DATA_TYPE_ULONG;
					xData.xValue.ulValue = strtoul(&pArgv[4][1], NULL, 10); 
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
				MESSAGE("Invalid arguments!\n");	
			}

			nRet = FTDMC_appendEPData(_hClient, &xData);

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
			FTM_ULONG		nStartIndex=0;
			FTM_EPID		pEPIDs[32];
			FTM_ULONG		nEPIDCount=0;

			optind = 2;
			while((opt = getopt(nArgc, pArgv, "s: c: b: e:")) != -1)
			{
				switch(toupper(opt))
				{
					case	'C':
						nMaxCount = strtoul(optarg, NULL, 10);
						break;

					case	'B':
						nBeginTime = strtoul(optarg, NULL, 10);
						break;

					case	'E':
						nEndTime = strtoul(optarg, NULL, 10);
						break;

					case	'S':
						nStartIndex = strtoul(optarg, NULL, 10);
						break;
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
			FTM_ULONG		nStartIndex=0;
			FTM_ULONG		nMaxCount=50;
			FTM_ULONG		nCount=0;

			optind = 2;
			while((opt = getopt(nArgc, pArgv, "c:b:e:s:")) != -1)
			{
				switch(toupper(opt))
				{
					case	'C':
						{
							FTM_ULONG	nNewMax = 	strtoul(optarg, 0, 10);

							if (nNewMax < nMaxCount)
							{
								nMaxCount = nNewMax;	
							}
						}
						break;

					case	'B':
						nBeginTime = atoi(optarg);
						break;

					case	'E':
						nEndTime = atoi(optarg);
						break;

					case	'S':
						nStartIndex = strtoul(optarg, 0, NULL);
				}
			}


			while((nEPIDCount < 32) && (optind < nArgc))
			{
				pEPIDs[nEPIDCount++] = strtol(pArgv[optind++], NULL, 16);
			}

			pEPData = (FTM_EP_DATA_PTR)malloc(sizeof(FTM_EP_DATA) * nMaxCount);
			if (pEPData == NULL)
			{
				MESSAGE("System not enough memory!\n");
				return	FTM_RET_NOT_ENOUGH_MEMORY;		
			}

			nRet = FTDMC_getEPData(_hClient, 
									pEPIDs, 
									nEPIDCount, 
									nBeginTime, 
									nEndTime, 
									pEPData, 
									nStartIndex,
									nMaxCount, 
									&nCount);
			TRACE("FTDMC_getEPData(hClient, pEPIDs, %d, %d, %d, pEPData, %d, %d) = %08lx\n",
				nEPIDCount, nBeginTime, nEndTime, nMaxCount, nCount, nRet);
			if (nRet == FTM_RET_OK)
			{
				FTM_INT	i;

				MESSAGE("%8s %8s %32s %8s\n", "INDEX", "EPID", "DATE", "VALUE");	
				for(i = 0 ; i < nCount ; i++)
				{
					FTM_CHAR	pTime[64];
					time_t		xTime = pEPData[i].nTime;

					strftime(pTime, sizeof(pTime), "%Y-%m-%d %H:%M:%S", gmtime(&xTime));
					switch(pEPData[i].xType)
					{
					case	FTM_EP_DATA_TYPE_ULONG:
						{
							MESSAGE("%8d %08lx %32s %8lu\n", 
									nStartIndex + i, pEPData[i].xEPID, pTime, pEPData[i].xValue.ulValue);	
						}
						break;

					case	FTM_EP_DATA_TYPE_FLOAT:
						{
							MESSAGE("%8d %08lx %32s %8.3lf\n", 
									nStartIndex + i, pEPData[i].xEPID, pTime, pEPData[i].xValue.fValue);	
						}
						break;
					case	FTM_EP_DATA_TYPE_INT:
					default:
						{
							MESSAGE("%8d %08lx %32s %8d\n", 
									nStartIndex + i, pEPData[i].xEPID, pTime, pEPData[i].xValue.nValue);	
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

	case	FTDMC_SUB_CMD_TEST_GEN:
		{
			FTM_EP_DATA	xData;
			FTM_INT		i, nDataGenCount;
			time_t		_startTime;
			time_t		_endTime;

			_startTime = mktime(&_testEPStartTime);
			_endTime = mktime(&_testEPEndTime);

			nDataGenCount = strtol(pArgv[2], NULL, 10);

			for(i = 0 ; i < nDataGenCount ; i++)
			{
				FTM_INT	nIndex = 0;

				nIndex = rand() % list_size(&_testEPList);

				xData.xEPID = (FTM_EPID)list_get_at(&_testEPList, nIndex);
				xData.xType = FTM_EP_DATA_TYPE_INT;
				xData.nTime = _startTime + rand() % (_endTime - _startTime);
				xData.xValue.nValue = rand();

				FTDMC_appendEPData(_hClient, &xData);
		}
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

FTM_RET	FTDMC_cmdHelp(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
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
				MESSAGE("%-16s Help\n", pArgv[0]) ;
			}
			else if (strcmp(pArgv[1], "help") == 0)
			{
				MESSAGE("Usage : %s [cmd]\n", pArgv[0]);
				MESSAGE("\tHelp\n");
			}
			else
			{
				FTDMC_CMD_PTR pCmd;

				if (FTDMC_getCmd(pArgv[1], &pCmd) == FTM_RET_OK)
				{
					FTM_CHAR_PTR	pNewArgv[2] = { pArgv[1], "help"};
					pCmd->function(2, pNewArgv);
				}
			}
		}
		break;

	default:
		{
			FTM_INT			i;
			FTDMC_CMD_PTR 	pCmd;

			for(i = 1; i < nArgc ; i++)
			{
				if (FTDMC_getCmd(pArgv[i], &pCmd) == FTM_RET_OK)
				{
					FTM_CHAR_PTR	pNewArgv[2] = { pArgv[i], "help"};
					pCmd->function(2, pNewArgv);
				}
			}
		}

	}

	return	FTM_RET_OK;
}

FTM_RET	FTDMC_cmdQuit(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	switch (nArgc)
	{
	case	1:
		{
			FTDMC_disconnect(_hClient);
			_bQuit = FTM_BOOL_TRUE;
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


	return	FTM_RET_OK;
}

