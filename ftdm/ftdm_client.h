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
	FTDM_CLIENT_CONFIG_PTR pConfig
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

FTM_RET FTDMC_appendNodeInfo
(
	FTDMC_SESSION_PTR		pSession,
	FTM_NODE_INFO_PTR		pInfo
);

FTM_RET FTDMC_removeNodeInfo
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pDID
);

FTM_RET FTDMC_getNodeInfoCount
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pnCount
);

FTM_RET FTDMC_getNodeInfoByIndex
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				nIndex,
	FTM_NODE_INFO_PTR		pInfo
);

FTM_RET FTDMC_getNodeInfo
(
	FTDMC_SESSION_PTR		pSession,
	FTM_CHAR_PTR			pDID,
	FTM_NODE_INFO_PTR		pInfo
);

FTM_RET FTDMC_appendEPInfo
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EP_INFO_PTR			pInfo
);

FTM_RET FTDMC_removeEPInfo
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID
);

FTM_RET FTDMC_getEPInfoCount
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG_PTR			pnCount
);

FTM_RET FTDMC_getEPInfo
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_EP_INFO_PTR			pEPInfo
);

FTM_RET FTDMC_getEPInfoByIndex
(
	FTDMC_SESSION_PTR		pSession,
	FTM_ULONG				nIndex,
	FTM_EP_INFO_PTR			pEPInfo
);

FTM_RET FTDMC_appendEPData
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_EP_DATA_PTR			pEPData
);

FTM_RET FTDMC_getEPData
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_ULONG				nStartIndex,
	FTM_EP_DATA_PTR			pData,
	FTM_ULONG				nMaxCount,
	FTM_ULONG_PTR			pnCount
);

FTM_RET FTDMC_getEPDataWithTime
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_ULONG				nBeginTime,
	FTM_ULONG				nEndTime,
	FTM_EP_DATA_PTR			pData,
	FTM_ULONG				nMaxCount,
	FTM_ULONG_PTR			pnCount
);

FTM_RET FTDMC_removeEPData
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_ULONG				nIndex,
	FTM_ULONG				nCount
);

FTM_RET FTDMC_removeEPDataWithTime
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_ULONG				nBeginTime,
	FTM_ULONG				nEndTime
);

FTM_RET	FTDMC_getEPDataCount
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_ULONG_PTR			pCount
);

FTM_RET	FTDMC_getEPDataCountWithTime
(
	FTDMC_SESSION_PTR		pSession,
	FTM_EPID				xEPID,
	FTM_ULONG				nBeginTime,
	FTM_ULONG				nEndTime,
	FTM_ULONG_PTR			pCount
);

#endif

