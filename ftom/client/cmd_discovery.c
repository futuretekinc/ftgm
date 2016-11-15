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

FTM_RET	FTOM_CLIENT_CMD_discovery
(
	FTM_SHELL_PTR	pShell,
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_VOID_PTR 	pData
)
{
	ASSERT(pShell != NULL);
	ASSERT(pArgv != NULL);

	FTM_RET			xRet;
	FTM_CHAR_PTR	pIP = "255.255.255.255";
	FTM_USHORT		usPort = 1234;
	FTM_ULONG		ulRetryCount = 3;
	FTM_BOOL		bUnregistered = FTM_FALSE;
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;
	FTM_INT			nOpt = 0;

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	nOpt = 2;
	while(nOpt < nArgc)
	{
		if (pArgv[nOpt][0] == '-')
		{
			switch(pArgv[nOpt][1])
			{
			case	'i':
				{
					if (++nOpt >= nArgc)
					{
						return	FTM_RET_INVALID_ARGUMENTS;	
					}

					pIP = pArgv[nOpt];
				}
				break;

			case	'p':
				{
					if (++nOpt >= nArgc)
					{
						return	FTM_RET_INVALID_ARGUMENTS;	
					}

					usPort = (FTM_USHORT)strtoul(pArgv[nOpt], 0, 10);
				}
				break;

			case	'c':
				{
					if (++nOpt >= nArgc)
					{
						return	FTM_RET_INVALID_ARGUMENTS;	
					}

					ulRetryCount = (FTM_USHORT)strtoul(pArgv[nOpt], 0, 10);
				}
				break;

			case	'u':
				{
					bUnregistered = FTM_TRUE;
				}
				break;
			}
		}

		nOpt++;
	}


	if (strcasecmp(pArgv[1], "start") == 0)
	{
		FTM_BOOL	bFinished = FTM_FALSE;
		FTM_ULONG	ulNodeCount, ulEPCount;

		xRet = FTOM_CLIENT_DISCOVERY_start(pClient, pIP, usPort, ulRetryCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to start discovery!\n");	
			goto finished;
		}

		while(!bFinished)
		{
			usleep(1000000);

			xRet = FTOM_CLIENT_DISCOVERY_getInfo(pClient, &bFinished, &ulNodeCount, &ulEPCount);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to get discovered information.\n");
				goto finished;
			}
		}

		MESSAGE("Discovery finished.\n");

	}
	else if (strcasecmp(pArgv[1], "list") == 0)
	{
		FTM_BOOL	bFinished = FTM_FALSE;
		FTM_ULONG	ulNodeCount, ulEPCount;
		FTM_INT		i, nIndex;

		xRet = FTOM_CLIENT_DISCOVERY_getInfo(pClient, &bFinished, &ulNodeCount, &ulEPCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to get discovered information.\n");
			goto finished;
		}

		if (!bFinished)
		{
			MESSAGE("discovering...\n");	
			goto finished;
		}

		nIndex = 0;
		for(i = 0 ; i < ulNodeCount ; i++)
		{
			FTM_NODE	xInfo;
			FTM_ULONG	ulCount;

			xRet = FTOM_CLIENT_DISCOVERY_getNodeList(pClient, i, 1, &xInfo, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				if (bUnregistered)
				{
					FTM_NODE	xInfo2;

					xRet = FTOM_CLIENT_NODE_get(pClient, xInfo.pDID, &xInfo2);
					if (xRet == FTM_RET_OBJECT_NOT_FOUND)
					{
						MESSAGE("%2d : %16s %16s\n", ++nIndex, xInfo.pDID, xInfo.pName);	
					}
				}
				else
				{
					MESSAGE("%2d : %16s %16s\n", ++nIndex, xInfo.pDID, xInfo.pName);	
				}
			}
		}

		nIndex = 0;
		for(i = 0 ; i < ulEPCount ; i++)
		{
			FTM_EP		xInfo;
			FTM_ULONG	ulCount;

			xRet = FTOM_CLIENT_DISCOVERY_getEPList(pClient, i, 1, &xInfo, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				if (bUnregistered)
				{
					FTM_EP	xInfo2;

					xRet = FTOM_CLIENT_EP_get(pClient, xInfo.pEPID, &xInfo2);
					if (xRet == FTM_RET_OBJECT_NOT_FOUND)
					{
						MESSAGE("%2d : %16s %16s %16s\n", ++nIndex, xInfo.pEPID, xInfo.pName, FTM_EP_typeString(xInfo.xType));	
					}
				}
				else
				{
					MESSAGE("%2d : %16s %16s %16s\n", ++nIndex, xInfo.pEPID, xInfo.pName, FTM_EP_typeString(xInfo.xType));	
				}
			}
		}


	}


finished:
	return	xRet;
}

