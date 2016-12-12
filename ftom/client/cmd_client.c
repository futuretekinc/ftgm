#include "ftm.h"
#include "ftm_trace.h"
#include "ftm_snmp.h"
#include "ftom.h"
#include "ftom_client.h"
#include "ftom_client_net.h"

FTM_RET	FTOM_CLIENT_CMD_client
(
	FTM_SHELL_PTR	pShell,
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_VOID_PTR 	pData
)
{
	ASSERT(pData != NULL);
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	if (strcasecmp(pArgv[1], "config") == 0)
	{
		FTM_TRACE_printConfig(NULL);
	}
	else if(strcasecmp(pArgv[1], "server") == 0)
	{
		FTOM_CLIENT_showConfig(pClient);
	}

	return	FTM_RET_OK;
}
