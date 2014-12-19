#include "ftdm.h"
#include "sqlite_if.h"
#include "debug.h"

FTDM_RET 	FTDM_init(void)
{
	if ((FTDM_DBIF_init() != FTDM_RET_OK) ||
		(FTDM_DBIF_initDeviceInfoTable() != FTDM_RET_OK) ||
		(FTDM_DBIF_initEPInfoTable() != FTDM_RET_OK) ||
		(FTDM_DBIF_initEPLogTable() != FTDM_RET_OK))
	{
		TRACE("FTDM initialization failed.\n");

		return	FTDM_RET_ERROR;
	
	}

	TRACE("FTDM initialization completed successfully.\n");

	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_final(void)
{
	if (FTDM_DBIF_final() != FTDM_RET_OK)
	{
		TRACE("FTDM finalization failed.\n");

		return	FTDM_RET_OK;
	}

	TRACE("FTDM finalization completed successfully.\n");

	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_createDevice
(
	FTDM_BYTE_PTR		pDID,
	FTDM_DEVICE_TYPE 	xType, 
	FTDM_BYTE_PTR		pURL,
	FTDM_INT			nURLLen,
	FTDM_BYTE_PTR		pLocation,
	FTDM_INT			nLocationLen
)
{
	return	FTDM_RET_OK;
}

FTDM_RET 	FTDM_destroyDevice
(
	FTDM_BYTE_PTR			pDID
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_getDeviceInfo
(
	FTDM_BYTE_PTR			pDID,
	FTDM_DEVICE_INFO_PTR 	pInfo
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_getDeviceType
(
	FTDM_BYTE_PTR			pDID,
	FTDM_DEVICE_TYPE_PTR 	pType
)
{
	return	FTDM_RET_OK;
}

FTDM_RET 	FTDM_getDeviceURL
(
	FTDM_BYTE_PTR			pDID, 
	FTDM_BYTE_PTR			pURL,
	FTDM_INT_PTR 			pURLLen
)
{
	return	FTDM_RET_OK;
}

FTDM_RET 	FTDM_setDeviceURL
(
	FTDM_BYTE_PTR			pDID, 
	FTDM_BYTE_PTR			pURL,
	FTDM_INT 				nURLLen
)
{
	return	FTDM_RET_OK;
}

FTDM_RET 	FTDM_getDeviceLocation
(
	FTDM_BYTE_PTR			pDID, 
	FTDM_BYTE_PTR			pLocation,
	FTDM_INT_PTR 			pLocationLen
)
{
	return	FTDM_RET_OK;
}

FTDM_RET 	FTDM_setDeviceLocation
(
	FTDM_BYTE_PTR			pDID, 
	FTDM_BYTE_PTR			pLocation,
	FTDM_INT 				nLocationLen
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_createEP
(
	FTDM_EP_ID 			xEPID, 
	FTDM_EP_INFO_PTR 	pInfo
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_destroyEP
(
	FTDM_EP_ID 			xEPID
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_getEPInfo
(
	FTDM_EP_ID			xEPID,
	FTDM_EP_INFO_PTR	pInfo
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_appendEPData
(
	FTDM_EP_ID 			xEPID, 
	FTDM_ULONG			nTime, 
	FTDM_ULONG 			nValue
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_getEPData
(
	FTDM_EP_ID 			xEPID, 
	FTDM_ULONG 			nBeginTime, 
	FTDM_ULONG 			nEndTime, 
	FTDM_ULONG_PTR		pnCount, 
	FTDM_EP_DATA_PTR 	pData
)
{
	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_removeEPData
(
	FTDM_EP_ID 			xEPID, 
	FTDM_ULONG 			nBeginTime, 
	FTDM_ULONG 			nEndTime, 
	FTDM_ULONG_PTR		nCount
) 
{
	return	FTDM_RET_OK;
}

