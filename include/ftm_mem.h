#ifndef	__FTM_MEM_H__
#define	__FTM_MEM_H__

FTM_RET			FTM_MEM_init(void);
FTM_RET			FTM_MEM_final(void);

FTM_VOID_PTR	FTM_MEM_malloc(size_t xSize);
FTM_VOID_PTR	FTM_MEM_calloc(size_t xNumber, size_t xSize);

FTM_VOID		FTM_MEM_free(FTM_VOID_PTR pMem);
#endif

