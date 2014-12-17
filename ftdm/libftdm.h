#ifndef	__LIB_FTDM_H__
#define	__LIB_FTDM_H__

FTGM_RET 	FTDM_init(void);
FTGM_RET	FTDM_final(void);

FTGM_RET	FTDM_devInsert
(
	FTGM_DEVICE_ID 		xDID, 
	FTGM_DEVICE_TYPE 	xType, 
	FTGM_STRING 		strURL, 
	FTGM_STRING 		strLocation
);
FTGM_RET 	FTDM_devRemove
(
	FTGM_DEVICE_ID 			xDID
);

FTGM_RET	FTDM_devInfo
(
	FTGM_DEVICE_ID 			xDID, 
	FTGM_DEVICE_INFO_PTR 	pInfo
);

FTGM_RET	FTDM_devType
(
	FTGM_DEVICE_ID 			xDID, 
	FTGM_DEVICE_TYPE_PTR 	pType
);

FTGM_RET 	FTDM_devURL
(
	FTGM_DEVICE_ID 		xDID, 
	FTGM_STRING 		strBuff, 
	FTGM_INT 			nBuffLen
);

FTGM_RET 	FTDM_devURLSet
(
	FTGM_DEVICE_ID 		xDID, 
	FTGM_STRING 		strBuff
);

FTGM_RET 	FTDM_devLocation
(
	FTGM_DEVICE_ID 		xDID, 
	FTGM_STRING 		strBuff, 
	FTGM_INT 			nBuffLen
);

FTGM_RET 	FTDM_devLocationSet
(
	FTGM_DEVICE_ID 		xDID, 
	FTGM_STRING 		strBuff
);

FTGM_RET	FTDM_epInsert
(
	FTGM_EP_ID 			xEPID, 
	FTGM_EP_INFO_PTR 	pInfo
);

FTGM_RET	FTDM_epRemove
(
	FTGM_EP_ID 			xEPID
);

FTGM_RET	FTDM_epInfo
(
	FTGM_EP_ID			xEPID,
	FTGM_EP_INFO_PTR	pInfo
);

FTGM_RET	FTDM_epDataAppend
(
	FTGM_EP_ID 			xEPID, 
	FTGM_ULONG			nTime, 
	FTGM_ULONG 			nValue
);

FTGM_RET	FTDM_epData
(
	FTGM_EP_ID 			xEPID, 
	FTGM_ULONG 			nBeginTime, 
	FTGM_ULONG 			nEndTime, 
	FTGM_ULONG_PTR		pnCount, 
	FTGM_EP_DATA_PTR 	pData
);

FTGM_RET	FTDM_epDataRemove
(
	FTGM_EP_ID 			xEPID, 
	FTGM_ULONG 			nBeginTime, 
	FTGM_ULONG 			nEndTime, 
	FTGM_ULONG_PTR		nCount
); 
#endif

