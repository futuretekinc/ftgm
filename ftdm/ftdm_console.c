#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftdm_console.h"
#include "ftm_error.h"
#include "ftm_debug.h"
#include "simclist.h"

static FTM_RET	FTDM_CONSOLE_parseLine
(
	FTM_CHAR_PTR 	pLine, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_INT 		nMaxArgs, 
	FTM_INT_PTR 	pArgc
);

static FTM_RET FTDM_CONSOLE_getCmd
(
	FTM_CHAR_PTR 				pCmdString, 
	FTDM_CONSOLE_CMD_PTR _PTR_ 	ppCmd
);

static FTM_RET	FTDM_CONSOLE_cmdHelp
(
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[]
);

static FTM_RET	FTDM_CONSOLE_cmdQuit
(
	FTM_INT nArgc, FTM_CHAR_PTR pArgv[]
);

static int FTDM_CONSOLE_compCmd
(
	const void *pItem, 
	const void *pKey
);

FTDM_CONSOLE_CMD	xDefaultCmds[] = 
{
	{	
		.pString	= "help",
		.function	= FTDM_CONSOLE_cmdHelp,
		.pShortHelp = "Help command.",
		.pHelp	    = "<COMMAND>\n"\
					  "\tHelp command.\n"\
					  "PARAMETERS:\n"\
					  "\tCOMMAND    Target command for help."
	},
	{	
		.pString	= "?",
		.function	= FTDM_CONSOLE_cmdHelp,
		.pShortHelp = "Help command.",
		.pHelp	    = "<COMMAND>\n"\
					  "\tHelp command.\n"\
					  "PARAMETERS:\n"\
					  "\tCOMMAND    Target command for help."
	},
	{	
		.pString	= "quit",
		.function	= FTDM_CONSOLE_cmdQuit,
		.pShortHelp = "Quit program.",
		.pHelp 		= "\n"\
					  "\tQuit program."
	},
	{	
		.pString	= NULL,
	}
};

FTM_CHAR		pConsolePrompt[128] = "FTDM > ";
list_t			xCmdList;

FTM_RET	FTDM_CONSOLE_run(void)
{
	FTM_RET			nRet;
	FTM_BOOL		bQuit = FTM_BOOL_FALSE;
	FTM_CHAR		pCmdLine[2048];
	FTM_INT			nArgc;
	FTM_CHAR_PTR	pArgv[FTDM_CONSOLE_MAX_ARGS];

	
	while(!bQuit)
	{
		printf("%s", pConsolePrompt);

		memset(pCmdLine, 0, sizeof(pCmdLine));
		fgets(pCmdLine, sizeof(pCmdLine), stdin);

		FTDM_CONSOLE_parseLine(pCmdLine, pArgv, FTDM_CONSOLE_MAX_ARGS, &nArgc);

		if (nArgc != 0)
		{
			FTDM_CONSOLE_CMD_PTR 	pCmd;

			nRet = FTDM_CONSOLE_getCmd(pArgv[0], &pCmd);
			if (nRet == FTM_RET_OK)
			{
				nRet = pCmd->function(nArgc, pArgv);
				switch(nRet)
				{
				case	FTM_RET_INVALID_ARGUMENTS:
					{
						MESSAGE("Usage : %s %s\n", pCmd->pString, pCmd->pHelp);
					}
					break;

				case	FTM_RET_CONSOLE_QUIT:
					{
						bQuit = FTM_BOOL_TRUE;
					}
				}
			}
			else
			{
				FTM_CHAR_PTR pArgv[] = {"help"};
				MESSAGE("%s is invalid command.\n", pArgv[0]);
				FTDM_CONSOLE_cmdHelp(1, pArgv);

			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET FTDM_CONSOLE_init(FTM_VOID)
{
	FTDM_CONSOLE_CMD_PTR	pCmd;

	list_init(&xCmdList);
	list_attributes_seeker(&xCmdList, FTDM_CONSOLE_compCmd);

	pCmd = xDefaultCmds;
	while(pCmd->pString != NULL)
	{
		list_append(&xCmdList, pCmd);
		pCmd++;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_CONSOLE_final(FTM_VOID)
{
	list_destroy(&xCmdList);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_CONSOLE_setPrompt
(
	FTM_CHAR_PTR		pPrompt
)
{
	strncpy(pConsolePrompt, pPrompt, sizeof(pConsolePrompt));

	return	FTM_RET_OK;
}

FTM_RET FTDM_CONSOLE_getCmd(FTM_CHAR_PTR pCmdString, FTDM_CONSOLE_CMD_PTR _PTR_ ppCmd)
{
	FTDM_CONSOLE_CMD_PTR pCmd;
	
	pCmd = (FTDM_CONSOLE_CMD_PTR)list_seek(&xCmdList, pCmdString);
	if (pCmd != NULL)
	{
		*ppCmd = pCmd;
		return	FTM_RET_OK;
	}

	return	FTM_RET_INVALID_COMMAND;
}

FTM_RET	FTDM_CONSOLE_parseLine(FTM_CHAR_PTR pLine, FTM_CHAR_PTR pArgv[], FTM_INT nMaxArgs, FTM_INT_PTR pArgc)
{
	FTM_INT		nCount = 0;
	FTM_CHAR_PTR	pWord = NULL;
	FTM_CHAR_PTR	pSeperator = "\t \n\r";	

	pWord = strtok(pLine, pSeperator); 
	while((pWord != NULL) && (nCount < nMaxArgs))
	{
		pArgv[nCount++] = pWord;
		pWord = strtok(NULL, pSeperator);
	}

	*pArgc = nCount;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_CONSOLE_cmdHelp(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{

	switch(nArgc)
	{
	case	1:
		{
			FTDM_CONSOLE_CMD_PTR	pCmd; 

			list_iterator_start(&xCmdList);
			while((pCmd = (FTDM_CONSOLE_CMD_PTR)list_iterator_next(&xCmdList)) != NULL)
			{
				MESSAGE("%-16s    %s\n", pCmd->pString, pCmd->pShortHelp);
				pCmd++;	
			}
		}
		break;

	case	2:
		{
			FTDM_CONSOLE_CMD_PTR pCmd;

			if (FTDM_CONSOLE_getCmd(pArgv[1], &pCmd) == FTM_RET_OK)
			{
				MESSAGE("Usage : %s %s\n", pArgv[1], pCmd->pHelp);
			}
		}
		break;

	default:
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_CONSOLE_cmdQuit(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	return	FTM_RET_CONSOLE_QUIT;
}

int FTDM_CONSOLE_compCmd(const void *pItem, const void *pKey)
{
	FTDM_CONSOLE_CMD_PTR	pCmd = (FTDM_CONSOLE_CMD_PTR)pItem;
	FTM_CHAR_PTR			pCmdString = (FTM_CHAR_PTR)pKey;

	return	(strcmp(pCmd->pString, pCmdString) == 0);
}
