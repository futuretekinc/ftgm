#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>
#include "ftom_cgi.h"
#include "ftom_client_cmdline.h"

// ftm manager

FTOM_CGI_COMMAND	pCmds[] =
{
	{	"ftom",		FTOM_CGI_main	},
	{	"ftom.cgi",		FTOM_CGI_main	},
	{	NULL,		NULL			}
};

FTM_INT	main(FTM_INT	nArgc, FTM_CHAR_PTR pArgv[])
{
    // Parse queries.
    qentry_t *pReq = qcgireq_parse(NULL, 0);

	FTM_RET			xRet;
	FTOM_CGI_COMMAND_PTR	pCmd = &pCmds[0];

	FTM_MEM_init();	


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

