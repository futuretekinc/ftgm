#include <stdio.h>
#include <stdlib.h>
#include "ftgm_type.h"
#include "ftdm_cmd.h"

void	FTDM_cmdVersion(FTGM_STRING strProgramName)
{
	printf("%s v1.0.0.0\n", strProgramName);
}

void	FTDM_cmdUsage(FTGM_STRING strProgramName)
{
	printf("Usage : %s [OPTIONS]\n", strProgramName);
	printf("OPTIONS include:\n");
	printf("    -v           Show version\n");
}

void	FTDM_cmdStartDaemon(void)
{
	FTDM_main();
}
