#ifndef	__FTM_EP_DATA_H__
#define	__FTM_EP_DATA_H__

FTM_RET	FTDM_EP_DATA_add
(
	FTM_EP_ID			xEPID,
	FTM_EP_DATA_PTR		pData
);

FTM_RET FTDM_EP_DATA_info
(
	FTM_EP_ID			xEPID,
	FTM_ULONG_PTR		pulBeginTime,
	FTM_ULONG_PTR		pulEndTime,
	FTM_ULONG_PTR		pulCount
);

FTM_RET	FTDM_EP_DATA_get
(
	FTM_EP_ID			xEPID, 
	FTM_ULONG			nStartIndex,
	FTM_EP_DATA_PTR 	pData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pnCount 
);

FTM_RET	FTDM_EP_DATA_getWithTime
(
	FTM_EP_ID			xEPID, 
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime, 
	FTM_EP_DATA_PTR 	pData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pnCount 
);

FTM_RET	FTDM_EP_DATA_del
(
	FTM_EP_ID			xEPID, 
	FTM_ULONG 			nIndex, 
	FTM_ULONG			nCount
); 

FTM_RET	FTDM_EP_DATA_delWithTime
(
	FTM_EP_ID			xEPID, 
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime 
); 

FTM_RET	FTDM_EP_DATA_count
(
	FTM_EP_ID			xEPID, 
	FTM_ULONG_PTR		pCount
); 

FTM_RET	FTDM_EP_DATA_countWithTime
(
	FTM_EP_ID			xEPID, 
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime,
	FTM_ULONG_PTR		pCount
);

#endif
