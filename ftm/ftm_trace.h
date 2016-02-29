#ifndef	__FTM_TRACE_H__
#define	__FTM_TRACE_H__

#include <stdio.h>
#include "ftm_types.h"

#define		FTM_PATH_LEN		1024
#define		FTM_FILENAME_LEN	256

#define	FTM_PRINT_LEVEL_ALL			0
#define	FTM_PRINT_LEVEL_TRACE		2
#define	FTM_PRINT_LEVEL_DEBUG		4
#define	FTM_PRINT_LEVEL_INFO		6
#define	FTM_PRINT_LEVEL_WARN		8
#define	FTM_PRINT_LEVEL_ERROR		10
#define	FTM_PRINT_LEVEL_FATAL		12
#define	FTM_PRINT_LEVEL_MESSAGE		100

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
}	FTM_PRINT_CFG, _PTR_ FTM_PRINT_CFG_PTR;

FTM_RET	FTM_PRINT_configSet(FTM_PRINT_CFG_PTR pCfg);
FTM_RET	FTM_PRINT_configLoad(FTM_PRINT_CFG_PTR pCfg, FTM_CHAR_PTR pFileName);

FTM_RET	FTM_PRINT_setLevel(FTM_ULONG		ulLevel);
FTM_RET	FTM_PRINT_getLevel(FTM_ULONG_PTR	pulLevel);
FTM_RET	FTM_PRINT_out
(
	unsigned long	ulLevel,
	const char *	pFunction,
	int				nLine,
	int				bTimeInfo,
	const char *	pFormat,
	...
);
FTM_CHAR_PTR	FTM_PRINT_levelString(FTM_ULONG ulLevel);

FTM_RET	FTM_PRINT_consoleCmd(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);


#define	ASSERT(x)	{ if (!(x)) FTM_PRINT_out(FTM_PRINT_LEVEL_FATAL, __func__, __LINE__, FTM_TRUE, "%s", #x); }

FTM_VOID	FTM_PRINT_packetDump
(
	FTM_CHAR_PTR	pName,
	FTM_BYTE_PTR	pPacket,
	FTM_INT			nLen
);

#define MESSAGE(format, ...) FTM_PRINT_out(FTM_PRINT_LEVEL_MESSAGE, NULL, 0, FTM_FALSE, format, ## __VA_ARGS__)

#ifdef	TRACE_OFF
#define	TRACE(format, ...) 
#else
#define	TRACE(format, ...) FTM_PRINT_out(FTM_PRINT_LEVEL_TRACE, __func__, __LINE__, FTM_TRUE, format, ## __VA_ARGS__)
#endif
#define	ERROR(format, ...) FTM_PRINT_out(FTM_PRINT_LEVEL_ERROR, __func__, __LINE__, FTM_TRUE, format, ## __VA_ARGS__)
#endif

FTM_RET	FTM_DEBUG_initSignals(FTM_VOID);
