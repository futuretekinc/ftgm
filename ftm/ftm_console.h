#ifndef	__FTM_CONSOLE_H__
#define	__FTM_CONSOLE_H__

#include "ftm_types.h"

#define		FTM_CONSOLE_MAX_ARGS	16

typedef struct	_FTMC_CMD
{
	FTM_CHAR_PTR	pString;
	FTM_RET			(*function)(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
	FTM_CHAR_PTR	pShortHelp;
	FTM_CHAR_PTR	pHelp;
}	FTM_CONSOLE_CMD, _PTR_ FTM_CONSOLE_CMD_PTR;

typedef	struct
{
	FTM_CHAR_PTR		pPrompt;
	FTM_CONSOLE_CMD_PTR	pCmdList;
	FTM_ULONG			ulCmdCount;
}	FTM_CONSOLE_CONFIG, _PTR_ FTM_CONSOLE_CONFIG_PTR;

FTM_RET	FTM_CONSOLE_init
(
	FTM_CONSOLE_CONFIG_PTR pConfig
);

FTM_RET	FTM_CONSOLE_final
(
	FTM_VOID
);

FTM_RET FTM_CONSOLE_run
(
	FTM_VOID
);

FTM_RET	FTM_CONSOLE_setPrompt
(
	FTM_CHAR_PTR	pPrompt
);

FTM_RET	FTM_CONSOLE_appendCmd
(
	FTM_CONSOLE_CMD_PTR pCmd
);

#endif
