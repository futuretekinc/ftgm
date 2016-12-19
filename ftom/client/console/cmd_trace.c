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
	return	FTM_TRACE_shellCmd(pShell, nArgc, pArgv, pData);
}
