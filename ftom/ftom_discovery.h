#ifndef	_FTOM_DISCOVERY_H_
#define	_FTOM_DISCOVERY_H_

#include "ftom.h"

typedef	struct FTOM_DISCOVERY_INFO_STRUCT
{
	FTM_CHAR		pName[FTM_DEVICE_NAME_LEN + 1];
	FTM_CHAR		pDID[FTM_DID_LEN + 1];
	FTM_CHAR		pIP[32];
	FTM_ULONG		ulCount;
	FTM_EP_TYPE		pEPTypes[];
}	FTOM_DISCOVERY_INFO, _PTR_ FTOM_DISCOVERY_INFO_PTR;

typedef	struct FTOM_DISCOVERY_STRUCT
{
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
	FTM_CHAR			pTargetIP[256];
	FTM_USHORT			usTargetPort;
	FTOM_ON_MESSAGE_CALLBACK	fOldCB;
	FTM_VOID_PTR				pOldData;
}	FTOM_DISCOVERY, _PTR_ FTOM_DISCOVERY_PTR;

FTM_RET	FTOM_DISCOVERY_create
(
	FTOM_DISCOVERY_PTR _PTR_ 	ppDiscovery
);

FTM_RET	FTOM_DISCOVERY_destroy
(
	FTOM_DISCOVERY_PTR _PTR_ 	ppDiscovery
);

FTM_RET	FTOM_DISCOVERY_init
(
	FTOM_DISCOVERY_PTR	pDiscovery
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

FTM_RET	FTOM_DISCOVERY_startSearch
(
	FTOM_DISCOVERY_PTR	pDiscovery,
	FTM_CHAR_PTR		pNetwork,
	FTM_USHORT			usPort,
	FTM_ULONG			ulRetryCount
);

FTM_RET	FTOM_DISCOVERY_isFinished
(
	FTOM_DISCOVERY_PTR	pDiscovery,
	FTM_BOOL_PTR		pbFinished
);

FTM_RET	FTOM_DISCOVERY_getNodeInfoCount
(
	FTOM_DISCOVERY_PTR	pDiscovery,
	FTM_ULONG_PTR		pulCount
);

FTM_RET	FTOM_DISCOVERY_getNodeInfoAt
(
	FTOM_DISCOVERY_PTR	pDiscovery,
	FTM_ULONG			ulIndex,
	FTM_NODE_PTR		pNodeInfo
);

FTM_RET	FTOM_DISCOVERY_getNodeInfo
(
	FTOM_DISCOVERY_PTR	pDiscovery,
	FTM_CHAR_PTR		pNodeID,
	FTM_NODE_PTR		pNodeInfo
);

FTM_RET	FTOM_DISCOVERY_getEPInfoCount
(
	FTOM_DISCOVERY_PTR	pDiscovery,
	FTM_ULONG_PTR		pulCount
);

FTM_RET	FTOM_DISCOVERY_getEPInfoAt
(
	FTOM_DISCOVERY_PTR	pDiscovery,
	FTM_ULONG			ulIndex,
	FTM_EP_PTR			pEPInfo
);

FTM_RET	FTOM_DISCOVERY_getEPInfo
(
	FTOM_DISCOVERY_PTR	pDiscovery,
	FTM_CHAR_PTR		pEPID,
	FTM_EP_PTR			pEPInfo
);

#endif
