#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm_console.h"
#include "ftm_error.h"
#include "ftm_debug.h"
#include "simclist.h"

static FTM_RET	FTM_CONSOLE_parseLine
(
	FTM_CHAR_PTR 	pLine, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_INT 		nMaxArgs, 
	FTM_INT_PTR 	pArgc
);

static FTM_RET FTM_CONSOLE_getCmd
(
	FTM_CHAR_PTR 				pCmdString, 
	FTM_CONSOLE_CMD_PTR _PTR_ 	ppCmd
);

static FTM_RET	FTM_CONSOLE_cmdHelp
(
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[]
);

static FTM_RET	FTM_CONSOLE_cmdQuit
(
	FTM_INT nArgc, FTM_CHAR_PTR pArgv[]
);

static int FTM_CONSOLE_compCmd
(
	const void *pItem, 
	const void *pKey
);

FTM_CONSOLE_CMD	xDefaultCmds[] = 
{
	{	
		.pString	= "help",
		.function	= FTM_CONSOLE_cmdHelp,
		.pShortHelp = "Help command.",
		.pHelp	    = "<COMMAND>\n"\
					  "\tHelp command.\n"\
					  "PARAMETERS:\n"\
					  "\tCOMMAND    Target command for help."
	},
	{	
		.pString	= "?",
		.function	= FTM_CONSOLE_cmdHelp,
		.pShortHelp = "Help command.",
		.pHelp	    = "<COMMAND>\n"\
					  "\tHelp command.\n"\
					  "PARAMETERS:\n"\
					  "\tCOMMAND    Target command for help."
	},
	{	
		.pString	= "quit",
		.function	= FTM_CONSOLE_cmdQuit,
		.pShortHelp = "Quit program.",
		.pHelp 		= "\n"\
					  "\tQuit program."
	},
	{	
		.pString	= NULL,
	}
};

FTM_CHAR		pConsolePrompt[128] = "FTM > ";
list_t			xCmdList;

FTM_RET FTM_CONSOLE_run(FTM_CONSOLE_CMD_PTR pExtCmds, FTM_ULONG ulExtCmds)
{
	FTM_RET			nRet;
	FTM_BOOL		bQuit = FTM_BOOL_FALSE;
	FTM_CHAR		pCmdLine[2048];
	FTM_INT			nArgc;
	FTM_CHAR_PTR	pArgv[FTM_CONSOLE_MAX_ARGS];

	FTM_CONSOLE_init(pExtCmds, ulExtCmds);
	
	while(!bQuit)
	{
		printf("%s", pConsolePrompt);

		memset(pCmdLine, 0, sizeof(pCmdLine));
		fgets(pCmdLine, sizeof(pCmdLine), stdin);

		FTM_CONSOLE_parseLine(pCmdLine, pArgv, FTM_CONSOLE_MAX_ARGS, &nArgc);

		if (nArgc != 0)
		{
			FTM_CONSOLE_CMD_PTR 	pCmd;

			nRet = FTM_CONSOLE_getCmd(pArgv[0], &pCmd);
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
				FTM_CHAR_PTR pNewArgv[] = {"help"};
				MESSAGE("%s is invalid command.\n", pArgv[0]);
				FTM_CONSOLE_cmdHelp(1, pNewArgv);

			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET FTM_CONSOLE_init(FTM_CONSOLE_CMD_PTR pExtCmds, FTM_ULONG ulExtCmds)
{
	FTM_CONSOLE_CMD_PTR	pCmd;
	FTM_ULONG			i;

	list_init(&xCmdList);
	list_attributes_seeker(&xCmdList, FTM_CONSOLE_compCmd);

	pCmd = xDefaultCmds;
	while(pCmd->pString != NULL)
	{
		list_append(&xCmdList, pCmd);
		pCmd++;	
	}

	for(i = 0 ; i < ulExtCmds ; i++)
	{
		list_append(&xCmdList, &pExtCmds[i]);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONSOLE_final(FTM_VOID)
{
	list_destroy(&xCmdList);

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONSOLE_setPrompt
(
	FTM_CHAR_PTR		pPrompt
)
{
	strncpy(pConsolePrompt, pPrompt, sizeof(pConsolePrompt));

	return	FTM_RET_OK;
}

FTM_RET	FTM_CONSOLE_appendCmd(FTM_CONSOLE_CMD_PTR pCmd)
{
	if (list_seek(&xCmdList, pCmd->pString) != NULL)
	{
		return	FTM_RET_ALREADY_EXISTS;	
	}

	list_append(&xCmdList, pCmd);

	return	FTM_RET_OK;
}

FTM_RET FTM_CONSOLE_getCmd(FTM_CHAR_PTR pCmdString, FTM_CONSOLE_CMD_PTR _PTR_ ppCmd)
{
	FTM_CONSOLE_CMD_PTR pCmd;
	
	pCmd = (FTM_CONSOLE_CMD_PTR)list_seek(&xCmdList, pCmdString);
	if (pCmd != NULL)
	{
		*ppCmd = pCmd;
		return	FTM_RET_OK;
	}

	return	FTM_RET_INVALID_COMMAND;
}

FTM_RET	FTM_CONSOLE_parseLine(FTM_CHAR_PTR pLine, FTM_CHAR_PTR pArgv[], FTM_INT nMaxArgs, FTM_INT_PTR pArgc)
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

FTM_RET	FTM_CONSOLE_cmdHelp(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{

	switch(nArgc)
	{
	case	1:
		{
			FTM_CONSOLE_CMD_PTR	pCmd; 
			
			list_iterator_start(&xCmdList);
			while((pCmd = (FTM_CONSOLE_CMD_PTR)list_iterator_next(&xCmdList)) != NULL)
			{
				MESSAGE("%-16s    %s\n", pCmd->pString, pCmd->pShortHelp);
			}
			list_iterator_stop(&xCmdList);
		}
		break;

	case	2:
		{
			FTM_CONSOLE_CMD_PTR pCmd;

			if (FTM_CONSOLE_getCmd(pArgv[1], &pCmd) == FTM_RET_OK)
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

FTM_RET	FTM_CONSOLE_cmdQuit(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	return	FTM_RET_CONSOLE_QUIT;
}

int FTM_CONSOLE_compCmd(const void *pItem, const void *pKey)
{
	FTM_CONSOLE_CMD_PTR	pCmd = (FTM_CONSOLE_CMD_PTR)pItem;
	FTM_CHAR_PTR			pCmdString = (FTM_CHAR_PTR)pKey;

	return	(strcmp(pCmd->pString, pCmdString) == 0);
}
