#ifndef	__FTM_OM_EP_CLASS_H__
#define	__FTM_OM_EP_CLASS_H__

#include <pthread.h>
#include "ftm.h"
#include "ftm_om_node.h"

FTM_RET	FTM_OM_EP_CLASS_init
(
	FTM_VOID
);

FTM_RET FTM_OM_EP_CLASS_final
(
	FTM_VOID
);

FTM_RET	FTM_OM_EP_CLASS_create
(
	FTM_EP_CLASS_PTR 	pInfo
);

FTM_RET	FTM_OM_EP_CLASS_destroy
(
	FTM_EP_CLASS_PTR 	pEP
);

FTM_RET	FTM_OM_EP_CLASS_count
(
	FTM_ULONG_PTR 		pulCount
);

FTM_RET	FTM_OM_EP_CLASS_setNode
(	
	FTM_EP_CLASS_PTR 	pEP, 
	FTM_OM_NODE_PTR 	pNode
);

FTM_RET FTM_OM_EP_CLASS_get
(
	FTM_EP_TYPE 			xType, 
	FTM_EP_CLASS_PTR _PTR_ 	ppEP
);

FTM_RET FTM_OM_EP_CLASS_getAt
(
	FTM_ULONG 				ulIndex, 
	FTM_EP_CLASS_PTR _PTR_ 	ppEP
);

#endif

