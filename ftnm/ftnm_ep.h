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

FTM_RET	FTNM_EP_count(FTM_ULONG_PTR pulCount);
FTM_RET	FTNM_EP_setNode(FTNM_EP_PTR pEP, FTNM_NODE_PTR pNode);

#endif

