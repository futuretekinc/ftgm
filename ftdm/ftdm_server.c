#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include "ftgm_type.h"
#include "ftdm.h"
#include "ftdm_cmd.h"

int	main(int argc, char *argv[])
{
	int	nOpt;
	FTDM_CMD	xCMD = FTDM_CMD_UNKNOWN;

	while((xCMD == FTDM_CMD_UNKNOWN) && ((nOpt = getopt(argc, argv, "dv")) != -1))
	{
		switch(nOpt)
		{
		case	'd':
			xCMD = FTDM_CMD_DAEMON;
			break;

		case	'v':
			xCMD = FTDM_CMD_VERSION;
			break;

		default:
			xCMD = FTDM_CMD_USAGE;
			break;
		}
	}

	switch(xCMD)
	{
	case	FTDM_CMD_VERSION:
		{
			FTDM_cmdVersion(argv[0]);
			return	0;
		}
		break;

	case	FTDM_CMD_DAEMON:
		{
			FTDM_cmdStartDaemon();
		}
		break;

	default:
		{
			FTDM_cmdUsage(argv[0]);	
			return	0;
		}
	}

	return	0;
}


