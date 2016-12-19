#ifndef	__FTOM_SESSION_H__
#define	__FTOM_SESSION_H__

#include <semaphore.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "ftm.h"
#include "ftom.h"

typedef	FTM_UINT64		FTOM_SESSION_ID, _PTR_ FTOM_SESSION_ID_PTR;

typedef	struct
{
	FTOM_SESSION_ID		xID;
	FTM_INT				hSocket;
	struct sockaddr_in	xPeer;
	pthread_t			xPThread;
	FTM_VOID_PTR		pData;
	FTM_BOOL			bStop;
	FTM_ULONG			ulReqID;
	FTM_BYTE			pReqBuff[FTOM_DEFAULT_PACKET_SIZE];
	FTM_BYTE			pRespBuff[FTOM_DEFAULT_PACKET_SIZE];
}	FTOM_SESSION, _PTR_ FTOM_SESSION_PTR;

FTM_RET	FTOM_SESSION_create
(
	FTM_INT		hSocket,
	struct sockaddr_in _PTR_ pPeer,
	FTM_VOID_PTR	pData,
	FTOM_SESSION_PTR _PTR_ ppSession
);

FTM_RET FTOM_SESSION_destroy
(
	FTOM_SESSION_PTR _PTR_ ppSession
);
#endif
