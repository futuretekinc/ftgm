#ifndef	__FTDM_CLIENT_H__
#define	__FTDM_CLIENT_H__

#include "ftdm_type.h"
#include "ftdm_cmd.h"
#include "ftdm_params.h"

typedef struct 
{
	FTDM_INT	hSock;
	FTDM_INT	nTimeout;
}	FTDM_CLIENT, * FTDM_CLIENT_PTR;

typedef	FTDM_CLIENT_PTR	FTDM_CLIENT_HANDLE, _PTR_ FTDM_CLIENT_HANDLE_PTR;

FTDM_RET FTDMC_connect
(
	FTDM_IP_ADDR			xIP,
	FTDM_USHORT 			nPort, 
	FTDM_CLIENT_HANDLE_PTR	phClient
);

FTDM_RET FTDMC_disconnect
(
	FTDM_CLIENT_HANDLE		hClient
);

FTDM_RET FTDMC_isConnected
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_BOOL_PTR			pbConnected
);

FTDM_RET FTDMC_createDevice
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_CHAR_PTR			pDID,
	FTDM_DEVICE_TYPE		xType,
	FTDM_CHAR_PTR			pURL,
	FTDM_CHAR_PTR			pLocation
);

FTDM_RET FTDMC_destroyDevice
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_CHAR_PTR			pDID
);

FTDM_RET FTDMC_gtDeviceCount
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_ULONG_PTR			pnCount
);

FTDM_RET FTDMC_gtDeviceInfoByIndex
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_ULONG				nIndex,
	FTDM_DEVICE_INFO_PTR	pInfo
);

FTDM_RET FTDMC_getDeviceInfo
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_CHAR_PTR			pDID,
	FTDM_DEVICE_INFO_PTR	pInfo
);

FTDM_RET FTDMC_createEP
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_EP_INFO_PTR		pInfo
);

FTDM_RET FTDMC_destroyEP
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_EP_ID				xEPID
);

FTDM_RET FTDMC_getEPCount
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_ULONG_PTR			pnCount
);

FTDM_RET FTDMC_getEPInfo
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_EP_ID				xEPID,
	FTDM_EP_INFO_PTR		pInfo
);

FTDM_RET FTDMC_getEPInfoByIndex
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_ULONG				nIndex,
	FTDM_EP_INFO_PTR		pInfo
);

FTDM_RET FTDMC_appendEPData
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_EP_ID				xEPID,
	FTDM_ULONG				nTime,
	FTDM_ULONG				nValue
);

FTDM_RET FTDMC_getEPData
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_EP_ID_PTR			pEPID,
	FTDM_ULONG				nEPIDCount,
	FTDM_ULONG				nBeginTime,
	FTDM_ULONG				nEndTime,
	FTDM_EP_DATA_PTR		pData,
	FTDM_ULONG				nMaxCount,
	FTDM_ULONG_PTR			pnCount
);

FTDM_RET FTDMC_removeEPData
(
	FTDM_CLIENT_HANDLE		hClient,
	FTDM_EP_ID_PTR			pEPID,
	FTDM_ULONG				nEPIDCount,
	FTDM_ULONG				nBeginTime,
	FTDM_ULONG				nEndTime,
	FTDM_ULONG				nCount
);

#endif

