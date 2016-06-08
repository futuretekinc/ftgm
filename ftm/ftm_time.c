#include <time.h>
#include <string.h>
#include "ftm_time.h"
#include "ftm_trace.h"

FTM_RET	FTM_TIME_getCurrent(FTM_TIME_PTR pTime)
{
	ASSERT(pTime != NULL);
	
	gettimeofday(&pTime->xTimeval, NULL);

	return	FTM_RET_OK;
}

FTM_BOOL	FTM_TIME_isZero(FTM_TIME_PTR pTime)
{
	return	((pTime == NULL) || ((pTime->xTimeval.tv_sec == 0) && (pTime->xTimeval.tv_usec == 0)));
}

FTM_RET	FTM_TIME_set(FTM_TIME_PTR pTime, FTM_ULONG ulMilliSeconds)
{
	ASSERT(pTime != NULL);
	
	pTime->xTimeval.tv_sec = ulMilliSeconds / 1000;
	pTime->xTimeval.tv_usec = ulMilliSeconds % 1000 * 1000;

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_setSeconds(FTM_TIME_PTR pTime, FTM_ULONG ulSeconds)
{
	ASSERT(pTime != NULL);
	
	pTime->xTimeval.tv_sec = ulSeconds;
	pTime->xTimeval.tv_usec = 0;

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_add(FTM_TIME_PTR pTime1, FTM_TIME_PTR pTime2, FTM_TIME_PTR pTimeR)
{
	ASSERT(pTime1 != NULL);
	ASSERT(pTime2 != NULL);
	ASSERT(pTimeR != NULL);
	
	FTM_TIME	xTimeR;

	xTimeR.xTimeval.tv_usec = pTime1->xTimeval.tv_usec + pTime2->xTimeval.tv_usec;
	xTimeR.xTimeval.tv_sec = pTime1->xTimeval.tv_sec + pTime2->xTimeval.tv_sec + xTimeR.xTimeval.tv_usec / 1000000;
	xTimeR.xTimeval.tv_usec = xTimeR.xTimeval.tv_usec % 1000000;

	memcpy(pTimeR, &xTimeR, sizeof(FTM_TIME));

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_sub(FTM_TIME_PTR pTime1, FTM_TIME_PTR pTime2, FTM_TIME_PTR pTimeR)
{
	ASSERT(pTime1 != NULL);
	ASSERT(pTime2 != NULL);
	ASSERT(pTimeR != NULL);

	FTM_INT64 	nValue1, nValue2, nValueR;

	nValue1 = (FTM_INT64)pTime1->xTimeval.tv_sec * 1000000 + pTime1->xTimeval.tv_usec;
	nValue2 = (FTM_INT64)pTime2->xTimeval.tv_sec * 1000000 + pTime2->xTimeval.tv_usec;

	nValueR = nValue1 - nValue2;
	if (nValueR > 0)
	{
		pTimeR->xTimeval.tv_sec = nValueR / 1000000;
		pTimeR->xTimeval.tv_usec = nValueR % 1000000;
	}
	else
	{
		pTimeR->xTimeval.tv_sec = 0;
		pTimeR->xTimeval.tv_usec = 0;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_TIME_align
(
	FTM_TIME_PTR	pTimeS,
	FTM_TIME_ALIGN	xAlign,
	FTM_TIME_PTR	pTimeR
)
{
	ASSERT(pTimeS != NULL);
	ASSERT(pTimeR != NULL);

	switch(xAlign)
	{
	case	FTM_TIME_ALIGN_1S:	
		{
			pTimeR->xTimeval.tv_sec	= pTimeS->xTimeval.tv_sec;
			pTimeR->xTimeval.tv_usec= 0;
		}
		break;

	case	FTM_TIME_ALIGN_10S:	
		{
			pTimeR->xTimeval.tv_sec	= pTimeS->xTimeval.tv_sec / 10 * 10;
			pTimeR->xTimeval.tv_usec= 0;
		}
		break;

	case	FTM_TIME_ALIGN_1M:	
		{
			pTimeR->xTimeval.tv_sec	= pTimeS->xTimeval.tv_sec / 60 * 60;
			pTimeR->xTimeval.tv_usec= 0;
		}
		break;

	case	FTM_TIME_ALIGN_10M:	
		{
			pTimeR->xTimeval.tv_sec	= pTimeS->xTimeval.tv_sec / 600 * 600;
			pTimeR->xTimeval.tv_usec= 0;
		}
		break;

	case	FTM_TIME_ALIGN_1H:	
		{
			pTimeR->xTimeval.tv_sec	= pTimeS->xTimeval.tv_sec / 3600 * 3600;
			pTimeR->xTimeval.tv_usec= 0;
		}
		break;

	case	FTM_TIME_ALIGN_1D:	
		{
			pTimeR->xTimeval.tv_sec	= pTimeS->xTimeval.tv_sec / (24 * 3600) * (24 * 3600);
			pTimeR->xTimeval.tv_usec= 0;
		}
		break;

	default:
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	return	FTM_RET_OK;
}

FTM_CHAR_PTR	FTM_TIME_toString
(
	FTM_TIME_PTR	pTime,
	FTM_CHAR_PTR	pFormat
)
{
	ASSERT(pTime != NULL);

	static	FTM_CHAR	pString[64];

	if (pFormat != NULL)
	{
		strftime(pString, sizeof(pString) - 1, pFormat, localtime(&pTime->xTimeval.tv_sec));
	}
	else
	{
		strftime(pString, sizeof(pString) - 1, "%Y-%m-%d %H:%M:%S", localtime(&pTime->xTimeval.tv_sec));
	}

	return	pString;
}

FTM_RET	FTM_TIME_toMS
(
	FTM_TIME_PTR	pTime,
	FTM_UINT64_PTR	pullMS
)
{
	ASSERT(pTime != NULL);
	ASSERT(pullMS != NULL);

	*pullMS = pTime->xTimeval.tv_sec * (FTM_UINT64)1000 + pTime->xTimeval.tv_usec / 1000;

	return	FTM_RET_OK;
}
