#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "ftnm.h"
#include "ftm_debug.h"
#include "ftnm_config.h"

extern char *program_invocation_short_name;

static FTNM_CONTEXT	xContext;
static FTNM_CFG	xConfig;

int main(int nArgc, char *pArgv[])
{
	FTM_INT		nOpt;
	FTM_BOOL	bDaemon = FTM_BOOL_FALSE;
	FTM_CHAR	pConfigFileName[1024];

	sprintf(pConfigFileName, "%s.conf", program_invocation_short_name);

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

	setPrintMode(2);

	FTNM_CFG_init(&xConfig);
	FTNM_CFG_load(&xConfig, pConfigFileName);
	FTNM_CFG_show(&xConfig);

	FTNM_init(&xContext, &xConfig);
	FTNM_run(&xContext);
	FTNM_final(&xContext);

	FTNM_CFG_final(&xConfig);

	return	0;
}
