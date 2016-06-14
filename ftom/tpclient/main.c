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
	FTM_ULONG			ulDebugLevel = FTM_TRACE_LEVEL_ERROR;
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
		ERROR("Memory initialization failed[%08x]\n", xRet);
		goto finish;	
	}

	FTM_TRACE_setLevel(0);

	xRet = FTOM_TP_CLIENT_create(&pTPClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Can't create a TPClient[%08x]\n", xRet);
		goto finish;
	}

	xRet = FTOM_TP_CLIENT_loadConfigFromFile(pTPClient, pConfigFileName);
	if (xRet != FTM_RET_OK)
	{
		ERROR("TPClient failed to load configuration from file[%08x].\n", xRet);	
		goto error;
	}

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
		//FTOM_SHELL_PTR pShell;

		FTOM_TP_CLIENT_start(pTPClient);
		//FTOM_SHELL_create(&pShell);
		//FTOM_SHELL_process(pShell);
		//FTOM_SHELL_destroy(&pShell);
		FTOM_TP_CLIENT_waitingForFinished(pTPClient);
	}
	
	xRet = FTOM_TP_CLIENT_destroy(&pTPClient);
	if (xRet != FTM_RET_OK)
	{
		ERROR("Remove the TPClient failed[%08x]\n");
	}

	xRet = FTM_MEM_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR("Memory finalization failed[%08x]\n", xRet);	
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
