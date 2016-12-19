#include "ftom.h"
#include "ftom_session.h"

FTM_RET	FTOM_SESSION_create
(
	FTM_INT		hSocket,
	struct sockaddr_in _PTR_ pPeer,
	FTM_VOID_PTR	pData,
	FTOM_SESSION_PTR _PTR_ ppSession
)
{
	ASSERT(ppSession != NULL);

	FTOM_SESSION_PTR pSession;
	
	pSession = (FTOM_SESSION_PTR)FTM_MEM_malloc(sizeof(FTOM_SESSION));
	if (pSession == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pSession->hSocket = hSocket;
	memcpy(&pSession->xPeer, pPeer, sizeof(struct sockaddr_in));
	pSession->pData = pData;

	*ppSession = pSession;

	return	FTM_RET_OK;
}

FTM_RET FTOM_SESSION_destroy
(
	FTOM_SESSION_PTR _PTR_ ppSession
)
{
	ASSERT(ppSession != NULL);

	FTM_MEM_free(*ppSession);
	*ppSession = NULL;

	return	FTM_RET_OK;
}
