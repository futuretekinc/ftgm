#ifndef	__DEBUG_H__
#define	__DEBUG_H__

#include <stdio.h>

#define ERROR(format, ...) fprintf(stderr, format, ## __VA_ARGS__)
#define TRACE(format, ...) fprintf(stderr, format, ## __VA_ARGS__)

#endif

