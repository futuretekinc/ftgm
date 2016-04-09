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

FTM_RET	FTOM_CLIENT_CMD_debug(FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData)
{
	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	if (strcasecmp(pArgv[1], "mode") == 0)
	{
		if (nArgc == 2)
		{
			FTM_ULONG	nMode;
			FTM_TRACE_getLevel(&nMode);
			switch(nMode)
			{
			case	0: MESSAGE("DEBUG OUT MODE : NONE\n"); break;
			case	1: MESSAGE("DEBUG OUT MODE : NORMAL\n"); break;
			case	2: MESSAGE("DEBUG OUT MODE : ALL\n"); break;
			}
		}
		else if (nArgc == 3)
		{
			FTM_ULONG	nMode, nNewMode;

			nNewMode = strtoul(pArgv[2], NULL, 10);
		
			FTM_TRACE_getLevel(&nMode);
			FTM_TRACE_setLevel(nNewMode);

			switch(nMode)
			{
			case	0: MESSAGE("DEBUG OUT MODE : NONE"); break;
			case	1: MESSAGE("DEBUG OUT MODE : NORMAL"); break;
			case	2: MESSAGE("DEBUG OUT MODE : ALL"); break;
			}

			switch(nNewMode)
			{
			case	0: MESSAGE(" to NONE\n"); break;
			case	1: MESSAGE(" to NORMAL\n"); break;
			case	2: MESSAGE(" to ALL\n"); break;
			}
		}
		else
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	return	FTM_RET_OK;
}


