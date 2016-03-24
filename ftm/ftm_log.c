#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "ftm_log.h"
#include "ftm_trace.h"

FTM_RET	FTM_PRINT_printToTerm(FTM_CHAR_PTR szmsg);
FTM_RET	FTM_PRINT_printToFile(FTM_CHAR_PTR szMsg, FTM_CHAR_PTR pPath, FTM_CHAR_PTR pPrefix);

static	FTM_LOG_TYPE	xOutputType = 0xFFFFFFFF;
static  FTM_BOOL		bTimeInfo = FTM_TRUE;

FTM_RET	FTM_LOG_out
(
	FTM_LOG_TYPE		xType,
	const FTM_CHAR_PTR	pFormat,
	...
)
{
	static	FTM_INT	nOutputLine = 0;
    va_list 		argptr;
	time_t			xTime;
	FTM_INT			nLen = 0;
	FTM_CHAR		szBuff[2048];
	FTM_CHAR		szTime[32];

	if ((xOutputType & xType) == 0)
	{
		return FTM_RET_OK;	
	}

	va_start ( argptr, pFormat );           
	xTime = time(NULL);
	strcpy(szTime, ctime(&xTime));
	szTime[strlen(szTime)-1] = '\0';

	if (bTimeInfo)
	{
		nLen  = snprintf( szBuff, sizeof(szBuff) - 1, "%4d : [%s] - ", ++nOutputLine, szTime);
	}
	nLen += vsnprintf( &szBuff[nLen], sizeof(szBuff) - nLen - 1, pFormat, argptr);
	va_end(argptr);

	szBuff[nLen] = '\0';

	FTM_TRACE_printToTerm(szBuff);

	return	FTM_RET_OK;
}

