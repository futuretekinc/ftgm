#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "ftnm.h"
#include "ftm_mem.h"
#include "ftm_debug.h"
#include "ftnm_cmds.h"
#include "ftnm_config.h"

extern char *program_invocation_short_name;

FTM_VOID_PTR	FTNM_process(FTM_VOID_PTR pData);

int main(int nArgc, char *pArgv[])
{
	FTM_INT				nOpt;
	FTM_BOOL			bDaemon = FTM_BOOL_FALSE;
	pthread_t			xMainThread;

	while((nOpt = getopt(nArgc, pArgv, "c:d?")) != -1)
	{
		switch(nOpt)
		{
		case	'c':
			break;
		
		case	'd':
			{
				bDaemon = FTM_BOOL_TRUE;	
			}
			break;
		}
	}

	setPrintMode(1);

	if (bDaemon)
	{
		if (fork() == 0)
		{
			FTNM_process(NULL);
		}
	}
	else
	{
		pthread_create(&xMainThread, NULL, FTNM_process, NULL);
		FTM_CONSOLE_run(FTNM_xCmds, FTNM_ulCmds);
	}

	return	0;
}

FTM_VOID_PTR	FTNM_process(FTM_VOID_PTR pData)
{
	FTM_CHAR			pConfigFileName[1024];

	sprintf(pConfigFileName, "%s.conf", program_invocation_short_name);

	FTM_MEM_init();

	FTNM_init(pConfigFileName);

	FTNM_run();

	FTNM_final();

	FTM_MEM_final();

	return	0;
}
