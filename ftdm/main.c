#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include "ftgm_type.h"
#include "libftdm.h"

typedef enum
{
	FTDM_CMD_UNKNOWN = 0,
	FTDM_CMD_DAEMON,
	FTDM_CMD_USAGE,
	FTDM_CMD_VERSION 
}	FTDM_CMD;

void	FTDM_printVersion(FTGM_STRING strProgramName);
void	FTDM_printUsage(FTGM_STRING strProgramName);

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
			FTDM_printVersion(argv[0]);
			return	0;
		}
		break;
	
	default:
		{
			FTDM_printUsage(argv[0]);	
			return	0;
		}
	}

	FTDM_init();

	while(1)
	{
	
	}


	FTDM_final();

	return	0;
}

void	FTDM_printVersion(FTGM_STRING strProgramName)
{
	printf("%s v1.0.0.0\n", strProgramName);
}

void	FTDM_printUsage(FTGM_STRING strProgramName)
{
	printf("Usage : %s [OPTIONS]\n", strProgramName);
	printf("OPTIONS include:\n");
	printf("    -v           Show version\n");
}

