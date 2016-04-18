#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include "ftm.h"
#include "ftom.h"
#include "ftm_trace.h"
#include "ftom_params.h"
#include "ftom_client_cmdline.h"
#include "ftom_cl_cmd.h"


FTM_RET	FTOM_CL_CGI_getSensorList
(
	FTOM_CLIENT_PTR	pClient,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[]
)
{
	FTM_RET		xRet;
	FTM_ULONG	ulCount;
	FTM_INT		i;

	xRet = FTOM_CLIENT_EP_count(pClient, 0, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}


	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_EP	xEPInfo;

		xRet = FTOM_CLIENT_EP_getAt(pClient, i, &xEPInfo);
		if (xRet == FTM_RET_OK)
		{
			MESSAGE("%s|%08x|%s|%s|1\n", 
				xEPInfo.pDID,
				xEPInfo.xEPID,
				FTM_EP_typeString(xEPInfo.xType),
				xEPInfo.pName);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CL_CGI_getSensingList
(
	FTOM_CLIENT_PTR	pClient,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[]
)
{
	FTM_RET		xRet;
	FTM_ULONG	ulEPCount;
	FTM_INT		i;
	FTM_INT		nIndex;
	FTM_EP_DATA_PTR	pData;
	FTM_ULONG	ulMaxCount = 100;

	xRet = FTOM_CLIENT_EP_count(pClient, 0, &ulEPCount);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	pData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA) * ulMaxCount);
	if (pData == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	nIndex = 0;
	for(i = 0 ; i < ulEPCount ; i++)
	{
		FTM_EP		xEPInfo;
		FTM_ULONG	ulDataCount = 0;

		xRet = FTOM_CLIENT_EP_getAt(pClient, i, &xEPInfo);
		if (xRet == FTM_RET_OK)
		{
			xRet = FTOM_CLIENT_EP_DATA_getList(	pClient, xEPInfo.xEPID, 0, pData, ulMaxCount, &ulDataCount);
			if (xRet == FTM_RET_OK)
			{
				FTM_INT		j;
				FTM_CHAR	pBuff[64];

				for(j = 0 ; j < ulDataCount ; j++)
				{
					FTM_EP_DATA_snprint(pBuff, sizeof(pBuff), &pData[i]);
					MESSAGE("%d|%s|%08x|%s|%s|1234|run|0|1|123456|10|123456|123456|1\n", 
						++nIndex,
						xEPInfo.pDID,
						xEPInfo.xEPID,
						FTM_EP_typeString(xEPInfo.xType),
						xEPInfo.pName);
				}
			}
		}
	}

	FTM_MEM_free(pData);

	return	xRet;
}

