#ifndef	__FTNM_EP_CLASS_INFO_H__
#define	__FTNM_EP_CLASS_INFO_H__

#include "ftm_types.h"
#include "ftm_object.h"
#include "ftm_list.h"
#include "ftnm_node.h"
#include <pthread.h>

FTM_RET	FTNM_EP_CLASS_INFO_init(FTM_VOID);
FTM_RET FTNM_EP_CLASS_INFO_final(FTM_VOID);

FTM_RET	FTNM_EP_CLASS_INFO_create(FTM_EP_CLASS_INFO_PTR pInfo);
FTM_RET	FTNM_EP_CLASS_INFO_destroy(FTM_EP_CLASS_INFO_PTR pEP);

FTM_RET	FTNM_EP_CLASS_INFO_count(FTM_ULONG_PTR pulCount);
FTM_RET	FTNM_EP_CLASS_INFO_setNode(FTM_EP_CLASS_INFO_PTR pEP, FTNM_NODE_PTR pNode);

FTM_RET FTNM_EP_CLASS_INFO_get(FTM_EP_CLASS xClass, FTM_EP_CLASS_INFO_PTR _PTR_ ppEP);
FTM_RET FTNM_EP_CLASS_INFO_getAt(FTM_ULONG ulIndex, FTM_EP_CLASS_INFO_PTR _PTR_ ppEP);
#endif

