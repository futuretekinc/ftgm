#ifndef	__DEBUG_H__
#define	__DEBUG_H__

#include <stdio.h>

#define ERROR(format, ...) fprintf(stderr, format, ## __VA_ARGS__)
#define TRACE(format, ...) fprintf(stderr, format, ## __VA_ARGS__)
#define	ASSERT(...)
#define CALL_TRACE(format, ...) fprintf(stderr, "%s[%d]: " format, __func__, __LINE__, ## __VA_ARGS__)
#endif

