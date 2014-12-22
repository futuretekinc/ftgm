#ifndef	__FTDM_DEBUG_H__
#define	__FTDM_DEBUG_H__

#include <stdio.h>

#define ERROR(format, ...) fprintf(stderr, format, ## __VA_ARGS__)
#define TRACE(format, ...) fprintf(stderr, format, ## __VA_ARGS__)
#define	ASSERT(...)
#define CALL_TRACE() fprintf(stderr, "%s[%d]\n" , __func__, __LINE__)

FTDM_VOID	FTDM_dumpPacket
(
	FTDM_CHAR_PTR	pName,
	FTDM_BYTE_PTR	pPacket,
	FTDM_INT		nLen
);

#endif
