#include <string.h>
#include <time.h>
#include "ftm_trace.h"
#include "ftm_mem.h"
#include "ftm_snmp.h"
#include "ftdm_shell_cmds.h"
#include "ftdm_node.h"
#include "ftdm_ep.h"
#include "ftdm_config.h"



FTM_RET	FTDM_SHELL_delEPData
(
	FTDM_EPM_PTR 	pEPM, 
	FTM_CHAR_PTR	pEPID, 
	FTM_INT 		nIndex, 
	FTM_ULONG 		ulCount
)
{
	FTM_RET	xRet;
	FTM_ULONG	ulTotalCount, ulNewCount;
	FTDM_EP_PTR	pEP;

	if (FTDM_EPM_get(pEPM, pEPID, &pEP) != FTM_RET_OK)
	{
		MESSAGE("Invalid EPID [%s]\n", pEPID);
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	xRet = FTDM_EP_getDataCount(pEP, &ulTotalCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "EP[%s] is not exists.\n", pEPID);
		return	xRet;
	}

	if (nIndex > 0)
	{
		xRet = FTDM_EP_deleteData(pEP, nIndex - 1, ulCount, &ulCount);
	}
	else
	{
		xRet = FTDM_EP_deleteData(pEP, ulTotalCount + nIndex , ulCount, &ulCount);
	}

	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTDM_EP_getDataCount(pEP, &ulNewCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "EP[%s] is not exists.\n", pEPID);
		return	xRet;
	}

	MESSAGE("%lu data has been deleted.\n", ulTotalCount - ulNewCount);

	return	xRet;
}
