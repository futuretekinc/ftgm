#ifndef	__FTM_DEBUG_H__
#define	__FTM_DEBUG_H__

#include <stdio.h>
#include "ftm_types.h"

#define	MSG_NORMAL		(1 << 0)	
#define	MSG_TRACE		(1 << 1)
#define	MSG_ERROR		(1 << 2)

#define	MSG_ALL			(MSG_NORMAL | MSG_TRACE | MSG_ERROR)

void setPrintMode(unsigned long	nMode);
void printOut(unsigned long nLevel, char *format, ...);

#define	ASSERT(...)

FTM_VOID	FTM_dumpPacket
(
	FTM_CHAR_PTR	pName,
	FTM_BYTE_PTR	pPacket,
	FTM_INT		nLen
);

#define MESSAGE(format, ...) printOut(MSG_NORMAL, format, ## __VA_ARGS__)

#define	TRACE(format, ...) printOut(MSG_TRACE, format, ## __VA_ARGS__)
#define	ERROR(format, ...) printOut(MSG_ERROR, format, ## __VA_ARGS__)
#endif
