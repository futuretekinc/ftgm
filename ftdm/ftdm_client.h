#ifndef	__FTDM_CLIENT_H__
#define	__FTDM_CLIENT_H__

#include "ftdm_type.h"
#include "ftdm_cmd.h"
#include "ftdm_params.h"
#include "ftdm_client_config.h"

typedef struct 
{
	FTM_INT	hSock;
	FTM_INT	nTimeout;
}	FTDMC_SESSION, _PTR_ FTDMC_SESSION_PTR;

FTM_RET	FTDMC_init
(
	FTDMC_CFG_PTR pConfig
);

FTM_RET FTDMC_final
(
);

FTM_RET FTDMC_connect
(
	FTDMC_SESSION_PTR 		pSession,
	FTM_IP_ADDR				xIP,
	FTM_USHORT 				usPort 
);

FTM_RET FTDMC_disconnect
(
	FTDMC_SESSION_PTR		pSession
);

FTM_RET FTDMC_isConnected
(
	FTDMC_SESSION_PTR		pSession,
	FTM_BOOL_PTR			pbConnected
);

FTM_RET FTDMC_NODE_INFO_append
(
	FTDMC_SESSION_PTR		pSession,
	FTM_NODE_INFO_PTR		pInfo
);

FTM_RET FTDMC_NODE_INFO_remove
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pDID
);

FTM_RET FTDMC_NODE_INFO_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pnCount
);

FTM_RET FTDMC_NODE_INFO_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				nIndex,
	FTM_NODE_INFO_PTR		pInfo
);

FTM_RET FTDMC_NODE_INFO_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pDID,
	FTM_NODE_INFO_PTR		pInfo
);

FTM_RET FTDMC_EP_INFO_append
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_INFO_PTR			pInfo
);

FTM_RET FTDMC_EP_INFO_remove
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID
);

FTM_RET FTDMC_EP_INFO_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_CLASS			xClass,
	FTM_ULONG_PTR			pnCount
);

FTM_RET FTDMC_EP_INFO_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_EP_INFO_PTR			pEPInfo
);

FTM_RET FTDMC_EP_INFO_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				nIndex,
	FTM_EP_INFO_PTR			pEPInfo
);

FTM_RET	FTDMC_EP_CLASS_INFO_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pnCount
);

FTM_RET	FTDMC_EP_CLASS_INFO_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_CLASS			xClass,
	FTM_EP_CLASS_INFO_PTR	pEPClassInfo
);

FTM_RET	FTDMC_EP_CLASS_INFO_getAt
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				nIndex,
	FTM_EP_CLASS_INFO_PTR	pInfo
);

FTM_RET FTDMC_EP_DATA_append
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_EP_DATA_PTR			pEPData
);

FTM_RET FTDMC_EP_DATA_remove
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_ULONG				nIndex,
	FTM_ULONG				nCount
);

FTM_RET FTDMC_EP_DATA_get
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_ULONG				nStartIndex,
	FTM_EP_DATA_PTR			pData,
	FTM_ULONG				nMaxCount,
	FTM_ULONG_PTR			pnCount
);

FTM_RET FTDMC_EP_DATA_getWithTime
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_ULONG				nBeginTime,
	FTM_ULONG				nEndTime,
	FTM_EP_DATA_PTR			pData,
	FTM_ULONG				nMaxCount,
	FTM_ULONG_PTR			pnCount
);

FTM_RET FTDMC_EP_DATA_removeWithTime
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_ULONG				nBeginTime,
	FTM_ULONG				nEndTime
);

FTM_RET	FTDMC_EP_DATA_count
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_ULONG_PTR			pCount
);

FTM_RET	FTDMC_EP_DATA_countWithTime
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_ULONG				nBeginTime,
	FTM_ULONG				nEndTime,
	FTM_ULONG_PTR			pCount
);

#endif

