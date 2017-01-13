#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>    
#include <string.h>    
#include <unistd.h>    
#define	__USE_XOPEN
#include <time.h>    
#include <sys/socket.h> 
#include <arpa/inet.h>
#include "ftm.h"
#include "ftm_sic.h"
#include "ftom_blocker.h"
#include "ftom_blocker_service_interface.h"

static FTM_RET  FTOM_BLOCKER_CLIENT_cmdConfig(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);

FTM_SHELL_CMD			FTOM_BLOCKER_CLIENT_pCmdList[] = 
{
	{
		.pString	= "config",
		.function	= FTOM_BLOCKER_CLIENT_cmdConfig,
		.pShortHelp	= "Configuration",
		.pHelp		= "\n"\
					  "\tConfiguration.\n"
	}
};

FTM_ULONG		FTOM_BLOCKER_CLIENT_ulCmdCount = sizeof(FTOM_BLOCKER_CLIENT_pCmdList) / sizeof(FTM_SHELL_CMD);
FTM_CHAR_PTR	FTOM_BLOCKER_CLIENT_pPrompt = "blocker";

FTM_RET  FTOM_BLOCKER_CLIENT_cmdConfig
(
	FTM_SHELL_PTR	pShell,
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[],
	FTM_VOID_PTR	pData
)
{
	ASSERT(pShell != NULL);
	ASSERT(pData != NULL);

	FTM_RET	xRet;
	FTM_SIC_PTR	pSIC = (FTM_SIC_PTR)pData;
	FTOM_BLOCKER_CONFIG	xConfig;

	xRet = FTOM_BLOCKER_SIC_getConfig(pSIC, &xConfig);
	if (xRet == FTM_RET_OK)
	{
		MESSAGE("%16s : %s\n", "Name", xConfig.pName);
		MESSAGE("%16s : %s\n", "FTOM", xConfig.xServerClient.pName);
		MESSAGE("%16s : %s\n", "Cloud",xConfig.xCloudClient.pName);
	}

	return	FTM_RET_OK;
}

