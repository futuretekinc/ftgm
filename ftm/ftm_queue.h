#ifndef	__FTM_QUEUE_H__
#define __FTM_QUEUE_H__

#include "ftm_types.h"
#include "ftm_list.h"

typedef	struct FTM_QUEUE_STRUCT
{
	FTM_LIST		xList;
} FTM_QUEUE, _PTR_ FTM_QUEUE_PTR;


FTM_RET FTM_QUEUE_create(FTM_QUEUE_PTR _PTR_ ppQueue);
FTM_RET	FTM_QUEUE_destroy(FTM_QUEUE_PTR pQueue);

FTM_RET FTM_QUEUE_init(FTM_QUEUE_PTR pQueue);
FTM_RET	FTM_QUEUE_final(FTM_QUEUE_PTR pQueue);

FTM_RET	FTM_QUEUE_count(FTM_QUEUE_PTR pQueue, FTM_ULONG_PTR pulCount);
FTM_RET	FTM_QUEUE_push(FTM_QUEUE_PTR pQueue, FTM_VOID_PTR pItem);
FTM_RET	FTM_QUEUE_pop(FTM_QUEUE_PTR pQueue, FTM_VOID_PTR _PTR_ ppItem);

#endif

