#ifndef	__SQLITE_IF_H__
#define	__SQLITE_IF_H__

FTGM_RET	FTDM_DBIF_init
(
	void
);

FTGM_RET	FTDM_DBIF_final
(
	void
);

FTGM_RET	FTDM_DBIF_initDeviceInfoTable
(
	void
);

FTGM_RET	FTDM_DBIF_insertDeviceInfo
(
	FTGM_DEVICE_ID 		xDID, 
	FTGM_DEVICE_TYPE 	xType,
	FTGM_STRING			strURL,
	FTGM_STRING			strLOC
);

FTGM_RET	FTDM_DBIF_removeDeviceInfo
(
	FTGM_DEVICE_ID		xDID
);

FTGM_RET	FTDM_DBIF_getDeviceInfo
(
	FTGM_DEVICE_ID			xDID,
	FTGM_DEVICE_TYPE_PTR	pType,
	FTGM_STRING				strURL,
	FTGM_ULONG_PTR			pURLLen,
	FTGM_STRING				strLoc,
	FTGM_ULONG_PTR			pLocLen
);

FTGM_RET	FTDM_DBIF_setDeviceURL
(
	FTGM_DEVICE_ID			xDID,
	FTGM_STRING				strURL
);

FTGM_RET	FTDM_DBIF_getDeviceURL
(
	FTGM_DEVICE_ID			xDID,
	FTGM_STRING				strURL,
	FTGM_ULONG_PTR			pURLLen
);

FTGM_RET	FTDM_DBIF_setDeviceLocation
(
	FTGM_DEVICE_ID			xDID,
	FTGM_STRING				strLoc
);

FTGM_RET	FTDM_DBIF_getDeviceLocation
(
	FTGM_DEVICE_ID			xDID,
	FTGM_STRING				strLoc,
	FTGM_ULONG_PTR			pLocLen
);

FTGM_RET	FTDM_DBIF_initEndPointInfoTable
(
	void
);

FTGM_RET	FTDM_DBIF_insertEndPointInfo
(
	FTGM_EP_ID 				xEPID, 
	FTGM_EP_TYPE 			xType,
	FTGM_DEVICE_ID			xDID,
	FTGM_STRING				strName,
	FTGM_ULONG				nInterval,
	FTGM_STRING				strUnit,
	FTGM_DEVICE_ID			xParentID
);

FTGM_RET	FTDM_DBIF_removeEndPointInfo
(
	FTGM_EP_ID				xEPID
);

FTGM_RET	FTDM_DBIF_getEndPointInfo
(
	FTGM_EP_ID 				xEPID, 
	FTGM_EP_TYPE_PTR		pType,
	FTGM_DEVICE_ID			xDID,
	FTGM_STRING				strName,
	FTGM_ULONG_PTR			pNameLen,
	FTGM_ULONG_PTR			pInterval,
	FTGM_STRING				strUnit,
	FTGM_ULONG_PTR			pUnit,
	FTGM_DEVICE_ID			xParentID
);

FTGM_RET	FTDM_DBIF_setEndPointName
(
	FTGM_EP_ID				xEPID,
	FTGM_STRING				strName
);

FTGM_RET	FTDM_DBIF_getEndPointName
(
	FTGM_EP_ID				xEPID,
	FTGM_STRING				strName,
	FTGM_ULONG_PTR			pNameLen
);

FTGM_RET	FTDM_DBIF_setEndPointInterval
(
	FTGM_EP_ID				xEPID,
	FTGM_ULONG				nInterval
);

FTGM_RET	FTDM_DBIF_getEndPointInterval
(
	FTGM_EP_ID				xEPID,
	FTGM_ULONG_PTR			pInterval
);

FTGM_RET	FTDM_DBIF_setEndPointUnit
(
	FTGM_EP_ID				xEPID,
	FTGM_STRING				strUnit
);

FTGM_RET	FTDM_DBIF_getEndPointUnit
(
	FTGM_EP_ID				xEPID,
	FTGM_STRING				strUnit,
	FTGM_ULONG_PTR			pUnitLen
);


FTGM_RET	FTDM_DBIF_initEndPointLogTable(void);
FTGM_RET	FTDM_DBIF_appendEndPointLog
(
	FTGM_ULONG				xTime,
	FTGM_EP_ID				xEPID,
	FTGM_STRING				strValue
);

FTGM_RET	FTDM_DBIF_EndPointLogCount
(
	FTGM_ULONG				xBeginTime,
	FTGM_ULONG				xEndTime,
	FTGM_EP_ID_PTR			pEPID,
	FTGM_ULONG				nEPID,
	FTGM_ULONG_PTR			pCount
);

#endif
