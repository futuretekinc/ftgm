#ifndef	__FTDM_EP_H__
#define	__FTDM_EP_H__

#include "ftdm_config.h"

typedef	struct
{
	FTM_EP		xInfo;
	FTM_ULONG	ulCount;
	FTM_ULONG	ulFirstTime;
	FTM_ULONG	ulLastTime;
}	FTDM_EP, _PTR_ FTDM_EP_PTR;
	
FTM_RET	FTDM_EP_init
(
	FTM_VOID
);

FTM_RET FTDM_EP_final
(
	FTM_VOID
);

FTM_RET	FTDM_EP_loadConfig
(
	FTDM_CFG_EP_PTR	pConfig
);

FTM_RET	FTDM_EP_loadFromFile
(
	FTM_CHAR_PTR	pFileName
);

FTM_RET	FTDM_EP_loadFromDB
(
	FTM_VOID
);

FTM_RET	FTDM_EP_saveToDB
(
	FTM_VOID
);

FTM_RET	FTDM_EP_create
(
	FTM_EP_PTR 		pInfo
);

FTM_RET	FTDM_EP_destroy
(
	FTM_EP_ID 		xEPID
);

FTM_RET	FTDM_EP_count
(
	FTM_EP_TYPE		xType,
	FTM_ULONG_PTR	pnCount
);

FTM_RET	FTDM_EP_get
(
	FTM_EP_ID			xEPID,
	FTDM_EP_PTR	_PTR_ 	ppEP
);

FTM_RET	FTDM_EP_getAt
(
	FTM_ULONG			ulIndex,
	FTDM_EP_PTR	_PTR_ 	ppEP
);

FTM_RET	FTDM_EP_showList
(
	FTM_VOID
);


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
	FTM_INT				nIndex, 
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
