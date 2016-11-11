#ifndef	__FTM_TRACE_H__
#define	__FTM_TRACE_H__

#include <stdio.h>
#include "ftm_types.h"
#include "ftm_config.h"

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

typedef	FTM_ULONG	FTM_TRACE_MODULE_TYPE, _PTR_ FTM_TRACE_MODULE_TYPE_PTR;
typedef	FTM_ULONG	FTM_TRACE_LEVEL, _PTR_ FTM_TRACE_LEVEL_PTR;

#define	FTM_TRACE_MODULE_UNKNOWN	0
#define	FTM_TRACE_MODULE_OBJECT		1
#define	FTM_TRACE_MODULE_MEMORY		2
#define	FTM_TRACE_MODULE_LOG		3
#define	FTM_TRACE_MODULE_ALL		FTM_TRACE_MAX_MODULES

#define	FTM_TRACE_LEVEL_DISABLE		0
#define	FTM_TRACE_LEVEL_FATAL		1
#define	FTM_TRACE_LEVEL_ERROR		2
#define	FTM_TRACE_LEVEL_WARN		3
#define	FTM_TRACE_LEVEL_INFO		4
#define	FTM_TRACE_LEVEL_DEBUG		5
#define	FTM_TRACE_LEVEL_TRACE		6
#define	FTM_TRACE_LEVEL_ALL			15


typedef struct FTM_TRACE_INFO_STRUCT
{
	FTM_TRACE_MODULE_TYPE	xType;
	FTM_CHAR				pName[FTM_NAME_LEN+1];
	FTM_TRACE_LEVEL			xLevel;
	FTM_TRACE_OUT			xOut;
}	FTM_TRACE_INFO, _PTR_ FTM_TRACE_INFO_PTR;

typedef	struct
{
	struct
	{
		FTM_BOOL			bModule;
		FTM_BOOL			bIndex;
		FTM_BOOL			bTime;
		FTM_BOOL			bDebug;
		FTM_BOOL			bLevel;
		FTM_ULONG			ulLine;
	}	xDisplayOpts;

	struct
	{
		FTM_BOOL		bForce;
		FTM_CHAR		pPath[FTM_PATH_LEN + 1];
		FTM_CHAR		pPrefix[FTM_FILENAME_LEN + 1];
	}	xFile;

	FTM_TRACE_INFO		pTraceInfos[FTM_TRACE_MAX_MODULES];
}	FTM_TRACE_CFG, _PTR_ FTM_TRACE_CFG_PTR;

FTM_RET	FTM_TRACE_configSet
(
	FTM_TRACE_CFG_PTR pConfig
);

FTM_RET	FTM_TRACE_loadConfig
(
	FTM_CONFIG_PTR	pConfig
);

FTM_RET	FTM_TRACE_loadConfigFromFile
(
	FTM_CHAR_PTR pFileName
);

FTM_RET	FTM_TRACE_printConfig
(
	FTM_TRACE_CFG_PTR pCfg
);

FTM_RET	FTM_TRACE_getType
(
	FTM_CHAR_PTR	pString,
	FTM_TRACE_MODULE_TYPE_PTR	pType
);

FTM_RET	FTM_TRACE_strToLevel
(
	FTM_CHAR_PTR	pString, 
	FTM_TRACE_LEVEL_PTR	pLevel
);

FTM_RET	FTM_TRACE_setLevel
(
	FTM_TRACE_MODULE_TYPE	xType,
	FTM_TRACE_LEVEL			xLevel
);

FTM_RET	FTM_TRACE_getLevel
(
	FTM_TRACE_MODULE_TYPE	xType,
	FTM_TRACE_LEVEL_PTR		pLevel
);

FTM_RET	FTM_TRACE_getInfo
(
	FTM_TRACE_MODULE_TYPE	xType,
	FTM_TRACE_INFO_PTR		pInfo
);

FTM_RET	FTM_TRACE_getInfoWithName
(
	FTM_CHAR_PTR		pName,
	FTM_TRACE_INFO_PTR	pInfo
);

FTM_RET	FTM_TRACE_setInfo
(
	FTM_TRACE_MODULE_TYPE	xType,
	FTM_TRACE_INFO_PTR		pInfo
);

FTM_RET	FTM_TRACE_setInfo2
(
	FTM_TRACE_MODULE_TYPE	xType,
	FTM_CHAR_PTR			pName,
	FTM_TRACE_LEVEL			xLevel,
	FTM_TRACE_OUT			xOut
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

FTM_RET	FTM_TRACE_strToOut
(
	FTM_CHAR_PTR		pString,
	FTM_TRACE_OUT_PTR	pxOut
);

FTM_RET	FTM_TRACE_printToTerm
(
	FTM_CHAR_PTR	szBuff
);

FTM_RET	FTM_TRACE_out
(
	FTM_TRACE_MODULE_TYPE	xType,
	FTM_TRACE_LEVEL	xLevel,
	const char *	pFuncName,
	FTM_INT			nLine,
	FTM_RET			xRet,
	const char *	pFormat,
	...
);

FTM_CHAR_PTR	FTM_TRACE_LEVEL_print
(
	FTM_TRACE_LEVEL	xLevel,
	FTM_BOOL		bFullName
);

FTM_CHAR_PTR	FTM_TRACE_OUT_print
(	
	FTM_TRACE_OUT 	xOut
);

struct FTM_SHELL_STRUCT;

FTM_RET	FTM_TRACE_shellCmd
(
	struct FTM_SHELL_STRUCT _PTR_ pShell, 
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[],
	FTM_VOID_PTR 	pData
);

FTM_VOID	FTM_TRACE_packetDump
(
	FTM_CHAR_PTR	pName,
	FTM_BYTE_PTR	pPacket,
	FTM_INT			nLen
);

#define	__MODULE__	0xFFUL

#define	ASSERT(x)	{ if (!(x)) FTM_TRACE_out(__MODULE__, FTM_TRACE_LEVEL_FATAL, __func__, __LINE__, FTM_RET_ASSERT, "%s\n", #x); }

#define MESSAGE(format, ...) printf(format, ## __VA_ARGS__)

#ifdef	TRACE_OFF
#define	TRACE(format, ...) 
#else
#define	TRACE(format, ...) 	FTM_TRACE_out(__MODULE__, FTM_TRACE_LEVEL_TRACE, 	__func__, __LINE__, FTM_RET_OK, format, ## __VA_ARGS__)
#endif

#define	INFO(format, ...) 	FTM_TRACE_out(__MODULE__, FTM_TRACE_LEVEL_INFO, 	__func__, __LINE__, FTM_RET_OK, format, ## __VA_ARGS__)
#define	WARN(format, ...) 	FTM_TRACE_out(__MODULE__, FTM_TRACE_LEVEL_WARN, 	__func__, __LINE__, FTM_RET_OK, format, ## __VA_ARGS__)
#define	ERROR(format, ...) 	FTM_TRACE_out(__MODULE__, FTM_TRACE_LEVEL_ERROR, 	__func__, __LINE__, FTM_RET_OK, format, ## __VA_ARGS__)
#define	FATAL(format, ...) 	FTM_TRACE_out(__MODULE__, FTM_TRACE_LEVEL_FATAL, 	__func__, __LINE__, FTM_RET_OK, format, ## __VA_ARGS__)

#define	ERROR2(code, format, ...)\
		{\
			if (format != NULL)\
			{\
				FTM_TRACE_out(__MODULE__, FTM_TRACE_LEVEL_ERROR, __func__, __LINE__, code, format, ## __VA_ARGS__);\
			}\
			else\
			{\
				FTM_TRACE_out(__MODULE__, FTM_TRACE_LEVEL_ERROR, __func__, __LINE__, code, "", 0);\
			}\
		}

#define	WARN2(code, format, ...)\
		{\
			if (format != NULL)\
			{\
				FTM_TRACE_out(__MODULE__, FTM_TRACE_LEVEL_WARN, __func__, __LINE__, code, format, ## __VA_ARGS__);\
			}\
			else\
			{\
				FTM_TRACE_out(__MODULE__, FTM_TRACE_LEVEL_WARN, __func__, __LINE__, code, "", 0);\
			}\
		}

#define	TRACE_ENTRY()	TRACE("ENTRY - %s\n", __func__)
#define	TRACE_EXIT()	TRACE("EXIT - %s\n", __func__)

FTM_RET	FTM_DEBUG_initSignals(FTM_VOID);

#endif

