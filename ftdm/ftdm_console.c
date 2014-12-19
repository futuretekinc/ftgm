#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include "ftdm.h"
#include "ftdm_shell.h"


int	main(int argc, char *argv[])
{
	int	nOpt;
	FTDM_SHELL_CMD	xCMD = FTDM_SHELL_CMD_UNKNOWN;

	while((xCMD == FTDM_SHELL_CMD_UNKNOWN) && ((nOpt = getopt(argc, argv, "dv")) != -1))
	{
		switch(nOpt)
		{
		case	'v':
			xCMD = FTDM_SHELL_CMD_VERSION;
			break;

		default:
			xCMD = FTDM_SHELL_CMD_USAGE;
			break;
		}
	}

	switch(xCMD)
	{
	case	FTDM_SHELL_CMD_VERSION:
		{
			FTDM_shellCmdVersion(argv[0]);
			return	0;
		}
		break;

	default:
		{
			FTDM_shellCmdUsage(argv[0]);	
			return	0;
		}
	}

	return	0;
}


