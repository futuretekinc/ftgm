#ifndef	__FTNM_CONSOLE_CMDS_H__
#define __FTNM_CONSOLE_CMDS_H__

#include "ftm_types.h"
#include "ftm_console.h"

FTM_RET	FTNM_CONSOLE_cmdConfig
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
);

FTM_RET	FTNM_CONSOLE_cmdList
(
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[]
);

extern FTM_CONSOLE_CMD	FTNM_xCmds[];
extern FTM_ULONG		FTNM_ulCmds;
#endif
