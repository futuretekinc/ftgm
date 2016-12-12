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
#include "ftom_client_net.h"

#undef	__MODULE__
#define	__MODULE__ FTOM_TRACE_MODULE_CLIENT

extern FTM_SHELL_CMD	_cmds[];
extern FTM_ULONG		ulCmds;

extern	char *		program_invocation_short_name;

static 
FTM_RET	FTOM_CLIENT_subscriber
(
	FTOM_MSG_PTR	pMsg,
	FTM_VOID_PTR	pData
);

static
FTM_VOID	FTOM_CLIENT_usage
(
	FTM_VOID
);

FTM_INT main(int nArgc , char *pArgv[])
{
	FTM_CHAR		pConfigFileName[FTM_FILE_NAME_LEN];
	FTM_INT			nOpt;
	FTOM_CLIENT_PTR	pClient;
	FTM_SHELL		xShell;
	FTM_RET			xRet;
	FTM_BOOL		bShowUsage = FTM_FALSE;
	FTM_ULONG		ulDebugLevel = 0;
	FTM_CHAR		pPrompt[32];
	FTM_CONFIG_PTR  pConfig;

	FTM_MEM_init();

	sprintf(pPrompt, "%s > ", program_invocation_short_name);

	sprintf(pConfigFileName, "/etc/%s.conf", program_invocation_short_name);
	while((nOpt = getopt(nArgc, pArgv, "cv:h?")) != -1)
	{
		switch(nOpt)
		{
		case	'c':
			{
				strncpy(pConfigFileName, optarg, sizeof(pConfigFileName));
			}
			break;
		
		case	'v':
			{
				xRet = FTM_TRACE_strToLevel(optarg, &ulDebugLevel);
				if (xRet != FTM_RET_OK)
				{
					bShowUsage  = FTM_TRUE;
				}
			}
			break;

		default:
			bShowUsage  = FTM_TRUE;
		}
	}

	if (bShowUsage == FTM_TRUE)
	{
		FTOM_CLIENT_usage();
		goto finish;
	}

	FTM_TRACE_setLevel(FTM_TRACE_MODULE_ALL, ulDebugLevel);
	FTM_TRACE_setInfo2( FTOM_TRACE_MODULE_CLIENT,	"CLIENT",	FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);

	xRet =FTM_CONFIG_create(pConfigFileName, &pConfig, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		FTM_TRACE_loadConfig(pConfig);
	}
	else
	{
		ERROR2(xRet, "Failed to create config!\n");	
	}

	xRet = FTOM_CLIENT_NET_create((FTOM_CLIENT_NET_PTR _PTR_)&pClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create client!\n");
		goto finish;
	}

	if (pConfig != NULL)
	{
		xRet = FTOM_CLIENT_loadConfig(pClient, pConfig);
		if (xRet != FTM_RET_OK)                                                                                                                                                                                           
		{
			ERROR2(xRet, "Failed to load configuration from file[%s]\n", pConfigFileName);
		}
	
		FTM_CONFIG_destroy(&pConfig);
	}

	TRACE("Configuraion loaded.\n");

	FTM_SHELL_init(&xShell, pPrompt, _cmds, ulCmds, NULL);
	FTOM_CLIENT_start(pClient);
	FTOM_CLIENT_setNotifyCB(pClient, FTOM_CLIENT_subscriber, pClient);
	 
	FTM_SHELL_run(&xShell);
	FTM_SHELL_final(&xShell);

	FTOM_CLIENT_NET_destroy((FTOM_CLIENT_NET_PTR _PTR_)&pClient);

finish:
	FTM_MEM_final();

	return	0;
}

FTM_RET	FTOM_CLIENT_subscriber
(
	FTOM_MSG_PTR	pBaseMsg,
	FTM_VOID_PTR	pData
)
{
	ASSERT(pBaseMsg != NULL);
	ASSERT(pData != NULL);

	switch(pBaseMsg->xType)
	{
	case	FTOM_MSG_TYPE_EP_STATUS:
		{
			FTOM_MSG_EP_STATUS_PTR pMsg = 	(FTOM_MSG_EP_STATUS_PTR)pBaseMsg;
			TRACE("EP[%s] status is %s\n", pMsg->pEPID, pMsg->bStatus?"run":"stop");
		}
		break;

	case	FTOM_MSG_TYPE_EP_DATA:
		{
			FTM_ULONG	i;
			FTM_CHAR	pBuff[4096];
			FTM_ULONG	ulLen = 0;
			FTOM_MSG_EP_DATA_PTR pMsg = 	(FTOM_MSG_EP_DATA_PTR)pBaseMsg;


			for(i = 0 ; i < pMsg->ulCount ; i++)
			{
				if (i == 0)
				{
					ulLen += sprintf(&pBuff[ulLen], "%s", FTM_EP_DATA_print(&pMsg->pData[i]));
				}
				else
				{
					ulLen += sprintf(&pBuff[ulLen], ", %s", FTM_EP_DATA_print(&pMsg->pData[i]));
				}
			}

			TRACE("EP[%s] : %s\n", pMsg->pEPID, pBuff);
		}
		break;

	default:
		{
			TRACE("Not supported messae[%08x] : Len = %lu\n", pBaseMsg->xType, pBaseMsg->ulLen);
		}
	}

	return	FTM_RET_OK;
}

FTM_VOID	FTOM_CLIENT_usage
(
	FTM_VOID
)
{
	MESSAGE("Usage: %s [-c CONFFILE] [-v LEVEL]\n\n", program_invocation_short_name);
	MESSAGE("Client for IoT gateway.\n");
	MESSAGE("OPTIONS:\n");
	MESSAGE("\t-c FILE\tConfiguration file (default /etc/%s.conf)\n", program_invocation_short_name);
	MESSAGE("\t-v LEVEL\tVerbose level\n");
}


