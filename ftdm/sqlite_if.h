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

FTDM_RET	FTDM_DBIF_getDeviceCount
(
	FTDM_ULONG_PTR		pCount
);

FTDM_RET	FTDM_DBIF_getDeviceList
(
	FTDM_DEVICE_INFO_PTR	pInfos,
	FTDM_ULONG				nMaxCount,
	FTDM_ULONG_PTR			pCount
);

FTDM_RET	FTDM_DBIF_insertDeviceInfo
(
 	FTDM_DEVICE_INFO_PTR	pInfo
);

FTDM_RET	FTDM_DBIF_removeDeviceInfo
(
	FTDM_CHAR_PTR			pDID
);

FTDM_RET	FTDM_DBIF_getDeviceInfo
(
	FTDM_CHAR_PTR			pDID,
	FTDM_DEVICE_INFO_PTR	pInfo
);

FTDM_RET	FTDM_DBIF_setDeviceURL
(
	FTDM_CHAR_PTR			pDID,
	FTDM_CHAR_PTR			pURL
);

FTDM_RET	FTDM_DBIF_getDeviceURL
(
	FTDM_CHAR_PTR			pDID,
	FTDM_CHAR_PTR			pBuff,
	FTDM_ULONG				nBuffLen
);

FTDM_RET	FTDM_DBIF_setDeviceLocation
(
	FTDM_CHAR_PTR			pDID,
	FTDM_CHAR_PTR			pLocation
);

FTDM_RET	FTDM_DBIF_getDeviceLocation
(
	FTDM_CHAR_PTR			pDID,
	FTDM_CHAR_PTR			pBuff,
	FTDM_ULONG				nBuffLen
);

FTDM_RET	FTDM_DBIF_initEPInfoTable
(
	void
);

FTDM_RET	FTDM_DBIF_insertEPInfo
(
 	FTDM_EP_INFO_PTR		pInfo
);

FTDM_RET	FTDM_DBIF_removeEPInfo
(
	FTDM_EP_ID				xEPID
);

FTDM_RET	FTDM_DBIF_getEPCount
(
	FTDM_ULONG_PTR			pCount
);

FTDM_RET	FTDM_DBIF_getEPList
(
	FTDM_EP_INFO_PTR		pInfos,
	FTDM_ULONG				nMaxCount,
	FTDM_ULONG_PTR			pCount
);

FTDM_RET	FTDM_DBIF_getEPInfo
(
	FTDM_EP_ID 				xEPID, 
	FTDM_EP_INFO_PTR		pInfo
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


FTDM_RET	FTDM_DBIF_initEPDataTable
(
	FTDM_VOID
);

FTDM_RET	FTDM_DBIF_appendEPData
(
	FTDM_EP_ID				xEPID,
	FTDM_ULONG				xTime,
	FTDM_ULONG				nValue
);

FTDM_RET	FTDM_DBIF_EPDataCount
(
	FTDM_EP_ID_PTR			pEPID,
	FTDM_ULONG				nEPID,
	FTDM_ULONG				xBeginTime,
	FTDM_ULONG				xEndTime,
	FTDM_ULONG_PTR			pCount
);

FTDM_RET	FTDM_DBIF_getEPData
(
	FTDM_EP_ID_PTR			pEPID,
	FTDM_ULONG				nEPIDCount,
	FTDM_ULONG				xBeginTime,
	FTDM_ULONG				xEndTime,
	FTDM_EP_DATA_PTR		pEPData,
	FTDM_ULONG				nMaxCount,
	FTDM_ULONG_PTR			pCount
);

FTDM_RET	FTDM_DBIF_removeEPData
(
	FTDM_EP_ID_PTR			pEPID,
	FTDM_ULONG				nEPIDCount,
	FTDM_ULONG				xBeginTime,
	FTDM_ULONG				xEndTime,
	FTDM_ULONG				nCount
);

#endif
