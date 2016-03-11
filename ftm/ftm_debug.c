#include <stdlib.h>
#include <stdarg.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "ftm_types.h"
#include "ftm_trace.h"
#include "libconfig.h"

FTM_RET	FTM_TRACE_printToTerm(FTM_CHAR_PTR szmsg);
FTM_RET	FTM_TRACE_printToFile(FTM_CHAR_PTR szMsg, FTM_CHAR_PTR pPath, FTM_CHAR_PTR pPrefix);

extern char *program_invocation_short_name;

static FTM_TRACE_CFG	_xConfig = 
{
	.ulLevel = FTM_TRACE_LEVEL_ALL,
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
		.bLine	= FTM_TRUE
	},
};


FTM_RET	FTM_TRACE_configLoad(FTM_TRACE_CFG_PTR pCfg, FTM_CHAR_PTR pFileName)
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
			pCfg->ulLevel = (FTM_ULONG)config_setting_get_int(pField);
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

FTM_RET	FTM_TRACE_configSet(FTM_TRACE_CFG_PTR pCfg)
{
	if (pCfg != NULL)
	{
		memcpy(&_xConfig, pCfg, sizeof(FTM_TRACE_CFG));	
	}

	FTM_TRACE_setLevel(_xConfig.ulLevel);

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

FTM_RET FTM_TRACE_setLevel
(
	FTM_ULONG ulLevel
)
{
	_xConfig.ulLevel = ulLevel;

	return	FTM_RET_OK;
}
FTM_RET	FTM_TRACE_getLevel
(
	FTM_ULONG_PTR pulLevel
)
{
	*pulLevel = _xConfig.ulLevel;

	return	FTM_RET_OK;
}


FTM_RET	FTM_TRACE_out
(
	unsigned long	ulLevel,
	const char *	pFunction,
	int				nLine,
	int				bTimeInfo,
	const char *	format, 
	...
)
{
	static	FTM_INT	nOutputLine = 0;
    va_list 		argptr;
	time_t			xTime;
	FTM_BOOL		bFile = FTM_FALSE;
	FTM_BOOL		bLine = FTM_FALSE;
	FTM_CHAR_PTR	pPath;
	FTM_CHAR_PTR	pPrefix;
	FTM_INT			nLen = 0;
	FTM_CHAR		szBuff[2048];
	FTM_CHAR		szTime[32];

	if (ulLevel < _xConfig.ulLevel)
	{
		return FTM_RET_OK;	
	}

	switch(ulLevel)
	{
	case	FTM_TRACE_LEVEL_TRACE:
		if (_xConfig.xTrace.bToFile)
		{
			bFile 	= FTM_TRUE;
			pPath	= _xConfig.xTrace.pPath;
			pPrefix = _xConfig.xTrace.pPrefix;
		}
		bLine	= _xConfig.xTrace.bLine;
		break;

	case	FTM_TRACE_LEVEL_ERROR:
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
	xTime = time(NULL);
	strcpy(szTime, ctime(&xTime));
	szTime[strlen(szTime)-1] = '\0';

	if (bTimeInfo)
	{
		nLen  = snprintf( szBuff, sizeof(szBuff) - 1, "%4d : [%s] - ", ++nOutputLine, szTime);
	}
	if (bLine && (pFunction != NULL))
	{
		nLen += snprintf( &szBuff[nLen], sizeof(szBuff) - nLen - 1, "%32s[%4d] - ", pFunction, nLine);
	}
	nLen += vsnprintf( &szBuff[nLen], sizeof(szBuff) - nLen - 1, format, argptr);
	va_end(argptr);

	szBuff[nLen] = '\0';

	if (bFile)
	{
		FTM_TRACE_printToFile(szBuff, pPath, pPrefix);

	}
	else
	{
		FTM_TRACE_printToTerm(szBuff);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_TRACE_printToTerm(FTM_CHAR_PTR szMsg)
{
	fprintf(stdout, "%s", szMsg);

	return	FTM_RET_OK;
}

FTM_RET	FTM_TRACE_printToFile(FTM_CHAR_PTR szMsg, FTM_CHAR_PTR pPath, FTM_CHAR_PTR pPrefix)
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
		return FTM_TRACE_printToTerm(szMsg);	
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

FTM_RET	FTM_TRACE_consoleCmd(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{

	return	FTM_RET_OK;
}

struct 
{
	FTM_ULONG		ulLevel;
	FTM_CHAR_PTR	pName;
} FTM_levelStrings[] =
{
	{ FTM_TRACE_LEVEL_ALL, "ALL" },
	{ FTM_TRACE_LEVEL_TRACE, "TRACE"},
	{ FTM_TRACE_LEVEL_DEBUG, "DEBUG"},
	{ FTM_TRACE_LEVEL_INFO, "INFO"},
	{ FTM_TRACE_LEVEL_WARN, "WARN"}, 
	{ FTM_TRACE_LEVEL_ERROR, "ERROR"},
	{ FTM_TRACE_LEVEL_FATAL, "FATAL"},
};

FTM_CHAR_PTR	FTM_TRACE_levelString(FTM_ULONG ulLevel)
{
	FTM_ULONG	i;

	for(i = 0 ; i < sizeof(FTM_levelStrings) / sizeof(FTM_levelStrings[0]) ; i++)
	{
		if (FTM_levelStrings[i].ulLevel == ulLevel)
		{
			return	FTM_levelStrings[i].pName;	
		}
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
