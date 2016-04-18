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

extern	char *		program_invocation_short_name;
FTM_CHAR	pReqBuff[FTOM_DEFAULT_PACKET_SIZE];
FTM_CHAR	pRespBuff[FTOM_DEFAULT_PACKET_SIZE];

FTOM_CL_CMD pCGICmds[] = 
{
	{ .pName	= 	"get_sensor_list",	.fService = FTOM_CL_CGI_getSensorList},	
	{ .pName	= 	"get_sensing_list",	.fService = FTOM_CL_CGI_getSensingList},	
};

FTOM_CL_CMD	pNodeCmds[] =
{
	{ .pName	=	"create",	.fService	=	FTOM_CL_NODE_create},
	{ .pName	=	"destroy",	.fService	=	FTOM_CL_NODE_destroy },
	{ .pName	=	"count",	.fService	=	FTOM_CL_NODE_count },
	{ .pName	=	"get",		.fService	=	FTOM_CL_NODE_get },	
	{ .pName	=	"getat",	.fService	=	FTOM_CL_NODE_getAt },	
};

FTOM_CL_CMD	pEPCmds[] =
{
	{ .pName	=	"destroy",	.fService	=	FTOM_CL_EP_destroy },
	{ .pName	=	"count",	.fService	=	FTOM_CL_EP_count },
	{ .pName	=	"get",		.fService	=	FTOM_CL_EP_get },	
	{ .pName	=	"getat",	.fService	=	FTOM_CL_EP_getAt },	
};

FTOM_CL_CMD	pEPDataCmds[] =
{
	{ .pName	=	"get",		.fService	=	FTOM_CL_EP_DATA_getLast },	
	{ .pName	=	"list",		.fService	=	FTOM_CL_EP_DATA_getList},	
};

int main(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	FTM_RET	xRet;
	FTOM_CLIENT_PTR	pClient;
	FTM_INT			i;

	if (nArgc < 3)
	{
		MESSAGE("Invalid arguments.\n");
		return	0;	
	}

	FTM_MEM_init();

	xRet = FTOM_CLIENT_CL_create((FTOM_CLIENT_CL_PTR _PTR_)&pClient);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("ERROR : CL clinet creation failed.\n");
		goto error;
	}

	xRet = FTM_RET_FUNCTION_NOT_SUPPORTED;

	if (strcasecmp(pArgv[1], "cgi") == 0)
	{
		for(i = 0 ; i < sizeof(pCGICmds) / sizeof(FTOM_CL_CMD) ; i++)
		{
			if (strcasecmp(pArgv[2], pCGICmds[i].pName) == 0)
			{
				xRet = pCGICmds[i].fService(pClient, nArgc - 3, &pArgv[3]);
				break;
			}
		}
	}
	else if (strcasecmp(pArgv[1], "node") == 0)
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
	else if (strcasecmp(pArgv[1], "data") == 0)
	{
		for(i = 0 ; i < sizeof(pEPDataCmds) / sizeof(FTOM_CL_CMD) ; i++)
		{
			if (strcasecmp(pArgv[2], pEPDataCmds[i].pName) == 0)
			{
				xRet = pEPDataCmds[i].fService(pClient, nArgc - 3, &pArgv[3]);
				break;
			}
		}
	}

	FTOM_CLIENT_CL_destroy((FTOM_CLIENT_CL_PTR _PTR_)&pClient);

	if (xRet != FTM_RET_OK)
	{
		MESSAGE("ERROR : %08x\n", xRet);
	}
error:
	FTM_MEM_final();

	return	0;
}

