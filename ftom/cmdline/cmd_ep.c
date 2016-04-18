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

FTM_RET	FTOM_CL_EP_destroy
(
	FTOM_CLIENT_PTR	pClient,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[]
)
{
	ASSERT(pClient != NULL);
	ASSERT(pArgv != NULL);

	FTM_RET		xRet;
	FTM_EP_ID	xEPID;

	if (nArgc != 1)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xEPID = strtoul(pArgv[0], 0, 16);

	xRet = FTOM_CLIENT_EP_destroy(pClient, xEPID);

	return	xRet;
}

FTM_RET	FTOM_CL_EP_count
(
	FTOM_CLIENT_PTR	pClient,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[]
)
{
	ASSERT(pClient != NULL);
	ASSERT(pArgv != NULL);

	FTM_RET		xRet;
	FTM_ULONG	ulCount;

	xRet = FTOM_CLIENT_EP_count(pClient, 0, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		MESSAGE("%d\n",	ulCount);
	}
	return	xRet;	
}

FTM_RET	FTOM_CL_EP_get
(
	FTOM_CLIENT_PTR pClient, 
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[]
)
{
	ASSERT(pClient != NULL);
	ASSERT(pArgv != NULL);

	FTM_RET		xRet;
	FTM_EP_ID	xEPID;
	FTM_EP		xInfo;

	if (nArgc < 1)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xEPID = strtoul(pArgv[0], 0, 16);
	xRet = FTOM_CLIENT_EP_get(pClient, xEPID, &xInfo);
	if (xRet == FTM_RET_OK)
	{
		MESSAGE("%08x|%s|%08x|%s|%s|%d|%d|%d|%s|%08x|%s|%08x\n", 
			xInfo.xEPID, 
			FTM_EP_typeString(xInfo.xType), 
			xInfo.xFlags, 
			xInfo.pName, 
			xInfo.pUnit,
			xInfo.bEnable,
			xInfo.ulTimeout,
			xInfo.ulInterval,
			xInfo.pDID,
			xInfo.xDEPID,
			xInfo.pPID,
			xInfo.xPEPID);
	}

	return	xRet;
}

FTM_RET	FTOM_CL_EP_getAt
(
	FTOM_CLIENT_PTR	pClient,
	FTM_INT			nArgc, 
	FTM_CHAR_PTR	pArgv[]
)
{
	ASSERT(pClient != NULL);
	ASSERT(pArgv != NULL);

	FTM_RET		xRet;
	FTM_ULONG	ulIndex;
	FTM_EP	xInfo;

	if (nArgc < 1)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	ulIndex = atoi(pArgv[0]);
	xRet = FTOM_CLIENT_EP_getAt(pClient, ulIndex, &xInfo);
	if (xRet == FTM_RET_OK)
	{
		MESSAGE("%08x|%s|%08x|%s|%s|%d:%d|%d|%s|%08x|%s|%08x\n", 
			xInfo.xEPID, 
			FTM_EP_typeString(xInfo.xType), 
			xInfo.xFlags, 
			xInfo.pName, 
			xInfo.pUnit,
			xInfo.bEnable,
			xInfo.ulTimeout,
			xInfo.ulInterval,
			xInfo.pDID,
			xInfo.xDEPID,
			xInfo.pPID,
			xInfo.xPEPID);
	}

	return	xRet;
}

