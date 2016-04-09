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
#include "ftom_client.h"
#include "ftom_client_config.h"

#define		FTOM_CLIENT_MAX_LINE	2048
#define		FTOM_CLIENT_MAX_ARGS	16


static FTM_RET	FTOM_CLIENT_notifyCallback(FTM_VOID_PTR pData);

extern FTM_CHAR_PTR 	_strPrompt;
extern FTM_SHELL_CMD	_cmds[];
extern FTM_ULONG		ulCmds;

extern	char *		program_invocation_short_name;

int main(int argc , char *argv[])
{
	FTM_CHAR		pConfigFileName[FTM_FILE_NAME_LEN];
	FTOM_CLIENT	xClient;
	FTM_SHELL		xShell;
	FTM_INT			i;

	sprintf(pConfigFileName, "%s.conf", program_invocation_short_name);

	FTM_MEM_init();

	FTOM_CLIENT_init(&xClient);

	/* load configuraton */
	FTOM_CLIENT_loadConfigFromFile(&xClient, pConfigFileName);
	FTOM_CLIENT_setNotifyCallback(&xClient, FTOM_CLIENT_notifyCallback);


	FTM_SHELL_init(&xShell);
	FTM_SHELL_setPrompt(&xShell, _strPrompt);
	for(i = 0 ; i < ulCmds;i++)
	{
		_cmds[i].pData = &xClient;
		FTM_SHELL_appendCmd(&xShell, &_cmds[i]);
	}

	FTOM_CLIENT_start(&xClient);

	FTM_SHELL_run(&xShell);
	FTM_SHELL_final(&xShell);

	FTOM_CLIENT_final(&xClient);

	FTM_MEM_final();

	return	0;
}

FTM_RET	FTOM_CLIENT_notifyCallback(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);

	return	FTM_RET_OK;

}
