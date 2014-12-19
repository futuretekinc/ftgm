#ifndef	__SQLITE_IF_H__
#define	__SQLITE_IF_H__

FTDM_RET	FTDM_DBIF_init
(
	void
);

FTDM_RET	FTDM_DBIF_final
(
	void
);

FTDM_RET	FTDM_DBIF_initDeviceInfoTable
(
	void
);

FTDM_RET	FTDM_DBIF_insertDeviceInfo
(
	FTDM_BYTE_PTR			pDID,
	FTDM_DEVICE_TYPE 		xType,
	FTDM_BYTE_PTR			pURL,
	FTDM_INT				nURLLen,
	FTDM_BYTE_PTR			pLocation,
	FTDM_INT				nLocationLen
);

FTDM_RET	FTDM_DBIF_removeDeviceInfo
(
	FTDM_BYTE_PTR			pDID
);

FTDM_RET	FTDM_DBIF_getDeviceInfo
(
	FTDM_BYTE_PTR			pDID,
	FTDM_DEVICE_TYPE_PTR	pType,
	FTDM_CHAR_PTR			pURL,
	FTDM_INT_PTR			pURLLen,
	FTDM_CHAR_PTR			pLoc,
	FTDM_INT_PTR			pLocLen
);

FTDM_RET	FTDM_DBIF_setDeviceURL
(
	FTDM_BYTE_PTR			pDID,
	FTDM_CHAR_PTR			pURL,
	FTDM_INT				nURLLen
);

FTDM_RET	FTDM_DBIF_getDeviceURL
(
	FTDM_BYTE_PTR			pDID,
	FTDM_CHAR_PTR			pURL,
	FTDM_INT_PTR			pURLLen
);

FTDM_RET	FTDM_DBIF_setDeviceLocation
(
	FTDM_BYTE_PTR			pDID,
	FTDM_CHAR_PTR			pLocation,
	FTDM_INT				nLocationLen
);

FTDM_RET	FTDM_DBIF_getDeviceLocation
(
	FTDM_BYTE_PTR			pDID,
	FTDM_CHAR_PTR			pLocation,
	FTDM_INT_PTR			pLocationLen
);

FTDM_RET	FTDM_DBIF_initEPInfoTable
(
	void
);

FTDM_RET	FTDM_DBIF_insertEPInfo
(
	FTDM_EP_ID 				xEPID, 
	FTDM_EP_TYPE 			xType,
	FTDM_BYTE_PTR			pDID,
	FTDM_CHAR_PTR			pName,
	FTDM_INT				nNameLen,
	FTDM_ULONG				nInterval,
	FTDM_CHAR_PTR			pUnit,
	FTDM_INT				nUnitLen,
	FTDM_BYTE_PTR			pParentID
);

FTDM_RET	FTDM_DBIF_removeEPInfo
(
	FTDM_EP_ID				xEPID
);

FTDM_RET	FTDM_DBIF_getEPInfo
(
	FTDM_EP_ID 				xEPID, 
	FTDM_EP_TYPE_PTR		pType,
	FTDM_BYTE_PTR			pDID,
	FTDM_CHAR_PTR			pName,
	FTDM_INT_PTR			pNameLen,
	FTDM_ULONG_PTR			pInterval,
	FTDM_CHAR_PTR			pUnit,
	FTDM_INT_PTR			pUnitLen,
	FTDM_BYTE_PTR			pParentID
);

FTDM_RET	FTDM_DBIF_setEPName
(
	FTDM_EP_ID				xEPID,
	FTDM_CHAR_PTR			pName,
	FTDM_INT				nNameLen
);

FTDM_RET	FTDM_DBIF_getEPName
(
	FTDM_EP_ID				xEPID,
	FTDM_CHAR_PTR			pName,
	FTDM_INT_PTR			pNameLen
);

FTDM_RET	FTDM_DBIF_setEPInterval
(
	FTDM_EP_ID				xEPID,
	FTDM_ULONG				nInterval
);

FTDM_RET	FTDM_DBIF_getEPInterval
(
	FTDM_EP_ID				xEPID,
	FTDM_ULONG_PTR			pInterval
);

FTDM_RET	FTDM_DBIF_setEPUnit
(
	FTDM_EP_ID				xEPID,
	FTDM_CHAR_PTR			pUnit,
	FTDM_INT				nUnitLen
);

FTDM_RET	FTDM_DBIF_getEPUnit
(
	FTDM_EP_ID				xEPID,
	FTDM_CHAR_PTR			pUnit,
	FTDM_INT_PTR			pUnitLen
);


FTDM_RET	FTDM_DBIF_initEPLogTable
(
	FTDM_VOID
);

FTDM_RET	FTDM_DBIF_appendEPLog
(
	FTDM_EP_ID				xEPID,
	FTDM_ULONG				xTime,
	FTDM_ULONG				nValue
);

FTDM_RET	FTDM_DBIF_EPLogCount
(
	FTDM_ULONG				xBeginTime,
	FTDM_ULONG				xEndTime,
	FTDM_EP_ID_PTR			pEPID,
	FTDM_ULONG				nEPID,
	FTDM_ULONG_PTR			pCount
);

#endif
