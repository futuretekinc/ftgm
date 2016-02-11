#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_list.h"
#include "ftm_mem.h"

FTM_RET FTM_LIST_create(FTM_LIST_PTR _PTR_ ppList)
{
	FTM_LIST_PTR	pList;

	pList = (FTM_LIST_PTR)FTM_MEM_malloc(sizeof(FTM_LIST));
	if (pList == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	FTM_LIST_init(pList);

	*ppList = pList;

	return	FTM_RET_OK;
}

FTM_RET FTM_LIST_destroy(FTM_LIST_PTR pList)
{
	ASSERT(pList != NULL);

	FTM_LIST_final(pList);

	FTM_MEM_free(pList);

	return	FTM_RET_OK;
}

FTM_RET FTM_LIST_init(FTM_LIST_PTR pList)
{
	ASSERT(pList != NULL);

	
	pList->xHead.pNext 	= &pList->xHead;
	pList->xHead.pPrev 	= &pList->xHead;
	pList->xHead.pData 	= NULL;

	pList->ulCount 		= 0;
	pList->bIterator 	= FTM_FALSE;
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_LIST_final(FTM_LIST_PTR pList)
{
	ASSERT(pList != NULL);

	FTM_ENTRY_PTR	pEntry;

	pEntry = pList->xHead.pNext;
	while(pEntry != &pList->xHead)
	{
		FTM_ENTRY_PTR	pNext = pEntry->pNext;
		
		FTM_MEM_free(pEntry);

		pEntry = pNext;
	}

	return	FTM_LIST_init(pList);
}

FTM_RET	FTM_LIST_seek(FTM_LIST_PTR pList, FTM_VOID_PTR pKey)
{
	ASSERT(pList != NULL);
	ASSERT(pKey != NULL);

	if (pList->fSeeker != NULL)
	{
		FTM_ENTRY_PTR	pEntry;

		pEntry = pList->xHead.pNext;
		while(pEntry != &pList->xHead)
		{
			FTM_ENTRY_PTR	pNext = pEntry->pNext;

			if (pList->fSeeker(pEntry, pKey) == FTM_TRUE)
			{
				return	FTM_RET_OK;
			}
			pEntry = pNext;
		}
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

FTM_RET	FTM_LIST_append(FTM_LIST_PTR pList, FTM_VOID_PTR pItem)
{
	ASSERT(pList != NULL);
	ASSERT(pItem != NULL);

	FTM_ENTRY_PTR pEntry;
	pEntry = FTM_MEM_malloc(sizeof(FTM_ENTRY));
	if (pEntry == NULL)
	{
		ERROR("Not enough memory[size = %d]\n", sizeof(FTM_ENTRY));
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	pEntry->pNext = &pList->xHead;
	pEntry->pPrev = pList->xHead.pPrev;
	pEntry->pData = pItem;

	pList->xHead.pPrev->pNext = pEntry;
	pList->xHead.pPrev = pEntry;

	pList->ulCount++;

	return	FTM_RET_OK;
}

FTM_RET	FTM_LIST_remove(FTM_LIST_PTR pList, FTM_VOID_PTR pItem)
{
	ASSERT(pList != NULL);
	ASSERT(pItem != NULL);

	FTM_ENTRY_PTR	pEntry;

	pEntry = pList->xHead.pNext;
	while(pEntry != &pList->xHead)
	{
		FTM_ENTRY_PTR	pNext = pEntry->pNext;
	
		if (pEntry->pData == pItem)
		{
			pEntry->pPrev->pNext = pNext;
			pNext->pPrev = pEntry->pPrev;
			FTM_MEM_free(pEntry);

			pList->ulCount--;

			return	FTM_RET_OK;
		}

		pEntry = pNext;
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

FTM_RET	FTM_LIST_removeAt(FTM_LIST_PTR pList, FTM_ULONG ulPosition)
{
	ASSERT(pList != NULL);

	if (ulPosition < pList->ulCount)
	{
		FTM_ENTRY_PTR	pEntry;

		pEntry = pList->xHead.pNext;
		while(pEntry != &pList->xHead)
		{
			FTM_ENTRY_PTR	pNext = pEntry->pNext;

			if (ulPosition == 0)
			{
				pEntry->pPrev->pNext = pNext;
				pNext->pPrev = pEntry->pPrev;
				FTM_MEM_free(pEntry);

				pList->ulCount--;

				return	FTM_RET_OK;
			}

			pEntry = pNext;
			ulPosition--;
		}
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

FTM_RET	FTM_LIST_get(FTM_LIST_PTR pList, FTM_VOID_PTR pKey, FTM_VOID_PTR _PTR_ ppElement)
{
	ASSERT(pList != NULL);
	ASSERT(pKey != NULL);
	ASSERT(ppElement != NULL);

	if (pList->fSeeker != NULL)
	{
		FTM_ENTRY_PTR	pEntry;

		pEntry = pList->xHead.pNext;
		while(pEntry != &pList->xHead)
		{
			FTM_ENTRY_PTR	pNext = pEntry->pNext;

			if (pList->fSeeker(pEntry, pKey) == FTM_TRUE)
			{
				*ppElement = pEntry->pData;

				return	FTM_RET_OK;
			}
			pEntry = pNext;
		}
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

FTM_RET	FTM_LIST_getAt(FTM_LIST_PTR pList, FTM_ULONG ulPosition, FTM_VOID_PTR _PTR_ ppElement)
{
	ASSERT(pList != NULL);
	ASSERT(ppElement != NULL);

	if (ulPosition < pList->ulCount)
	{
		FTM_ENTRY_PTR	pEntry;

		pEntry = pList->xHead.pNext;
		while(pEntry != &pList->xHead)
		{
			FTM_ENTRY_PTR	pNext = pEntry->pNext;

			if (ulPosition == 0)
			{
				*ppElement = pEntry->pData;

				return	FTM_RET_OK;
			}

			pEntry = pNext;
			ulPosition--;
		}
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

FTM_RET	FTM_LIST_iteratorStart(FTM_LIST_PTR pList)
{
	ASSERT(pList != NULL);

	pList->bIterator = FTM_TRUE;
	pList->pIter = &pList->xHead;
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_LIST_iteratorNext(FTM_LIST_PTR pList, FTM_VOID_PTR _PTR_ ppElement)
{
	ASSERT(pList != NULL);
	ASSERT(ppElement != NULL);

	if (pList->bIterator != FTM_TRUE)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	pList->pIter = pList->pIter->pNext;
	if (pList->pIter == &pList->xHead)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	*ppElement = pList->pIter->pData;
	return	FTM_RET_OK;
}

FTM_RET	FTM_LIST_count(FTM_LIST_PTR pList, FTM_ULONG_PTR pulCount)
{
	ASSERT(pList != NULL);
	ASSERT(pulCount != NULL);
	
	*pulCount = pList->ulCount;

	return	FTM_RET_OK;
}

FTM_RET FTM_LIST_setSeeker(FTM_LIST_PTR pList, FTM_LIST_ELEM_SEEKER fSeeker)
{
	ASSERT(pList != NULL);
	ASSERT(fSeeker != NULL);

	pList->fSeeker = fSeeker;

	return	FTM_RET_OK;
}


FTM_RET FTM_LIST_setComparator(FTM_LIST_PTR pList, FTM_LIST_ELEM_COMPARATOR fComparator)
{
	ASSERT(pList != NULL);
	ASSERT(fComparator != NULL);

	pList->fComparator = fComparator;

	return	FTM_RET_OK;
}


