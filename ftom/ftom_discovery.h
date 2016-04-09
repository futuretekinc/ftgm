#ifndef	_FTOM_DISCOVERY_H_
#define	_FTOM_DISCOVERY_H_

#include "ftom.h"

typedef	struct FTOM_DISCOVERY_INFO_STRUCT
{
	FTM_CHAR		pName[FTM_DEVICE_NAME_LEN + 1];
	FTM_CHAR		pDID[FTM_DID_LEN + 1];
	FTM_ULONG		ulCount;
	FTM_EP_TYPE		pEPTypes[];
}	FTOM_DISCOVERY_INFO, _PTR_ FTOM_DISCOVERY_INFO_PTR;

typedef	struct
{
	FTOM_PTR			pOM;
	pthread_t			xThread;

	FTM_BOOL			bStop;
	FTM_BOOL			bInProgress;
	FTM_LIST			xInfoList;
	FTM_LIST			xNodeList;
	FTM_LIST			xEPList;
	FTOM_MSG_QUEUE_PTR	pMsgQ;

	FTM_ULONG			ulTimeout;	
	FTM_ULONG			ulRetryCount;	
	FTM_ULONG			ulLoopCount;	
	FTM_TIMER			xTimer;
	FTOM_ON_MESSAGE_CALLBACK	fOldCB;
	FTM_VOID_PTR				pOldData;
}	FTOM_DISCOVERY, _PTR_ FTOM_DISCOVERY_PTR;

FTM_RET	FTOM_DISCOVERY_init
(
	FTOM_DISCOVERY_PTR	pDiscovery,
	FTOM_PTR			pOM
);

FTM_RET	FTOM_DISCOVERY_final
(
	FTOM_DISCOVERY_PTR	pDiscovery
);

FTM_RET	FTOM_DISCOVERY_start
(
	FTOM_DISCOVERY_PTR	pDiscovery
);

FTM_RET	FTOM_DISCOVERY_stop
(
	FTOM_DISCOVERY_PTR	pDiscovery
);

FTM_RET	FTOM_DISCOVERY_call
(
	FTOM_DISCOVERY_PTR	pDiscovery,
	FTM_CHAR_PTR		pNetwork,
	FTM_USHORT			usPort
);

#endif
