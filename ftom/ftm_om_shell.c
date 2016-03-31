#include "ftm_om_shell.h"
#include "ftm_om_shell_cmds.h"

static FTM_VOID_PTR	FTM_OM_SHELL_process
(
	FTM_VOID_PTR	pData
);

FTM_RET	FTM_OM_SHELL_init
(
	FTM_OM_SHELL_PTR 	pShell,
	FTM_OM_PTR pOM 
)
{
	ASSERT(pShell != NULL);
	
	FTM_RET	xRet;

	xRet = FTM_SHELL_init(&pShell->xShell);
	FTM_SHELL_setPrompt(&pShell->xShell, "FTOM> ");
	if (xRet == FTM_RET_OK)
	{
		FTM_INT	i;

		for(i = 0 ; i < FTM_OM_shellCmdCount; i++)
		{
			FTM_OM_shellCmds[i].pData = pOM;
			FTM_SHELL_appendCmd(&pShell->xShell, &FTM_OM_shellCmds[i]);
		}
	}

	return	xRet;
}

FTM_RET	FTM_OM_SHELL_final
(
	FTM_OM_SHELL_PTR 	pShell
)
{
	ASSERT(pShell != NULL);

	FTM_SHELL_final(&pShell->xShell);

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_SHELL_start
(
	FTM_OM_SHELL_PTR	pShell
)
{
	ASSERT(pShell != NULL);

	pthread_create(&pShell->xThread, NULL, FTM_OM_SHELL_process, pShell);

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_SHELL_stop
(
	FTM_OM_SHELL_PTR	pShell
)
{
	ASSERT(pShell != NULL);

	pShell->bStop = FTM_TRUE;
	pthread_join(pShell->xThread, NULL);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTM_OM_SHELL_process
(
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);
	FTM_OM_SHELL_PTR	pShell = (FTM_OM_SHELL_PTR)pData;

	FTM_SHELL_run(&pShell->xShell);

	return	0;
}

FTM_RET	FTM_OM_SHELL_loadFromFile
(
	FTM_OM_SHELL_PTR	pShell, 
	FTM_CHAR_PTR		pFileName
)
{
	ASSERT(pShell != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_SHELL_showConfig
(
	FTM_OM_SHELL_PTR 	pShell
)
{
	ASSERT(pShell != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_SHELL_notify
(
	FTM_OM_SHELL_PTR 	pShell, 
	FTM_OM_MSG_PTR 		pMsg
)
{
	ASSERT(pShell != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTM_OM_SHELL_setServiceCallback
(
	FTM_OM_SHELL_PTR 	pShell, 
	FTM_OM_SERVICE_ID 	xID, 
	FTM_OM_SERVICE_CALLBACK fServiceCB
)
{
	ASSERT(pShell != NULL);

	return	FTM_RET_OK;
}
