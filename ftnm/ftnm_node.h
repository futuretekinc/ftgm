#ifndef	__FTNM_NODE_H__
#define	__FTNM_NODE_H__

#include "ftm_types.h"
#include "ftm_object.h"
#include "ftm_list.h"
#include "ftnm_object.h"
#include <pthread.h>
	
FTM_RET FTNM_NODE_MNGR_init(FTM_VOID);
FTM_RET FTNM_NODE_MNGR_final(FTM_VOID);

FTM_RET	FTNM_NODE_create(FTM_NODE_INFO_PTR pInfo, FTNM_NODE_PTR _PTR_ ppNode);
FTM_RET	FTNM_NODE_destroy(FTNM_NODE_PTR	pNode);

FTM_RET FTNM_NODE_get(FTM_CHAR_PTR pDID, FTNM_NODE_PTR _PTR_ ppNode);

FTM_RET	FTNM_NODE_linkEP(FTNM_NODE_PTR pNode, FTNM_EP_PTR pEP);
FTM_RET	FTNM_NODE_unlinkEP(FTNM_NODE_PTR pNode, FTNM_EP_PTR pEP);

FTM_RET	FTNM_NODE_start(FTNM_NODE_PTR pNode);
FTM_RET	FTNM_NODE_stop(FTNM_NODE_PTR pNode);
FTM_RET	FTNM_NODE_restart(FTNM_NODE_PTR pNode);
#endif

