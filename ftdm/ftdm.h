#ifndef	__FTDM_H__
#define	__FTDM_H__

#include "ftdm_type.h"
#include "ftdm_cmd.h"

FTDM_RET 	FTDM_init(void);
FTDM_RET	FTDM_final(void);

FTDM_RET	FTDM_createDevice
(
	FTDM_BYTE_PTR		pDID,
	FTDM_DEVICE_TYPE 	xType, 
	FTDM_BYTE_PTR		pURL,
	FTDM_INT			nURLLen,
	FTDM_BYTE_PTR		pLocation,
	FTDM_INT			nLocationLen
);
FTDM_RET 	FTDM_destroyDevice
(
	FTDM_BYTE_PTR		pDID
);

FTDM_RET	FTDM_getDeviceInfo
(
	FTDM_BYTE_PTR			pDID,
	FTDM_DEVICE_INFO_PTR 	pInfo
);

FTDM_RET	FTDM_getDeviceType
(
	FTDM_BYTE_PTR			pDID,
	FTDM_DEVICE_TYPE_PTR 	pType
);

FTDM_RET 	FTDM_getDeviceURL
(
	FTDM_BYTE_PTR			pDID,
	FTDM_BYTE_PTR			pURL,
	FTDM_INT_PTR			nURLLen
);

FTDM_RET 	FTDM_setDeviceURL
(
	FTDM_BYTE_PTR			pDID,
	FTDM_BYTE_PTR			pURL,
	FTDM_INT 				nURLLen
);

FTDM_RET 	FTDM_getDeviceLocation
(
	FTDM_BYTE_PTR			pDID,
	FTDM_BYTE_PTR			pLocation, 
	FTDM_INT_PTR			pLocationLen
);

FTDM_RET 	FTDM_setDeviceLocation
(
	FTDM_BYTE_PTR			pDID,
	FTDM_BYTE_PTR			pLocation, 
	FTDM_INT				nLocationLen
);

FTDM_RET	FTDM_createEP
(
	FTDM_EP_ID 			xEPID, 
	FTDM_EP_INFO_PTR 	pInfo
);

FTDM_RET	FTDM_destroyEP
(
	FTDM_EP_ID 			xEPID
);

FTDM_RET	FTDM_getEPInfo
(
	FTDM_EP_ID			xEPID,
	FTDM_EP_INFO_PTR	pInfo
);

FTDM_RET	FTDM_appendEPData
(
	FTDM_EP_ID 			xEPID, 
	FTDM_ULONG			nTime, 
	FTDM_ULONG 			nValue
);

FTDM_RET	FTDM_epData
(
	FTDM_EP_ID 			xEPID, 
	FTDM_ULONG 			nBeginTime, 
	FTDM_ULONG 			nEndTime, 
	FTDM_ULONG_PTR		pnCount, 
	FTDM_EP_DATA_PTR 	pData
);

FTDM_RET	FTDM_removeEPData
(
	FTDM_EP_ID 			xEPID, 
	FTDM_ULONG 			nBeginTime, 
	FTDM_ULONG 			nEndTime, 
	FTDM_ULONG_PTR		nCount
); 
#endif

