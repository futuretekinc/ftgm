#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>    
#include <string.h>    
#include <unistd.h>    
#define	__USE_XOPEN
#include <time.h>    
#include <sys/socket.h> 
#include <arpa/inet.h>
#include "libconfig.h"
#include "ftm_shell.h"
#include "ftm_sic.h"
#include "ftom_blocker_console_cmds.h"


FTM_INT	main
(
	FTM_INT			nArgc , 
	FTM_CHAR_PTR	pArgv[]
)
{
	FTM_RET		xRet;
	FTM_INT		nOpt;
	FTM_ULONG	ulDebugLevel;
	FTM_CHAR	pConfigFileName[FTM_FILE_NAME_LEN];
	FTM_SIC_PTR	pSIC = NULL;
	FTM_CONFIG_PTR	pConfig = NULL;

	sprintf(pConfigFileName, "%s.conf", FTM_getProgramName());

	while((nOpt = getopt(nArgc, pArgv, "c:dv:h?")) != -1)
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
				FTM_TRACE_strToLevel(optarg, &ulDebugLevel);
			}
			break;

		default:
			return	0;
		}
	}

	FTM_MEM_init();
	FTM_TRACE_setLevel(FTM_TRACE_MODULE_ALL, ulDebugLevel);
	FTM_TRACE_setInfo2(100, "TEST", FTM_TRACE_LEVEL_TRACE, FTM_TRACE_OUT_TERM);

	xRet =FTM_CONFIG_create(pConfigFileName, &pConfig, FTM_FALSE);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTM_TRACE_loadConfig(pConfig);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to load configuration from file[%s]\n", pConfigFileName);
			FTM_CONFIG_destroy(&pConfig);

			goto finish;
		}
	}

	xRet = FTM_SIC_create(&pSIC);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create server interface client!\n");
		goto finish;
	}

	if (pConfig != NULL)
	{
		FTM_CONFIG_destroy(&pConfig);
	}

	xRet = FTM_SIC_start(pSIC);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to start service interface client!\n");
		goto finish;	
	}

	/* load configuraton */
	FTM_SHELL_run2(FTOM_BLOCKER_CLIENT_pPrompt, FTOM_BLOCKER_CLIENT_pCmdList, FTOM_BLOCKER_CLIENT_ulCmdCount, pSIC);

	xRet = FTM_SIC_stop(pSIC);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to stop service interface client!\n");	
	}

finish:

	if (pConfig != NULL)
	{
		FTM_CONFIG_destroy(&pConfig);	
	}

	if (pSIC != NULL)
	{
		FTM_SIC_destroy(&pSIC);
	}

	FTM_MEM_final();

	return	0;
}

