#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>    
#include <string.h>    
#include <unistd.h>    
#include <sys/socket.h> 
#include <arpa/inet.h>
#include "ftdm_client.h"
#include "ftdm_debug.h"

#define		FTDMC_MAX_LINE	2048
#define		FTDMC_MAX_ARGS	16

#define		FTDMC_SUB_CMD_HELP	1
#define		FTDMC_SUB_CMD_ADD	2
#define		FTDMC_SUB_CMD_DEL	3
#define		FTDMC_SUB_CMD_LIST	4
#define		FTDMC_SUB_CMD_DATA	5

static FTDM_RET	_parseLine(FTDM_CHAR_PTR pLine, FTDM_CHAR_PTR pArgv[], FTDM_INT nMaxArgs, FTDM_INT_PTR pArgc);
static FTDM_RET	_connect(FTDM_INT nArgc, FTDM_CHAR_PTR pArgv[]);
static FTDM_RET	_disconnect(FTDM_INT nArgc, FTDM_CHAR_PTR pArgv[]);
static FTDM_RET	_device(FTDM_INT nArgc, FTDM_CHAR_PTR pArgv[]);
static FTDM_RET	_ep(FTDM_INT nArgc, FTDM_CHAR_PTR pArgv[]);
static FTDM_RET	_data(FTDM_INT nArgc, FTDM_CHAR_PTR pArgv[]);
static FTDM_RET	_help(FTDM_INT nArgc, FTDM_CHAR_PTR pArgv[]);
static FTDM_RET	_quit(FTDM_INT nArgc, FTDM_CHAR_PTR pArgv[]);

typedef struct	_FTDMC_CMD
{
	FTDM_CHAR_PTR	pString;
	FTDM_RET		(*function)(FTDM_INT nArgc, FTDM_CHAR_PTR pArgv[]);
}	FTDMC_CMD, _PTR_ FTDMC_CMD_PTR;

FTDM_CLIENT_HANDLE	_hClient = 0;
FTDM_CHAR_PTR 		_strPrompt = "FTDMC> ";
FTDMC_CMD			_cmds[] = 
{
	{	"connect",		_connect },
	{	"disconnect",	_disconnect},
	{	"device",		_device},
	{	"ep",			_ep},
	{	"data",			_data},
	{	"help",			_help},
	{	"?",			_help},
	{	"quit",			_quit},
	{	NULL,}
};

int main(int argc , char *argv[])
{
	FTDM_INT		nArgc;
	FTDM_CHAR_PTR	pArgv[FTDMC_MAX_ARGS];
	FTDM_CHAR		pCmdLine[FTDMC_MAX_LINE];

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
					if (pCmd->function(nArgc, pArgv) != FTDM_RET_OK)
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

FTDM_RET	_parseLine(FTDM_CHAR_PTR pLine, FTDM_CHAR_PTR pArgv[], FTDM_INT nMaxArgs, FTDM_INT_PTR pArgc)
{
	FTDM_INT		nCount = 0;
	FTDM_CHAR_PTR	pWord = NULL;
	FTDM_CHAR_PTR	pSeperator = "\t \n\r";	

	pWord = strtok(pLine, pSeperator); 
	while((pWord != NULL) && (nCount < nMaxArgs))
	{
		pArgv[nCount++] = pWord;
		pWord = strtok(NULL, pSeperator);
	}

	*pArgc = nCount;

	return	FTDM_RET_OK;
}

FTDM_RET	_connect(FTDM_INT nArgc, FTDM_CHAR_PTR pArgv[])
{
	in_addr_t	xAddr = 0;
	FTDM_USHORT	nPort = 0;

	switch(nArgc)
	{
	case	1:
		{
			xAddr = inet_addr("127.0.0.1");
			nPort = 8888;
		}
		break;

	case	2:
		{
			xAddr = inet_addr(pArgv[1]);
			nPort = 8888;
		}
		break;

	case	3:
		{
			xAddr = inet_addr(pArgv[1]);
			nPort = atoi(pArgv[2]);
		}
		break;
	}

	if (xAddr == 0)
	{
		printf("Usage : %s [ip address] [port]\n", pArgv[0]);
		return	FTDM_RET_ERROR;
	}

	if (FTDMC_connect(xAddr, nPort, &_hClient) != FTDM_RET_OK)
	{
		perror("connect failed. Error");
		return	FTDM_RET_ERROR;	
	}

	printf("Connected\n");

	return	FTDM_RET_OK;
}

FTDM_RET	_disconnect(FTDM_INT nArgc, FTDM_CHAR_PTR pArgv[])
{
	FTDMC_disconnect(_hClient);

	return	FTDM_RET_OK;
}

FTDM_RET	_device(FTDM_INT nArgc, FTDM_CHAR_PTR pArgv[])
{
	FTDM_RET	nRet;
	FTDM_INT	i;
	FTDM_BOOL	bShowUsage = FTDM_BOOL_FALSE;
	FTDM_CHAR	pDID[FTDM_DEVICE_ID_LEN + 1];
	FTDM_CHAR	pURL[FTDM_DEVICE_URL_LEN + 1];
	FTDM_CHAR	pLocation[FTDM_DEVICE_LOCATION_LEN + 1];
	FTDM_ULONG	xType;

	memset(pDID, 0, sizeof(pDID));
	memset(pURL, 0, sizeof(pURL));
	memset(pLocation, 0, sizeof(pLocation));

	switch(nArgc)
	{
	case	6:
		for(i = 0 ; i < strlen(pArgv[5]) ; i++)
		{
			pLocation[i] = toupper(pArgv[5][i]);	
		}

	case	5:
		for(i = 0 ; i < strlen(pArgv[4]) ; i++)
		{
			pURL[i] = toupper(pArgv[4][i]);	
		}

	case	4:
		for(i = 0 ; i < strlen(pArgv[2]) ; i++)
		{
			pDID[i] = toupper(pArgv[2][i]);	
		}
	
		xType = atoi(pArgv[3]);

		if (strcasecmp(pArgv[1], "add") == 0)
		{
			nRet = FTDMC_createDevice(_hClient, pDID, xType, pURL, pLocation);
			if (nRet != FTDM_RET_OK)
			{
				printf("%s : ERROR - %lx\n", pArgv[0], nRet);
			}
			else
			{
				printf("%s : The device[%s] has been created successfully.\n", 
					pArgv[0], pDID);
			}
		}
		break;

	case	3:
		{
			FTDM_INT	i;

			if (strlen(pArgv[2]) > FTDM_DEVICE_ID_LEN)
			{
				printf("%s : ERROR - Invalid argument\n", pArgv[0]);	
				bShowUsage = FTDM_BOOL_TRUE;
				break;
			}

			memset(pDID, 0, sizeof(pDID));
			for(i = 0 ; i < strlen(pArgv[2]) ; i++)
			{
				pDID[i] = toupper(pArgv[2][i]);	
			}

			if (strcasecmp(pArgv[1], "del") == 0)
			{
				nRet = FTDMC_destroyDevice(_hClient, pDID);
				if (nRet != FTDM_RET_OK)
				{
					printf("%s : ERROR - %lu\n", pArgv[0], nRet);
				}
				else
				{
					printf("%s : The device[%s] has been destroied successfully.\n", 
						pArgv[0], pDID);
				}
			}
			else if (strcasecmp(pArgv[1], "info") == 0)
			{
				FTDM_DEVICE_INFO	xInfo;

				nRet = FTDMC_getDeviceInfo(_hClient, pDID, &xInfo);
				if (nRet != FTDM_RET_OK)
				{
					printf("%s : ERROR - %lu\n", pArgv[0], nRet);
				}
				else
				{
					printf("DEVICE INFORMATION\n");
					printf("      DID : %s\n", xInfo.pDID);
					printf("     TYPE : %lu\n", xInfo.xType);
					printf("      URL : %s\n", xInfo.pURL);
					printf(" LOCATION : %s\n", xInfo.pLocation);
				}
			}
		}
		break;

	case	2:
		{
			if (strcasecmp(pArgv[1], "list") == 0)
			{
				FTDM_ULONG	i, nDeviceCount = 0;

				nRet = FTDMC_getDeviceCount(_hClient, &nDeviceCount);
				if (nRet != FTDM_RET_OK)
				{
					printf("%s : ERROR - %lu\n", pArgv[0], nRet);
				}

				for(i = 0 ; i < nDeviceCount; i++)
				{
					FTDM_DEVICE_INFO	xInfo;

					nRet = FTDMC_getDeviceInfoByIndex(_hClient, i, &xInfo);
					if (nRet == FTDM_RET_OK)
					{
						printf("%32s %8lu %16s %16s\n", 
							xInfo.pDID, 
							xInfo.xType, 
							xInfo.pURL, 
							xInfo.pLocation);
					}
				
				}
			}
		}
		break;

	}

	if (bShowUsage)
	{
		printf("Usage : %s <cmd> <DID> ...\n", pArgv[0]);
		printf("    cmd - add, del, info\n");
		printf("    DID - string for Device ID [Max %d] \n", FTDM_DEVICE_ID_LEN);
	}

	return	FTDM_RET_OK;
}

FTDM_RET	_ep(FTDM_INT nArgc, FTDM_CHAR_PTR pArgv[])
{
	FTDM_RET		nRet;
	FTDM_INT		i;
	FTDM_BOOL		bShowUsage = FTDM_BOOL_FALSE;
	FTDM_CHAR		pPID[FTDM_DEVICE_ID_LEN + 1];
	FTDM_CHAR		pDID[FTDM_DEVICE_ID_LEN + 1];
	FTDM_CHAR		pName[FTDM_EP_NAME_LEN + 1];
	FTDM_CHAR		pUnit[FTDM_EP_UNIT_LEN + 1];
	FTDM_ULONG		nInterval = 0;
	FTDM_EP_TYPE	nType = 0;
	FTDM_EP_ID		xEPID = 0;

	memset(pPID, 0, sizeof(pPID));
	memset(pDID, 0, sizeof(pDID));
	memset(pUnit, 0, sizeof(pUnit));
	memset(pName, 0, sizeof(pName));

	if (nArgc < 2)
	{
		bShowUsage = FTDM_BOOL_TRUE;	
	}
	else
	{
		FTDM_ULONG	nSubCommand = 0;

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

		switch(nArgc)
		{
		case	9:
			if (strlen(pArgv[8]) > FTDM_DEVICE_ID_LEN)
			{
				bShowUsage = FTDM_BOOL_TRUE;
				break;
			}

			for(i = 0 ; i < strlen(pArgv[8]) ; i++)
			{
				pPID[i] = toupper(pArgv[8][i]);	
			}

		case	8:
			if (strlen(pArgv[7]) > FTDM_DEVICE_ID_LEN)
			{
				bShowUsage = FTDM_BOOL_TRUE;
				break;
			}

			for(i = 0 ; i < strlen(pArgv[7]) ; i++)
			{
				pDID[i] = toupper(pArgv[7][i]);	
			}

		case	7:
			nInterval = atoi(pArgv[6]);

		case	6:
			if (strlen(pArgv[5]) > FTDM_EP_UNIT_LEN)
			{
				bShowUsage = FTDM_BOOL_TRUE;
				break;
			}

			for(i = 0 ; i < strlen(pArgv[5]) ; i++)
			{
				pUnit[i] = toupper(pArgv[5][i]);	
			}

		case	5:
			if (strlen(pArgv[4]) > FTDM_EP_NAME_LEN)
			{
				bShowUsage = FTDM_BOOL_TRUE;
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
				switch(nSubCommand)
				{
				case	FTDMC_SUB_CMD_HELP:
					{
						bShowUsage = FTDM_BOOL_TRUE;
					}
					break;

				case	FTDMC_SUB_CMD_ADD:
					{
						FTDM_EP_INFO	xInfo;

						xInfo.xEPID 	= xEPID;
						xInfo.xType 	= nType;
						xInfo.nInterval = nInterval;
						strcpy(xInfo.pName, pName);
						strcpy(xInfo.pUnit, pUnit);
						strcpy(xInfo.pPID, pPID);
						strcpy(xInfo.pDID, pDID);

						nRet = FTDMC_createEP(_hClient, &xInfo);
						if (nRet != FTDM_RET_OK)
						{
							printf("%s : ERROR - %lu\n", pArgv[0], nRet);
						}
					}
					break;

				case	FTDMC_SUB_CMD_DEL:
					{
						nRet = FTDMC_destroyEP(_hClient, xEPID);	
						if (nRet != FTDM_RET_OK)
						{
							printf("%s : ERROR - %lu\n", pArgv[0], nRet);
						}
					}
					break;

				case	FTDMC_SUB_CMD_LIST:
					{
						FTDM_ULONG	nCount;

						nRet = FTDMC_getEPCount(_hClient, &nCount);
						if (nRet != FTDM_RET_OK)
						{
							printf("%s : ERROR - %lu\n", pArgv[0], nRet);
							break;
						}

						for(i = 0 ; i< nCount ; i++)
						{
							FTDM_EP_INFO	xInfo;

							nRet = FTDMC_getEPInfoByIndex(_hClient, i, &xInfo);
							if (nRet == FTDM_RET_OK)
							{
								printf("%08lx %08lx %16s %16s %4lu %16s %16s\n",
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
				}
			}
			break;
		}

	}

	if (bShowUsage)
	{
		printf("Usage : %s <COMMAND> ...\n", pArgv[0]);
		printf("EndPoint management.\n");
		printf("COMMANDS:\n");
		printf("  add <EPID> <Type> [Name] [Unit] [Interval] [DID] [PID]\n");
		printf("  del <EPID>\n");
		printf("  list\n");
	}

	return	FTDM_RET_OK;
}

FTDM_RET	_data(FTDM_INT nArgc, FTDM_CHAR_PTR pArgv[])
{
	FTDM_RET		nRet;
	FTDM_BOOL		bShowUsage = FTDM_BOOL_FALSE;

	if (nArgc < 2)
	{
		return	FTDM_RET_INVALID_ARGUMENTS;	
	}

	if (strcmp(pArgv[1], "add") == 0)
	{
		if (nArgc != 5)
		{
			bShowUsage = FTDM_BOOL_FALSE;	
		}
		else
		{
			FTDM_EP_ID	xEPID = atoi(pArgv[2]);
			FTDM_ULONG	nTime = atoi(pArgv[3]);
			FTDM_ULONG	nValue= atoi(pArgv[4]);

			nRet = FTDMC_appendEPData(_hClient, xEPID, nTime, nValue);
			
			if (nRet == FTDM_RET_OK)
			{
				printf("EndPoint data appending done successfully!\n");	
			}
			else
			{
				printf("EndPoint data appending failed [ ERROR = %08lx ]\n", nRet);	
			
			}
		}
	}
	else if (strcmp(pArgv[1], "del") == 0)
	{
		FTDM_INT		opt = 0;
		FTDM_ULONG		nMaxCount=0;
		FTDM_ULONG		nBeginTime = 0;
		FTDM_ULONG		nEndTime = 0;
		FTDM_EP_ID		pEPIDs[32];
		FTDM_ULONG		nEPIDCount=0;

		optind = 0;
		while((opt = getopt(nArgc, pArgv, "c: b: e:")) != -1)
		{
			printf("OPT : %c, ARG : %s\n", opt, optarg);
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
			pEPIDs[nEPIDCount++] = pArgv[optind++];	
		}

		nRet = FTDMC_removeEPData(_hClient, pEPIDs, nEPIDCount, nBeginTime, nEndTime, nMaxCount);
		if (nRet == FTDM_RET_OK)
		{
			printf("EndPoint data deleted successfully!\n");	
		}
		else
		{
			printf("EndPoint data deleting failed [ ERROR = %08lx ]\n", nRet);	
		}
	}
	else if (strcmp(pArgv[1], "count") == 0)
	{
	
	}
	else if (strcmp(pArgv[1], "get") == 0)
	{
		FTDM_INT		opt = 0;
		FTDM_ULONG		nBeginTime = 0;
		FTDM_ULONG		nEndTime = 0;
		static FTDM_EP_ID		pEPIDs[32];
		FTDM_ULONG		nEPIDCount=0;
		FTDM_EP_DATA_PTR	pEPData;	
		FTDM_ULONG		nMaxCount=100;
		FTDM_ULONG		nCount=0;

		optind = 0;
		while((opt = getopt(nArgc, pArgv, "c:b:e:")) != -1)
		{
			switch(opt)
			{
				case	'c':
					{
						FTDM_ULONG	nNewMax = 	atoi(optarg);
						
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
			pEPIDs[nEPIDCount++] = pArgv[optind++];	
		}

		pEPData = (FTDM_EP_DATA_PTR)malloc(sizeof(FTDM_EP_DATA) * nMaxCount);
		if (pEPData == NULL)
		{
			printf("Not enough memory!\n");
			return	FTDM_RET_NOT_ENOUGH_MEMORY;		
		}

		nRet = FTDMC_getEPData(_hClient, 
					pEPIDs, nEPIDCount, 
					nBeginTime, nEndTime, 
					pEPData, nMaxCount, &nCount);
		if (nRet == FTDM_RET_OK)
		{
			FTDM_INT	i;

			for(i = 0 ; i < nCount ; i++)
			{
				printf("%08lx, %d, %d\n", pEPData[i].xEPID, pEPData[i].nTime, pEPData[i].nValue);	
			}
			printf("EndPoint data load completed!\n");	
		}
		else
		{
			printf("EndPoint data loading failed [ ERROR = %08lx ]\n", nRet);	
		}

		free(pEPData);
	}
	else
	{
		bShowUsage = FTDM_BOOL_TRUE;	
	}

	if (bShowUsage)
	{
		printf("Usage : %s <COMMAND> ...\n", pArgv[0]);
		printf("EndPoint data management.\n");
		printf("COMMANDS:\n");
		printf("  add <EPID> <Time> <Value>\n");
		printf("  del [-c MaxCount] [-b BeginTime] [-e EndTime] [EPID ...]\n");
		printf("  count [-b BeginTime] [-e EndTime] [EPID ...]\n");
		printf("  get [-b BeginTime] [-e EndTime] [EPID ...]\n");
	}

	return	FTDM_RET_OK;
}

FTDM_RET	_help(FTDM_INT nArgc, FTDM_CHAR_PTR pArgv[])
{

	switch(nArgc)
	{
	case	1:
		{
			FTDMC_CMD_PTR	pCmd = _cmds; 
			FTDM_CHAR_PTR	pArgv[2] = { NULL, "short_help"};

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
				printf("%s - Help\n", pArgv[0]) ;
			}
			else if (strcmp(pArgv[1], "help") == 0)
			{
				printf("%s - Help\n", pArgv[0]);
			}
			else
			{
				FTDMC_CMD_PTR	pCmd = _cmds; 
				FTDM_CHAR_PTR	pArgv[2] = { NULL, "help"};

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

	return	FTDM_RET_OK;
}

FTDM_RET	_quit(FTDM_INT nArgc, FTDM_CHAR_PTR pArgv[])
{

	FTDMC_disconnect(_hClient);

	return	FTDM_RET_ERROR;
}

