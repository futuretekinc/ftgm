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
#include "ftm.h"
#include "ftm_snmp.h"
#include "ftom_client.h"

static
FTM_RET	FTOM_CLIENT_CMD_NODE_list
(
	FTOM_CLIENT_PTR	pClient
);

static
FTM_RET	FTOM_CLIENT_CMD_NODE_info
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pDID
);

FTM_RET	FTOM_CLIENT_CMD_NODE
(
	FTM_SHELL_PTR	pShell,
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_VOID_PTR 	pData
)
{
	ASSERT(pData != NULL);

	FTM_RET		xRet;
	FTM_INT		i;
	FTM_NODE	xNodeInfo;
	FTM_CHAR	pDID[FTM_DID_LEN + 1];
	FTM_CHAR	pURL[FTM_URL_LEN + 1];
	FTM_CHAR	pLocation[FTM_LOCATION_LEN + 1];
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;
	
	memset(pDID, 0, sizeof(pDID));
	memset(pURL, 0, sizeof(pURL));
	memset(pLocation, 0, sizeof(pLocation));

	if ((nArgc == 1) || (strcasecmp(pArgv[1], "list") == 0))
	{
		FTOM_CLIENT_CMD_NODE_list(pClient);
	}
	else if (strcasecmp(pArgv[1], "add") == 0)
	{
		if (nArgc < 4)
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}

		FTM_NODE_setDefault(&xNodeInfo);

		if (strcasecmp(pArgv[3], "SNMP") == 0)
		{
			xNodeInfo.xType = FTM_NODE_TYPE_SNMP;	
		}
		else if (strcasecmp(pArgv[3], "MBTCP") == 0)
		{
			xNodeInfo.xType = FTM_NODE_TYPE_MODBUS_OVER_TCP;	
		}
		else if (strcasecmp(pArgv[3], "MB") == 0)
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
				xRet = FTM_RET_INVALID_ARGUMENTS;
				MESSAGE("Invalid DID length[< %d]\n", FTM_DID_LEN);
				return	xRet;
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

			xRet = FTOM_CLIENT_NODE_create(pClient, &xNodeInfo, pDID, FTM_ID_LEN);
			if (xRet != FTM_RET_OK)
			{
				MESSAGE("Failed to create node[%s]\n", pArgv[2]);
			}
			else
			{
				MESSAGE("%s : The node[%s] has been created successfully.\n", pArgv[0], pDID);
			}
		}
		else
		{
			xRet = FTM_RET_INVALID_ARGUMENTS;
			MESSAGE("Invalid type or arguments [ Type = %08lx ]\n", xNodeInfo.xType);
			return	xRet;
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

		xRet = FTOM_CLIENT_NODE_destroy(pClient, pDID);
		if (xRet != FTM_RET_OK)
		{
			MESSAGE("Failed to destroy node[%s]\n", pArgv[2]);
		}
		else
		{
			MESSAGE("%s : The node[%s] has been destroied successfully.\n", pArgv[0], pDID);
		}
	}
	else if (strcasecmp(pArgv[1], "info") == 0)
	{
		FTM_INT			i;

		if ((nArgc < 3) || (strlen(pArgv[2]) > FTM_DID_LEN))
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}

		memset(pDID, 0, sizeof(pDID));
		for(i = 0 ; i < strlen(pArgv[2]) ; i++)
		{
			pDID[i] = toupper(pArgv[2][i]);	
		}

		FTOM_CLIENT_CMD_NODE_info(pClient, pDID);
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_CMD_NODE_info
(
	FTOM_CLIENT_PTR	pClient,
	FTM_CHAR_PTR	pDID
)
{
	ASSERT(pClient != NULL);
	ASSERT(pDID != NULL);

	FTM_RET		xRet;
	FTM_NODE	xInfo;

	xRet = FTOM_CLIENT_NODE_get(pClient, pDID, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Node[%s] not found.\n", pDID);
	}
	else
	{
		FTM_BOOL	bRun;

		MESSAGE("Node Information\n");
		FTM_NODE_print(&xInfo);

		FTOM_CLIENT_NODE_isRun(pClient, pDID, &bRun);
		MESSAGE("%16s : %s\n", "Status", (bRun)?"Run":"Stop");
	}

	return	xRet;
}

FTM_RET	FTOM_CLIENT_CMD_NODE_list
(
	FTOM_CLIENT_PTR	pClient
)
{
	FTM_RET		xRet;
	FTM_INT		i; 
	FTM_ULONG	ulNodeCount = 0;

	xRet = FTOM_CLIENT_NODE_count(pClient, &ulNodeCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get node count.\n");
	}

	MESSAGE("\n# Node Information\n");
	MESSAGE("%16s %16s %16s %16s %16s %8s %8s %8s\n", "DID", "MODEL", "NAME", "TYPE", "STATE", "REPORT", "TIMEOUT", "EPs");

	for(i = 0 ; i < ulNodeCount; i++)
	{
		FTM_NODE	xInfo;

		xRet = FTOM_CLIENT_NODE_getAt(pClient, i, &xInfo);
		if (xRet == FTM_RET_OK)
		{
			FTM_BOOL	bRun;
			FTM_ULONG	ulCount = 0;

			MESSAGE("%16s %16s %16s %16s", xInfo.pDID, xInfo.pModel, xInfo.pName, FTM_NODE_typeString(xInfo.xType)); 
				
			xRet = FTOM_CLIENT_NODE_isRun(pClient, xInfo.pDID, &bRun);
			if (xRet == FTM_RET_OK)
			{
				MESSAGE(" %16s", (bRun)?"run":"stop");
			}
			else
			{
				MESSAGE(" %16s", "unknown");
			}

			MESSAGE(" %8lu %8lu", xInfo.ulReportInterval, xInfo.ulTimeout);
			
			xRet = FTOM_CLIENT_EP_count(pClient, 0, xInfo.pDID, &ulCount);
			if (xRet == FTM_RET_OK)
			{
				MESSAGE(" %8lu\n", ulCount);
			}
			else
			{
				MESSAGE(" %8d\n", 0);
			}
		}
	}

	return	FTM_RET_OK;
}
