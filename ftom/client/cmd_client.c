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
	FTM_RET	xRet;

	if (nArgc == 1)
	{
		MESSAGE("\n# Client configuration\n");
	}
	else if (nArgc == 2)
	{
		FTOM_CLIENT_NET_PTR	pNetClient = (FTOM_CLIENT_NET_PTR)pClient;

		if(strcasecmp(pArgv[1], "server") == 0)
		{
			MESSAGE("# Server Configuration.\n");
			MESSAGE("%16s : %s\n", "Host", pNetClient->xConfig.xServer.pHost);
			MESSAGE("%16s : %lu\n", "Port", (FTM_ULONG)pNetClient->xConfig.xServer.usPort);

		}
	}

	return	FTM_RET_OK;
}
