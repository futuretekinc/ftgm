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
#include "ftm_opt.h"
#include "ftom_client.h"

#define	FTOM_CLIENT_DISCOVERY_CHECK_INTERVAL	100

static
FTM_RET	FTOM_CLIENT_CMD_discoveryRun
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pIP,
	FTM_USHORT		usPort,
	FTM_ULONG		ulRetryCount,
	FTM_ULONG		ulTimeout
)
{
	ASSERT(pClient != NULL);
	FTM_RET		xRet;
	FTM_BOOL	bFinished = FTM_FALSE;
	FTM_ULONG	ulNodeCount, ulEPCount;
	FTM_TIMER	xTimeout;
	
	xRet = FTOM_CLIENT_DISCOVERY_start(pClient, pIP, usPort, ulRetryCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to start discovery!\n");	
		return	xRet;
	}

	FTM_TIMER_initMS(&xTimeout, ulTimeout);

	while(!FTM_TIMER_isExpired(&xTimeout))
	{
		FTM_ULONG	ulRemain;

		xRet = FTM_TIMER_remainMS(&xTimeout, &ulRemain);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Internal timer error!\n");
			return	xRet;	
		}

		if (ulRemain > FTOM_CLIENT_DISCOVERY_CHECK_INTERVAL)
		{
			usleep(FTOM_CLIENT_DISCOVERY_CHECK_INTERVAL * 100);

			xRet = FTOM_CLIENT_DISCOVERY_getInfo(pClient, &bFinished, &ulNodeCount, &ulEPCount);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to get discovered information.\n");

				return	xRet;
			}

			if (bFinished)
			{
				MESSAGE("Discovery finished.\n");

				return	FTM_RET_OK;		
			}
		}
	}


	return	FTM_RET_TIMEOUT;
}
static
FTM_RET	FTOM_CLIENT_CMD_discoveryList
(
	FTOM_CLIENT_PTR	pClient,
	FTM_BOOL		bUnregistered
)
{
	ASSERT(pClient != NULL);

	FTM_RET		xRet;
	FTM_BOOL	bFinished = FTM_FALSE;
	FTM_ULONG	ulNodeCount, ulEPCount;
	FTM_INT		i, nIndex;

	xRet = FTOM_CLIENT_DISCOVERY_getInfo(pClient, &bFinished, &ulNodeCount, &ulEPCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get discovered information.\n");
		return	xRet;
	}

	if (!bFinished)
	{
		ERROR2(xRet, "discovering...\n");	
		return	FTM_RET_RUNNING;
	}

	MESSAGE("\n[ NODE INFORMATION ]\n");
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

	MESSAGE("\n[ EP INFORMATION ]\n");
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

	return	FTM_RET_OK;
}

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
	ASSERT(pData != NULL);

	FTM_RET			xRet;
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;
	FTM_CHAR_PTR	pIP = "255.255.255.255";
	FTM_USHORT		usPort = 1234;
	FTM_ULONG		ulRetryCount = 3;
	FTM_ULONG		ulTimeout = 5 * 1000;
	FTM_BOOL		bUnregistered = FTM_FALSE;
	FTM_ULONG		ulOpts = 0;
	FTM_OPT			pOpts[8];
	FTM_INT			i;

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xRet = FTM_getOptions(nArgc, pArgv, "i:p:c:u", pOpts, 8, &ulOpts);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Invalid arguments!\n");
		return	xRet;
	}

	for(i = 0 ; i < ulOpts ; i++)
	{
		switch(pOpts[i].xOpt)
		{
		case	'i':
			{
				pIP = pOpts[i].pParam;
			}
			break;

		case	'p':
			{
				usPort = (FTM_USHORT)strtoul(pOpts[i].pParam, 0, 10);
			}
			break;

		case	'c':
			{
				ulRetryCount = (FTM_USHORT)strtoul(pOpts[i].pParam, 0, 10);
			}
			break;

		case	'u':
			{
				bUnregistered = FTM_TRUE;
			}
			break;
		}
	}


	if (strcasecmp(pArgv[1], "start") == 0)
	{
		xRet = FTOM_CLIENT_CMD_discoveryRun(pClient, pIP, usPort, ulRetryCount, ulTimeout);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to run object discovery.\n");
			goto finished;
		}
	}
	else if (strcasecmp(pArgv[1], "list") == 0)
	{
		xRet = FTOM_CLIENT_CMD_discoveryList(pClient, bUnregistered);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to display the list of discovered objects.\n");
			goto finished;
		}
	}


finished:
	return	xRet;
}

