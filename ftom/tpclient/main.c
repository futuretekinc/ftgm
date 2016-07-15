#include <stdio.h>
#include <curl/curl.h>
#include "ftom.h"
#include "ftom_shell.h"
#include "ftom_tp_client.h"
#include "ftom_tp_restapi.h"

extern	FTM_SHELL_CMD	FTOM_shellCmds[];
extern	FTM_ULONG		FTOM_shellCmdCount;
static
FTM_VOID	FTOM_TP_CLIENT_usage
(
	FTM_VOID
);

int main(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	FTM_RET				xRet;
	FTM_INT				nOpt;
	FTM_BOOL			bDaemon = FTM_FALSE;
	FTM_ULONG			ulDebugLevel = FTM_TRACE_LEVEL_ALL;
	FTOM_TP_CLIENT_PTR	pTPClient = NULL;
	FTM_CHAR			pConfigFileName[1024];

	sprintf(pConfigFileName, "/etc/%s.conf", FTM_getProgramName());

	while((nOpt = getopt(nArgc, pArgv, "c:dv:h?")) != -1)
	{
		switch(nOpt)
		{
		case	'c':
			{
				strncpy(pConfigFileName, optarg, sizeof(pConfigFileName));
			}
			break;
		
		case	'd':
			{
				bDaemon = FTM_TRUE;	
			}
			break;

		case	'v':
			{
				ulDebugLevel = strtoul(optarg, 0, 10);
			}
			break;

		default:
			{
				FTOM_TP_CLIENT_usage();
			
				goto finish;
			}
		}
	}

	xRet = FTM_MEM_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Memory initialization failed.\n");
		goto finish;	
	}

	FTM_TRACE_setLevel(FTM_TRACE_MODULE_ALL, ulDebugLevel);
	//FTM_TRACE_setModule(0xFFFFFFFF, FTM_TRUE);

	xRet = FTOM_TP_CLIENT_create(&pTPClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Can't create a TPClient.\n");
		goto finish;
	}

	xRet = FTOM_TP_CLIENT_loadConfigFromFile(pTPClient, pConfigFileName);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "TPClient failed to load configuration from file.\n");	
		goto error;
	}

	FTOM_TP_CLIENT_showConfig(pTPClient);

	if (bDaemon)
	{
		if (fork() != 0)
		{
			return	0;
		}
		FTOM_TP_CLIENT_start(pTPClient);
		FTOM_TP_CLIENT_waitingForFinished(pTPClient);
	}
	else
	{
		FTM_SHELL			xShell;
		FTM_INT				i;
		for(i = 0 ; i < FTOM_shellCmdCount ; i++)
		{
			FTOM_shellCmds[i].pData = pTPClient;	
		}

		FTOM_TP_CLIENT_start(pTPClient);
		FTM_SHELL_init(&xShell,pTPClient);
		FTM_SHELL_setPrompt(&xShell, "tpclient> ");
		FTM_SHELL_addCmds(&xShell, FTOM_shellCmds, FTOM_shellCmdCount);
		FTM_SHELL_run(&xShell);
		FTM_SHELL_final(&xShell);
		FTOM_TP_CLIENT_waitingForFinished(pTPClient);
	}
	
	xRet = FTOM_TP_CLIENT_destroy(&pTPClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Remove the TPClient failed\n");
	}

	xRet = FTM_MEM_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Memory finalization failed.\n");	
	}

error:
	if (pTPClient != NULL)
	{
		FTOM_TP_CLIENT_destroy(&pTPClient);
	}

finish:
  	return 0;
}

FTM_VOID	FTOM_TP_CLIENT_usage
(
	FTM_VOID
)
{
	MESSAGE("Usage: %s [-d] [-c CONFFILE] [-v LEVEL]\n\n", FTM_getProgramName());
	MESSAGE("Thing+ client.\n");
	MESSAGE("OPTIONS:\n");
	MESSAGE("\t-d\tRun as a daemon\n");
	MESSAGE("\t-c FILE\tConfiguration file (default /etc/%s.conf)\n", FTM_getProgramName());
	MESSAGE("\t-v LEVEL\tVerbose level\n");
}
