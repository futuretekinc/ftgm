#ifndef	__FTDM_DEBUG_H__
#define	__FTDM_DEBUG_H__

#include <stdio.h>

#define	MSG_NORMAL		(1 << 0)	
#define	MSG_TRACE		(1 << 1)
#define	MSG_ERROR		(1 << 2)

#define	MSG_ALL			(MSG_NORMAL | MSG_TRACE | MSG_ERROR)

void setPrintMode(unsigned long	nMode);
void printOut(unsigned long nLevel, char *format, ...);

#define	ASSERT(...)

FTDM_VOID	FTDM_dumpPacket
(
	FTDM_CHAR_PTR	pName,
	FTDM_BYTE_PTR	pPacket,
	FTDM_INT		nLen
);

#define MESSAGE(format, ...) printOut(MSG_NORMAL, format, ## __VA_ARGS__)

#define	TRACE(format, ...) printOut(MSG_TRACE, format, ## __VA_ARGS__)
#define	ERROR(format, ...) printOut(MSG_ERROR, format, ## __VA_ARGS__)
#endif
