#ifndef	__FTOM_SHELL_H__
#define	__FTOM_SHELL_H__

#include <semaphore.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "ftm_shell.h"
#include "ftom_service.h"

typedef	struct FTOM_SHELL_STRUCT
{
	FTM_SHELL		xShell;
	pthread_t 		xThread;

	FTM_BOOL		bStop;
}	FTOM_SHELL, _PTR_ FTOM_SHELL_PTR;

FTM_RET	FTOM_SHELL_create
(
	FTOM_SHELL_PTR _PTR_ ppShell
);

FTM_RET	FTOM_SHELL_init
(
	FTOM_SHELL_PTR 	pShell
);

FTM_RET	FTOM_SHELL_final
(
	FTOM_SHELL_PTR 	pShell
);

FTM_RET	FTOM_SHELL_start
(
	FTOM_SHELL_PTR	pShell
);

FTM_RET	FTOM_SHELL_stop
(
	FTOM_SHELL_PTR	pShell
);

FTM_RET	FTOM_SHELL_loadFromFile
(
	FTOM_SHELL_PTR	pShell, 
	FTM_CHAR_PTR		pFileName
);

FTM_RET	FTOM_SHELL_showConfig
(
	FTOM_SHELL_PTR 	pShell
);

FTM_RET	FTOM_SHELL_notify
(
	FTOM_SHELL_PTR 	pShell, 
	FTOM_MSG_PTR 		pMsg
);

FTM_RET	FTOM_SHELL_setServiceCallback
(
	FTOM_SHELL_PTR 	pShell, 
	FTOM_SERVICE_ID 	xID, 
	FTOM_SERVICE_CALLBACK fServiceCB
);
#endif
