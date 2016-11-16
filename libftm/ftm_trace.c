#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <syslog.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "ftm_config.h"

#define	FTM_TRACE_LINE_LEN	2047

FTM_RET	FTM_TRACE_printToTerm(FTM_CHAR_PTR szmsg);
FTM_RET	FTM_TRACE_printToFile(FTM_CHAR_PTR pMsg, FTM_CHAR_PTR pPath, FTM_CHAR_PTR pPrefix);

extern 
FTM_CHAR_PTR	program_invocation_short_name;

static 
FTM_TRACE_CFG	_xConfig = 
{
	.xDisplayOpts = 
	{
		.bModule= FTM_FALSE,
		.bIndex = FTM_FALSE,
		.bTime	= FTM_FALSE,
		.bDebug	= FTM_TRUE,
		.bLevel	= FTM_FALSE,
		.ulLine	= 0
	},
	.xFile = 
	{
		.bForce= FTM_FALSE,
		.pPath = "./",
		.pPrefix = "ftm_trace"
	},

	.pTraceInfos =
	{
		{	.xType	=	FTM_TRACE_MODULE_OBJECT,	.pName = "OBJECT",	.xLevel = FTM_TRACE_LEVEL_ERROR, 	.xOut = FTM_TRACE_OUT_TERM	},
		{	.xType	=	FTM_TRACE_MODULE_MEMORY,	.pName = "MEMORY", 	.xLevel = FTM_TRACE_LEVEL_ERROR, 	.xOut = FTM_TRACE_OUT_TERM	},
		{	.xType	=	FTM_TRACE_MODULE_LOG,		.pName = "LOG", 	.xLevel = FTM_TRACE_LEVEL_ERROR, 	.xOut = FTM_TRACE_OUT_TERM	},
		{	.xType	=	FTM_TRACE_MODULE_UNKNOWN,	.pName = "", 		.xLevel = FTM_TRACE_LEVEL_DISABLE, 	.xOut = FTM_TRACE_OUT_TERM	},
	},
};

static 
struct 
{
	FTM_ULONG		ulLevel;
	FTM_CHAR_PTR	pName;
	FTM_CHAR_PTR	pFullName;
} FTM_levelStrings[] =
{
	{ FTM_TRACE_LEVEL_DISABLE,	"OFF",	"OFF" },
	{ FTM_TRACE_LEVEL_ALL, 		"ALL",	"ALL" },
	{ FTM_TRACE_LEVEL_TRACE, 	"TRCE",	"TRACE"},
	{ FTM_TRACE_LEVEL_DEBUG, 	"DBUG",	"DEBUG"},
	{ FTM_TRACE_LEVEL_INFO, 	"INFO",	"INFORMATION"},
	{ FTM_TRACE_LEVEL_WARN, 	"WARN",	"WARNING"}, 
	{ FTM_TRACE_LEVEL_ERROR, 	"EROR",	"ERROR"},
	{ FTM_TRACE_LEVEL_FATAL, 	"FATL",	"FATAL"},
};

FTM_RET	FTM_TRACE_loadConfig
(
	FTM_CONFIG_PTR	pConfig
)
{
	ASSERT(pConfig != NULL);

	FTM_RET			xRet;
	FTM_CONFIG_ITEM	xSection;
	FTM_CONFIG_ITEM	xSubSection;

	xRet = FTM_CONFIG_getItem(pConfig, "trace", &xSection);
	if (xRet == FTM_RET_OBJECT_NOT_FOUND)
	{
		return	FTM_RET_OK;	
	}
	else if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	FTM_CONFIG_ITEM_getItemBOOL(&xSection, "module",&_xConfig.xDisplayOpts.bModule);
	FTM_CONFIG_ITEM_getItemBOOL(&xSection, "index", &_xConfig.xDisplayOpts.bIndex);
	FTM_CONFIG_ITEM_getItemBOOL(&xSection, "time", 	&_xConfig.xDisplayOpts.bTime);
	FTM_CONFIG_ITEM_getItemBOOL(&xSection, "debug", &_xConfig.xDisplayOpts.bDebug);
	FTM_CONFIG_ITEM_getItemBOOL(&xSection,"level",  &_xConfig.xDisplayOpts.bLevel);
	FTM_CONFIG_ITEM_getItemULONG(&xSection,"line", 	&_xConfig.xDisplayOpts.ulLine);
		
	xRet = FTM_CONFIG_ITEM_getChildItem(&xSection, "file", &xSubSection);
	if (xRet == FTM_RET_OK)
	{
		FTM_CONFIG_ITEM_getItemBOOL(&xSubSection,"force", 	&_xConfig.xFile.bForce);
		FTM_CONFIG_ITEM_getItemString(&xSubSection, "path", _xConfig.xFile.pPath, sizeof(_xConfig.xFile.pPath) - 1);
		FTM_CONFIG_ITEM_getItemString(&xSubSection, "prefix", _xConfig.xFile.pPrefix, sizeof(_xConfig.xFile.pPrefix) - 1);
	}

	xRet = FTM_CONFIG_ITEM_getChildItem(&xSection, "modules", &xSubSection);
	if (xRet == FTM_RET_OK)
	{
		FTM_ULONG	ulModules = 0;

		xRet = FTM_CONFIG_LIST_getItemCount(&xSubSection, &ulModules);
		if (xRet == FTM_RET_OK)
		{
			FTM_INT	i;

			for(i = 0 ; i < ulModules ; i++)
			{
				FTM_CONFIG_ITEM	xItem;

				xRet = FTM_CONFIG_LIST_getItemAt(&xSubSection, i, &xItem);
				if (xRet == FTM_RET_OK)
				{
					FTM_CHAR	pBuffer[64];

					memset(pBuffer, 0, sizeof(pBuffer));
					xRet = FTM_CONFIG_ITEM_getItemString(&xItem, "name", pBuffer, sizeof(pBuffer) - 1);
					if (xRet == FTM_RET_OK)
					{	
						FTM_TRACE_MODULE_TYPE	xType;

						xRet = FTM_TRACE_getType(pBuffer, &xType);
						if (xRet != FTM_RET_OK)
						{
							ERROR2(xRet, "Invalid module type[%s]\n", pBuffer);
							continue;	
						}

						xRet = FTM_CONFIG_ITEM_getItemString(&xItem, "level", pBuffer, sizeof(pBuffer) - 1);
						if (xRet == FTM_RET_OK)
						{
							FTM_TRACE_LEVEL	xLevel;

							xRet = FTM_TRACE_strToLevel(pBuffer, &xLevel);
							if (xRet == FTM_RET_OK)
							{
								FTM_TRACE_setLevel(xType, xLevel);
							}
						}

						xRet = FTM_CONFIG_ITEM_getItemString(&xItem, "out", pBuffer, sizeof(pBuffer) - 1);
						if (xRet == FTM_RET_OK)
						{
							FTM_TRACE_OUT	xOut;
							xRet = FTM_TRACE_strToOut(pBuffer, &xOut);
							if (xRet == FTM_RET_OK)
							{
								FTM_TRACE_setOut(xType, xOut);
							}
						}
					}
				}
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_TRACE_loadConfigFromFile
(
	FTM_CHAR_PTR pFileName
)
{
	FTM_RET			xRet;
	FTM_CONFIG_PTR	pConfig;

	xRet = FTM_CONFIG_create(pFileName, &pConfig, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to crate configuration!\n");	
		return	xRet;
	}

	xRet = FTM_TRACE_loadConfig(pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to load configuration!\n");	
	}

	FTM_CONFIG_destroy(&pConfig);

	return	FTM_RET_OK;
}

FTM_RET	FTM_TRACE_configSet(FTM_TRACE_CFG_PTR pConfig)
{
	if (pConfig != NULL)
	{
		memcpy(&_xConfig, pConfig, sizeof(FTM_TRACE_CFG));	
	}

	//FTM_TRACE_setLevel(_xConfig.ulLevel);

	return	FTM_RET_OK;
}

FTM_RET	FTM_TRACE_printConfig
(
	FTM_TRACE_CFG_PTR pConfig
)
{
	FTM_INT	i;
	FTM_INT	nCount = 0;
	ASSERT(pConfig != NULL);

	if (pConfig == NULL)
	{
		pConfig = &_xConfig;	
	}

	MESSAGE("# Trace Configuration!\n");
	MESSAGE("%16s : %s\n", "Index", 		pConfig->xDisplayOpts.bIndex?"true":"false");
	MESSAGE("%16s : %s\n", "Time", 			pConfig->xDisplayOpts.bTime?"true":"false");
	MESSAGE("%16s : %s\n", "Function", 		pConfig->xDisplayOpts.bDebug?"true":"false");
	MESSAGE("%16s : %lu\n","Line Length", 	pConfig->xDisplayOpts.ulLine);

	MESSAGE("# Output\n");
	MESSAGE("%16s : %s\n", "Force File",pConfig->xFile.bForce?"true":"false");
	MESSAGE("%16s : %s\n", "Path", 		pConfig->xFile.pPath);
	MESSAGE("%16s : %s\n", "Prefix", 	pConfig->xFile.pPrefix);

	MESSAGE("    %16s %16s %8s\n", "NAME", "LEVEL", "OUTPUT");
	for(i = 0 ; i < FTM_TRACE_MAX_MODULES ; i++)
	{
		if (pConfig->pTraceInfos[i].xType != FTM_TRACE_MODULE_UNKNOWN)
		{
			MESSAGE("%3d %16s %16s %8s\n", 
				++nCount,
				pConfig->pTraceInfos[i].pName, 
				FTM_TRACE_LEVEL_print(pConfig->pTraceInfos[i].xLevel, FTM_TRUE),
				FTM_TRACE_OUT_print(pConfig->pTraceInfos[i].xOut));
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_TRACE_dumpPacket
(
	FTM_CHAR_PTR	pName,
	FTM_BYTE_PTR	pPacket,
	FTM_INT		nLen
)
{
	FTM_INT	i;

	printf("NAME : %s\n", pName);
	for(i = 0 ; i < nLen ; i++)
	{
		printf("%02x ", pPacket[i]);	
		if ((i+1) % 8 == 0)
		{
			printf("\n");	
		}
	}

	if (i % 8 != 0)
	{
		printf("\n");	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_TRACE_getType
(
	FTM_CHAR_PTR	pName,
	FTM_TRACE_MODULE_TYPE_PTR	pType
)
{
	FTM_INT	i;

	for(i = 0 ; i < FTM_TRACE_MAX_MODULES ; i++)
	{
		if ((_xConfig.pTraceInfos[i].pName[0] != 0)	&& (strcasecmp(_xConfig.pTraceInfos[i].pName, pName) == 0))
		{
			*pType = _xConfig.pTraceInfos[i].xType;	

			return	FTM_RET_OK;
		}
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

FTM_RET	FTM_TRACE_strToLevel
(
	FTM_CHAR_PTR	pString, 
	FTM_ULONG_PTR 	pulLevel
)
{
	ASSERT(pString != NULL);
	ASSERT(pulLevel != NULL);

	if (strcasecmp(pString, "all") == 0)
	{
		*pulLevel = FTM_TRACE_LEVEL_ALL;	
	}
	else if ((strcasecmp(pString, "off") == 0) || strcasecmp(pString, "disable") == 0)
	{
		*pulLevel = FTM_TRACE_LEVEL_DISABLE;	
	}
	else if (strcasecmp(pString, "trace") == 0)
	{
		*pulLevel = FTM_TRACE_LEVEL_TRACE;	
	}
	else if (strcasecmp(pString, "debug") == 0)
	{
		*pulLevel = FTM_TRACE_LEVEL_DEBUG;	
	}
	else if (strcasecmp(pString, "info") == 0)
	{
		*pulLevel = FTM_TRACE_LEVEL_INFO;	
	}
	else if (strcasecmp(pString, "warning") == 0)
	{
		*pulLevel = FTM_TRACE_LEVEL_WARN;	
	}
	else if (strcasecmp(pString, "error") == 0)
	{
		*pulLevel = FTM_TRACE_LEVEL_ERROR;	
	}
	else if (strcasecmp(pString, "critical") == 0)
	{
		*pulLevel = FTM_TRACE_LEVEL_FATAL;	
	}
	else if ((strlen(pString) == 1) && (isdigit(pString[0]) != 0))
	{
		FTM_ULONG	ulLevel = pString[0] - '0';
		if (ulLevel > FTM_TRACE_LEVEL_FATAL)
		{
			return	FTM_RET_INVALID_ARGUMENTS;
		}

		*pulLevel = ulLevel;
	}
	else 
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	return	FTM_RET_OK;
}

FTM_RET FTM_TRACE_setLevel
(
	FTM_TRACE_MODULE_TYPE	xType,
	FTM_TRACE_LEVEL			xLevel
)
{
	FTM_RET			xRet;
	FTM_TRACE_INFO	xInfo;

	xRet = FTM_TRACE_getInfo(xType, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get trace info!\n");
		return	xRet;
	}

	if (xInfo.xLevel != xLevel)
	{
		xInfo.xLevel = xLevel;

		return	FTM_TRACE_setInfo(xType, &xInfo);
	}

	return	FTM_RET_OK;
}
FTM_RET	FTM_TRACE_getLevel
(
	FTM_TRACE_MODULE_TYPE	xType,
	FTM_TRACE_LEVEL_PTR		pLevel
)
{
	FTM_RET	xRet;
	FTM_TRACE_INFO	xInfo;

	xRet = FTM_TRACE_getInfo(xType, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	*pLevel = xInfo.xLevel;

	return	FTM_RET_OK;
}

FTM_RET	FTM_TRACE_getInfo
(
	FTM_TRACE_MODULE_TYPE	xType,
	FTM_TRACE_INFO_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);
	
	FTM_INT	i;

	for(i = 0 ; i < FTM_TRACE_MAX_MODULES ; i++)
	{
		if (_xConfig.pTraceInfos[i].xType == xType)
		{
			memcpy(pInfo, &_xConfig.pTraceInfos[i], sizeof(FTM_TRACE_INFO));
			return	FTM_RET_OK;	
		}
	}


	return	FTM_RET_INVALID_TYPE;
}

FTM_RET	FTM_TRACE_setInfo
(
	FTM_TRACE_MODULE_TYPE	xType,
	FTM_TRACE_INFO_PTR	pInfo
)
{
	ASSERT(pInfo != NULL);

	FTM_INT	i;

	for(i = 0 ; i < FTM_TRACE_MAX_MODULES ; i++)
	{
		if (_xConfig.pTraceInfos[i].xType == xType)
		{
			memcpy(&_xConfig.pTraceInfos[i], pInfo, sizeof(FTM_TRACE_INFO));
			return	FTM_RET_OK;	
		}
	}


	for(i = 0 ; i < FTM_TRACE_MAX_MODULES ; i++)
	{
		if (_xConfig.pTraceInfos[i].xType == FTM_TRACE_MODULE_UNKNOWN)
		{
			memcpy(&_xConfig.pTraceInfos[i], pInfo, sizeof(FTM_TRACE_INFO));
			return	FTM_RET_OK;	
		}
	}

	return	FTM_RET_INVALID_TYPE;
}

FTM_RET	FTM_TRACE_setInfo2
(
	FTM_TRACE_MODULE_TYPE	xType,
	FTM_CHAR_PTR			pName,
	FTM_TRACE_LEVEL			xLevel,
	FTM_TRACE_OUT			xOut
)
{
	ASSERT(pName != NULL);

	FTM_TRACE_INFO	xInfo;

	memset(&xInfo, 0, sizeof(FTM_TRACE_INFO));

	xInfo.xType 	= xType;
	strncpy(xInfo.pName, pName, FTM_NAME_LEN);
	xInfo.xLevel	= xLevel;
	xInfo.xOut		= xOut;

	return	FTM_TRACE_setInfo(xType, &xInfo);
}

FTM_RET	FTM_TRACE_strToOut
(
	FTM_CHAR_PTR		pString,
	FTM_TRACE_OUT_PTR	pxOut
)
{
	if (strcasecmp(pString, "TERM") == 0)
	{
		*pxOut = FTM_TRACE_OUT_TERM;	
	}
	else if (strcasecmp(pString, "FILE") == 0)
	{
		*pxOut = FTM_TRACE_OUT_FILE;	
	}
	else if (strcasecmp(pString, "SYSLOG") == 0)
	{
		*pxOut = FTM_TRACE_OUT_SYSLOG;	
	}
	else if (strcasecmp(pString, "USER") == 0)
	{
		*pxOut = FTM_TRACE_OUT_USER;	
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_TRACE_setOut
(
	FTM_TRACE_MODULE_TYPE	xType,
	FTM_TRACE_OUT 	xOut
)
{
	FTM_RET			xRet;
	FTM_TRACE_INFO	xInfo;

	xRet = FTM_TRACE_getInfo(xType, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}
	
	if (xInfo.xOut != xOut)
	{
		xInfo.xOut = xOut;

		return	FTM_TRACE_setInfo(xType, &xInfo);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_TRACE_out
(
	FTM_TRACE_MODULE_TYPE	xType,
	FTM_TRACE_LEVEL			xLevel,
	const char *			pFunction,
	FTM_INT					nLine,
	FTM_RET					xResult,
	const char *			pFormat, 
	...
)
{
	static	FTM_INT	nOutputLine = 0;
    va_list 		argptr;
	time_t			xTime;
	FTM_RET			xRet;
	FTM_INT			nLen = 0;
	FTM_CHAR		szBuff[FTM_TRACE_LINE_LEN + 1];
	FTM_CHAR		szTime[32];
	FTM_BOOL		bDebugInfo = FTM_FALSE;
	FTM_TRACE_INFO	xInfo;

	xRet = FTM_TRACE_getInfo(xType, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	if ((xInfo.xLevel == FTM_TRACE_LEVEL_DISABLE) || (xLevel > xInfo.xLevel))
	{
		return	FTM_RET_OK;
	}

	va_start ( argptr, pFormat );           
	xTime = time(NULL);

	strftime(szTime, sizeof(szTime), "%Y-%m-%d %H:%M:%S", localtime(&xTime));

	if (_xConfig.xDisplayOpts.bIndex)
	{
		nLen  = snprintf( szBuff, FTM_TRACE_LINE_LEN, "%4d : ", ++nOutputLine);
	}

	if (_xConfig.xDisplayOpts.bTime)
	{
		nLen  += snprintf( &szBuff[nLen], FTM_TRACE_LINE_LEN - nLen, "[%s] ", szTime);
	}

	if (_xConfig.xDisplayOpts.bLevel)
	{
		nLen  += snprintf( &szBuff[nLen], FTM_TRACE_LINE_LEN - nLen, "[%s] ", FTM_TRACE_LEVEL_print(xLevel, FTM_FALSE));
	}

	if (bDebugInfo || (_xConfig.xDisplayOpts.bDebug && (pFunction != NULL)))
	{
		nLen += snprintf( &szBuff[nLen], FTM_TRACE_LINE_LEN - nLen, "%s[%04d] - ", pFunction, nLine);
	}

	if (_xConfig.xDisplayOpts.bModule)
	{
		nLen  += snprintf( &szBuff[nLen], FTM_TRACE_LINE_LEN - nLen, "[%8s] ", xInfo.pName);
	}

	nLen += vsnprintf( &szBuff[nLen], FTM_TRACE_LINE_LEN - nLen, pFormat, argptr);
	va_end(argptr);

	if ((_xConfig.xDisplayOpts.ulLine == 0) || (_xConfig.xDisplayOpts.ulLine >= nLen))
	{
		szBuff[nLen] = '\0';
	}
	else
	{
		szBuff[_xConfig.xDisplayOpts.ulLine - 5] = ' ';
		szBuff[_xConfig.xDisplayOpts.ulLine - 4] = '.';
		szBuff[_xConfig.xDisplayOpts.ulLine - 3] = '.';
		szBuff[_xConfig.xDisplayOpts.ulLine - 2] = '.';
		szBuff[_xConfig.xDisplayOpts.ulLine - 1] = '\n';
		szBuff[_xConfig.xDisplayOpts.ulLine    ] = '\0';
	}

	if ((_xConfig.xFile.bForce) || (xInfo.xOut == FTM_TRACE_OUT_FILE))
	{
		FTM_TRACE_printToFile(szBuff, _xConfig.xFile.pPath, _xConfig.xFile.pPrefix);
	}
	else if (xInfo.xOut == FTM_TRACE_OUT_SYSLOG)
	{
		syslog(LOG_INFO, "%s", szBuff);
	}
	else
	{
		FTM_TRACE_printToTerm(szBuff);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_TRACE_printToTerm(FTM_CHAR_PTR pMsg)
{
	fprintf(stdout, "%s", pMsg);

	return	FTM_RET_OK;
}

FTM_RET	FTM_TRACE_printToFile
(	FTM_CHAR_PTR 	pMsg, 
	FTM_CHAR_PTR 	pPath, 
	FTM_CHAR_PTR 	pPrefix
)
{
	FILE 		*pFile;
	time_t		rawTime;
	struct tm 	*localTime;
	FTM_CHAR	szTime[32];
	FTM_CHAR	szFileName[1024];

	time(&rawTime);
	localTime = localtime(&rawTime);
	strftime(szTime, 32, "%Y:%m:%d", localTime);

	if(sprintf(szFileName, "%s/%s-%s.log", pPath, pPrefix, szTime) <= 0)
	{
		return FTM_TRACE_printToTerm(pMsg);	
	}

	pFile = fopen(szFileName, "a");
	if (pFile == 0)
	{
		fprintf(stderr, "Can't open log file[%s]\n", szFileName);
		return FTM_RET_ERROR;	
	}


	fprintf(pFile, "%s", pMsg);
	fclose(pFile);

	return	FTM_RET_OK;
}

FTM_RET	FTM_TRACE_shellCmd
(
	struct FTM_SHELL_STRUCT _PTR_ pShell, 
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[],
	FTM_VOID_PTR 	pData
)
{
	FTM_RET	xRet;
	FTM_INT	i;

	switch(nArgc)
	{
	case	1:
		{
			FTM_INT	nCount = 0;

			fprintf(stdout, "# Trace Configuration\n");
			fprintf(stdout, "[ Modules ]\n");
			fprintf(stdout, "     %16s %8s %8s\n", "NAME", "LEVEL", "OUTPUT");
			for(i = 0 ; i < FTM_TRACE_MAX_MODULES ; i++)
			{
				if (_xConfig.pTraceInfos[i].pName[0] != 0)
				{
					fprintf(stdout, "%4d %12s(%2lu) %8s %8s\n", 
						++nCount, 
						_xConfig.pTraceInfos[i].pName, 
						_xConfig.pTraceInfos[i].xType, 
						FTM_TRACE_LEVEL_print(_xConfig.pTraceInfos[i].xLevel, FTM_TRUE),
						FTM_TRACE_OUT_print(_xConfig.pTraceInfos[i].xOut));
				}
			}

			fprintf(stdout, "\n[ Display Options ]\n");
			fprintf(stdout, "%16s : %s\n", "Index", _xConfig.xDisplayOpts.bIndex?"Enabled":"Disabled");
			fprintf(stdout, "%16s : %s\n", "Time",  _xConfig.xDisplayOpts.bTime?"Enabled":"Disabled");
			fprintf(stdout, "%16s : %s\n", "Debug", _xConfig.xDisplayOpts.bDebug?"Enabled":"Disabled");
			fprintf(stdout, "%16s : %lu\n","Line Length", _xConfig.xDisplayOpts.ulLine);
			fprintf(stdout, "%16s : %s\n", "Force File",_xConfig.xFile.bForce?"true":"false");
			fprintf(stdout, "%16s : %s\n", "Path", 		_xConfig.xFile.pPath);
			fprintf(stdout, "%16s : %s\n", "Prefix", _xConfig.xFile.pPrefix);
		}
		break;

	case	3:
		{
			
			FTM_TRACE_INFO		xInfo;
			FTM_TRACE_LEVEL		xOldLevel, xNewLevel;
			FTM_TRACE_OUT		xOldOut, xNewOut;

			xRet = FTM_TRACE_getInfoWithName(pArgv[1], &xInfo);
			if (xRet != FTM_RET_OK)
			{
				FTM_BOOL	bEnable = FTM_FALSE;

				if ((strcasecmp(pArgv[2], "enable") == 0) ||
				    (strcasecmp(pArgv[2], "on") == 0))
				{
					bEnable = FTM_TRUE;	
				}
				else if ((strcasecmp(pArgv[2], "disable") == 0) ||
						 (strcasecmp(pArgv[2], "off") == 0))
				{
					bEnable = FTM_FALSE;	
				}
				else
				{
					fprintf(stdout, "Invalid arguments[%s].\n"	, pArgv[2]);
					break;
				}

				if (strcasecmp(pArgv[1], "index")  == 0)
				{
					_xConfig.xDisplayOpts.bModule = bEnable;
				}
				else if (strcasecmp(pArgv[1], "index")  == 0)
				{
					_xConfig.xDisplayOpts.bIndex = bEnable;
				}
				else if (strcasecmp(pArgv[1], "time")  == 0)
				{
					_xConfig.xDisplayOpts.bTime = bEnable;
				}
				else if (strcasecmp(pArgv[1], "debug")  == 0)
				{
					_xConfig.xDisplayOpts.bDebug= bEnable;
				}
				else if (strcasecmp(pArgv[1], "level")  == 0)
				{
					_xConfig.xDisplayOpts.bLevel = bEnable;
				}
				else if (strcasecmp(pArgv[1], "line")  == 0)
				{
					FTM_ULONG	ulLine;

					ulLine = strtoul(pArgv[2], 0, 10);
					if (ulLine < 16 || FTM_TRACE_LINE_LEN < ulLine)
					{
						fprintf(stdout, "Trace line length is between 16 to %d.\n", FTM_TRACE_LINE_LEN);
						break;
					}

					_xConfig.xDisplayOpts.ulLine = ulLine;
				}
				else
				{
					fprintf(stdout, "Can't found module[%s].\n"	, pArgv[1]);
				}
				break;
			}

			xRet = FTM_TRACE_strToLevel(pArgv[2], &xNewLevel);
			if (xRet == FTM_RET_OK)
			{
				xOldLevel = xInfo.xLevel;
				xInfo.xLevel = xNewLevel;

				xRet = FTM_TRACE_setInfo(xInfo.xType, &xInfo);
				if (xRet == FTM_RET_OK)
				{
					fprintf(stdout, "The module[%s] trace level was changed[%s -> %s].\n", 
						pArgv[1], FTM_TRACE_LEVEL_print(xOldLevel, FTM_FALSE),FTM_TRACE_LEVEL_print(xNewLevel, FTM_FALSE));
				}
				else
				{
					fprintf(stdout, "Failed to change module[%s] trace level.\n", pArgv[1]);
				}

				break;
			}
			
			xRet = FTM_TRACE_strToOut(pArgv[2], &xNewOut);
			if (xRet == FTM_RET_OK)
			{
				xOldOut = xInfo.xOut;
				xInfo.xOut = xNewOut;

				xRet = FTM_TRACE_setInfo(xInfo.xType, &xInfo);
				if (xRet == FTM_RET_OK)
				{
					fprintf(stdout, "The module[%s] output was changed[%s -> %s].\n", 
						pArgv[1], FTM_TRACE_OUT_print(xOldOut),FTM_TRACE_OUT_print(xNewOut));
				}
				else
				{
					fprintf(stdout, "Failed to change module[%s] output.\n", pArgv[1]);
				}
				break;
			}

			fprintf(stdout, "Unknown trace command[%s]\n", pArgv[2]);
			
		}
		break;
	}
	return	FTM_RET_OK;
}

FTM_RET	FTM_TRACE_getInfoWithName
(
	FTM_CHAR_PTR	pName,
	FTM_TRACE_INFO_PTR	pInfo
)
{
	FTM_INT	i;

	for(i = 0 ; i < FTM_TRACE_MAX_MODULES ; i++)
	{
		if (_xConfig.pTraceInfos[i].xType != FTM_TRACE_MODULE_UNKNOWN)
		{
			if (strcasecmp(_xConfig.pTraceInfos[i].pName, pName) == 0)
			{
				memcpy(pInfo, &_xConfig.pTraceInfos[i], sizeof(FTM_TRACE_INFO));	
				return	FTM_RET_OK;
			
			}
		}
	}

	return	FTM_RET_INVALID_ARGUMENTS;
}

FTM_CHAR_PTR	FTM_TRACE_LEVEL_print
(
	FTM_ULONG ulLevel,
	FTM_BOOL	bFull
)
{
	FTM_ULONG	i;

	for(i = 0 ; i < sizeof(FTM_levelStrings) / sizeof(FTM_levelStrings[0]) ; i++)
	{
		if (FTM_levelStrings[i].ulLevel == ulLevel)
		{
			return	(bFull)?FTM_levelStrings[i].pFullName:FTM_levelStrings[i].pName;	
		}
	}

	return	(bFull)?"UNKNOWN":"UNX";	
}

FTM_CHAR_PTR	FTM_TRACE_OUT_print
(
	FTM_TRACE_OUT xOut
)
{
	switch(xOut)
	{
	case	FTM_TRACE_OUT_TERM:		return	"TERM";
	case	FTM_TRACE_OUT_FILE:		return	"FILE";	
	case	FTM_TRACE_OUT_SYSLOG:	return	"SYSLOG";
	case	FTM_TRACE_OUT_USER:		return	"USER";
	}

	return	"UNKNOWN";
}

static void signal_handler(int);
static void dumpstack(void);
static void cleanup(void);
void init_signals(void);
void panic(const char *, ...);

static struct sigaction sigact;

FTM_RET	FTM_DEBUG_initSignals(FTM_VOID)
{
	atexit(cleanup);

	sigact.sa_handler = signal_handler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, (struct sigaction *)NULL);

	sigaddset(&sigact.sa_mask, SIGSEGV);
	sigaction(SIGSEGV, &sigact, (struct sigaction *)NULL);

	sigaddset(&sigact.sa_mask, SIGBUS);
	sigaction(SIGBUS, &sigact, (struct sigaction *)NULL);

	sigaddset(&sigact.sa_mask, SIGQUIT);
	sigaction(SIGQUIT, &sigact, (struct sigaction *)NULL);

	sigaddset(&sigact.sa_mask, SIGHUP);
	sigaction(SIGHUP, &sigact, (struct sigaction *)NULL);

	sigaddset(&sigact.sa_mask, SIGKILL);
	sigaction(SIGKILL, &sigact, (struct sigaction *)NULL);

	return	FTM_RET_OK;
}


static void signal_handler(int sig)
{
	switch(sig)
	{
	case	SIGHUP:
		{
			panic("FATAL: Program hanged up\n");
		}
		break;

	case	SIGSEGV:
	case	SIGBUS:
		{
			dumpstack();
			panic("FATAL: %s Fault. Logged StackTrace\n", (sig == SIGSEGV) ? "Segmentation" : ((sig == SIGBUS) ? "Bus" : "Unknown"));
		}
		break;

	case SIGQUIT:
		{
			panic("QUIT signal ended program\n");
		}
		break;

	case	SIGKILL:
		{
			panic("KILL signal ended program\n");
		}
		break;
	
	//case	SIGINT:
	}

}

void panic(const char *fmt, ...)
{
		char buf[50];
		va_list argptr;
		va_start(argptr, fmt);
		vsprintf(buf, fmt, argptr);
		va_end(argptr);
		fprintf(stderr, "%s", buf);
		exit(-1);
}

static void dumpstack(void)
{
	/* Got this routine from http://www.whitefang.com/unix/faq_toc.html
	 ** Section 6.5. Modified to redirect to file to prevent clutter
	 */
	/* This needs to be changed... */
	char dbx[160];

	sprintf(dbx, "echo 'where\ndetach' | dbx -a %d > %s.dump", getpid(), program_invocation_short_name);
	/* Change the dbx to gdb */

	system(dbx);
	return;
}

void cleanup(void)
{
	sigemptyset(&sigact.sa_mask);
	/* Do any cleaning up chores here */
}
