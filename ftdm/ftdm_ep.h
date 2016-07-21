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
	FTDM_EP_PTR	pEP,
	FTM_EP_PTR	pInfo
);

FTM_RET FTDM_EP_final
(
	FTDM_EP_PTR	pEP
);

FTM_RET	FTDM_EP_create
(
	FTM_EP_PTR	pInfo,
	FTDM_EP_PTR	_PTR_ ppEP
);

FTM_RET	FTDM_EP_destroy
(
	FTDM_EP_PTR	_PTR_ ppEP
);

FTM_RET	FTDM_EP_destroy2
(
	FTDM_EP_PTR	_PTR_ ppEP
);

FTM_RET	FTDM_EP_get
(
	FTDM_EP_PTR	pEP,
	FTM_EP_PTR	pInfo
);

FTM_RET	FTDM_EP_set
(
	FTDM_EP_PTR	pEP,
	FTM_EP_PTR	pInfo
);

FTM_RET	FTDM_EP_DATA_add
(
	FTDM_EP_PTR			pEP,
	FTM_EP_DATA_PTR		pData
);

FTM_RET FTDM_EP_DATA_info
(
	FTDM_EP_PTR			pEP,
	FTM_ULONG_PTR		pulBeginTime,
	FTM_ULONG_PTR		pulEndTime,
	FTM_ULONG_PTR		pulCount
);

FTM_RET FTDM_EP_DATA_setLimit
(	
	FTDM_EP_PTR			pEP,
	FTM_EP_LIMIT_PTR	pLimit
);

FTM_RET	FTDM_EP_DATA_get
(
	FTDM_EP_PTR			pEP,
	FTM_ULONG			nStartIndex,
	FTM_EP_DATA_PTR 	pData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pnCount 
);

FTM_RET	FTDM_EP_DATA_getWithTime
(
	FTDM_EP_PTR			pEP,
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime, 
	FTM_BOOL			bAscending,
	FTM_EP_DATA_PTR 	pData,
	FTM_ULONG			nMaxCount,
	FTM_ULONG_PTR		pnCount 
);

FTM_RET	FTDM_EP_DATA_del
(
	FTDM_EP_PTR			pEP,
	FTM_INT				nIndex, 
	FTM_ULONG			nCount,
	FTM_ULONG_PTR		pulCount
); 

FTM_RET	FTDM_EP_DATA_delWithTime
(
	FTDM_EP_PTR			pEP,
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime,
	FTM_ULONG_PTR		pulCount
); 

FTM_RET	FTDM_EP_DATA_count
(
	FTDM_EP_PTR			pEP,
	FTM_ULONG_PTR		pCount
); 

FTM_RET	FTDM_EP_DATA_countWithTime
(
	FTDM_EP_PTR			pEP,
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime,
	FTM_ULONG_PTR		pCount
);

FTM_RET	FTDM_EP_print
(
	FTDM_EP_PTR			pEP
);

#endif
