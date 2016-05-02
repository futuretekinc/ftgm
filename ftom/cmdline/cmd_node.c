#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include "ftm.h"
#include "ftom.h"
#include "ftom_params.h"
#include "ftom_client_cmdline.h"
#include "ftom_cl_cmd.h"


FTM_RET	FTOM_CL_NODE_create
(
	FTOM_CLIENT_PTR	pClient,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[]
)
{
	FTM_RET		xRet;
	FTM_NODE	xInfo;
	FTM_INT		nOpt;
	FTM_CHAR	pDID[FTM_ID_LEN+1];

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	FTM_NODE_setDefault(&xInfo);

	if (strlen(pArgv[0]) > FTM_DID_LEN)
	{
		return	FTM_RET_INVALID_DID;
	}

	strcpy(xInfo.pDID, pArgv[0]);

	if (strcasecmp(pArgv[1], "snmp") == 0)
	{
		xInfo.xType = FTM_NODE_TYPE_SNMP;	

		MESSAGE("Create snmp node.\n");
		while((nOpt = getopt(nArgc, pArgv, "Sl:i:t:v:c:?")) != -1)
		{
   			switch(nOpt)
    		{
	 		case    'd':
	  			{
				   	xInfo.xFlags = FTM_NODE_FLAG_DYNAMIC;	
				}
				break;

			case	'l':
	  			{
					if (strlen(optarg) > FTM_LOCATION_LEN) 
					{
						return	FTM_RET_INVALID_LOCATION;
					}

					strcpy(xInfo.pLocation, optarg);
				}
				break;

			case	'i':
				{
					FTM_ULONG	ulInterval = strtoul(optarg, 0, 10);
					if (ulInterval > FTOM_MAX_NODE_INTERVAL)
					{
						return	FTM_RET_INVALID_INTERVAL;	
					}

					xInfo.ulInterval = ulInterval;
				}
				break;

			case	't':
				{
					FTM_ULONG	ulTimeout = strtoul(optarg, 0, 10);
					if (ulTimeout > FTOM_MAX_NODE_TIMEOUT)
					{
						return	FTM_RET_INVALID_TIMEOUT;	
					}

					xInfo.ulTimeout = ulTimeout;
				}
				break;

			case	'v':
				{
					FTM_ULONG	ulVersion = strtoul(optarg, 0, 10);
					if (ulVersion < 1 || 3 < ulVersion)
					{
						return	FTM_RET_INVALID_TIMEOUT;	
					}

					xInfo.xOption.xSNMP.ulVersion = ulVersion;
				}
			case	'c':
				{
					if (strlen(optarg) > FTM_SNMP_COMMUNITY_LEN)
					{
						return	FTM_RET_INVALID_COMMUNITY;	
					}

					strcpy(xInfo.xOption.xSNMP.pCommunity, optarg);
				}
				break;
			}
		}
	}
	else
	{
		return	FTM_RET_INVALID_TYPE;	
	}

	xRet = FTOM_CLIENT_NODE_create(pClient, &xInfo, pDID, FTM_ID_LEN);

	return	xRet;
}

FTM_RET	FTOM_CL_NODE_destroy
(
	FTOM_CLIENT_PTR	pClient,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[]
)
{
	ASSERT(pClient != NULL);
	ASSERT(pArgv != NULL);

	FTM_RET		xRet;

	if (nArgc != 1)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xRet = FTOM_CLIENT_NODE_destroy(pClient, pArgv[0]);

	return	xRet;
}

FTM_RET	FTOM_CL_NODE_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[]
)
{
	ASSERT(pClient != NULL);
	ASSERT(pArgv != NULL);

	FTM_RET		xRet;
	FTM_ULONG	ulCount;

	xRet = FTOM_CLIENT_NODE_count(pClient, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		MESSAGE("%d\n",	ulCount);
	}
	return	xRet;	
}

FTM_RET	FTOM_CL_NODE_get
(
	FTOM_CLIENT_PTR pClient, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[]
)
{
	ASSERT(pClient != NULL);
	ASSERT(pArgv != NULL);

	FTM_RET		xRet;
	FTM_NODE	xInfo;

	if (nArgc < 1)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xRet = FTOM_CLIENT_NODE_get(pClient, pArgv[0], &xInfo);
	if (xRet == FTM_RET_OK)
	{
		MESSAGE("%s|%s|%08x|%s|%d|%d|%d|%s|%s\n", 
			xInfo.pDID, 
			FTM_NODE_typeString(xInfo.xType),
			xInfo.xFlags,
			xInfo.pLocation,
			xInfo.ulInterval,
			xInfo.ulTimeout,
			xInfo.xOption.xSNMP.ulVersion,
			xInfo.xOption.xSNMP.pURL,
			xInfo.xOption.xSNMP.pCommunity);
	}

	return	xRet;
}

FTM_RET	FTOM_CL_NODE_getAt
(
	FTOM_CLIENT_PTR	pClient,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[]
)
{
	ASSERT(pClient != NULL);
	ASSERT(pArgv != NULL);

	FTM_RET		xRet;
	FTM_ULONG	ulIndex;
	FTM_NODE	xInfo;

	if (nArgc < 1)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	ulIndex = atoi(pArgv[0]);
	xRet = FTOM_CLIENT_NODE_getAt(pClient, ulIndex, &xInfo);
	if (xRet == FTM_RET_OK)
	{
		MESSAGE("%s|%s|%08x|%s|%d|%d|%d|%s|%s\n", 
			xInfo.pDID, 
			FTM_NODE_typeString(xInfo.xType),
			xInfo.xFlags,
			xInfo.pLocation,
			xInfo.ulInterval,
			xInfo.ulTimeout,
			xInfo.xOption.xSNMP.ulVersion,
			xInfo.xOption.xSNMP.pURL,
			xInfo.xOption.xSNMP.pCommunity);
	}

	return	xRet;
}

