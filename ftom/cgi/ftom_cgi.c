#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ftom_client.h"
#include "ftom_cgi.h"
#include "cmd_ep.h"
#include "cmd_node.h"
#include "cmd_trigger.h"
#include "cmd_action.h"
#include "cmd_rule.h"

static 
FTM_RET	FTOM_CGI_service
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t *pReq,
	FTOM_CGI_COMMAND_PTR	pCmds
);

static 
FTOM_CGI_COMMAND	pEPCmds[] =
{
	{	"add",		FTOM_CGI_addEP			},
	{	"del",		FTOM_CGI_delEP			},
	{	"get",		FTOM_CGI_getEP			},
	{	"set",		FTOM_CGI_setEP			},
	{	"list",		FTOM_CGI_getEPList		},
	{	NULL,		NULL					}
};

static 
FTOM_CGI_COMMAND	pDataCmds[] =
{
	{	"get",		FTOM_CGI_getEPData		},
	{	"last",		FTOM_CGI_getEPDataLast	},
	{	NULL,		NULL					}
};

static 
FTOM_CGI_COMMAND	pNodeCmds[] =
{
	{	"get",		FTOM_CGI_getNode		},
	{	"list",		FTOM_CGI_getNodeList	},
	{	NULL,		NULL					}
};

static 
FTOM_CGI_COMMAND	pTriggerCmds[] =
{
	{	"get",	FTOM_CGI_getTrigger			},
	{	"add",	FTOM_CGI_addTrigger			},
	{	"del",	FTOM_CGI_delTrigger			},
	{	"list",	FTOM_CGI_getTriggerList		},
	{	NULL,		NULL					}
};

static 
FTOM_CGI_COMMAND	pActionCmds[] =
{
	{	"get",	FTOM_CGI_getAction			},
	{	"add",	FTOM_CGI_addAction			},
	{	"del",	FTOM_CGI_delAction			},
	{	"list",	FTOM_CGI_getActionList		},
	{	NULL,		NULL					}
};

static 
FTOM_CGI_COMMAND	pRuleCmds[] =
{
	{	"get",	FTOM_CGI_getRule			},
	{	"add",	FTOM_CGI_addRule			},
	{	"del",	FTOM_CGI_delRule			},
	{	"list",	FTOM_CGI_getRuleList		},
	{	NULL,		NULL					}
};

FTM_RET	FTOM_CGI_finish
(
	qentry_t _PTR_ pReq,
	cJSON _PTR_ pRoot, 
	FTM_RET xRet
)
{
	cJSON_AddStringToObject(pRoot, "result", (xRet == FTM_RET_OK)?"success":"failed");	

	qcgires_setcontenttype(pReq, "text/xml");
	printf("%s", cJSON_Print(pRoot));

	if (pRoot != NULL)
	{
		cJSON_Delete(pRoot);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_node
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTOM_CGI_service(pClient, pReq, pNodeCmds);
}

FTM_RET	FTOM_CGI_ep
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTOM_CGI_service(pClient, pReq, pEPCmds);
}

FTM_RET	FTOM_CGI_data
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTOM_CGI_service(pClient, pReq, pDataCmds);
}

FTM_RET	FTOM_CGI_trigger
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTOM_CGI_service(pClient, pReq, pTriggerCmds);
}

FTM_RET	FTOM_CGI_action
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTOM_CGI_service(pClient, pReq, pActionCmds);
}

FTM_RET	FTOM_CGI_rule
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTOM_CGI_service(pClient, pReq, pRuleCmds);
}

FTM_RET	FTOM_CGI_service
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t *pReq,
	FTOM_CGI_COMMAND_PTR	pCmds
)
{
	ASSERT(pReq != NULL);

	FTM_CHAR_PTR			pCmdName = pReq->getstr(pReq, "cmd", false);
	FTOM_CGI_COMMAND_PTR	pCmd = pCmds;

	while(pCmd->pName != NULL)
	{
		if (strcasecmp(pCmdName, pCmd->pName) == 0)
		{
			return	pCmd->fService(pClient, pReq);	
		}

		pCmd++;
	}

	return	FTM_RET_OK;
}

/*
 * 'whitespace_cb()' - Let the mxmlSaveFile() function know when to insert
 *                     newlines and tabs...
 */

const 
char *FTOM_CGI_whitespaceCB
(
	mxml_node_t *node,
	int			where
)
{
	mxml_node_t	*parent;		/* Parent node */
	int		level;			/* Indentation level */
	const char	*name;			/* Name of element */
	static const char *tabs = "\t\t\t\t\t\t\t\t";
	/* Tabs for indentation */


	/*
	 * We can conditionally break to a new line before or after any element.
	 * These are just common HTML elements...
	 */

	name = node->value.element.name;

	if (!strcmp(name, "html") || !strcmp(name, "head") || !strcmp(name, "body") ||
		!strcmp(name, "pre") || !strcmp(name, "p") ||
		!strcmp(name, "h1") || !strcmp(name, "h2") || !strcmp(name, "h3") ||
		!strcmp(name, "h4") || !strcmp(name, "h5") || !strcmp(name, "h6"))
	{
		/*
		 * Newlines before open and after close...
		 */

		if (where == MXML_WS_BEFORE_OPEN || where == MXML_WS_AFTER_CLOSE)
			return ("\n");
	}
	else if (!strcmp(name, "dl") || !strcmp(name, "ol") || !strcmp(name, "ul"))
	{
		/*
		 * Put a newline before and after list elements...
		 */

		return ("\n");
	}
	else if (!strcmp(name, "dd") || !strcmp(name, "dt") || !strcmp(name, "li"))
	{
		/*
		 * Put a tab before <li>'s, <dd>'s, and <dt>'s, and a newline after them...
		 */

		if (where == MXML_WS_BEFORE_OPEN)
			return ("\t");
		else if (where == MXML_WS_AFTER_CLOSE)
			return ("\n");
	}
	else if (!strncmp(name, "?xml", 4))
	{
		if (where == MXML_WS_AFTER_OPEN)
			return ("\n");
		else
			return (NULL);
	}
	else if (where == MXML_WS_BEFORE_OPEN ||
			((!strcmp(name, "choice") || !strcmp(name, "option")) &&
			 where == MXML_WS_BEFORE_CLOSE))
	{
		for (level = -1, parent = node->parent;
				parent;
				level ++, parent = parent->parent);

		if (level > 8)
			level = 8;
		else if (level < 0)
			level = 0;

		return (tabs + 8 - level);
	}
	else if (where == MXML_WS_AFTER_CLOSE ||
			((!strcmp(name, "group") || !strcmp(name, "option") ||
			  !strcmp(name, "choice")) &&
			 where == MXML_WS_AFTER_OPEN))
		return ("\n");
	else if (where == MXML_WS_AFTER_OPEN && !node->child)
		return ("\n");

	/*
	 * Return NULL for no added whitespace...
	 */

	return (NULL);
}

FTM_RET	FTOM_CGI_getEPID
(
	qentry_t *pReq, 
	FTM_CHAR_PTR pEPID,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pEPID != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "epid", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}
	else if((strlen(pValue) > FTM_EPID_LEN))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	else
	{
		strcpy(pEPID, pValue);
	}
	
	return	FTM_RET_OK;
}

FTM_RET FTOM_CGI_getEPType
(
	qentry_t *pReq, 
	FTM_EP_TYPE_PTR pType,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pType != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "type", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}
	else if (strcasecmp(pValue, "temperature") == 0)
	{
		*pType = FTM_EP_TYPE_TEMPERATURE;
	}
	else if (strcasecmp(pValue, "humidity" ) == 0)
	{
		*pType = FTM_EP_TYPE_HUMIDITY;
	}
	else if (strcasecmp(pValue, "voltage") == 0)
	{
		*pType = FTM_EP_TYPE_VOLTAGE;
	}
	else if (strcasecmp(pValue, "current") == 0)
	{
		*pType = FTM_EP_TYPE_CURRENT;
	}
	else if (strcasecmp(pValue, "di") == 0)
	{
		*pType = FTM_EP_TYPE_DI;
	}
	else if (strcasecmp(pValue, "do") == 0)
	{
		*pType = FTM_EP_TYPE_DO;
	}
	else if (strcasecmp(pValue, "gas") == 0)
	{
		*pType = FTM_EP_TYPE_GAS;
	}
	else if (strcasecmp(pValue, "power") == 0)
	{
		*pType = FTM_EP_TYPE_POWER;
	}
	else if (strcasecmp(pValue, "count") == 0)
	{
		*pType = FTM_EP_TYPE_COUNT;
	}
	else if (strcasecmp(pValue, "multi") == 0)
	{
		*pType = FTM_EP_TYPE_MULTI;
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getEPFlags
(
	qentry_t *pReq, 
	FTM_EP_FLAG_PTR	pFlags,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pFlags != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "flags", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getName
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pName,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pName != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "name", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}
	else if((strlen(pValue) > FTM_NAME_LEN))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	else
	{
		strcpy(pName, pValue);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getUnit
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pUnit,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pUnit != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "unit", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}
	else if(strlen(pValue) > FTM_UNIT_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	else
	{ 
		strcpy(pUnit, pValue);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getEnable
(
	qentry_t *pReq, 
	FTM_BOOL_PTR	pEnable,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pEnable != NULL);
	
	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "enable", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}
	else if(strcasecmp(pValue, "true") == 0)
	{
		*pEnable = FTM_TRUE;	
	}
	else if(strcasecmp(pValue, "false") == 0)
	{
		*pEnable = FTM_FALSE;	
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getTimeout
(
	qentry_t *pReq, 
	FTM_ULONG_PTR	pTimeout,	
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pTimeout != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "timeout", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}
	else	
	{
		*pTimeout = strtoul(pValue, 0, 10);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getInterval
(
	qentry_t *pReq, 
	FTM_ULONG_PTR	pInterval,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pInterval != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "interval", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}
	else
	{
		*pInterval = strtoul(pValue, 0, 10);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getDID
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pDID,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pDID != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "did", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}
	else if((strlen(pValue) > FTM_DID_LEN))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	else
	{
		strcpy(pDID, pValue);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getLimit
(
	qentry_t *pReq,
	FTM_EP_LIMIT_PTR pLimit,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pLimit != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "lc", false);
	if (pValue != NULL)
	{
		pLimit->xType = FTM_EP_LIMIT_TYPE_COUNT;
		pLimit->xParams.ulMonths = strtoul(pValue, 0, 10);
	}
	else
	{
		pValue = pReq->getstr(pReq, "lh", false);
		if (pValue != NULL)
		{
			pLimit->xType = FTM_EP_LIMIT_TYPE_HOURS;
			pLimit->xParams.ulHours = strtoul(pValue, 0, 10);
		}
		else 
		{
			pValue = pReq->getstr(pReq, "ld", false);
			if (pValue != NULL)
			{
				pLimit->xType = FTM_EP_LIMIT_TYPE_DAYS;
				pLimit->xParams.ulDays = strtoul(pValue, 0, 10);
			}
			else
			{
				pValue = pReq->getstr(pReq, "lm", false);
				if (pValue != NULL)
				{
					pLimit->xType = FTM_EP_LIMIT_TYPE_MONTHS;
					pLimit->xParams.ulMonths = strtoul(pValue, 0, 10);
				}
				else if (!bAllowEmpty)
				{
					return	FTM_RET_INVALID_ARGUMENTS;	
				}
			}
		}
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getCount
(
	qentry_t *pReq, 
	FTM_ULONG_PTR pulCount,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pulCount != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "count", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}
	else
	{
		*pulCount = strtoul(pValue, 0, 10);
	}
	
	return	FTM_RET_OK;
}

FTM_RET FTOM_CGI_getTriggerID
(
	qentry_t *pReq, 
	FTM_TRIGGER_ID_PTR	pTriggerID,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pTriggerID != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "id", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}
	else
	{
		*pTriggerID = strtoul(pValue, 0, 10);
	}
	
	return	FTM_RET_OK;
}

FTM_RET FTOM_CGI_getTriggerType
(
	qentry_t *pReq, 
	FTM_TRIGGER_TYPE_PTR pType,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pType != NULL);

	FTM_CHAR_PTR		pValue;
	FTM_TRIGGER_TYPE	xType;


	pValue = pReq->getstr(pReq, "type", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}
	else
	{
		if (strcasecmp(pValue, "above") == 0)
		{
			xType =  FTM_TRIGGER_TYPE_ABOVE;
		}
		else if (strcasecmp(pValue, "below") == 0)
		{
			xType = FTM_TRIGGER_TYPE_BELOW;
		}
		else if (strcasecmp(pValue, "include") == 0)
		{
			xType = FTM_TRIGGER_TYPE_INCLUDE;
		}
		else if (strcasecmp(pValue, "except") == 0)
		{
			xType = FTM_TRIGGER_TYPE_EXCEPT;
		}
		else if (strcasecmp(pValue, "change") == 0)
		{
			xType = FTM_TRIGGER_TYPE_CHANGE;
		}
		else
		{
			xType = strtoul(pValue, 0, 10);

			if ((xType < FTM_TRIGGER_TYPE_ABOVE) || (FTM_TRIGGER_TYPE_CHANGE < xType))
			{
				return	FTM_RET_INVALID_ARGUMENTS;	
			}
		}
	}
	
	*pType = xType;

	return	FTM_RET_OK;
}

FTM_RET FTOM_CGI_getDetectTime
(
	qentry_t *pReq, 
	FTM_ULONG_PTR	pulTime,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pulTime != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "detect", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}
	else
	{
		*pulTime = strtoul(pValue, 0, 10);
	}
	
	return	FTM_RET_OK;
}

FTM_RET FTOM_CGI_getHoldTime
(
	qentry_t *pReq, 
	FTM_ULONG_PTR pulTime,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pulTime != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "hold", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}
	else
	{
		*pulTime = strtoul(pValue, 0, 10);
	}
	
	return	FTM_RET_OK;
}

FTM_RET FTOM_CGI_getValue
(
	qentry_t *pReq, 
	FTM_VALUE_TYPE	xType,
	FTM_VALUE_PTR	pValue,
	FTM_BOOL	bAllowEmpty
)
{
	FTM_CHAR_PTR	pItem;

	pItem = pReq->getstr(pReq, "value", false);
		
	if (pItem == NULL)
	{
		if (!bAllowEmpty)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}
	else
	{
		return	FTM_VALUE_init(pValue, xType, pItem);
	}

	return	FTM_RET_OK;
}

FTM_RET FTOM_CGI_getLowerValue
(
	qentry_t *pReq, 
	FTM_VALUE_TYPE	xType,
	FTM_VALUE_PTR	pValue,
	FTM_BOOL	bAllowEmpty
)
{
	FTM_CHAR_PTR	pItem;

	pItem = pReq->getstr(pReq, "lower", false);
		
	if (pItem == NULL)
	{
		if (!bAllowEmpty)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}
	else
	{
		return	FTM_VALUE_init(pValue, xType, pItem);
	}

	return	FTM_RET_OK;
}
FTM_RET FTOM_CGI_getUpperValue
(
	qentry_t *pReq, 
	FTM_VALUE_TYPE	xType,
	FTM_VALUE_PTR	pValue,
	FTM_BOOL	bAllowEmpty
)
{
	FTM_CHAR_PTR	pItem;

	pItem = pReq->getstr(pReq, "upper", false);
		
	if (pItem == NULL)
	{
		if (!bAllowEmpty)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}
	else
	{
		return	FTM_VALUE_init(pValue, xType, pItem);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getActionID
(
	qentry_t *pReq,
	FTM_ACTION_ID_PTR	pID,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pID != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "id", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}
	else
	{
		*pID = strtoul(pValue, 0, 10);
	}
	
	return	FTM_RET_OK;
}


FTM_RET	FTOM_CGI_getActionType
(
	qentry_t *pReq,
	FTM_ACTION_TYPE_PTR	pType,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pType != NULL);

	FTM_CHAR_PTR	pValue;
	FTM_ACTION_TYPE	xType;


	pValue = pReq->getstr(pReq, "type", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}
	else
	{
		if (strcasecmp(pValue, "set") == 0)
		{
			xType =  FTM_ACTION_TYPE_SET;
		}
		else if (strcasecmp(pValue, "sms") == 0)
		{
			xType =  FTM_ACTION_TYPE_SMS;
		}
		else if (strcasecmp(pValue, "push") == 0)
		{
			xType =  FTM_ACTION_TYPE_PUSH;
		}
		else if (strcasecmp(pValue, "email") == 0)
		{
			xType =  FTM_ACTION_TYPE_MAIL;
		}
		else
		{
			xType = strtoul(pValue, 0, 10);

			if ((xType < FTM_ACTION_TYPE_SET) || (FTM_ACTION_TYPE_MAIL < xType))
			{
				return	FTM_RET_INVALID_ARGUMENTS;	
			}
		}
	}

	*pType = xType;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getRuleID
(
	qentry_t *pReq,
	FTM_RULE_ID_PTR	pID,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pID != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "id", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}
	else
	{
		*pID = strtoul(pValue, 0, 10);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getRuleState
(
	qentry_t *pReq,
	FTM_RULE_STATE_PTR	pState,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pState != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "state", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}
	else
	{
		if (strcasecmp(pValue, "activate") == 0)
		{
			*pState = FTM_RULE_STATE_ACTIVATE;
		}
		else if (strcasecmp(pValue, "deactivate") == 0)
		{
			*pState = FTM_RULE_STATE_DEACTIVATE;
		}
	}
	
	return	FTM_RET_OK;
}

