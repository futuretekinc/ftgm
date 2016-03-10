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
#include "ftdm_client.h"
#include "ftdm_client_cmds.h"
#include "ftdm_client_config.h"

FTDMC_SESSION	_xSession;
FTDMC_CFG		xClientConfig;
FTM_BOOL		_bQuit = FTM_FALSE;

extern	char *		program_invocation_short_name;

int main(int argc , char *argv[])
{
	FTM_CHAR		pConfigFileName[FTM_FILE_NAME_LEN];

	sprintf(pConfigFileName, "%s.conf", program_invocation_short_name);

	/* load configuraton */
	FTDMC_CFG_init(&xClientConfig);
	FTDMC_CFG_load(&xClientConfig, pConfigFileName);

	/* apply configuraton */
	FTDMC_init(&xClientConfig);

	FTM_SHELL_init();
	FTM_SHELL_setPrompt(FTDMC_pPrompt);
	FTM_SHELL_addCmds(FTDMC_pCmdList,FTDMC_ulCmdCount);
	FTM_SHELL_run();

	FTDMC_final();

	FTDMC_CFG_final(&xClientConfig);

	return	0;
}

