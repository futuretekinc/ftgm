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
#include "ftom_client_config.h"

#define		FTOM_CLIENT_MAX_LINE	2048
#define		FTOM_CLIENT_MAX_ARGS	16


extern FTM_CHAR_PTR 	_strPrompt;
extern FTM_SHELL_CMD	_cmds[];
extern FTM_ULONG		ulCmds;

extern	char *		program_invocation_short_name;

int main(int argc , char *argv[])
{
	FTM_CHAR		pConfigFileName[FTM_FILE_NAME_LEN];
	FTOM_CLIENT_PTR	pClient;
	FTM_SHELL		xShell;
	FTM_INT			i;

	sprintf(pConfigFileName, "%s.conf", program_invocation_short_name);

	FTM_MEM_init();

	FTOM_CLIENT_NET_create((FTOM_CLIENT_NET_PTR _PTR_)&pClient);

	/* load configuraton */
	FTOM_CLIENT_loadConfigFromFile(pClient, pConfigFileName);


	FTM_SHELL_init(&xShell, NULL);
	FTM_SHELL_setPrompt(&xShell, _strPrompt);
	for(i = 0 ; i < ulCmds;i++)
	{
		_cmds[i].pData = pClient;
		FTM_SHELL_appendCmd(&xShell, &_cmds[i]);
	}

	FTOM_CLIENT_start(pClient);

	FTM_SHELL_run(&xShell);
	FTM_SHELL_final(&xShell);

	FTOM_CLIENT_NET_destroy((FTOM_CLIENT_NET_PTR _PTR_)&pClient);

	FTM_MEM_final();

	return	0;
}

