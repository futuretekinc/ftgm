#include "ftom_shell.h"
#include "ftom_shell_cmds.h"

FTM_RET	FTOM_SHELL_create
(
	FTOM_SHELL_PTR _PTR_ 	ppShell
)
{
	ASSERT(ppShell != NULL);

	FTM_RET	xRet;
	FTOM_SHELL_PTR	pShell;

	pShell = (FTOM_SHELL_PTR)FTM_MEM_malloc(sizeof(FTOM_SHELL));
	if (pShell == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_SHELL_init(pShell);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pShell);
		return	xRet;	
	}

	*ppShell = pShell;

	return	FTM_RET_OK;
}
	
FTM_RET	FTOM_SHELL_destroy
(
	FTOM_SHELL_PTR _PTR_ 	ppShell
)
{
	ASSERT(ppShell != NULL);

	FTOM_SHELL_final(*ppShell);

	FTM_MEM_free(*ppShell);

	*ppShell = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SHELL_init
(
	FTOM_SHELL_PTR 	pShell
)
{
	ASSERT(pShell != NULL);
	
	FTM_RET	xRet;

	xRet = FTM_SHELL_init(&pShell->xShell, NULL, NULL, 0, NULL);
	FTM_SHELL_setPrompt(&pShell->xShell, "FTOM");
	if (xRet == FTM_RET_OK)
	{
		FTM_INT	i;
		for(i = 0 ; i < FTOM_shellCmdCount; i++)
		{
			FTM_SHELL_appendCmd(&pShell->xShell, &FTOM_shellCmds[i]);
		}
	}

	return	xRet;
}

FTM_RET	FTOM_SHELL_final
(
	FTOM_SHELL_PTR 	pShell
)
{
	ASSERT(pShell != NULL);

	FTM_SHELL_final(&pShell->xShell);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SHELL_start
(
	FTOM_SHELL_PTR	pShell
)
{
	ASSERT(pShell != NULL);

	if (pShell->bStop)
	{
		return	FTM_RET_ALREADY_STARTED;	
	}

	pthread_create(&pShell->xThread, NULL, FTOM_SHELL_process, pShell);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SHELL_stop
(
	FTOM_SHELL_PTR	pShell
)
{
	ASSERT(pShell != NULL);

	if (!pShell->bStop)
	{
		return	FTM_RET_NOT_START;	
	}

	pShell->xShell.bStop = FTM_TRUE;
	pthread_cancel(pShell->xThread);
//	pthread_join(pShell->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTOM_SHELL_process
(
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);
	FTOM_SHELL_PTR	pShell = (FTOM_SHELL_PTR)pData;

	FTM_SHELL_run(&pShell->xShell);

	return	0;
}

FTM_RET	FTOM_SHELL_loadConfig
(
	FTOM_SHELL_PTR	pShell, 
	FTM_CONFIG_PTR	pConfig
)
{
	ASSERT(pShell != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SHELL_loadFromFile
(
	FTOM_SHELL_PTR	pShell, 
	FTM_CHAR_PTR		pFileName
)
{
	ASSERT(pShell != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SHELL_showConfig
(
	FTOM_SHELL_PTR 	pShell
)
{
	ASSERT(pShell != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SHELL_notify
(
	FTOM_SHELL_PTR 	pShell, 
	FTOM_MSG_PTR 		pMsg
)
{
	ASSERT(pShell != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_SHELL_setServiceCallback
(
	FTOM_SHELL_PTR 	pShell, 
	FTOM_SERVICE_ID	xID, 
	FTOM_SERVICE_CB fServiceCB
)
{
	ASSERT(pShell != NULL);

	return	FTM_RET_OK;
}
