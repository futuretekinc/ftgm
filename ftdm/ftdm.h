#ifndef	__FTDM_H__
#define	__FTDM_H__

#include "ftdm_type.h"
#include "ftdm_cmd.h"
#include "ftdm_debug.h"

FTDM_RET 	FTDM_init(void);
FTDM_RET	FTDM_final(void);

FTDM_RET	FTDM_createDevice
(
	FTDM_CHAR_PTR		pDID,
	FTDM_DEVICE_TYPE 	xType, 
	FTDM_CHAR_PTR		pURL,
	FTDM_CHAR_PTR		pLocation
);

FTDM_RET 	FTDM_destroyDevice
(
	FTDM_CHAR_PTR			pDID
);

FTDM_RET	FTDM_getDeviceCount
(
	FTDM_ULONG_PTR			pnCount
);

FTDM_RET	FTDM_getDeviceInfo
(
	FTDM_CHAR_PTR			pDID,
	FTDM_DEVICE_INFO_PTR 	pInfo
);

FTDM_RET	FTDM_getDeviceInfoByIndex
(
	FTDM_ULONG				nIndex,
	FTDM_DEVICE_INFO_PTR 	pInfo
);

FTDM_RET	FTDM_getDeviceType
(
	FTDM_CHAR_PTR			pDID,
	FTDM_DEVICE_TYPE_PTR 	pType
);

FTDM_RET 	FTDM_getDeviceURL
(
	FTDM_CHAR_PTR			pDID,
	FTDM_CHAR_PTR			pBuff,
	FTDM_ULONG				nBuffLen
);

FTDM_RET 	FTDM_setDeviceURL
(
	FTDM_CHAR_PTR			pDID,
	FTDM_CHAR_PTR			pURL
);

FTDM_RET 	FTDM_getDeviceLocation
(
	FTDM_CHAR_PTR			pDID,
	FTDM_CHAR_PTR			pBuff, 
	FTDM_ULONG				nBuffLen
);

FTDM_RET 	FTDM_setDeviceLocation
(
	FTDM_CHAR_PTR			pDID,
	FTDM_CHAR_PTR			pLocation 
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

FTDM_RET	FTDM_getEPCount
(
	FTDM_ULONG_PTR		pnCount
);

FTDM_RET	FTDM_getEPInfo
(
	FTDM_EP_ID			xEPID,
	FTDM_EP_INFO_PTR	pInfo
);

FTDM_RET	FTDM_getEPInfoByIndex
(
	FTDM_ULONG			nIndex,
	FTDM_EP_INFO_PTR	pInfo
);

FTDM_RET	FTDM_appendEPData
(
	FTDM_EP_ID 			xEPID, 
	FTDM_ULONG			nTime, 
	FTDM_ULONG 			nValue
);

FTDM_RET	FTDM_getEPData
(
	FTDM_EP_ID_PTR		pEPID, 
	FTDM_ULONG 			nEPID, 
	FTDM_ULONG 			nBeginTime, 
	FTDM_ULONG 			nEndTime, 
	FTDM_EP_DATA_PTR 	pData,
	FTDM_ULONG			nMaxCount,
	FTDM_ULONG_PTR		pnCount 
);

FTDM_RET	FTDM_removeEPData
(
	FTDM_EP_ID_PTR		pEPID, 
	FTDM_ULONG 			nEPID, 
	FTDM_ULONG 			nBeginTime, 
	FTDM_ULONG 			nEndTime, 
	FTDM_ULONG			nCount
); 

#endif

