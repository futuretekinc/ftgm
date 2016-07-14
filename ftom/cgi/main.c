#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "ftom_cgi.h"
#include "ftom_client_cmdline.h"
#include "ftm_trace.h"

// ftm manager

FTOM_CGI_COMMAND	pCmds[] =
{
	{	"node",		FTOM_CGI_node	},
	{	"ep",		FTOM_CGI_ep		},
	{	"data",		FTOM_CGI_data	},
	{	"trigger",	FTOM_CGI_trigger},
	{	"action",	FTOM_CGI_action	},
	{	"rule",		FTOM_CGI_rule	},
	{	"log",		FTOM_CGI_log	},
	{	"discovery",FTOM_CGI_discovery},
	{	NULL,		NULL			}
};

FTM_INT	main(FTM_INT	nArgc, FTM_CHAR_PTR pArgv[])
{
    // Parse queries.
    qentry_t *pReq = qcgireq_parse(NULL, 0);

	FTM_RET			xRet;
	FTOM_CGI_COMMAND_PTR	pCmd = &pCmds[0];

	FTM_MEM_init();	

	FTM_TRACE_setModule(FTOM_TRACE_MODULE_NODE, FTM_TRUE);
	FTM_TRACE_setLevel(FTOM_TRACE_MODULE_NODE, FTM_TRACE_LEVEL_ALL);

	while(pCmd->pName != NULL)
	{
		if (strcasecmp(pCmd->pName, pArgv[0]) == 0)
		{
			break;
		}

		pCmd++;
	}

	if (pCmd->pName == NULL)
	{
		qcgires_error(pReq, "Not supported command!");
	}
	else	
	{
		FTOM_CLIENT_PTR	pClient;
		
		xRet = FTOM_CLIENT_CL_create((FTOM_CLIENT_CL_PTR _PTR_)&pClient);
		if (xRet != FTM_RET_OK)
		{
			qcgires_error(pReq, "Internal Error!");
		}


       	pCmd->fService(pClient, pReq);


		FTOM_CLIENT_CL_destroy((FTOM_CLIENT_CL_PTR _PTR_)&pClient);

/*
		char	*lpszSSID = pReq->getstr(pReq, "ssid", false);
		time_t	xTime = time(0);
		if (strcmp(pCmd->lpszName, "public") == 0)
		{
			pCmd->fService(pReq);
		}
		else if ((lpszSSID != NULL) && (strlen(lpszSSID) == 32) &&  FTMC_IsValidSSID(lpszSSID, xTime))
		{
			FTMC_SaveSSID(lpszSSID, xTime);
			pCmd->fService(pReq);
		}
		else
		{
			FTMC_LoadIndex(pReq);
		}
        */
	}	

	FTM_MEM_final();

    // De-allocate memories
    pReq->free(pReq);

    return 0;
}

