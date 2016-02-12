#ifndef	__FTNM_EP_H__
#define	__FTNM_EP_H__

#include "ftm_types.h"
#include "ftm_object.h"
#include "ftm_list.h"
#include "ftnm_object.h"
#include <pthread.h>

FTM_RET	FTNM_EP_init(FTM_VOID);
FTM_RET FTNM_EP_final(FTM_VOID);

FTM_RET	FTNM_EP_create(FTM_EP_INFO_PTR pInfo, FTNM_EP_PTR _PTR_ ppNode);
FTM_RET	FTNM_EP_destroy(FTNM_EP_PTR pEP);

FTM_RET	FTNM_EP_count(FTM_EP_CLASS xClass, FTM_ULONG_PTR pulCount);

FTM_RET FTNM_EP_getList
(
	FTM_EP_CLASS 	xClass, 
	FTM_EPID_PTR 	pEPID, 
	FTM_ULONG 		ulMaxCount, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET FTNM_EP_get(FTM_EPID xEPID, FTNM_EP_PTR _PTR_ ppEP);
FTM_RET FTNM_EP_getAt(FTM_ULONG ulIndex, FTNM_EP_PTR _PTR_ ppEP);

FTM_RET	FTNM_EP_attach(FTNM_EP_PTR pEP, FTNM_NODE_PTR pNode);
FTM_RET	FTNM_EP_detach(FTNM_EP_PTR pEP);

FTM_RET	FTNM_EP_DATA_info(FTM_EPID xEPID, FTM_ULONG_PTR pulBeginTime, FTM_ULONG_PTR pulEndTime, FTM_ULONG_PTR pulCount);
FTM_RET	FTNM_EP_DATA_count(FTM_EPID xEPID, FTM_ULONG_PTR pulCount);

#endif

