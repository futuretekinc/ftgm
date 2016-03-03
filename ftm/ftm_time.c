#include "ftm_time.h"
#include "ftm_error.h"
#include "ftm_debug.h"

FTM_RET	FTM_TIME_getCurrentTime(FTM_TIME_PTR pTime)
{
	ASSERT(pTime != NULL);
	
	gettimeofday(&pTime->xTimeVal, NULL);

	return	FTM_RET_OK;
}
