#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <getopt.h>
#include "ftm.h"
#include "ftom.h"
#include "ftom_params.h"
#include "ftom_client_cmdline.h"
#include "ftom_cl_cmd.h"


FTM_RET	FTOM_CL_EP_DATA_getLast
(
	FTOM_CLIENT_PTR	pClient,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[]
)
{
	ASSERT(pClient != NULL);
	ASSERT(pArgv != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pEPID[FTM_EPID_LEN+1];
	FTM_EP_DATA	xData;

	if (nArgc != 1)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	strncpy(pEPID, pArgv[0], FTM_EPID_LEN);

	xRet = FTOM_CLIENT_EP_DATA_getLast(	pClient, pEPID, &xData);
	if (xRet == FTM_RET_OK)
	{
		FTM_CHAR	pBuff[64];

		FTM_EP_DATA_snprint(pBuff, sizeof(pBuff), &xData);
		MESSAGE("%s|%lu|%s\n",
			pEPID,
			xData.ulTime,
			pBuff);
	}

	return	xRet;
}

FTM_RET	FTOM_CL_EP_DATA_getList
(
	FTOM_CLIENT_PTR	pClient,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[]
)
{
	ASSERT(pClient != NULL);
	ASSERT(pArgv != NULL);

	FTM_RET		xRet;
	FTM_CHAR	pEPID[FTM_EPID_LEN+1];
	FTM_ULONG	ulIndex;
	FTM_ULONG	ulMaxCount;
	FTM_ULONG	ulCount;
	FTM_EP_DATA_PTR	pData;

	if (nArgc != 3)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	strncpy(pEPID, pArgv[0], FTM_EPID_LEN);
	ulIndex=strtoul(pArgv[1], 0, 10);
	ulMaxCount=strtoul(pArgv[2], 0, 10);

	if (ulMaxCount > 100)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	pData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA) * ulMaxCount);
	if (pData == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	xRet = FTOM_CLIENT_EP_DATA_getList(	pClient, pEPID, ulIndex, pData, ulMaxCount, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		FTM_INT		i;
		FTM_CHAR	pBuff[64];

		for(i = 0 ; i < ulCount ; i++)
		{
			FTM_EP_DATA_snprint(pBuff, sizeof(pBuff), &pData[i]);
			MESSAGE("%d|%16s|%lu|%s\n", i+1, pEPID, pData[i].ulTime, pBuff);
		}
	}

	FTM_MEM_free(pData);

	return	xRet;
}

