#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm_shell.h"
#include "ftm_trace.h"
#include "ftm_list.h"

static FTM_RET	FTM_SHELL_parseLine
(
	FTM_CHAR_PTR 	pLine, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_INT 		nMaxArgs, 
	FTM_INT_PTR 	pArgc
);

static FTM_RET FTM_SHELL_getCmd
(
	FTM_CHAR_PTR 				pCmdString, 
	FTM_SHELL_CMD_PTR _PTR_ 	ppCmd
);

static FTM_RET	FTM_SHELL_cmdHelp
(
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[]
);

static FTM_RET	FTM_SHELL_cmdQuit
(
	FTM_INT nArgc, FTM_CHAR_PTR pArgv[]
);

static int FTM_SHELL_seeker
(
	const void *pItem, 
	const void *pKey
);

static int FTM_SHELL_comparator
(
	const void *pItem1, 
	const void *pItem2
);

FTM_SHELL_CMD	xDefaultCmds[] = 
{
	{	
		.pString	= "help",
		.function	= FTM_SHELL_cmdHelp,
		.pShortHelp = "Help command.",
		.pHelp	    = "<COMMAND>\n"\
					  "\tHelp command.\n"\
					  "PARAMETERS:\n"\
					  "\tCOMMAND    Target command for help."
	},
	{	
		.pString	= "?",
		.function	= FTM_SHELL_cmdHelp,
		.pShortHelp = "Help command.",
		.pHelp	    = "<COMMAND>\n"\
					  "\tHelp command.\n"\
					  "PARAMETERS:\n"\
					  "\tCOMMAND    Target command for help."
	},
	{	
		.pString	= "quit",
		.function	= FTM_SHELL_cmdQuit,
		.pShortHelp = "Quit program.",
		.pHelp 		= "\n"\
					  "\tQuit program."
	},
	{	
		.pString	= NULL,
	}
};

FTM_CHAR		pConsolePrompt[128] = "FTM > ";
FTM_LIST_PTR	pCmdList = NULL;

FTM_RET FTM_SHELL_run(void)
{
	FTM_RET			nRet;
	FTM_BOOL		bQuit = FTM_FALSE;
	FTM_CHAR		pCmdLine[2048];
	FTM_INT			nArgc;
	FTM_CHAR_PTR	pArgv[FTM_SHELL_MAX_ARGS];

	
	while(!bQuit)
	{
		printf("%s", pConsolePrompt);

		memset(pCmdLine, 0, sizeof(pCmdLine));
		fgets(pCmdLine, sizeof(pCmdLine), stdin);

		FTM_SHELL_parseLine(pCmdLine, pArgv, FTM_SHELL_MAX_ARGS, &nArgc);

		if (nArgc != 0)
		{
			FTM_SHELL_CMD_PTR 	pCmd;

			nRet = FTM_SHELL_getCmd(pArgv[0], &pCmd);
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

				case	FTM_RET_SHELL_QUIT:
					{
						bQuit = FTM_TRUE;
					}
				}
			}
			else
			{
				FTM_CHAR_PTR pNewArgv[] = {"help"};
				MESSAGE("%s is invalid command.\n", pArgv[0]);
				FTM_SHELL_cmdHelp(1, pNewArgv);

			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET FTM_SHELL_init(FTM_VOID)
{
	FTM_SHELL_CMD_PTR	pCmd;

	FTM_LIST_create(&pCmdList);
	FTM_LIST_setSeeker(pCmdList, FTM_SHELL_seeker);
	FTM_LIST_setComparator(pCmdList, FTM_SHELL_comparator);

	pCmd = xDefaultCmds;
	while(pCmd->pString != NULL)
	{
		FTM_LIST_insert(pCmdList, pCmd, FTM_LIST_POS_ASSENDING);
		pCmd++;	
	}

	return	FTM_RET_OK;
}	

FTM_RET	FTM_SHELL_final(FTM_VOID)
{
	FTM_LIST_destroy(pCmdList);

	return	FTM_RET_OK;
}

FTM_RET	FTM_SHELL_setPrompt
(
	FTM_CHAR_PTR		pPrompt
)
{
	strncpy(pConsolePrompt, pPrompt, sizeof(pConsolePrompt));

	return	FTM_RET_OK;
}


FTM_RET	FTM_SHELL_addCmds(FTM_SHELL_CMD_PTR pCmds, FTM_ULONG ulCmds)
{
	FTM_ULONG	i;

	for(i = 0 ; i < ulCmds; i++)
	{
		FTM_LIST_insert(pCmdList, &pCmds[i], FTM_LIST_POS_ASSENDING);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_SHELL_appendCmd(FTM_SHELL_CMD_PTR pCmd)
{
	FTM_SHELL_CMD_PTR	pExistCmd;

	if (FTM_LIST_get(pCmdList, pCmd->pString, (FTM_VOID_PTR _PTR_)&pExistCmd) == FTM_RET_OK)
	{
		FTM_LIST_remove(pCmdList, pExistCmd);
	}

	FTM_LIST_insert(pCmdList, pCmd, FTM_LIST_POS_ASSENDING);

	return	FTM_RET_OK;
}

FTM_RET FTM_SHELL_getCmd(FTM_CHAR_PTR pCmdString, FTM_SHELL_CMD_PTR _PTR_ ppCmd)
{
	FTM_SHELL_CMD_PTR pCmd;

	if (FTM_LIST_get(pCmdList, pCmdString, (FTM_VOID_PTR _PTR_)&pCmd) == FTM_RET_OK)
	{
		*ppCmd = pCmd;
		return	FTM_RET_OK;
	}

	return	FTM_RET_INVALID_COMMAND;
}

FTM_RET	FTM_SHELL_parseLine(FTM_CHAR_PTR pLine, FTM_CHAR_PTR pArgv[], FTM_INT nMaxArgs, FTM_INT_PTR pArgc)
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

FTM_RET	FTM_SHELL_cmdHelp(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{

	switch(nArgc)
	{
	case	1:
		{
			FTM_SHELL_CMD_PTR	pCmd; 
		
			FTM_LIST_iteratorStart(pCmdList);
			while(FTM_LIST_iteratorNext(pCmdList, (FTM_VOID_PTR _PTR_)&pCmd) == FTM_RET_OK)
			{
				MESSAGE("%-16s    %s\n", pCmd->pString, pCmd->pShortHelp); 
			}
		}
		break;

	case	2:
		{
			FTM_SHELL_CMD_PTR pCmd;

			if (FTM_SHELL_getCmd(pArgv[1], &pCmd) == FTM_RET_OK)
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

FTM_RET	FTM_SHELL_cmdQuit(FTM_INT nArgc, FTM_CHAR_PTR pArgv[])
{
	return	FTM_RET_SHELL_QUIT;
}

int FTM_SHELL_seeker(const void *pItem, const void *pKey)
{
	FTM_SHELL_CMD_PTR	pCmd = (FTM_SHELL_CMD_PTR)pItem;
	FTM_CHAR_PTR			pCmdString = (FTM_CHAR_PTR)pKey;

	return	(strcmp(pCmd->pString, pCmdString) == 0);
}

int FTM_SHELL_comparator(const void *pItem1, const void *pItem2)
{
	FTM_SHELL_CMD_PTR	pCmd1 = (FTM_SHELL_CMD_PTR)pItem1;
	FTM_SHELL_CMD_PTR	pCmd2 = (FTM_SHELL_CMD_PTR)pItem2;

	return	strcmp(pCmd1->pString, pCmd2->pString);
}
