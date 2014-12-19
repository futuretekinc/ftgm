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

FTDM_RET FTDMC_connect
(
	FTDM_IP_ADDR			xIP,
	FTDM_USHORT 			nPort, 
	FTDM_CLIENT_PTR 		pClient
);

FTDM_RET FTDMC_disconnect
(
	FTDM_CLIENT_PTR 		pClient
);

FTDM_RET FTDMC_createDevice
(
 	FTDM_CLIENT_PTR			pClient,
	FTDM_BYTE_PTR			pDID,
	FTDM_DEVICE_TYPE		xType,
	FTDM_BYTE_PTR			pURL,
	FTDM_INT				nURLLen,
	FTDM_BYTE_PTR			pLocation,
	FTDM_INT				nLocationLen
);

FTDM_RET FTDMC_destroyDevice
(
	FTDM_CLIENT_PTR			pClient,
	FTDM_BYTE_PTR			pDID
);

FTDM_RET FTDMC_getDeviceInfo
(
 	FTDM_CLIENT_PTR			pClient,
	FTDM_BYTE_PTR			pDID,
	FTDM_DEVICE_INFO_PTR	pInfo
);

#endif

