#ifndef	__FTDM_CONSOLE_H__
#define	__FTDM_CONSOLE_H__

#include "ftm_types.h"

#define		FTDM_CONSOLE_MAX_ARGS	16

typedef struct	_FTDMC_CMD
{
	FTM_CHAR_PTR	pString;
	FTM_RET			(*function)(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
	FTM_CHAR_PTR	pShortHelp;
	FTM_CHAR_PTR	pHelp;
}	FTDM_CONSOLE_CMD, _PTR_ FTDM_CONSOLE_CMD_PTR;

FTM_RET	FTDM_CONSOLE_init
(
	FTM_VOID
);

FTM_RET	FTDM_CONSOLE_final
(
	FTM_VOID
);

FTM_RET	FTDM_CONSOLE_run
(
	FTM_VOID
);

FTM_RET	FTDM_CONSOLE_setPrompt
(
	FTM_CHAR_PTR	pPrompt
);

#endif
