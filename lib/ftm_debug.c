#include <stdarg.h>
#include <time.h>
#include "ftm_types.h"
#include "ftm_debug.h"

void printToTerm(char *szmsg);
void printToFile(char *szMsg);

void (*printToMessage)(char *szMsg) = printToTerm;
void (*printToError)(char *szMsg) = printToTerm;
void (*printToTrace)(char *szMsg) = printToTerm;

static unsigned long	_nPrintLevel = MSG_NORMAL;

FTM_VOID	FTM_dumpPacket
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
}

void setPrintMode
(
	FTM_ULONG nMode
)
{
	switch(nMode)
	{
	case	0:	_nPrintLevel = 0; break;
	case	1:	_nPrintLevel = MSG_NORMAL; break;
	case	2:	_nPrintLevel = MSG_ALL; break;
	}
}
void getPrintMode
(
	FTM_ULONG_PTR pMode
)
{
	*pMode = _nPrintLevel;
}


void printOut
(
	unsigned long	nLevel,
	const char		*pFunction,
	int				nLine,
	char 			*format, 
	...
)
{
    va_list argptr;
	int		nLen = 0;
	char	szBuff[2048];

	if (nLevel & _nPrintLevel)
	{
		va_start ( argptr, format );           
		if (pFunction != NULL)
		{
			nLen  = snprintf( szBuff, sizeof(szBuff) - 1, "%s[%3d] : ", pFunction, nLine);
		}
		nLen += vsnprintf( &szBuff[nLen], sizeof(szBuff) - nLen - 1, format, argptr);
		va_end(argptr);

		szBuff[nLen] = '\0';

		if (nLevel & MSG_NORMAL)
		{
			printToMessage(szBuff);
		}

		if (nLevel & MSG_TRACE)
		{
			printToTrace(szBuff);
		}

		if (nLevel & MSG_ERROR)
		{
			printToError(szBuff);
		}
	}
}

void printToTerm(char *szMsg)
{
	fprintf(stdout, "%s", szMsg);
}

void printToFile(char *szMsg)
{
	FILE 		*pFile;
	time_t		rawTime;
	struct tm 	*localTime;
	char		szTime[32];

	pFile = fopen("/var/log/ftdm.log", "a");
	if (pFile == 0)
	{
		fprintf(stderr, "Can't open log file\n");
		return;	
	}

	
	time(&rawTime);
	localTime = localtime(&rawTime);

	strftime(szTime, 32, "%Y:%m:%d %H:%M:%S", localTime);
	fprintf(pFile, "%s : %s", szTime, szMsg);
	fclose(pFile);
}
