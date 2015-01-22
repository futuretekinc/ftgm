#include "ftdm_console_cmds.h"
#include "ftdm.h"

FTM_RET	FTDM_CONSOLE_CMD_config(FTM_INT	nArgc, FTM_CHAR_PTR	pArgv[]);

extern FTDM_CFG	xConfig;
FTM_CONSOLE_CMD	FTDM_xCmds[] =
{
	{
		.pString	= "config",
		.function	= FTDM_CONSOLE_CMD_config,
		.pShortHelp	= "Configuration Data Manager.",
		.pHelp		= "\n"\
					  "\tConfiguration Data Manager.\n"
	}
};

FTM_ULONG	FTDM_ulCmds = sizeof(FTDM_xCmds) / sizeof(FTM_CONSOLE_CMD);

FTM_RET	FTDM_CONSOLE_CMD_config
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
)
{
	if (nArgc == 1)
	{
		FTDM_CFG_show(&xConfig);
	}
	else
	{
	}

	return	FTM_RET_OK;
}
