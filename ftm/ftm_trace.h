#ifndef	__FTM_TRACE_H__
#define	__FTM_TRACE_H__

#include <stdio.h>
#include "ftm_types.h"

#define		FTM_PATH_LEN		1024
#define		FTM_FILENAME_LEN	256

#define	FTM_TRACE_LEVEL_ALL			0
#define	FTM_TRACE_LEVEL_TRACE		1
#define	FTM_TRACE_LEVEL_DEBUG		2
#define	FTM_TRACE_LEVEL_INFO		3
#define	FTM_TRACE_LEVEL_WARN		4
#define	FTM_TRACE_LEVEL_ERROR		5
#define	FTM_TRACE_LEVEL_FATAL		6
#define	FTM_TRACE_LEVEL_MESSAGE		100

typedef	struct
{
	FTM_ULONG				ulLevel;
	struct
	{
		FTM_BOOL		bToFile;
		FTM_CHAR		pPath[FTM_PATH_LEN + 1];
		FTM_CHAR		pPrefix[FTM_FILENAME_LEN + 1];
		FTM_BOOL		bLine;
	}	xTrace;

	struct
	{
		FTM_BOOL		bToFile;
		FTM_CHAR		pPath[FTM_PATH_LEN + 1];
		FTM_CHAR		pPrefix[FTM_FILENAME_LEN + 1];
		FTM_BOOL		bLine;
	}	xError;
}	FTM_TRACE_CFG, _PTR_ FTM_TRACE_CFG_PTR;

FTM_RET	FTM_TRACE_configSet(FTM_TRACE_CFG_PTR pCfg);
FTM_RET	FTM_TRACE_configLoad(FTM_TRACE_CFG_PTR pCfg, FTM_CHAR_PTR pFileName);

FTM_RET	FTM_TRACE_printConfig(FTM_TRACE_CFG_PTR pCfg);

FTM_RET	FTM_TRACE_strToLevel(FTM_CHAR_PTR	pString, FTM_ULONG_PTR pulLevel);
FTM_RET	FTM_TRACE_setLevel(FTM_ULONG		ulLevel);
FTM_RET	FTM_TRACE_getLevel(FTM_ULONG_PTR	pulLevel);

FTM_RET	FTM_TRACE_printToTerm(FTM_CHAR_PTR	szBuff);
FTM_RET	FTM_TRACE_out
(
	unsigned long	ulLevel,
	const char *	pFunction,
	int				nLine,
	int				bTimeInfo,
	const char *	pFormat,
	...
);
FTM_CHAR_PTR	FTM_TRACE_levelString(FTM_ULONG ulLevel);

FTM_RET	FTM_TRACE_consoleCmd(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);


#define	ASSERT(x)	{ if (!(x)) FTM_TRACE_out(FTM_TRACE_LEVEL_FATAL, __func__, __LINE__, FTM_TRUE, "%s\n", #x); }

FTM_VOID	FTM_TRACE_packetDump
(
	FTM_CHAR_PTR	pName,
	FTM_BYTE_PTR	pPacket,
	FTM_INT			nLen
);

#define MESSAGE(format, ...) FTM_TRACE_out(FTM_TRACE_LEVEL_MESSAGE, NULL, 0, FTM_FALSE, format, ## __VA_ARGS__)

#ifdef	TRACE_OFF
#define	TRACE(format, ...) 
#else
#define	TRACE(format, ...) 	FTM_TRACE_out(FTM_TRACE_LEVEL_TRACE, __func__, __LINE__, FTM_TRUE, format, ## __VA_ARGS__)
#endif

#define	INFO(format, ...) 	FTM_TRACE_out(FTM_TRACE_LEVEL_INFO, __func__, __LINE__, FTM_TRUE, format, ## __VA_ARGS__)
#define	WARN(format, ...) 	FTM_TRACE_out(FTM_TRACE_LEVEL_WARN, __func__, __LINE__, FTM_TRUE, format, ## __VA_ARGS__)
#define	ERROR(format, ...) 	FTM_TRACE_out(FTM_TRACE_LEVEL_ERROR, __func__, __LINE__, FTM_TRUE, format, ## __VA_ARGS__)
#define	FATAL(format, ...) 	FTM_TRACE_out(FTM_TRACE_LEVEL_FATAL, __func__, __LINE__, FTM_TRUE, format, ## __VA_ARGS__)

#define	TRACE_ENTRY()	TRACE("ENTRY - %s\n", __func__)
#define	TRACE_EXIT()	TRACE("EXIT - %s\n", __func__)

FTM_RET	FTM_DEBUG_initSignals(FTM_VOID);

#endif

