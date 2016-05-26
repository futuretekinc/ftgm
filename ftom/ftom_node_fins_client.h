#ifndef	__FTOM_NODE_FINS_CLIENT_H__
#define	__FTOM_NODE_FINS_CLIENT_H__

#include <semaphore.h>
#include "ftom.h"
#include "ftm_timer.h"
#include "ftom_node.h"
#include "ftm_lock.h"

typedef	struct 
{
	FTOM_NODE				xCommon;
	FTM_INT					xSockFD;
	FTM_LOCK_PTR			pLock;
	struct sockaddr_in 		xLocal;
	struct sockaddr_in 		xRemote;
	FTM_VOID_PTR			pData;
}	FTOM_NODE_FINSC, _PTR_ FTOM_NODE_FINSC_PTR;

typedef	FTM_RET	(*FTOM_NODE_FINSC_GET)(FTOM_NODE_FINSC_PTR	pFINSC, FTOM_EP_PTR pEP, FTM_EP_DATA_PTR PData);
typedef	FTM_RET	(*FTOM_NODE_FINSC_SET)(FTOM_NODE_FINSC_PTR	pFINSC, FTOM_EP_PTR pEP, FTM_EP_DATA_PTR PData);

typedef	struct
{
	FTM_CHAR			pModel[FTM_NAME_LEN+1];
	FTOM_NODE_FINSC_GET	fSet;
	FTOM_NODE_FINSC_SET	fGet;
}	FTOM_NODE_FINSC_DESCRIPT, _PTR_ FTOM_NODE_FINSC_DESCRIPT_PTR;

FTM_RET	FTOM_NODE_FINSC_create
(
	FTM_NODE_PTR 	pInfo, 
	FTOM_NODE_PTR _PTR_ ppNode
);

FTM_RET	FTOM_NODE_FINSC_destroy
(
	FTOM_NODE_FINSC_PTR _PTR_ pNode
);

FTM_BOOL	FTOM_NODE_FINSC_isRunning
(
	FTOM_NODE_FINSC_PTR pNode
);

FTM_RET	FTOM_NODE_FINSC_start
(
	FTOM_NODE_FINSC_PTR pNode
);

FTM_RET	FTOM_NODE_FINSC_stop
(
	FTOM_NODE_FINSC_PTR pNode
);

#endif

