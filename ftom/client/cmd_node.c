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
#include "ftm_opt.h"
#include "ftom_client.h"

FTM_RET	FTOM_CLIENT_CMD_printNodeList
(
	FTOM_CLIENT_PTR	pClient,
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_VOID_PTR 	pData
)
{
	ASSERT(pClient != NULL);
	
	FTM_RET		xRet = FTM_RET_OK;
	FTM_INT		i;
	FTM_ULONG	ulNodeCount;

	xRet = FTOM_CLIENT_NODE_count(pClient, &ulNodeCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get node count.\n");
	}

	MESSAGE("\n[ NODE INFORMATION ]\n");
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

	return	xRet;
}

FTM_RET	FTOM_CLIENT_CMD_addNode
(
	FTOM_CLIENT_PTR	pClient,
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_VOID_PTR 	pData
)
{
	ASSERT(pClient != NULL);
	
	FTM_RET		xRet;
	FTM_NODE	xInfo;
	FTM_CHAR	pDID[FTM_DID_LEN + 1];
	FTM_ULONG	ulOpts = 0;
	FTM_OPT		pOpts[8];
	FTM_INT		i;

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	FTM_NODE_setDefault(&xInfo);

	if (strlen(pArgv[1]) > FTM_ID_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	strncpy(xInfo.pDID, pArgv[1], FTM_ID_LEN);

	xRet = FTM_getOptions(nArgc, pArgv, "i:t:c:u:l:", pOpts, 8, &ulOpts);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Invalid arguments!\n");
		return	xRet;
	}

	for(i = 0 ; i < ulOpts ; i++)
	{
		switch(pOpts[i].xOpt)
		{
		case	't':
			{
				if (strcasecmp(pOpts[i].pParam, "SNMP") == 0)
				{
					xInfo.xType = FTM_NODE_TYPE_SNMP;	
				}
				else if (strcasecmp(pOpts[i].pParam, "MBTCP") == 0)
				{
					xInfo.xType = FTM_NODE_TYPE_MODBUS_OVER_TCP;	
				}
				else if (strcasecmp(pOpts[i].pParam, "MB") == 0)
				{
					xInfo.xType = FTM_NODE_TYPE_MODBUS_OVER_SERIAL;	
				}
				else
				{
					return	FTM_RET_INVALID_ARGUMENTS;	
				}
			}
			break;

		case	'c':
			{
				if (xInfo.xType == FTM_NODE_TYPE_SNMP)
				{
					strncpy(xInfo.xOption.xSNMP.pCommunity, pOpts[i].pParam, FTM_SNMP_COMMUNITY_LEN);
				}
				else
				{
					return	FTM_RET_INVALID_ARGUMENTS;
				}
			}
			break;

		case	'u':
			{
				if (xInfo.xType == FTM_NODE_TYPE_SNMP)
				{
					strncpy(xInfo.xOption.xSNMP.pURL, pOpts[i].pParam, FTM_URL_LEN);
				}
				else
				{
					return	FTM_RET_INVALID_ARGUMENTS;	
				}
			}
			break;

		case	'l':
			{
				strncpy(xInfo.pLocation, pOpts[i].pParam, FTM_LOCATION_LEN);
			}
			break;
		}
	}

	xRet = FTOM_CLIENT_NODE_create(pClient, &xInfo, pDID, FTM_ID_LEN);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Failed to create NODE[%s]\n", xInfo.pDID);
	}
	else
	{
		MESSAGE("The node[%s] has been created successfully.\n", xInfo.pDID);
	}

	return	xRet;
}

FTM_RET	FTOM_CLIENT_CMD_delNode
(
	FTOM_CLIENT_PTR	pClient,
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_VOID_PTR 	pData
)
{
	ASSERT(pClient != NULL);
	
	FTM_RET		xRet;
	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	xRet = FTOM_CLIENT_NODE_destroy(pClient, pArgv[1]);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Failed to destroy node[%s]\n", pArgv[1]);
	}
	else
	{
		MESSAGE("The node[%s] has been destroied successfully.\n", pArgv[1]);
	}

	return	xRet;
}

FTM_RET	FTOM_CLIENT_CMD_printNodeInfo
(
	FTOM_CLIENT_PTR	pClient,
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_VOID_PTR 	pData
)
{
	ASSERT(pClient != NULL);
	
	FTM_RET		xRet;
	FTM_INT		i;
	FTM_NODE	xInfo;

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	for(i = 1 ; i < nArgc ; i++)
	{
		xRet = FTOM_CLIENT_NODE_get(pClient, pArgv[i], &xInfo);
		if (xRet != FTM_RET_OK)
		{
			MESSAGE("Node[%s] not found.\n", pArgv[i]);
			return	xRet;
		}

		FTM_BOOL	bRun;

		MESSAGE("\n[ NODE INFORMATION ]\n");
		FTM_NODE_print(&xInfo);

		FTOM_CLIENT_NODE_isRun(pClient, pArgv[i], &bRun);
		MESSAGE("%16s : %s\n", "Status", (bRun)?"Run":"Stop");
	}

	return	xRet;
}

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
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	if (strcasecmp(pArgv[1], "list") == 0)
	{
		xRet = FTOM_CLIENT_CMD_printNodeList(pClient, nArgc - 1, &pArgv[1], pData);
	}
	else if (strcasecmp(pArgv[1], "add") == 0)
	{
		xRet = FTOM_CLIENT_CMD_addNode(pClient, nArgc - 1, &pArgv[1], pData);
	}
	else if (strcasecmp(pArgv[1], "del") == 0)
	{
		xRet = FTOM_CLIENT_CMD_delNode(pClient, nArgc - 1, &pArgv[1], pData);
	}
	else if (strcasecmp(pArgv[1], "info") == 0)
	{
		xRet = FTOM_CLIENT_CMD_printNodeInfo(pClient, nArgc - 1, &pArgv[1], pData);
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	return	xRet;
}

