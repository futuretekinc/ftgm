#ifndef	__FTDM_EP_H__
#define	__FTDM_EP_H__

#include "ftm.h"
//#include "ftdm_config.h"

struct FTDM_CONTEXT_STRUCT;
typedef struct FTDM_EP_STRUCT _PTR_ FTDM_EP_PTR;

FTM_RET	FTDM_EP_create
(
	struct FTDM_CONTEXT_STRUCT _PTR_ pFTDM,
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

FTM_RET	FTDM_EP_init
(
	FTDM_EP_PTR	pEP
);

FTM_RET FTDM_EP_final
(
	FTDM_EP_PTR	pEP
);

FTM_RET	FTDM_EP_getInfo
(
	FTDM_EP_PTR	pEP,
	FTM_EP_PTR	pInfo
);

FTM_RET	FTDM_EP_setInfo
(
	FTDM_EP_PTR	pEP,
	FTM_EP_PTR	pInfo
);

FTM_RET	FTDM_EP_isInfoChanged
(
	FTDM_EP_PTR		pEP,
	FTM_EP_PTR		pInfo,
	FTM_BOOL_PTR	pbChanged
);

FTM_RET	FTDM_EP_setFields
(
	FTDM_EP_PTR		pEP,
	FTM_EP_FIELD	xFields,
	FTM_EP_PTR		pInfo
);

FTM_RET	FTDM_EP_getType
(
	FTDM_EP_PTR		pEP,
	FTM_EP_TYPE_PTR	pType
);

FTM_RET	FTDM_EP_getID
(
	FTDM_EP_PTR		pEP,
	FTM_CHAR_PTR	pBuff,
	FTM_ULONG		ulBuffLen
);

FTM_RET	FTDM_EP_addData
(
	FTDM_EP_PTR		pEP,
	FTM_EP_DATA_PTR	pData
);

FTM_RET	FTDM_EP_deleteData
(
	FTDM_EP_PTR		pEP,
	FTM_INT			nIndex, 
	FTM_ULONG		nCount,
	FTM_ULONG_PTR	pulCount
); 

FTM_RET	FTDM_EP_deleteDataWithTime
(
	FTDM_EP_PTR			pEP,
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime,
	FTM_ULONG_PTR		pulCount
); 

FTM_RET	FTDM_EP_getDataCount
(
	FTDM_EP_PTR			pEP,
	FTM_ULONG_PTR		pCount
); 

FTM_RET	FTDM_EP_getDataCountWithTime
(
	FTDM_EP_PTR			pEP,
	FTM_ULONG 			nBeginTime, 
	FTM_ULONG 			nEndTime,
	FTM_ULONG_PTR		pCount
);

FTM_RET FTDM_EP_getDataInfo
(
	FTDM_EP_PTR		pEP,
	FTM_ULONG_PTR	pulBeginTime,
	FTM_ULONG_PTR	pulEndTime,
	FTM_ULONG_PTR	pulCount
);

FTM_RET FTDM_EP_setDataLimit
(	
	FTDM_EP_PTR		pEP,
	FTM_EP_LIMIT_PTR	pLimit
);

FTM_RET	FTDM_EP_getData
(
	FTDM_EP_PTR		pEP,
	FTM_ULONG		nStartIndex,
	FTM_EP_DATA_PTR pData,
	FTM_ULONG		nMaxCount,
	FTM_ULONG_PTR	pnCount 
);

FTM_RET	FTDM_EP_getDataWithTime
(
	FTDM_EP_PTR		pEP,
	FTM_ULONG 		nBeginTime, 
	FTM_ULONG 		nEndTime, 
	FTM_BOOL		bAscending,
	FTM_EP_DATA_PTR pData,
	FTM_ULONG		nMaxCount,
	FTM_ULONG_PTR	pnCount 
);

FTM_INT	FTDM_EP_seeker
(
	const FTM_VOID_PTR	pElement, 
	const FTM_VOID_PTR	pKey
);

FTM_BOOL FTDM_EP_comparator
(
	const FTM_VOID_PTR pElement1,
	const FTM_VOID_PTR pElement2
);

FTM_RET	FTDM_EP_print
(
	FTDM_EP_PTR			pEP
);

FTM_RET	FTDM_EP_showData
(
	FTDM_EP_PTR 	pEP, 
	FTM_ULONG 		ulBegin, 
	FTM_ULONG 		ulCount
);

#endif
