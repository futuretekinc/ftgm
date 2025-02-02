#include <stdlib.h>
#include <string.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_list.h"
#include "ftm_mem.h"

#undef	__MODULE__
#define	__MODULE__	FTM_TRACE_MODULE_MEMORY

typedef struct
{
	FTM_CHAR_PTR	pFile;
	FTM_ULONG		ulLine;
	size_t			xSize;
	FTM_BYTE		pMem[];
}	FTM_MEM_BLOCK, _PTR_ FTM_MEM_BLOCK_PTR;

static FTM_INT	FTM_MEM_seeker(const FTM_VOID_PTR pElement, const FTM_VOID_PTR pIndicator);
static FTM_INT	FTM_MEM_comparator(const FTM_VOID_PTR pA, const FTM_VOID_PTR pB);

static FTM_BOOL	bInitialized = FTM_FALSE;
static FTM_LIST_PTR	pMemList = NULL;
static FTM_BOOL bTrace = FTM_FALSE;

FTM_RET			FTM_MEM_init(void)
{
	FTM_LIST_create(&pMemList);
	FTM_LIST_setSeeker(pMemList, FTM_MEM_seeker);
	FTM_LIST_setComparator(pMemList, FTM_MEM_comparator);
	bInitialized = FTM_TRUE;

	return	FTM_RET_OK;
}

FTM_RET			FTM_MEM_final(void)
{
	FTM_RET		xRet;
	FTM_ULONG	i, ulLeakedBlockCount;

	xRet = FTM_LIST_count(pMemList, &ulLeakedBlockCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if (ulLeakedBlockCount != 0)
	{
		FTM_MEM_BLOCK_PTR	pMB;

		MESSAGE("Memory leak detected : %lu\n", ulLeakedBlockCount);	
		for(i = 0 ; i < ulLeakedBlockCount ; i++)
		{
			FTM_LIST_getAt(pMemList, i, (FTM_VOID_PTR _PTR_)&pMB);
			MESSAGE("%3lu : %s[%3lu] - %08lx(%lu)\n", i, pMB->pFile, pMB->ulLine, (FTM_ULONG)pMB->pMem, (FTM_ULONG)pMB->xSize);
			if (pMB->pFile != NULL)
			{
				free(pMB->pFile);	
			}
			free(pMB);
		}
	}

	FTM_LIST_final(pMemList);

	return	FTM_RET_OK;
}

FTM_VOID_PTR	FTM_MEM_TRACE_malloc(size_t xSize, const char *pFile, unsigned long ulLine)
{
	FTM_MEM_BLOCK_PTR	pMB;

	if (bInitialized == FTM_FALSE)
	{
		return	malloc(xSize);	
	}

	pMB = malloc(sizeof(FTM_MEM_BLOCK) + xSize);
	if (pMB == NULL)
	{
		return	NULL;
	}
	memset(pMB, 0,sizeof(FTM_MEM_BLOCK) + xSize);

	pMB->pFile = strdup(pFile);
	pMB->ulLine= ulLine;
	pMB->xSize = xSize;
	FTM_LIST_append(pMemList, pMB);

	if (bTrace)
	{
		TRACE("Memory allocated.- %08lx(%3d) \n", pMB->pMem, xSize);
	}
	return	pMB->pMem;
}

FTM_VOID_PTR	FTM_MEM_TRACE_calloc(size_t xNumber, size_t xSize, const char *pFile, unsigned long ulLine)
{
	FTM_MEM_BLOCK_PTR	pMB;

	if (bInitialized == FTM_FALSE)
	{
		return	calloc(xNumber, xSize);	
	}

	pMB = malloc(sizeof(FTM_MEM_BLOCK) + xNumber * xSize);
	if (pMB == NULL)
	{
		return	NULL;
	}

	memset(pMB, 0, sizeof(FTM_MEM_BLOCK) + xNumber * xSize);
	if (pFile != NULL)
	{
		pMB->pFile = malloc(strlen(pFile) + 1);
		strcpy(pMB->pFile, pFile); 
	}
	pMB->ulLine= ulLine;
	pMB->xSize = xNumber * xSize;
	FTM_LIST_append(pMemList, pMB);

	if (bTrace)
	{
		TRACE("Memory allocated.- %08lx(%3d) \n", pMB->pMem, xSize);
	}
	return	pMB->pMem;
}

FTM_RET	FTM_MEM_TRACE_free(FTM_VOID_PTR pMem, const char *pFile, unsigned long ulLine)
{
	FTM_MEM_BLOCK_PTR	pMB;

	if (bInitialized == FTM_FALSE)
	{
		free(pMem);	
	}
	else
	{
		if (FTM_LIST_get(pMemList, pMem, (FTM_VOID_PTR _PTR_)&pMB) != FTM_RET_OK)
		{
			ERROR("The memory block(%08lx) not found. - %s[%3d]\n", pMem, pFile, ulLine);
			return	FTM_RET_OUT_OF_MEMORY;
		}
	
		if (bTrace)
		{
			MESSAGE("%s[%3lu] - %08lx(%lu)\n", pMB->pFile, pMB->ulLine, (FTM_ULONG)pMB->pMem, (FTM_ULONG)pMB->xSize);
		}
		FTM_LIST_remove(pMemList, pMB);
		if (pMB->pFile != NULL)
		{
			free(pMB->pFile);
		}
		free(pMB);
	}
	
	return	FTM_RET_OK;
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
