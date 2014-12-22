#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>    
#include <string.h>    
#include <unistd.h>    
#include <sys/socket.h> 
#include <arpa/inet.h>
#include "ftdm_client.h"
#include "debug.h"

#define		FTDMC_MAX_LINE	2048
#define		FTDMC_MAX_ARGS	16

static FTDM_RET	_parseLine(FTDM_CHAR_PTR pLine, FTDM_CHAR_PTR pArgs[], FTDM_INT nMaxArgs, FTDM_INT_PTR pArgc);
static FTDM_RET	_connect(FTDM_INT nArgc, FTDM_CHAR_PTR pArgs[]);
static FTDM_RET	_disconnect(FTDM_INT nArgc, FTDM_CHAR_PTR pArgs[]);
static FTDM_RET	_device(FTDM_INT nArgc, FTDM_CHAR_PTR pArgs[]);
static FTDM_RET	_ep(FTDM_INT nArgc, FTDM_CHAR_PTR pArgs[]);
static FTDM_RET	_help(FTDM_INT nArgc, FTDM_CHAR_PTR pArgs[]);
static FTDM_RET	_quit(FTDM_INT nArgc, FTDM_CHAR_PTR pArgs[]);

typedef struct	_FTDMC_CMD
{
	FTDM_CHAR_PTR	pString;
	FTDM_RET		(*function)(FTDM_INT nArgc, FTDM_CHAR_PTR pArgs[]);
}	FTDMC_CMD, _PTR_ FTDMC_CMD_PTR;

FTDM_CLIENT_HANDLE	_hClient = 0;
FTDM_CHAR_PTR 		_strPrompt = "FTDMC> ";
FTDMC_CMD			_cmds[] = 
{
	{	"connect",		_connect },
	{	"disconnect",	_disconnect},
	{	"device",		_device},
	{	"ep",			_ep},
	{	"help",			_help},
	{	"?",			_help},
	{	"quit",			_quit},
	{	NULL,}
};

int main(int argc , char *argv[])
{
	FTDM_INT		nArgc;
	FTDM_CHAR_PTR	pArgs[FTDMC_MAX_ARGS];
	FTDM_CHAR		pCmdLine[FTDMC_MAX_LINE];

	while(1)
	{
		printf("%s", _strPrompt);
		fgets(pCmdLine, sizeof(pCmdLine), stdin);

		_parseLine(pCmdLine, pArgs, FTDMC_MAX_ARGS, &nArgc);

		if (nArgc != 0)
		{
			FTDMC_CMD_PTR pCmd = _cmds;
			while(pCmd->pString != NULL)
			{
				if (strcmp(pCmd->pString, pArgs[0]) == 0)
				{
					if (pCmd->function(nArgc, pArgs) != FTDM_RET_OK)
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

FTDM_RET	_parseLine(FTDM_CHAR_PTR pLine, FTDM_CHAR_PTR pArgs[], FTDM_INT nMaxArgs, FTDM_INT_PTR pArgc)
{
	FTDM_INT		nCount = 0;
	FTDM_CHAR_PTR	pWord;
	FTDM_CHAR_PTR	pSeperator = "\t \n\r";	
	pWord = strtok(pLine, pSeperator); 
	while((pWord != NULL) && (nCount < nMaxArgs))
	{
		pArgs[nCount++] = pWord;
		pWord = strtok(NULL, pSeperator);
	}

	*pArgc = nCount;

	return	FTDM_RET_OK;
}

FTDM_RET	_connect(FTDM_INT nArgc, FTDM_CHAR_PTR pArgs[])
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
			xAddr = inet_addr(pArgs[1]);
			nPort = 8888;
		}
		break;

	case	3:
		{
			xAddr = inet_addr(pArgs[1]);
			nPort = atoi(pArgs[2]);
		}
		break;
	}

	if (xAddr == 0)
	{
		printf("Usage : %s [ip address] [port]\n", pArgs[0]);
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

FTDM_RET	_disconnect(FTDM_INT nArgc, FTDM_CHAR_PTR pArgs[])
{
	FTDMC_disconnect(_hClient);

	return	FTDM_RET_OK;
}

FTDM_RET	_device(FTDM_INT nArgc, FTDM_CHAR_PTR pArgs[])
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
		memset(pLocation, 0, sizeof(pLocation));
		for(i = 0 ; i < strlen(pArgs[5]) ; i++)
		{
			pLocation[i] = toupper(pArgs[5][i]);	
		}

	case	5:
		memset(pURL, 0, sizeof(pURL));
		for(i = 0 ; i < strlen(pArgs[4]) ; i++)
		{
			pURL[i] = toupper(pArgs[4][i]);	
		}

	case	4:
		memset(pDID, 0, sizeof(pDID));
		for(i = 0 ; i < strlen(pArgs[2]) ; i++)
		{
			pDID[i] = toupper(pArgs[2][i]);	
		}
	
		xType = atoi(pArgs[3]);

		if (strcasecmp(pArgs[1], "add") == 0)
		{
			nRet = FTDMC_createDevice(_hClient, pDID, xType, pURL, pLocation);
			if (nRet != FTDM_RET_OK)
			{
				printf("%s : ERROR - %lx\n", pArgs[0], nRet);
			}
			else
			{
				printf("%s : The device[%s] has been created successfully.\n", 
					pArgs[0], pDID);
			}
		}
		break;

	case	3:
		{
			FTDM_INT	i;

			if (strlen(pArgs[2]) > FTDM_DEVICE_ID_LEN)
			{
				printf("%s : ERROR - Invalid argument\n", pArgs[0]);	
				bShowUsage = FTDM_BOOL_TRUE;
				break;
			}

			memset(pDID, 0, sizeof(pDID));
			for(i = 0 ; i < strlen(pArgs[2]) ; i++)
			{
				pDID[i] = toupper(pArgs[2][i]);	
			}

			if (strcasecmp(pArgs[1], "del") == 0)
			{
				nRet = FTDMC_destroyDevice(_hClient, pDID);
				if (nRet != FTDM_RET_OK)
				{
					printf("%s : ERROR - %lu\n", pArgs[0], nRet);
				}
				else
				{
					printf("%s : The device[%s] has been destroied successfully.\n", 
						pArgs[0], pDID);
				}
			}
			else if (strcasecmp(pArgs[1], "info") == 0)
			{
				FTDM_DEVICE_INFO	xInfo;

				nRet = FTDMC_getDeviceInfo(_hClient, pDID, &xInfo);
				if (nRet != FTDM_RET_OK)
				{
					printf("%s : ERROR - %lu\n", pArgs[0], nRet);
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
			if (strcasecmp(pArgs[1], "list") == 0)
			{
				FTDM_ULONG	i, nDeviceCount = 0;

				nRet = FTDMC_getDeviceCount(_hClient, &nDeviceCount);
				if (nRet != FTDM_RET_OK)
				{
					printf("%s : ERROR - %lu\n", pArgs[0], nRet);
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
		printf("Usage : %s <cmd> <DID> ...\n", pArgs[0]);
		printf("    cmd - add, del, info\n");
		printf("    DID - string for Device ID [Max %d] \n", FTDM_DEVICE_ID_LEN);
	}

	return	FTDM_RET_OK;
}

FTDM_RET	_ep(FTDM_INT nArgc, FTDM_CHAR_PTR pArgs[])
{
	return	FTDM_RET_OK;
}

FTDM_RET	_help(FTDM_INT nArgc, FTDM_CHAR_PTR pArgs[])
{

	switch(nArgc)
	{
	case	1:
		{
			FTDMC_CMD_PTR	pCmd = _cmds; 
			FTDM_CHAR_PTR	pArgs[2] = { NULL, "short_help"};

			while(pCmd->pString != NULL)
			{
				pArgs[0] = pCmd->pString;
				pCmd->function(2, pArgs);
				pCmd++;	
			}
		}
		break;

	case	2:
		{
			if (strcmp(pArgs[1], "short_help") == 0)
			{
				printf("%s - Help\n", pArgs[0]) ;
			}
			else if (strcmp(pArgs[1], "help") == 0)
			{
				printf("%s - Help\n", pArgs[0]);
			}
			else
			{
				FTDMC_CMD_PTR	pCmd = _cmds; 
				FTDM_CHAR_PTR	pArgs[2] = { NULL, "help"};

				while(pCmd->pString != NULL)
				{
					if (strcmp(pArgs[1], pCmd->pString) == 0)
					{
						pArgs[0] = pCmd->pString;
						pCmd->function(2, pArgs);
						break;
					}
					pCmd++;	
				}

			}

		}
	};

	return	FTDM_RET_OK;
}

FTDM_RET	_quit(FTDM_INT nArgc, FTDM_CHAR_PTR pArgs[])
{

	FTDMC_disconnect(_hClient);

	return	FTDM_RET_ERROR;
}

