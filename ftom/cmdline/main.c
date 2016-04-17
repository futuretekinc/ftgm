#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include "ftm.h"
#include "ftom.h"
#include "ftom_params.h"
#include "ftom_client_cmdline.h"

#define	FTOM_CL_NODE	1
#define	FTOM_CL_EP		2

typedef	FTM_RET	(*FTOM_CL_CMD_FUNCTION)
(
	FTOM_CLIENT_PTR	pClient,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
);

typedef	struct
{
	FTM_CHAR_PTR			pName;
	FTOM_CL_CMD_FUNCTION	fService;
}	FTOM_CL_CMD, _PTR_ FTOM_CL_CMD_PTR;

extern	char *		program_invocation_short_name;
FTM_CHAR	pReqBuff[FTOM_DEFAULT_PACKET_SIZE];
FTM_CHAR	pRespBuff[FTOM_DEFAULT_PACKET_SIZE];

static FTM_RET	FTOM_CL_NODE_count(FTOM_CLIENT_PTR pClient, FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[]);
static FTM_RET	FTOM_CL_NODE_get(FTOM_CLIENT_PTR pClient, FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[]);
static FTM_RET	FTOM_CL_NODE_getAt(FTOM_CLIENT_PTR	pClient, FTM_INT nArgc, FTM_CHAR_PTR	pArgv[]);
static FTM_RET	FTOM_CL_EP_count(FTOM_CLIENT_PTR pClient, FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[]);
static FTM_RET	FTOM_CL_EP_get(FTOM_CLIENT_PTR pClient, FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[]);
static FTM_RET	FTOM_CL_EP_getAt(FTOM_CLIENT_PTR	pClient, FTM_INT nArgc, FTM_CHAR_PTR	pArgv[]);


FTOM_CL_CMD	pNodeCmds[] =
{
	{ .pName	=	"count",	.fService	=	FTOM_CL_NODE_count },
	{ .pName	=	"get",		.fService	=	FTOM_CL_NODE_get },	
	{ .pName	=	"getat",	.fService	=	FTOM_CL_NODE_getAt },	
};

FTOM_CL_CMD	pEPCmds[] =
{
	{ .pName	=	"count",	.fService	=	FTOM_CL_EP_count },
	{ .pName	=	"get",		.fService	=	FTOM_CL_EP_get },	
	{ .pName	=	"getat",	.fService	=	FTOM_CL_EP_getAt },	
};

int main(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	FTM_RET	xRet;
	FTOM_CLIENT_PTR	pClient;
	FTM_INT			i, j;

	if (nArgc < 3)
	{
		MESSAGE("Invalid arguments.\n");
		return	0;	
	}

	FTM_MEM_init();

	xRet = FTOM_CLIENT_CL_create((FTOM_CLIENT_CL_PTR _PTR_)&pClient);
	if (xRet != FTM_RET_OK)
	{
		goto error;
	}

	for(j = 0 ; j < 100 ; j++)
	{
	if (strcasecmp(pArgv[1], "node") == 0)
	{
		for(i = 0 ; i < sizeof(pNodeCmds) / sizeof(FTOM_CL_CMD) ; i++)
		{
			if (strcasecmp(pArgv[2], pNodeCmds[i].pName) == 0)
			{
				xRet = pNodeCmds[i].fService(pClient, nArgc - 3, &pArgv[3]);
				break;
			}
		}
	}
	else if (strcasecmp(pArgv[1], "ep") == 0)
	{
		for(i = 0 ; i < sizeof(pEPCmds) / sizeof(FTOM_CL_CMD) ; i++)
		{
			if (strcasecmp(pArgv[2], pEPCmds[i].pName) == 0)
			{
				xRet = pEPCmds[i].fService(pClient, nArgc - 3, &pArgv[3]);
				break;
			}
		}
	}
}
	FTOM_CLIENT_CL_destroy((FTOM_CLIENT_CL_PTR _PTR_)&pClient);

error:
	FTM_MEM_final();

	return	0;
}

FTM_RET	FTOM_CL_NODE_create
(
	FTOM_CLIENT_PTR	pClient,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[]
)
{
	FTM_RET		xRet;

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

FTM_RET	FTOM_CL_EP_count
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

	xRet = FTOM_CLIENT_EP_count(pClient, 0, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		MESSAGE("%d\n",	ulCount);
	}
	return	xRet;	
}

FTM_RET	FTOM_CL_EP_get
(
	FTOM_CLIENT_PTR pClient, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[]
)
{
	ASSERT(pClient != NULL);
	ASSERT(pArgv != NULL);

	FTM_RET		xRet;
	FTM_EP_ID	xEPID;
	FTM_EP		xInfo;

	if (nArgc < 1)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xEPID = strtoul(pArgv[0], 0, 16);
	xRet = FTOM_CLIENT_EP_get(pClient, xEPID, &xInfo);
	if (xRet == FTM_RET_OK)
	{
		MESSAGE("%08x|%s|%08x|%s|%s|%d|%d|%d|%s|%08x|%s|%08x\n", 
			xInfo.xEPID, 
			FTM_EP_typeString(xInfo.xType), 
			xInfo.xFlags, 
			xInfo.pName, 
			xInfo.pUnit,
			xInfo.bEnable,
			xInfo.ulTimeout,
			xInfo.ulInterval,
			xInfo.pDID,
			xInfo.xDEPID,
			xInfo.pPID,
			xInfo.xPEPID);
	}

	return	xRet;
}

FTM_RET	FTOM_CL_EP_getAt
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
	FTM_EP	xInfo;

	if (nArgc < 1)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	ulIndex = atoi(pArgv[0]);
	xRet = FTOM_CLIENT_EP_getAt(pClient, ulIndex, &xInfo);
	if (xRet == FTM_RET_OK)
	{
		MESSAGE("%08x|%s|%08x|%s|%s|%d:%d|%d|%s|%08x|%s|%08x\n", 
			xInfo.xEPID, 
			FTM_EP_typeString(xInfo.xType), 
			xInfo.xFlags, 
			xInfo.pName, 
			xInfo.pUnit,
			xInfo.bEnable,
			xInfo.ulTimeout,
			xInfo.ulInterval,
			xInfo.pDID,
			xInfo.xDEPID,
			xInfo.pPID,
			xInfo.xPEPID);
	}

	return	xRet;
}
