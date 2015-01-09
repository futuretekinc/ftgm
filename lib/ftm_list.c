#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_list.h"

FTM_RET FTM_LIST_init(FTM_LIST_PTR pList)
{
	ASSERT(pList != NULL);

	list_init(&pList->xList);
	
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
	ASSERT((pList != NULL) && (pItem != NULL));

	if (list_append(&pList->xList, pItem) != 0)
	{
		return	FTM_RET_ERROR;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_LIST_remove(FTM_LIST_PTR pList, FTM_VOID_PTR pKey)
{
	ASSERT((pList != NULL) && (pKey != NULL));

	if (list_delete(&pList->xList, pKey) != 0)
	{
		return	FTM_RET_ERROR;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_LIST_removeAt(FTM_LIST_PTR pList, FTM_ULONG ulPosition)
{
	ASSERT(pList != NULL);

	if (list_delete_at(&pList->xList, ulPosition) != 0)
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


