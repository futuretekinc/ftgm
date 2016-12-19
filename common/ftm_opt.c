#include <string.h>
#include "ftm_opt.h"
#include "ftm_trace.h"

FTM_RET	FTM_getOptions
(
	FTM_INT			nArgLen,
	FTM_CHAR_PTR	pArgv[],
	FTM_CHAR_PTR	pIndicators,
	FTM_OPT_PTR		pOpts,
	FTM_ULONG		ulMaxOpts,
	FTM_ULONG_PTR	pulOptCount
)
{
	ASSERT(pArgv != NULL);
	ASSERT(pIndicators != NULL);
	ASSERT(pOpts != NULL);
	ASSERT(pulOptCount != NULL);

	FTM_INT		nArg = 0;
	FTM_INT		nOpts = 0;
	FTM_INT		nIndicatorsLen = strlen(pIndicators);

	while(nArg < nArgLen)
	{   
		if (pArgv[nArg][0] == '-')
		{  
			FTM_INT	i;

			i = 0;
			while(i < nIndicatorsLen)
			{
				if (pIndicators[i] == pArgv[nArg][1])
				{
					if (((i+1) < nIndicatorsLen) && (pIndicators[i+1] == ':'))
					{
						if (nArg+1 >= nArgLen)
						{
							return	FTM_RET_INVALID_ARGUMENTS;
						}

						pOpts[nOpts].xOpt = pArgv[nArg++][1];
						pOpts[nOpts].pParam = pArgv[nArg++];
					}
					else
					{
						pOpts[nOpts].xOpt = pArgv[nArg++][1];
						pOpts[nOpts].pParam = NULL;
					}

					nOpts += 1;

					break;
				}

				i++;
			}

			if (i == nIndicatorsLen)
			{
				return	FTM_RET_INVALID_ARGUMENTS;	
			}
		}
		else
		{
			nArg++;
		}
	}   

	*pulOptCount = nOpts;

	return	FTM_RET_OK;
}
