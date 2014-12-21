#include <stdio.h>
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
static FTDM_RET	_quit(FTDM_INT nArgc, FTDM_CHAR_PTR pArgs[]);

typedef struct	_FTDMC_CMD
{
	FTDM_CHAR_PTR	pString;
	FTDM_RET		(*function)(FTDM_INT nArgc, FTDM_CHAR_PTR pArgs[]);
}	FTDMC_CMD, _PTR_ FTDMC_CMD_PTR;

FTDM_CHAR_PTR _strPrompt = "FTDMC> ";
FTDMC_CMD	_cmds[] = 
{
	{	"connect",		_connect },
	{	"disconnect",	_disconnect},
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
					pCmd->function(nArgc, pArgs);
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
	return	FTDM_RET_OK;
}

FTDM_RET	_disconnect(FTDM_INT nArgc, FTDM_CHAR_PTR pArgs[])
{
	return	FTDM_RET_OK;
}
#if 0
	FTDM_CLIENT	xClient;
	FTDM_BYTE	pSendMessage[1000], pRecvBuff[2000];

	
	if (FTDMC_connect(inet_addr("127.0.0.1"), 8888, &xClient) != FTDM_RET_OK)
	{
		perror("connect failed. Error");
		return	0;	
	}

	printf("Connected\n");

	while(1)
	{
		FTDM_INT	nBuffLen = 0;
		printf("%s", _strPrompt);
		fgets((char *)pSendMessage, sizeof(pSendMessage), stdin);

		if (pSendMessage[0] == 'q')
		{
			break;	
		}

		nBuffLen = sizeof(pRecvBuff);
	}

	close(xClient.hSock);

	return 0;
}
#endif
