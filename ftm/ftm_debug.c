#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "ftm_types.h"
#include "ftm_debug.h"
#include "ftm_error.h"
#include "libconfig.h"

FTM_RET	FTM_DEBUG_printToTerm(FTM_CHAR_PTR szmsg);
FTM_RET	FTM_DEBUG_printToFile(FTM_CHAR_PTR szMsg, FTM_CHAR_PTR pPath, FTM_CHAR_PTR pPrefix);

static FTM_DEBUG_CFG	_xConfig = 
{
	.ulMode =MSG_NORMAL,
	.xTrace = 
	{
		.bToFile = FTM_FALSE,
		.pPath = "./",
		.pPrefix = "ftm_trace",
		.bLine	= FTM_FALSE
	},

	.xError = 
	{
		.bToFile = FTM_FALSE,
		.pPath = "./",
		.pPrefix = "ftm_error",
		.bLine	= FTM_FALSE
	},
};


FTM_RET	FTM_DEBUG_configLoad(FTM_DEBUG_CFG_PTR pCfg, FTM_CHAR_PTR pFileName)
{
	config_t		xLibConfig;
	config_setting_t	*pSection;
	config_setting_t	*pSubSection;
	config_setting_t	*pField;

	config_init(&xLibConfig);

	if (config_read_file(&xLibConfig, pFileName) == CONFIG_FALSE)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	pSection = config_lookup(&xLibConfig, "DEBUG");
	if (pSection != NULL)
	{
		pField = config_setting_get_member(pSection, "mode");
		if (pField != NULL)
		{
			pCfg->ulMode = (FTM_ULONG)config_setting_get_int(pField);
		}

		pSubSection = config_setting_get_member(pSection, "trace");
		if (pSubSection != NULL)
		{
			pField = config_setting_get_member(pSubSection, "path");
			if (pField != NULL)
			{
				strncpy(pCfg->xTrace.pPath, 
						config_setting_get_string(pField),
						sizeof(pCfg->xTrace.pPath) - 1);
			}
	
			pField = config_setting_get_member(pSubSection, "prefix");
			if (pField != NULL)
			{
				strncpy(pCfg->xTrace.pPrefix, 
						config_setting_get_string(pField),
						sizeof(pCfg->xTrace.pPath) - 1);
			}
	
			pField = config_setting_get_member(pSubSection, "to_file");
			if (pField != NULL)
			{
				pCfg->xTrace.bToFile = (FTM_BOOL)config_setting_get_int(pField);
			}
	
			pField = config_setting_get_member(pSubSection, "print_line");
			if (pField != NULL)
			{
				pCfg->xTrace.bLine = (FTM_BOOL)config_setting_get_int(pField);
			}
		}

		pSubSection = config_setting_get_member(pSection, "error");
		if (pSubSection != NULL)
		{
			pField = config_setting_get_member(pSubSection, "path");
			if (pField != NULL)
			{
				strncpy(pCfg->xError.pPath, 
						config_setting_get_string(pField),
						sizeof(pCfg->xError.pPath) - 1);
			}
	
			pField = config_setting_get_member(pSubSection, "prefix");
			if (pField != NULL)
			{
				strncpy(pCfg->xError.pPrefix, 
						config_setting_get_string(pField),
						sizeof(pCfg->xError.pPath) - 1);
			}
	
			pField = config_setting_get_member(pSubSection, "to_file");
			if (pField != NULL)
			{
				pCfg->xError.bToFile = (FTM_BOOL)config_setting_get_int(pField);
			}
	
			pField = config_setting_get_member(pSubSection, "print_line");
			if (pField != NULL)
			{
				pCfg->xError.bLine = (FTM_BOOL)config_setting_get_int(pField);
			}
		}

	}

	config_destroy(&xLibConfig);

	return	FTM_RET_OK;
}

FTM_RET	FTM_DEBUG_configSet(FTM_DEBUG_CFG_PTR pCfg)
{
	if (pCfg != NULL)
	{
		memcpy(&_xConfig, pCfg, sizeof(FTM_DEBUG_CFG));	
	}

	FTM_DEBUG_printModeSet(_xConfig.ulMode);

	return	FTM_RET_OK;
}

FTM_RET	FTM_DEBUG_dumpPacket
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

FTM_RET FTM_DEBUG_printModeSet
(
	FTM_ULONG ulMode
)
{
	switch(ulMode)
	{
	case	0:	_xConfig.ulMode = 0; break;
	case	1:	_xConfig.ulMode = MSG_NORMAL; break;
	case	2:	_xConfig.ulMode = MSG_ALL; break;
	}

	return	FTM_RET_OK;
}
FTM_RET	FTM_DEBUG_printModeGet
(
	FTM_ULONG_PTR pMode
)
{
	*pMode = _xConfig.ulMode;

	return	FTM_RET_OK;
}


FTM_RET	FTM_DEBUG_printOut
(
	unsigned long	nLevel,
	const char		*pFunction,
	int				nLine,
	char 			*format, 
	...
)
{
    va_list 		argptr;
	FTM_BOOL		bFile = FTM_FALSE;
	FTM_BOOL		bLine = FTM_FALSE;
	FTM_CHAR_PTR	pPath;
	FTM_CHAR_PTR	pPrefix;
	FTM_INT			nLen = 0;
	FTM_CHAR		szBuff[2048];

	if ((nLevel & _xConfig.ulMode) == 0)
	{
		return FTM_RET_OK;	
	}

	switch(nLevel)
	{
	case	MSG_TRACE:
		if (_xConfig.xTrace.bToFile)
		{
			bFile 	= FTM_TRUE;
			pPath	= _xConfig.xTrace.pPath;
			pPrefix = _xConfig.xTrace.pPrefix;
		}
		bLine	= _xConfig.xTrace.bLine;
		break;

	case	MSG_ERROR:
		if (_xConfig.xError.bToFile)
		{
			bFile = FTM_TRUE;
			pPath	= _xConfig.xTrace.pPath;
			pPrefix = _xConfig.xTrace.pPrefix;
		}
		bLine	= _xConfig.xError.bLine;
		break;
	}

	va_start ( argptr, format );           
	if (bLine && (pFunction != NULL))
	{
		nLen  = snprintf( szBuff, sizeof(szBuff) - 1, "%32s[%4d] : ", pFunction, nLine);
	}
	nLen += vsnprintf( &szBuff[nLen], sizeof(szBuff) - nLen - 1, format, argptr);
	va_end(argptr);

	szBuff[nLen] = '\0';

	if (bFile)
	{
		FTM_DEBUG_printToFile(szBuff, pPath, pPrefix);

	}
	else
	{
		FTM_DEBUG_printToTerm(szBuff);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_DEBUG_printToTerm(FTM_CHAR_PTR szMsg)
{
	fprintf(stdout, "%s", szMsg);

	return	FTM_RET_OK;
}

FTM_RET	FTM_DEBUG_printToFile(FTM_CHAR_PTR szMsg, FTM_CHAR_PTR pPath, FTM_CHAR_PTR pPrefix)
{
	FILE 		*pFile;
	time_t		rawTime;
	struct tm 	*localTime;
	FTM_CHAR	szTime[32];
	FTM_CHAR	szFileName[1024];

	time(&rawTime);
	localTime = localtime(&rawTime);
	strftime(szTime, 32, "%Y:%m:%d", localTime);

	if(sprintf(szFileName, "%s%s-%s.log", pPath, pPrefix, szTime) <= 0)
	{
		return FTM_DEBUG_printToTerm(szMsg);	
	}

	pFile = fopen(szFileName, "a");
	if (pFile == 0)
	{
		fprintf(stderr, "Can't open log file\n");
		return FTM_RET_ERROR;	
	}

	

	strftime(szTime, 32, "%Y:%m:%d %H:%M:%S", localTime);
	fprintf(pFile, "%s : %s", szTime, szMsg);
	fclose(pFile);

	return	FTM_RET_OK;
}

FTM_RET	FTM_DEBUG_consoleCmd(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{

	return	FTM_RET_OK;
}
