#ifndef	__FTM_MEM_H__
#define	__FTM_MEM_H__

FTM_RET			FTM_MEM_init(void);
FTM_RET			FTM_MEM_final(void);

FTM_VOID_PTR	FTM_MEM_TRACE_malloc(size_t xSize, const char *pFile, unsigned long ulLine);
FTM_VOID_PTR	FTM_MEM_TRACE_calloc(size_t xNumber, size_t xSize, const char *pFile, unsigned long ulLine);

FTM_VOID		FTM_MEM_TRACE_free(FTM_VOID_PTR pMem, const char *pFile, unsigned long ulLine);

#define	FTM_MEM_malloc(xSize)	FTM_MEM_TRACE_malloc(xSize, __func__, __LINE__)
#define	FTM_MEM_calloc(xNumber, xSize)	FTM_MEM_TRACE_calloc(xNumber, xSize, __func__, __LINE__)
#define	FTM_MEM_free(pMem)		FTM_MEM_TRACE_free(pMem, __func__, __LINE__)
#endif

