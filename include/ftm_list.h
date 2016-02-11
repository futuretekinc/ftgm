#ifndef	__FTM_LIST_H__
#define __FTM_LIST_H__

#include "ftm_types.h"

typedef	FTM_BOOL	(*FTM_LIST_ELEM_SEEKER)(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
typedef	FTM_INT		(*FTM_LIST_ELEM_COMPARATOR)(const FTM_VOID_PTR pElement1, const FTM_VOID_PTR pElement2);

typedef	struct FTM_ENTRY_STRUCT
{
	struct FTM_ENTRY_STRUCT _PTR_ pPrev;
	struct FTM_ENTRY_STRUCT _PTR_ pNext;

	FTM_VOID_PTR	pData;
} FTM_ENTRY, _PTR_ FTM_ENTRY_PTR;

typedef	struct FTM_LIST_STRUCT
{
	FTM_UINT32		ulCount;
	FTM_ENTRY		xHead;

	FTM_BOOL		bIterator;
	FTM_ENTRY_PTR	pIter;

	FTM_LIST_ELEM_SEEKER		fSeeker;
	FTM_LIST_ELEM_COMPARATOR	fComparator;
} FTM_LIST, _PTR_ FTM_LIST_PTR;


FTM_RET FTM_LIST_create(FTM_LIST_PTR _PTR_ ppList);
FTM_RET	FTM_LIST_destroy(FTM_LIST_PTR pList);

FTM_RET FTM_LIST_init(FTM_LIST_PTR pList);
FTM_RET	FTM_LIST_final(FTM_LIST_PTR pList);

FTM_RET	FTM_LIST_seek(FTM_LIST_PTR pList, FTM_VOID_PTR pKey);

FTM_RET	FTM_LIST_append(FTM_LIST_PTR pList, FTM_VOID_PTR pItem);

FTM_RET	FTM_LIST_remove(FTM_LIST_PTR pList, FTM_VOID_PTR pItem);
FTM_RET	FTM_LIST_removeAt(FTM_LIST_PTR pList, FTM_ULONG ulPosition);

FTM_RET	FTM_LIST_get(FTM_LIST_PTR pList, FTM_VOID_PTR pKey, FTM_VOID_PTR _PTR_ ppElement);
FTM_RET	FTM_LIST_getAt(FTM_LIST_PTR pList, FTM_ULONG ulPosition, FTM_VOID_PTR _PTR_ ppElement);

FTM_RET	FTM_LIST_iteratorStart(FTM_LIST_PTR pList);
FTM_RET	FTM_LIST_iteratorNext(FTM_LIST_PTR pList, FTM_VOID_PTR _PTR_ ppElement);

FTM_RET	FTM_LIST_count(FTM_LIST_PTR pList, FTM_ULONG_PTR pulCount);

FTM_RET FTM_LIST_setSeeker(FTM_LIST_PTR pList, FTM_LIST_ELEM_SEEKER fSeeker);
FTM_RET FTM_LIST_setComparator(FTM_LIST_PTR pList, FTM_LIST_ELEM_COMPARATOR fComparator);

#endif

