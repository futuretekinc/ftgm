#ifndef	__FTDM_CLIENT_H__
#define	__FTDM_CLIENT_H__

#include "ftdm_type.h"
#include "ftdm_cmd.h"
#include "ftdm_params.h"

typedef struct 
{
	FTM_INT	hSock;
	FTM_INT	nTimeout;
}	FTDM_CLIENT, * FTDM_CLIENT_PTR;

typedef	FTDM_CLIENT_PTR	FTDM_CLIENT_HANDLE, _PTR_ FTDM_CLIENT_HANDLE_PTR;

FTM_RET FTDMC_connect
(
	FTM_IP_ADDR			xIP,
	FTM_USHORT 			nPort, 
	FTDM_CLIENT_HANDLE_PTR	phClient
);

FTM_RET FTDMC_disconnect
(
	FTDM_CLIENT_HANDLE		hClient
);

FTM_RET FTDMC_isConnected
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_BOOL_PTR			pbConnected
);

FTM_RET FTDMC_createNode
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_NODE_INFO_PTR		pInfo
);

FTM_RET FTDMC_destroyNode
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_CHAR_PTR			pDID
);

FTM_RET FTDMC_getNodeCount
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_ULONG_PTR			pnCount
);

FTM_RET FTDMC_getNodeInfoByIndex
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_ULONG				nIndex,
	FTM_NODE_INFO_PTR		pInfo
);

FTM_RET FTDMC_getNodeInfo
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_CHAR_PTR			pDID,
	FTM_NODE_INFO_PTR		pInfo
);

FTM_RET FTDMC_createEP
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_EP_INFO_PTR			pInfo
);

FTM_RET FTDMC_destroyEP
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_EPID				xEPID
);

FTM_RET FTDMC_getEPCount
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_ULONG_PTR			pnCount
);

FTM_RET FTDMC_getEPInfo
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_EPID				xEPID,
	FTM_EP_INFO_PTR		pInfo
);

FTM_RET FTDMC_getEPInfoByIndex
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_ULONG				nIndex,
	FTM_EP_INFO_PTR		pInfo
);

FTM_RET FTDMC_appendEPData
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_EPID				xEPID,
	FTM_ULONG				nTime,
	FTM_ULONG				nValue
);

FTM_RET FTDMC_getEPData
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_EPID_PTR			pEPID,
	FTM_ULONG				nEPIDCount,
	FTM_ULONG				nBeginTime,
	FTM_ULONG				nEndTime,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG				nMaxCount,
	FTM_ULONG_PTR			pnCount
);

FTM_RET FTDMC_removeEPData
(
	FTDM_CLIENT_HANDLE		hClient,
	FTM_EPID_PTR			pEPID,
	FTM_ULONG				nEPIDCount,
	FTM_ULONG				nBeginTime,
	FTM_ULONG				nEndTime,
	FTM_ULONG				nCount
);

#endif

