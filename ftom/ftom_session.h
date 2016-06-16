#ifndef	__FTOM_SESSION_H__
#define	__FTOM_SESSION_H__

#include <semaphore.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "ftm.h"
#include "ftom.h"

typedef	struct
{
	FTM_INT				hSocket;
	struct sockaddr_in	xPeer;
	pthread_t			xPThread;
	FTM_VOID_PTR		pData;
	FTM_BOOL			bStop;
	FTM_ULONG			ulReqID;
	FTM_BYTE			pReqBuff[FTOM_DEFAULT_PACKET_SIZE];
	FTM_BYTE			pRespBuff[FTOM_DEFAULT_PACKET_SIZE];
}	FTOM_SESSION, _PTR_ FTOM_SESSION_PTR;

#endif
