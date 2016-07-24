#ifndef	__FTM_SHELL_H__
#define	__FTM_SHELL_H__

#include "ftm_types.h"
#include "ftm_list.h"
#include <pthread.h>

#define		FTM_SHELL_MAX_ARGS	16

typedef	struct FTM_SHELL_STRUCT
{
	FTM_CHAR		pPrompt[128];
	FTM_LIST_PTR	pCmdList;
	pthread_t		xThread;
	FTM_BOOL		bStop;
	FTM_VOID_PTR	pData;
}	FTM_SHELL, _PTR_ FTM_SHELL_PTR;

typedef struct	_FTMC_CMD
{
	FTM_CHAR_PTR	pString;
	FTM_RET			(*function)(FTM_SHELL_PTR pShell, FTM_INT nArgc, FTM_CHAR_PTR pArgv[], FTM_VOID_PTR pData);
	FTM_CHAR_PTR	pShortHelp;
	FTM_CHAR_PTR	pHelp;
	FTM_VOID_PTR	pData;
}	FTM_SHELL_CMD, _PTR_ FTM_SHELL_CMD_PTR;

typedef	struct
{
	FTM_CHAR_PTR		pPrompt;
	FTM_SHELL_CMD_PTR	pCmdList;
	FTM_ULONG			ulCmdCount;
}	FTM_SHELL_CONFIG, _PTR_ FTM_SHELL_CONFIG_PTR;

FTM_RET	FTM_SHELL_init
(
	FTM_SHELL_PTR	pShell,
	FTM_VOID_PTR	pData
);

FTM_RET	FTM_SHELL_final
(
	FTM_SHELL_PTR	pShell
);

FTM_RET	FTM_SHELL_run
(
	FTM_SHELL_PTR	pShell
);

FTM_RET	FTM_SHELL_setPrompt
(
	FTM_SHELL_PTR	pShell,
	FTM_CHAR_PTR	pPrompt
);

FTM_RET	FTM_SHELL_addCmds
(
	FTM_SHELL_PTR		pShell,
	FTM_SHELL_CMD_PTR 	pCmds, 
	FTM_ULONG 			ulCmds
);

FTM_RET	FTM_SHELL_appendCmd
(
	FTM_SHELL_PTR		pShell,
	FTM_SHELL_CMD_PTR 	pCmd
);

#endif
