#ifndef	__FTM_NODE_MANAGEMENT_H__
#define __FTM_NODE_MANAGEMENT_H__

#include "ftm_types.h"
#include "ftm_list.h"

typedef	struct
{
	FTM_LIST_PTR	pList;
}	FTM_NODEM, _PTR_ FTM_NODEM_PTR;


FTM_RET	FTM_NODEM_init
(
	FTM_NODEM_PTR pNodeM
);

FTM_RET	FTM_NODEM_final
(
	FTM_NODEM_PTR pNodeM
);

FTM_RET	FTM_NODEM_append
(
	FTM_NODEM_PTR 	pNodeM, 
	FTM_NODE_PTR 	pNode
);

FTM_RET	FTM_NODEM_remove
(
	FTM_NODEM_PTR 	pNodeM, 
	FTM_NODE_PTR 	pNode
);

FTM_RET	FTM_NODEM_count
(
	FTM_NODEM_PTR 	pNodeM, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTM_NODEM_get
(
	FTM_NODEM_PTR 	pNodeM, 
	FTM_CHAR_PTR 	pDID, 
	FTM_NODE_PTR _PTR_ ppNode
);

FTM_RET	FTM_NODEM_getAt
(
	FTM_NODEM_PTR 	pNodeM, 
	FTM_ULONG 		ulIndex, 
	FTM_NODE_PTR _PTR_ ppNode
);

#endif
