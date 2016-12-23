#include <ctype.h>
#include <stdlib.h>    
#define	__USE_XOPEN
#include <stdio.h>
#include <string.h>    
#include <unistd.h>    
#define _XOPEN_SOURCE       /* See feature_test_macros(7) */
#include <time.h>    
#include <sys/socket.h> 
#include <arpa/inet.h>
#include "libconfig.h"
#include "ftm.h"
#include "ftm_opt.h"
#include "ftom_client.h"
#include "ftom_modules.h"

#undef	__MODULE__
#define	__MODULE__	FTOM_TRACE_MODULE_CLIENT

FTM_RET	FTOM_CLIENT_CMD_printEventList
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)

{
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;
	FTM_RET		xRet;
	FTM_ULONG	ulCount;
	FTM_ULONG	ulIndex;
	FTM_EVENT	xInfo;

	xRet = FTOM_CLIENT_EVENT_count(pClient, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Failed to get trigger count!");
		return	xRet;
	}

	MESSAGE("\n[ EVENT INFORMATION ]\n");
	MESSAGE("%16s %16s %16s %8s %8s %32s %16s\n", "ID", "NAME", "TYPE", "DETECT", "HOLD", "CONDITION", "EPID");
	for(ulIndex = 0; ulIndex < ulCount ; ulIndex++)
	{
		xRet = FTOM_CLIENT_EVENT_getAt(pClient, ulIndex, &xInfo);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to get the trigger information for the 10th position.\n");
		}
		else
		{
			FTM_CHAR	pCondition[1024];

			FTM_EVENT_conditionToString(&xInfo, pCondition, sizeof(pCondition));

			MESSAGE("%16s %16s %16s %8lu %8lu %32s %16s\n", 
					xInfo.pID, xInfo.pName, FTM_EVENT_typeString(xInfo.xType), 
					xInfo.xParams.xCommon.ulDetectionTime / 1000000,
					xInfo.xParams.xCommon.ulHoldingTime / 1000000,
					pCondition, xInfo.pEPID);
		}
	}

	return	xRet;
}

FTM_RET	FTOM_CLIENT_CMD_printEventInfo
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
	ASSERT(pData != NULL);
	ASSERT(pArgv != NULL);

	FTM_RET		xRet;
	FTM_EVENT	xInfo;
	FTM_INT		i;
	FTM_CHAR	pCondition[1024];
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	for(i = 1 ; i < nArgc ; i++)
	{
		xRet = FTOM_CLIENT_EVENT_get(pClient, pArgv[i], &xInfo);	
		if (xRet != FTM_RET_OK)
		{
			MESSAGE("Event[%s] not found!\n", pArgv[i]);
			return	FTM_RET_OK;
		}
	
	
		MESSAGE("\n[ EVENT INFORMATION ]\n");
		FTM_EVENT_conditionToString(&xInfo, pCondition, sizeof(pCondition));
	
		MESSAGE("%16s : %s\n", 	"ID", 		xInfo.pID); 
		MESSAGE("%16s : %s\n", 	"Name",		xInfo.pName); 
		MESSAGE("%16s : %s\n", 	"Type",		FTM_EVENT_typeString(xInfo.xType));
		MESSAGE("%16s : %8.3f\n","Detect",	xInfo.xParams.xCommon.ulDetectionTime / 1000000.0);
		MESSAGE("%16s : %8.3f\n","Hold", 	xInfo.xParams.xCommon.ulHoldingTime / 1000000.0);
		MESSAGE("%16s : %s\n", 	"Condition", pCondition);
		MESSAGE("%16s : %s\n", 	"EPID", 	xInfo.pEPID);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_CMD_addEvent
(
	FTM_SHELL_PTR	pShell,
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_VOID_PTR 	pData
)
{
	ASSERT(pShell != NULL);
	ASSERT(pArgv != NULL);

	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;
	FTM_RET	xRet;
	FTM_INT	i;
	FTM_EVENT	xInfo;
	FTM_EP		xEPInfo;
	FTM_OPT	pOpts[10];
	FTM_ULONG	ulOpts;
	FTM_ULONG	ulFields = 0;
	FTM_VALUE_TYPE	xValueType;

	FTM_EVENT_setDefault(&xInfo);

	if (nArgc < 5)
	{
		xRet = FTM_RET_INVALID_ARGUMENTS;
		ERROR2(xRet, "The parameters low.\n");
		goto finished;
	}

	xRet = FTOM_CLIENT_EP_get(pClient, pArgv[2], &xEPInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "The EP[%s] was not found!\n", pArgv[2]);
		goto finished;	
	}
	
	xRet = FTM_EP_getDataType(&xEPInfo, &xValueType);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "The data type of EP[%s] is unknown.\n", pArgv[2]);
		goto finished;
	}

	xRet = FTM_getOptions(nArgc, pArgv, "n:t:u:l:v:d:h:", pOpts, 10, &ulOpts);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Invalid arguments.\n");
		goto finished;
	}

	for(i = 0 ; i < ulOpts ; i++)
	{
		switch(pOpts[i].xOpt)
		{
		case	'n':	
			{
				strncpy(xInfo.pName, pOpts[i].pParam, sizeof(xInfo.pName) - 1);
			}
			break;

		case	't':
			{
				xRet = FTM_EVENT_strToType(pOpts[i].pParam, &xInfo.xType);
				if (xRet != FTM_RET_OK)
				{
					ERROR2(xRet, "Invalid trigger type[%s]\n", pOpts[i].pParam);	
					goto finished;
				}
				ulFields |= FTM_EVENT_FIELD_TYPE;
			}
			break;

		case	'd':
			{
				xInfo.xParams.xCommon.ulDetectionTime = strtoul(pOpts[i].pParam, NULL, 10);
			}
			break;

		case	'h':
			{
				xInfo.xParams.xCommon.ulHoldingTime = strtoul(pOpts[i].pParam, NULL, 10);
			}
			break;

		case	'u':
			{
				if ((xInfo.xType != FTM_EVENT_TYPE_INCLUDE) && (xInfo.xType != FTM_EVENT_TYPE_EXCEPT))
				{
					xRet = FTM_RET_INVALID_ARGUMENTS;
					goto finished;
				}

				FTM_VALUE_init(&xInfo.xParams.xInclude.xUpper, xValueType);

				xRet = FTM_VALUE_setFromString(&xInfo.xParams.xInclude.xUpper, pOpts[i].pParam);
				if (xRet != FTM_RET_OK)
				{
					ERROR2(xRet, "Failed to set upper limit!\n");
					goto finished;
				}
				ulFields |= FTM_EVENT_FIELD_UPPER;
			}
			break;

		case	'l':
			{
				if ((xInfo.xType != FTM_EVENT_TYPE_INCLUDE) && (xInfo.xType != FTM_EVENT_TYPE_EXCEPT))
				{
					xRet = FTM_RET_INVALID_ARGUMENTS;
					goto finished;
				}

				FTM_VALUE_init(&xInfo.xParams.xInclude.xLower, xValueType);

				xRet = FTM_VALUE_setFromString(&xInfo.xParams.xInclude.xLower, pOpts[i].pParam);
				if (xRet != FTM_RET_OK)
				{
					ERROR2(xRet, "Failed to set lower limit!\n");
					goto finished;
				}
				ulFields |= FTM_EVENT_FIELD_LOWER;
			}
			break;

		case	'v':
			{
				if ((xInfo.xType != FTM_EVENT_TYPE_ABOVE) && (xInfo.xType != FTM_EVENT_TYPE_BELOW))
				{
					xRet = FTM_RET_INVALID_ARGUMENTS;
					goto finished;
				}

				FTM_VALUE_init(&xInfo.xParams.xAbove.xValue, xValueType);

				xRet = FTM_VALUE_setFromString(&xInfo.xParams.xAbove.xValue, pOpts[i].pParam);
				if (xRet != FTM_RET_OK)
				{
					ERROR2(xRet, "Failed to set value!\n");
					goto finished;
				}
				ulFields |= FTM_EVENT_FIELD_VALUE;
			}
			break;

		default:
			xRet = FTM_RET_INVALID_ARGUMENTS;
			goto finished;
		}
	}

	if (((ulFields & (FTM_EVENT_FIELD_TYPE | FTM_EVENT_FIELD_UPPER | FTM_EVENT_FIELD_LOWER)) != ulFields) &&
	    ((ulFields & (FTM_EVENT_FIELD_TYPE | FTM_EVENT_FIELD_VALUE)) != ulFields))
	{
		ERROR2(xRet, "Invalid arguments.\n");
		xRet = FTM_RET_INVALID_ARGUMENTS;
		goto finished;
	}

	strncpy(xInfo.pID, pArgv[1], FTM_ID_LEN);
	strncpy(xInfo.pEPID, pArgv[2], FTM_ID_LEN);

	xRet = FTOM_CLIENT_EVENT_add(pClient, &xInfo);

finished:
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to add new trigger[%s]\n", pArgv[1]);
	}
	else
	{
		MESSAGE("The trigger[%s] successfully added.\n", pArgv[1]);
	}
	return	xRet;

}

FTM_RET	FTOM_CLIENT_CMD_delEvent
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
	FTM_RET	xRet;
	FTM_INT	i;
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	for(i = 1 ; i < nArgc ;i++)
	{
		xRet = FTOM_CLIENT_EVENT_del(pClient, pArgv[1]);	
		if (xRet != FTM_RET_OK)
		{
			MESSAGE("Failed to remove the trigger[%s].\n", pArgv[i]);
		}
		else
		{
			MESSAGE("The trigger[%s] is removed.\n", pArgv[i]);
		}
	}

	return	xRet;
}

FTM_RET	FTOM_CLIENT_CMD_printEventCount
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
	FTM_RET		xRet;
	FTM_ULONG	ulCount;
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;

	xRet = FTOM_CLIENT_EVENT_count(pClient, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Failed to get Event count\n");
	}
	else
	{
		MESSAGE("Event Count : %lu\n", ulCount);	
	}

	return	xRet;
}

FTM_RET	FTOM_CLIENT_CMD_enableEvent
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
#if 0
	FTM_RET		xRet;
	FTM_INT		i;
	FTM_EVENT		xInfo;
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xInfo.bEnable = FTM_TRUE;

	for(i = 1 ; i < nArgc ; i++)
	{
		xRet = FTOM_CLIENT_EVENT_set(pClient, pArgv[i], FTM_EVENT_FIELD_ENABLE, &xInfo);
		if (xRet != FTM_RET_OK)
		{
			MESSAGE("Failed to activate Event[%s].\n", pArgv[i]);
		}
		else
		{
			MESSAGE("The Event[%s] is activated.\n", pArgv[i]);	
		}
	}
#endif 

	return	FTM_RET_OK;
}
	
FTM_RET	FTOM_CLIENT_CMD_disableEvent
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
#if 0
	FTM_RET		xRet;
	FTM_INT		i;
	FTM_Event		xInfo;
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	xInfo.bEnable = FTM_FALSE;

	for(i = 1 ; i < nArgc ; i++)
	{
		xRet = FTOM_CLIENT_EVENT_set(pClient, pArgv[i], FTM_EVENT_FIELD_ENABLE, &xInfo);
		if (xRet != FTM_RET_OK)
		{
			MESSAGE("Failed to stop Event[%s].\n", pArgv[i]);
		}
		else
		{
			MESSAGE("The Event[%s] is stopped.\n", pArgv[i]);	
		}
	}
#endif

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CLIENT_CMD_setEvent
(
	FTM_SHELL_PTR	pShell,
	FTM_INT			nArgc,
	FTM_CHAR_PTR	pArgv[],
	FTM_VOID_PTR	pData
)
{
	FTM_RET		xRet;
	FTM_INT		i;
	FTM_EVENT		xInfo;
	FTOM_CLIENT_PTR	pClient = (FTOM_CLIENT_PTR)pData;
	FTM_CHAR_PTR	pID;
	FTM_OPT		pOpts[8];
	FTM_ULONG	ulOpts;
	FTM_ULONG	ulFields = 0;

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	pID = pArgv[1];

	xRet = FTOM_CLIENT_EVENT_get(pClient, pID, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get the trigger[%s] information.\n", pID);
		return	xRet;	
	}

	xRet = FTM_getOptions(nArgc, pArgv, "n:d:h:u:l:", pOpts, 8, &ulOpts);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Invalid arguments!\n");
		return	xRet;
	}

	for(i = 0 ; i < ulOpts ; i++)
	{
		switch(pOpts[i].xOpt)
		{
		case	'n':	
			{
				if (strlen(pOpts[i].pParam) > FTM_NAME_LEN)
				{
					return	FTM_RET_INVALID_ARGUMENTS;
				}

				strncpy(xInfo.pName, pOpts[i].pParam, sizeof(xInfo.pName) - 1);
				ulFields |= FTM_EVENT_FIELD_NAME;
			}
			break;

		case	'd':
			{
				xInfo.xParams.xCommon.ulDetectionTime = strtoul(pOpts[i].pParam, NULL, 10);
				ulFields |= FTM_EVENT_FIELD_DETECT_TIME;
			}
			break;

		case	'h':
			{
				xInfo.xParams.xCommon.ulHoldingTime = strtoul(pOpts[i].pParam, NULL, 10);
				ulFields |= FTM_EVENT_FIELD_HOLD_TIME;
			}
			break;

		case	'u':
			{
				if ((xInfo.xType != FTM_EVENT_TYPE_INCLUDE) && (xInfo.xType != FTM_EVENT_TYPE_EXCEPT))
				{
					return	FTM_RET_INVALID_ARGUMENTS;
				}

				xRet = FTM_VALUE_setFromString(&xInfo.xParams.xInclude.xUpper, pOpts[i].pParam);
				if (xRet != FTM_RET_OK)
				{
					ERROR2(xRet, "Failed to set upper limit!\n");
					return	xRet;	
				}

				ulFields |= FTM_EVENT_FIELD_UPPER;
			}
			break;

		case	'l':
			{
				if ((xInfo.xType != FTM_EVENT_TYPE_INCLUDE) && (xInfo.xType != FTM_EVENT_TYPE_EXCEPT))
				{
					return	FTM_RET_INVALID_ARGUMENTS;
				}

				xRet = FTM_VALUE_setFromString(&xInfo.xParams.xInclude.xLower, pOpts[i].pParam);
				if (xRet != FTM_RET_OK)
				{
					ERROR2(xRet, "Failed to set lower limit!\n");
					return	xRet;	
				}

				ulFields |= FTM_EVENT_FIELD_LOWER;
			}
			break;

		}
	}

	xRet = FTOM_CLIENT_EVENT_set(pClient, pID, ulFields, &xInfo);
	if (xRet != FTM_RET_OK)
	{
		MESSAGE("Failed to set Event[%s].\n", pID);
	}
	else
	{
		MESSAGE("The Event[%s] setting changed.\n", pID);	
	}

	return	xRet;
}


FTM_RET	FTOM_CLIENT_CMD_trigger
(
	FTM_SHELL_PTR	pShell,
	FTM_INT 		nArgc, 
	FTM_CHAR_PTR 	pArgv[], 
	FTM_VOID_PTR 	pData
)
{
	FTM_RET		xRet;

	if (nArgc < 2)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	if (strcmp(pArgv[1], "add") == 0)
	{
		xRet = FTOM_CLIENT_CMD_addEvent(pShell, nArgc - 1, &pArgv[1], pData);
	}
	else if (strcmp(pArgv[1], "del") == 0)
	{
		xRet = FTOM_CLIENT_CMD_delEvent(pShell, nArgc - 1, &pArgv[1], pData);
	}
	else if (strcmp(pArgv[1], "list") == 0)
	{
		xRet = FTOM_CLIENT_CMD_printEventList(pShell, nArgc - 1, &pArgv[1], pData);
	}
	else if (strcasecmp(pArgv[1], "info") == 0)
	{
		xRet = FTOM_CLIENT_CMD_printEventInfo(pShell, nArgc - 1, &pArgv[1], pData);
	}
	else if (strcasecmp(pArgv[1], "count") == 0)
	{
		xRet = FTOM_CLIENT_CMD_printEventCount(pShell, nArgc - 1, &pArgv[1], pData);
	}
	else if (strcasecmp(pArgv[1], "enable") == 0)
	{
		xRet = FTOM_CLIENT_CMD_enableEvent(pShell, nArgc - 1, &pArgv[1], pData);	
	}
	else if (strcasecmp(pArgv[1], "disable") == 0)
	{
		xRet = FTOM_CLIENT_CMD_enableEvent(pShell, nArgc - 1, &pArgv[1], pData);	
	}
	else if (strcasecmp(pArgv[1], "set") == 0)
	{
		xRet = FTOM_CLIENT_CMD_setEvent(pShell, nArgc - 1, &pArgv[1], pData);
	}
	else
	{
		xRet = FTM_RET_INVALID_ARGUMENTS;	
	}

	return	xRet;
}

