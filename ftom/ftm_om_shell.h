#ifndef	__FTM_OM_SHELL_H__
#define	__FTM_OM_SHELL_H__

#include <semaphore.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "ftm_shell.h"
#include "ftm_om_service.h"

typedef	struct FTM_OM_SHELL_STRUCT
{
	FTM_SHELL		xShell;
	pthread_t 		xThread;

	FTM_BOOL		bStop;
	FTM_OM_PTR		pOM;
}	FTM_OM_SHELL, _PTR_ FTM_OM_SHELL_PTR;

FTM_RET	FTM_OM_SHELL_init
(
	FTM_OM_SHELL_PTR 	pShell,
	FTM_OM_PTR pOM 
);

FTM_RET	FTM_OM_SHELL_final
(
	FTM_OM_SHELL_PTR 	pShell
);

FTM_RET	FTM_OM_SHELL_start
(
	FTM_OM_SHELL_PTR	pShell
);

FTM_RET	FTM_OM_SHELL_stop
(
	FTM_OM_SHELL_PTR	pShell
);

FTM_RET	FTM_OM_SHELL_loadFromFile
(
	FTM_OM_SHELL_PTR	pShell, 
	FTM_CHAR_PTR		pFileName
);

FTM_RET	FTM_OM_SHELL_showConfig
(
	FTM_OM_SHELL_PTR 	pShell
);

FTM_RET	FTM_OM_SHELL_notify
(
	FTM_OM_SHELL_PTR 	pShell, 
	FTM_OM_MSG_PTR 		pMsg
);

FTM_RET	FTM_OM_SHELL_setServiceCallback
(
	FTM_OM_SHELL_PTR 	pShell, 
	FTM_OM_SERVICE_ID 	xID, 
	FTM_OM_SERVICE_CALLBACK fServiceCB
);
#endif
