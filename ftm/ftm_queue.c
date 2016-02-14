#include <string.h>
#include <stdlib.h>
#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_list.h"
#include "ftm_queue.h"
#include "ftm_mem.h"

FTM_RET FTM_QUEUE_create(FTM_QUEUE_PTR _PTR_ ppQueue)
{
	FTM_QUEUE_PTR	pQueue;

	pQueue = (FTM_QUEUE_PTR)malloc(sizeof(FTM_QUEUE));
	if (pQueue == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	FTM_QUEUE_init(pQueue);

	*ppQueue = pQueue;

	return	FTM_RET_OK;
}

FTM_RET FTM_QUEUE_destroy(FTM_QUEUE_PTR pQueue)
{
	ASSERT(pQueue != NULL);

	FTM_QUEUE_final(pQueue);

	free(pQueue);

	return	FTM_RET_OK;
}

FTM_RET FTM_QUEUE_init(FTM_QUEUE_PTR pQueue)
{
	ASSERT(pQueue != NULL);

	return	FTM_LIST_init(&pQueue->xList);
}

FTM_RET	FTM_QUEUE_final(FTM_QUEUE_PTR pQueue)
{
	return	FTM_LIST_final(&pQueue->xList);
}

FTM_RET	FTM_QUEUE_push(FTM_QUEUE_PTR pQueue, FTM_VOID_PTR pItem)
{
	ASSERT(pQueue != NULL);
	ASSERT(pItem != NULL);

	return	FTM_LIST_append(&pQueue->xList, pItem);
}

FTM_RET	FTM_QUEUE_pop(FTM_QUEUE_PTR pQueue, FTM_VOID_PTR _PTR_ ppItem)
{
	ASSERT(pQueue != NULL);
	ASSERT(ppItem != NULL);

	return	FTM_LIST_getAt(&pQueue->xList, 0, ppItem);
}

FTM_RET	FTM_QUEUE_count(FTM_QUEUE_PTR pQueue, FTM_ULONG_PTR pulCount)
{
	ASSERT(pQueue != NULL);
	ASSERT(pulCount != NULL);

	return	FTM_LIST_count(&pQueue->xList, pulCount);
}

