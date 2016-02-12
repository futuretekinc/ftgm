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
#include "ftm_console.h"
#include "ftdm_client.h"
#include "ftdm_client_cmds.h"
#include "ftdm_client_config.h"
#include "ftm_debug.h"

FTDMC_SESSION	_xSession;
FTDMC_CFG		xClientConfig;
FTM_BOOL		_bQuit = FTM_FALSE;

extern	char *		program_invocation_short_name;

int main(int argc , char *argv[])
{
	FTM_CHAR		pConfigFileName[FTM_FILE_NAME_LEN];
	FTM_CONSOLE_CONFIG	xConsoleConfig;

	xConsoleConfig.pCmdList 	= FTDMC_pCmdList;
	xConsoleConfig.ulCmdCount	= FTDMC_ulCmdCount;
	xConsoleConfig.pPrompt		= FTDMC_pPrompt;

	sprintf(pConfigFileName, "%s.conf", program_invocation_short_name);

	/* load configuraton */
	FTDMC_CFG_init(&xClientConfig);
	FTDMC_CFG_load(&xClientConfig, pConfigFileName);

	/* apply configuraton */
	FTDMC_init(&xClientConfig);

	FTM_CONSOLE_init(&xConsoleConfig);
	FTM_CONSOLE_run();

	FTDMC_final();

	FTDMC_CFG_final(&xClientConfig);

	return	0;
}

