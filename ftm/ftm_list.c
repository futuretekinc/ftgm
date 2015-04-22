#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_list.h"

FTM_RET FTM_LIST_init(FTM_LIST_PTR pList)
{
	ASSERT(pList != NULL);

	list_init(&pList->xList);
	pList->bIterator = FTM_BOOL_FALSE;
	
	return	FTM_RET_OK;
}

FTM_RET	FTM_LIST_destroy(FTM_LIST_PTR pList)
{
	ASSERT(pList != NULL);

	list_destroy(&pList->xList);

	return	FTM_RET_OK;
}


FTM_RET	FTM_LIST_append(FTM_LIST_PTR pList, FTM_VOID_PTR pItem)
{
	FTM_INT	nRet;

	ASSERT((pList != NULL) && (pItem != NULL));

	nRet = list_append(&pList->xList, pItem);
	if (nRet < 0)
	{
		ERROR("list_append error[%d]\n", nRet);
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_LIST_remove(FTM_LIST_PTR pList, FTM_VOID_PTR pItem)
{
	ASSERT((pList != NULL) && (pItem != NULL));

	if (list_delete(&pList->xList, pItem) < 0)
	{
		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_LIST_removeAt(FTM_LIST_PTR pList, FTM_ULONG ulPosition)
{
	ASSERT(pList != NULL);

	if (list_delete_at(&pList->xList, ulPosition) < 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_LIST_get(FTM_LIST_PTR pList, FTM_VOID_PTR pKey, FTM_VOID_PTR _PTR_ ppElement)
{
	FTM_VOID_PTR	pElement;

	ASSERT((pList != NULL) && (pKey != NULL) && (ppElement != NULL));

	pElement = list_seek(&pList->xList, pKey);
	if (pElement == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	*ppElement = pElement;

	return	FTM_RET_OK;
}

FTM_RET	FTM_LIST_getAt(FTM_LIST_PTR pList, FTM_ULONG ulPosition, FTM_VOID_PTR _PTR_ ppElement)
{
	FTM_VOID_PTR	pElement;

	ASSERT((pList != NULL) && (ppElement != NULL));

	pElement = list_get_at(&pList->xList, ulPosition);
	if (pElement == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	*ppElement = pElement;

	return	FTM_RET_OK;
}

FTM_RET	FTM_LIST_iteratorStart(FTM_LIST_PTR pList)
{
	ASSERT(pList != NULL);

	if (pList->bIterator == FTM_BOOL_TRUE)
	{
		list_iterator_stop(&pList->xList);
		pList->bIterator = FTM_BOOL_FALSE;
	}

	if (list_iterator_start(&pList->xList) < 0)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}
	pList->bIterator = FTM_BOOL_TRUE;

	return	FTM_RET_OK;
}

FTM_RET	FTM_LIST_iteratorNext(FTM_LIST_PTR pList, FTM_VOID_PTR _PTR_ ppElement)
{
	ASSERT((pList != NULL) && (ppElement != NULL));
	
	*ppElement = list_iterator_next(&pList->xList);
	if (*ppElement == NULL)
	{
		list_iterator_stop(&pList->xList);
		pList->bIterator = FTM_BOOL_FALSE;

		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_LIST_count(FTM_LIST_PTR pList, FTM_ULONG_PTR pulCount)
{
	ASSERT((pList != NULL) && (pulCount != NULL));
	
	*pulCount = list_size(&pList->xList);

	return	FTM_RET_OK;
}

FTM_RET FTM_LIST_setSeeker(FTM_LIST_PTR pList, FTM_LIST_ELEM_seeker fSeeker)
{
	ASSERT((pList != NULL) && (fSeeker != NULL));

	list_attributes_seeker(&pList->xList, fSeeker);

	return	FTM_RET_OK;
}


FTM_RET FTM_LIST_setComparator(FTM_LIST_PTR pList, FTM_LIST_ELEM_comparator fComparator)
{
	ASSERT((pList != NULL) && (fComparator != NULL));

	list_attributes_comparator(&pList->xList, fComparator);

	return	FTM_RET_OK;
}


