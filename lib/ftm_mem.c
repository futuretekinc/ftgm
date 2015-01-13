#define	__FTM_MEM_H__

#include <stdlib.h>
#include <string.h>
#include "ftm_types.h"
#include "ftm_error.h"
#include "ftm_debug.h"
#include "simclist.h"

typedef struct
{
	FTM_CHAR_PTR	pFile;
	FTM_ULONG		ulLine;
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

		MESSAGE("Memory leak detected\n");	
		for(i = 0 ; i < ulLeakedBlockCount ; i++)
		{
			pMB = list_get_at(&xMemList, i);
			MESSAGE("%3d : %s[%3d] - %08lx(%d)\n", i, pMB->pFile, pMB->ulLine, pMB->pMem, pMB->xSize);
			if (pMB->pFile != NULL)
			{
				free(pMB->pFile);	
			}
			free(pMB);
		}
	}

	list_destroy(&xMemList);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTM_MEM_TRACE_malloc(size_t xSize, const char *pFile, unsigned long ulLine)
{
	FTM_MEM_BLOCK_PTR	pMB;

	pMB = malloc(sizeof(FTM_MEM_BLOCK) + xSize);
	if (pMB == NULL)
	{
		return	NULL;
	}

	pMB->pFile = strdup(pFile);
	pMB->ulLine= ulLine;
	pMB->xSize = xSize;
	list_append(&xMemList, pMB);

	TRACE("Memory allocated.- %08lx(%3d) \n", pMB->pMem, xSize);
	return	pMB->pMem;
}

FTM_VOID_PTR	FTM_MEM_TRACE_calloc(size_t xNumber, size_t xSize, const char *pFile, unsigned long ulLine)
{
	FTM_MEM_BLOCK_PTR	pMB;

	pMB = malloc(sizeof(FTM_MEM_BLOCK) + xNumber * xSize);
	if (pMB == NULL)
	{
		return	NULL;
	}

	memset(pMB, 0, sizeof(FTM_MEM_BLOCK) + xNumber * xSize);
	pMB->pFile = strdup(pFile);
	pMB->ulLine= ulLine;
	pMB->xSize = xNumber * xSize;
	list_append(&xMemList, pMB);

	TRACE("Memory allocated.- %08lx(%3d) \n", pMB->pMem, xSize);
	return	pMB->pMem;
}

FTM_VOID	FTM_MEM_TRACE_free(FTM_VOID_PTR pMem, const char *pFile, unsigned long ulLine)
{
	FTM_MEM_BLOCK_PTR	pMB;

	MESSAGE("Free(%08lx)\n", pMem);
	pMB = list_seek(&xMemList, pMem);
	if (pMB != NULL)
	{
		MESSAGE("%s[%3d] - %08lx(%d)\n", pMB->pFile, pMB->ulLine, pMB->pMem, pMB->xSize);
		list_delete(&xMemList, pMB);
		free(pMB->pFile);
		free(pMB);
	}
	else
	{
		ERROR("The memory block(%08lx) not found. - %s[%3d]\n", pMem, pFile, ulLine);
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
