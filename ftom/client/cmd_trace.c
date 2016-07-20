#include "ftm.h"
#include "ftm_trace.h"
#include "ftm_snmp.h"
#include "ftom.h"
#include "ftom_client.h"

FTM_RET	FTOM_CLIENT_CMD_trace
(
	FTM_SHELL_PTR	pShell,
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_VOID_PTR 	pData
)
{
	ASSERT(pData != NULL);

	FTM_RET	xRet;

	if (nArgc == 1)
	{
		FTM_ULONG	ulLevel;

		MESSAGE("\n# Trace configuration\n");
		FTM_TRACE_getLevel(FTOM_TRACE_MODULE_CLIENT, &ulLevel);
		MESSAGE("%16s : %s\n", "Level", FTM_TRACE_LEVEL_print(ulLevel, FTM_TRUE));
	}
	else if (nArgc == 3)
	{
		if(strcasecmp(pArgv[1], "level") == 0)
		{
			FTM_ULONG	ulLevel = 0;
			xRet = FTM_TRACE_strToLevel(pArgv[2], &ulLevel);
			if (xRet == FTM_RET_OK)
			{
				FTM_TRACE_setLevel(FTM_TRACE_MODULE_ALL, ulLevel);
			}
			else
			{
				MESSAGE("Failed to set trace level[%s]!\n", pArgv[2]);
			}
		}
	}

	return	FTM_RET_OK;
}
