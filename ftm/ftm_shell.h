#ifndef	__FTM_SHELL_H__
#define	__FTM_SHELL_H__

#include "ftm_types.h"

#define		FTM_SHELL_MAX_ARGS	16

typedef struct	_FTMC_CMD
{
	FTM_CHAR_PTR	pString;
	FTM_RET			(*function)(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);
	FTM_CHAR_PTR	pShortHelp;
	FTM_CHAR_PTR	pHelp;
}	FTM_SHELL_CMD, _PTR_ FTM_SHELL_CMD_PTR;

typedef	struct
{
	FTM_CHAR_PTR		pPrompt;
	FTM_SHELL_CMD_PTR	pCmdList;
	FTM_ULONG			ulCmdCount;
}	FTM_SHELL_CONFIG, _PTR_ FTM_SHELL_CONFIG_PTR;

FTM_RET	FTM_SHELL_init
(
	FTM_SHELL_CONFIG_PTR pConfig
);

FTM_RET	FTM_SHELL_final
(
	FTM_VOID
);

FTM_RET FTM_SHELL_run
(
	FTM_VOID
);

FTM_RET	FTM_SHELL_setPrompt
(
	FTM_CHAR_PTR	pPrompt
);

FTM_RET	FTM_SHELL_appendCmd
(
	FTM_SHELL_CMD_PTR pCmd
);

#endif
