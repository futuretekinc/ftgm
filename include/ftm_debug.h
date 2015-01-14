#ifndef	__FTM_DEBUG_H__
#define	__FTM_DEBUG_H__

#include <stdio.h>
#include "ftm_types.h"

#define	MSG_NORMAL		(1 << 0)	
#define	MSG_TRACE		(1 << 1)
#define	MSG_ERROR		(1 << 2)

#define	MSG_ALL			(MSG_NORMAL | MSG_TRACE | MSG_ERROR)

void setPrintMode(FTM_ULONG		nMode);
void getPrintMode(FTM_ULONG_PTR	pMode);
void printOut
(
	unsigned long 	nLevel, 
	const char		*pFunction,
	int				nLine,
	char 			*pFormat, 
	...
);

#define	ASSERT(x)	{ if (!(x)) printOut(MSG_ERROR, __func__, __LINE__, "%s", #x); }

FTM_VOID	FTM_dumpPacket
(
	FTM_CHAR_PTR	pName,
	FTM_BYTE_PTR	pPacket,
	FTM_INT		nLen
);

#define MESSAGE(format, ...) printOut(MSG_NORMAL, NULL, 0, format, ## __VA_ARGS__)

#define	TRACE(format, ...) printOut(MSG_TRACE, __func__, __LINE__, format, ## __VA_ARGS__)
#define	ERROR(format, ...) printOut(MSG_ERROR, __func__, __LINE__, format, ## __VA_ARGS__)
#endif
