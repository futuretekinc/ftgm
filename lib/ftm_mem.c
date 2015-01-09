#include <stdlib.h>
#include <string.h>
#include "ftm_types.h"
#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_mem.h"
#include "simclist.h"

typedef struct
{
	size_t			xSize;
	FTM_BYTE		pMem[];
}	FTM_MEM_BLOCK, _PTR_ FTM_MEM_BLOCK_PTR;

static FTM_INT	FTM_MEM_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
static FTM_INT	FTM_MEM_comparator(const FTM_VOID_PTR pA, const FTM_VOID_PTR pB);

static list_t	xMemList;

FTM_RET			FTM_MEM_init(void)
{
	list_init(&xMemList);
	list_attributes_seeker(&xMemList, FTM_MEM_seeker);
	list_attributes_comparator(&xMemList, FTM_MEM_comparator);

	return	FTM_RET_OK;
}

FTM_RET			FTM_MEM_final(void)
{
	FTM_ULONG	i, ulLeakedBlockCount;

	ulLeakedBlockCount = list_size(&xMemList);

	if (ulLeakedBlockCount != 0)
	{
		FTM_MEM_BLOCK_PTR	pMB;

		ERROR("Memory leak detected\n");	
		for(i = 0 ; i < ulLeakedBlockCount ; i++)
		{
			pMB = list_get_at(&xMemList, i);
			ERROR("%3d : %08lx(%d)\n", i, pMB->pMem, pMB->xSize);
			free(pMB);
		}
	}

	list_destroy(&xMemList);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTM_MEM_malloc(size_t xSize)
{
	FTM_MEM_BLOCK_PTR	pMB;

	pMB = malloc(sizeof(FTM_MEM_BLOCK) + xSize);
	if (pMB == NULL)
	{
		return	NULL;
	}

	pMB->xSize = xSize;
	list_append(&xMemList, pMB);

	return	pMB->pMem;
}

FTM_VOID_PTR	FTM_MEM_calloc(size_t xNumber, size_t xSize)
{
	FTM_MEM_BLOCK_PTR	pMB;

	pMB = malloc(sizeof(FTM_MEM_BLOCK) + xNumber * xSize);
	if (pMB == NULL)
	{
		return	NULL;
	}

	pMB->xSize = xNumber * xSize;
	memset(pMB->pMem, 0, xNumber * xSize);
	list_append(&xMemList, pMB);

	return	pMB->pMem;
}

FTM_VOID	FTM_MEM_free(FTM_VOID_PTR pMem)
{
	FTM_MEM_BLOCK_PTR	pMB;

	pMB = list_seek(&xMemList, pMem);
	if (pMB != NULL)
	{
		list_delete(&xMemList, pMB);
		free(pMB);
	}
}

FTM_INT	FTM_MEM_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator)
{
	FTM_MEM_BLOCK_PTR	pMB = (FTM_MEM_BLOCK_PTR)pElement;

	return	(pMB->pMem == pIndicator);
}

FTM_INT	FTM_MEM_comparator(const FTM_VOID_PTR pA, const FTM_VOID_PTR pB)
{
	if (pA == pB)
	{
		return	0;	
	}

	return	1;
}
