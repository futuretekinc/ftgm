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

FTM_RET	FTOM_CLIENT_CMD_quit(FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData)
{
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;

	if (nArgc != 1)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	
	FTOM_CLIENT_disconnect(pClient);

	return	FTM_RET_SHELL_QUIT;
}

