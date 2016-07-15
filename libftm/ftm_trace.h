#ifndef	__FTM_TRACE_H__
#define	__FTM_TRACE_H__

#include <stdio.h>
#include "ftm_types.h"

typedef enum	FTM_TRACE_OUT_ENUM
{
	FTM_TRACE_OUT_TERM	= 0,
	FTM_TRACE_OUT_FILE	= 1,
	FTM_TRACE_OUT_SYSLOG= 2,
	FTM_TRACE_OUT_USER	= 3
} FTM_TRACE_OUT, _PTR_ FTM_TRACE_OUT_PTR;

#define		FTM_PATH_LEN		1024
#define		FTM_FILENAME_LEN	256

#define	FTM_TRACE_MAX_MODULES		256

#define	FTM_TRACE_MODULE_OBJECT		0
#define	FTM_TRACE_MODULE_MEMORY		1
#define	FTM_TRACE_MODULE_LOG		2
#define	FTM_TRACE_MODULE_ALL		FTM_TRACE_MAX_MODULES

#define	FTM_TRACE_LEVEL_ALL			0
#define	FTM_TRACE_LEVEL_TRACE		1
#define	FTM_TRACE_LEVEL_DEBUG		2
#define	FTM_TRACE_LEVEL_INFO		3
#define	FTM_TRACE_LEVEL_WARN		4
#define	FTM_TRACE_LEVEL_ERROR		5
#define	FTM_TRACE_LEVEL_FATAL		6
#define	FTM_TRACE_LEVEL_MESSAGE		31


typedef struct FTM_TRACE_INFO_STRUCT
{
	FTM_BOOL			bEnabled;
	FTM_CHAR			pName[FTM_NAME_LEN+1];
	FTM_ULONG			ulLevel;
	FTM_TRACE_OUT		xOut;
}	FTM_TRACE_INFO, _PTR_ FTM_TRACE_INFO_PTR;

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

FTM_RET	FTM_TRACE_strToLevel
(
	FTM_CHAR_PTR	pString, 
	FTM_ULONG_PTR pulLevel
);

FTM_RET	FTM_TRACE_setLevel
(
	FTM_ULONG		ulModule,
	FTM_ULONG		ulLevel
);

FTM_RET	FTM_TRACE_getLevel
(
	FTM_ULONG		ulModule,
	FTM_ULONG_PTR	pulLevel
);

FTM_RET	FTM_TRACE_setModule
(
	FTM_ULONG	ulModule,
	FTM_BOOL	bTraceOn
);

FTM_RET	FTM_TRACE_getModule
(
	FTM_ULONG		ulModule,
	FTM_BOOL_PTR	pbEnabled
);

FTM_RET	FTM_TRACE_getInfo
(
	FTM_ULONG		ulModule,
	FTM_TRACE_INFO_PTR	pInfo
);

FTM_RET	FTM_TRACE_setInfo
(
	FTM_ULONG		ulModule,
	FTM_TRACE_INFO_PTR	pInfo
);

FTM_RET	FTM_TRACE_setInfo2
(
	FTM_ULONG		ulModule,
	FTM_BOOL		bEnabled,
	FTM_CHAR_PTR	pName,
	FTM_ULONG		ulLevel,
	FTM_TRACE_OUT	xOut
);

FTM_RET	FTM_TRACE_getID
(
	FTM_CHAR_PTR	pName,
	FTM_ULONG_PTR	pulID
);

FTM_RET	FTM_TRACE_setOut
(
	FTM_ULONG		ulModule,
	FTM_TRACE_OUT 	xOut
);

FTM_RET	FTM_TRACE_printToTerm(FTM_CHAR_PTR	szBuff);

FTM_RET	FTM_TRACE_out
(
	FTM_ULONG		ulModule,
	FTM_ULONG		ulLevel,
	const char *	pFunction,
	FTM_INT			nLine,
	FTM_INT			bTimeInfo,
	FTM_INT			bFunctionInfo,
	const char *	pFormat,
	...
);

FTM_RET	FTM_TRACE_out2
(
	FTM_ULONG		ulModule,
	FTM_ULONG		ulLevel,
	const char *	pFuncName,
	FTM_INT			nLine,
	FTM_RET			xRet,
	const char *	pFormat,
	...
);
FTM_CHAR_PTR	FTM_TRACE_LEVEL_print
(
	FTM_ULONG 	ulLevel,
	FTM_BOOL	bFullName
);

FTM_CHAR_PTR	FTM_TRACE_OUT_print(FTM_TRACE_OUT xOut);

FTM_RET	FTM_TRACE_consoleCmd(FTM_INT nArgc, FTM_CHAR_PTR pArgv[]);

#define	__MODULE__	0xFFUL

#define	ASSERT(x)	{ if (!(x)) FTM_TRACE_out(__MODULE__, FTM_TRACE_LEVEL_FATAL, __func__, __LINE__, FTM_TRUE, FTM_TRUE, "%s\n", #x); }

FTM_VOID	FTM_TRACE_packetDump
(
	FTM_CHAR_PTR	pName,
	FTM_BYTE_PTR	pPacket,
	FTM_INT			nLen
);

#define MESSAGE(format, ...) printf(format, ## __VA_ARGS__)

#ifdef	TRACE_OFF
#define	TRACE(format, ...) 
#else
#define	TRACE(format, ...) 	FTM_TRACE_out(__MODULE__, FTM_TRACE_LEVEL_TRACE, __func__, __LINE__, FTM_TRUE, FTM_FALSE, format, ## __VA_ARGS__)
#endif

#define	INFO(format, ...) 	FTM_TRACE_out(__MODULE__, FTM_TRACE_LEVEL_INFO, __func__, __LINE__, FTM_TRUE, FTM_FALSE, format, ## __VA_ARGS__)
#define	WARN(format, ...) 	FTM_TRACE_out(__MODULE__, FTM_TRACE_LEVEL_WARN, __func__, __LINE__, FTM_TRUE, FTM_TRUE, format, ## __VA_ARGS__)
#define	ERROR(format, ...) 	FTM_TRACE_out(__MODULE__, FTM_TRACE_LEVEL_ERROR, __func__, __LINE__, FTM_TRUE, FTM_TRUE, format, ## __VA_ARGS__)
#define	FATAL(format, ...) 	FTM_TRACE_out(__MODULE__, FTM_TRACE_LEVEL_FATAL, __func__, __LINE__, FTM_TRUE, FTM_FALSE, format, ## __VA_ARGS__)

#define	ERROR2(code, format, ...)\
		{\
			if (format != NULL)\
			{\
				FTM_TRACE_out2(__MODULE__, FTM_TRACE_LEVEL_ERROR, __func__, __LINE__, code, format, ## __VA_ARGS__);\
			}\
			else\
			{\
				FTM_TRACE_out2(__MODULE__, FTM_TRACE_LEVEL_ERROR, __func__, __LINE__, code, "", 0);\
			}\
		}

#define	WARN2(code, format, ...)\
		{\
			if (format != NULL)\
			{\
				FTM_TRACE_out2(__MODULE__, FTM_TRACE_LEVEL_WARN, __func__, __LINE__, code, format, ## __VA_ARGS__);\
			}\
			else\
			{\
				FTM_TRACE_out2(__MODULE__, FTM_TRACE_LEVEL_WARN, __func__, __LINE__, code, "", 0);\
			}\
		}

#define	TRACE_ENTRY()	TRACE("ENTRY - %s\n", __func__)
#define	TRACE_EXIT()	TRACE("EXIT - %s\n", __func__)

FTM_RET	FTM_DEBUG_initSignals(FTM_VOID);

#endif

