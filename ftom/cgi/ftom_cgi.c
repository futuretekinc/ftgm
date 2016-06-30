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
#include "cmd_log.h"
#include "cmd_discovery.h"

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
	{	"info",		FTOM_CGI_getEPDataInfo	},
	{	"del",		FTOM_CGI_delEPData		},
	{	"get",		FTOM_CGI_getEPData		},
	{	"last",		FTOM_CGI_getEPDataLast	},
	{	NULL,		NULL					}
};

static 
FTOM_CGI_COMMAND	pNodeCmds[] =
{
	{	"add",		FTOM_CGI_addNode		},
	{	"del",		FTOM_CGI_delNode		},
	{	"get",		FTOM_CGI_getNode		},
	{	"set",		FTOM_CGI_setNode		},
	{	"list",		FTOM_CGI_getNodeList	},

	{	NULL,		NULL					}
};

static 
FTOM_CGI_COMMAND	pTriggerCmds[] =
{
	{	"add",	FTOM_CGI_addTrigger			},
	{	"del",	FTOM_CGI_delTrigger			},
	{	"get",	FTOM_CGI_getTrigger			},
	{	"set",	FTOM_CGI_setTrigger			},
	{	"list",	FTOM_CGI_getTriggerList		},
	{	NULL,		NULL					}
};

static 
FTOM_CGI_COMMAND	pActionCmds[] =
{
	{	"get",	FTOM_CGI_getAction			},
	{	"set",	FTOM_CGI_setAction			},
	{	"add",	FTOM_CGI_addAction			},
	{	"del",	FTOM_CGI_delAction			},
	{	"list",	FTOM_CGI_getActionList		},
	{	NULL,		NULL					}
};

static 
FTOM_CGI_COMMAND	pRuleCmds[] =
{
	{	"get",	FTOM_CGI_getRule			},
	{	"set",	FTOM_CGI_setRule			},
	{	"add",	FTOM_CGI_addRule			},
	{	"del",	FTOM_CGI_delRule			},
	{	"list",	FTOM_CGI_getRuleList		},
	{	NULL,		NULL					}
};

static 
FTOM_CGI_COMMAND	pLogCmds[] =
{
	{	"get",	FTOM_CGI_getLog			},
	{	"del",	FTOM_CGI_delLog			},
	{	NULL,		NULL					}
};
static 
FTOM_CGI_COMMAND	pDiscoveryCmds[] =
{
	{	"start",	FTOM_CGI_startDiscovery	},
	{	"get",		FTOM_CGI_getDiscoveryInfo},
	{	"nodes",	FTOM_CGI_getDiscoveryNodeList},
	{	"eps",		FTOM_CGI_getDiscoveryEPList},
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
	TRACE("Node Server!\n");
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

FTM_RET	FTOM_CGI_log
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTOM_CGI_service(pClient, pReq, pLogCmds);
}

FTM_RET	FTOM_CGI_discovery
(
	FTOM_CLIENT_PTR pClient, 
	qentry_t *pReq
)
{
	return	FTOM_CGI_service(pClient, pReq, pDiscoveryCmds);
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

FTM_RET	FTOM_CGI_getUSHORT
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pName,
	FTM_USHORT_PTR	pusValue,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pName != NULL);
	ASSERT(pusValue != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, pName, false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		*pusValue = (FTM_USHORT)strtoul(pValue, 0, 10);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getULONG
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pName,
	FTM_ULONG_PTR pulValue,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pName != NULL);
	ASSERT(pulValue != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, pName, false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		*pulValue = strtoul(pValue, 0, 10);
	}
	
	return	FTM_RET_OK;
}

FTM_RET FTOM_CGI_getNodeType
(
	qentry_t *pReq, 
	FTM_NODE_TYPE_PTR pType,
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
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if (strcasecmp(pValue, "snmp") == 0)
	{
		*pType = FTM_NODE_TYPE_SNMP;
	}
	else
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	return	FTM_RET_OK;
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
			return	FTM_RET_OBJECT_NOT_FOUND;	
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
			return	FTM_RET_OBJECT_NOT_FOUND;	
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
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getModel
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pModel,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pModel != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "model", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if((strlen(pValue) > FTM_NAME_LEN))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	else
	{
		strcpy(pModel, pValue);
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
			return	FTM_RET_OBJECT_NOT_FOUND;	
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
			return	FTM_RET_OBJECT_NOT_FOUND;	
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

FTM_RET	FTOM_CGI_getLocation
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pLocation,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pLocation != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "location", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if(strlen(pValue) > FTM_LOCATION_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	else
	{ 
		strcpy(pLocation, pValue);
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
			return	FTM_RET_OBJECT_NOT_FOUND;	
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
			return	FTM_RET_OBJECT_NOT_FOUND;	
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
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		*pInterval = strtoul(pValue, 0, 10);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getReportInterval
(
	qentry_t *pReq, 
	FTM_ULONG_PTR	pInterval,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pInterval != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "report", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
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
			return	FTM_RET_OBJECT_NOT_FOUND;	
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
					return	FTM_RET_OBJECT_NOT_FOUND;	
				}
			}
		}
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getIndex
(
	qentry_t *pReq, 
	FTM_ULONG_PTR pulIndex,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pulIndex != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "index", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		*pulIndex = strtoul(pValue, 0, 10);
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
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		*pulCount = strtoul(pValue, 0, 10);
	}
	
	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getIPString
(
	qentry_t *pReq,
	FTM_CHAR_PTR	pBuff,
	FTM_ULONG		ulBuffLen,
	FTM_BOOL bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pBuff != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "ip", false);
	if (pValue == NULL)
	{
		if (!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		memset(pBuff, 0, ulBuffLen);
		strncpy(pBuff, pValue, ulBuffLen - 1);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getPort
(
	qentry_t *pReq, 
	FTM_USHORT_PTR	pusPort,
	FTM_BOOL	bAllowEmpty
)
{
	return	FTOM_CGI_getUSHORT(pReq, "port", pusPort, bAllowEmpty);
}

FTM_RET	FTOM_CGI_getRetry
(
	qentry_t *pReq, 
	FTM_ULONG_PTR pulCount,
	FTM_BOOL	bAllowEmpty
)
{
	return	FTOM_CGI_getULONG(pReq, "retry", pulCount, bAllowEmpty);
}

FTM_RET	FTOM_CGI_getNodeOptSNMP
(
	qentry_t *pReq,
	FTM_NODE_OPT_SNMP_PTR pSNMP,
	FTM_BOOL bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pSNMP != NULL);

	FTM_CHAR_PTR		pValue;
	FTM_NODE_OPT_SNMP	xSNMP;

	memcpy(&xSNMP, pSNMP, sizeof(FTM_NODE_OPT_SNMP));

	pValue = pReq->getstr(pReq, "version", false);
	if (pValue == NULL)
	{
		xSNMP.ulVersion = FTM_SNMP_VERSION_2;	
	}
	else 
	{
		xSNMP.ulVersion = strtoul(pValue, 0, 10);
		if ((xSNMP.ulVersion < FTM_SNMP_VERSION_1) || (xSNMP.ulVersion > FTM_SNMP_VERSION_3))
		{
			return	FTM_RET_INVALID_ARGUMENTS;	
		}
	}

	pValue = pReq->getstr(pReq, "url", false);

	if (pValue == NULL)
	{
		if (!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if (strlen(pValue) > FTM_URL_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	else
	{
		strcpy(xSNMP.pURL, pValue);	
	}

	pValue = pReq->getstr(pReq, "community", false);
	if (pValue == NULL)
	{
		if (!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if (strlen(pValue) > FTM_SNMP_COMMUNITY_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	else
	{
		strcpy(xSNMP.pCommunity, pValue);	
	}

	pValue = pReq->getstr(pReq, "mib", false);
	if (pValue == NULL)
	{
		if (!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if (strlen(pValue) > FTM_SNMP_MIB_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	else
	{
		strcpy(xSNMP.pMIB, pValue);	
	}

	pValue = pReq->getstr(pReq, "retry", false);
	if (pValue == NULL)
	{
		if (!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else 
	{
		xSNMP.ulMaxRetryCount = strtoul(pValue, 0, 10);
	}

	memcpy(pSNMP, &xSNMP, sizeof(FTM_NODE_OPT_SNMP));

	return	FTM_RET_OK;

}

FTM_RET FTOM_CGI_getTriggerID
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pTriggerID,
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
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if (strlen(pValue) > FTM_ID_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	
	}
	else
	{
		strcpy(pTriggerID, pValue);
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
			return	FTM_RET_OBJECT_NOT_FOUND;	
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

FTM_RET FTOM_CGI_getBeginTime
(
	qentry_t *pReq, 
	FTM_ULONG_PTR	pulTime,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pulTime != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "begin", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		*pulTime = strtoul(pValue, 0, 10);
	}
	
	return	FTM_RET_OK;
}

FTM_RET FTOM_CGI_getEndTime
(
	qentry_t *pReq, 
	FTM_ULONG_PTR	pulTime,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pulTime != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "end", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else
	{
		*pulTime = strtoul(pValue, 0, 10);
	}
	
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
			return	FTM_RET_OBJECT_NOT_FOUND;	
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
			return	FTM_RET_OBJECT_NOT_FOUND;	
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
			return	FTM_RET_OBJECT_NOT_FOUND;	
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
			return	FTM_RET_OBJECT_NOT_FOUND;	
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
			return	FTM_RET_OBJECT_NOT_FOUND;	
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
	FTM_CHAR_PTR	pActionID,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pActionID != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "id", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if (strlen(pValue) > FTM_ID_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	else
	{
		strncpy(pActionID, pValue, FTM_ID_LEN);	
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
			return	FTM_RET_OBJECT_NOT_FOUND;	
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
	FTM_CHAR_PTR	pRuleID,
	FTM_BOOL	bAllowEmpty
)
{
	ASSERT(pReq != NULL);
	ASSERT(pRuleID != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "id", false);
	if(pValue == NULL)
	{
		if(!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;	
		}
	}
	else if (strlen(pValue) > FTM_ID_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}
	else
	{
		strcpy(pRuleID, pValue);
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
			return	FTM_RET_OBJECT_NOT_FOUND;	
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

FTM_RET	FTOM_CGI_getRuleTrigger
(
	qentry_t 	*pReq,
	FTM_INT		nIndex,
	FTM_CHAR_PTR pTriggerID,
	FTM_ULONG	ulIDLen,
	FTM_BOOL	bAllowEmpty
)
{
	FTM_CHAR		pTitle[32];
	FTM_CHAR_PTR	pValue;

	if (nIndex >= 8)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	sprintf(pTitle, "trigger%d", nIndex+1);
	pValue = pReq->getstr(pReq, pTitle, false);
	if (pValue == NULL)
	{
		if (!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;
		}
	}
	else
	{
		strncpy(pTriggerID, pValue, ulIDLen);
	}	

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getRuleAction
(
	qentry_t 	*pReq,
	FTM_INT		nIndex,
	FTM_CHAR_PTR pActionID,
	FTM_ULONG	ulIDLen,
	FTM_BOOL	bAllowEmpty
)
{
	FTM_CHAR		pTitle[32];
	FTM_CHAR_PTR	pValue;

	if (nIndex > 8)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}

	sprintf(pTitle, "action%d", nIndex+1);
	pValue = pReq->getstr(pReq, pTitle, false);
	if (pValue == NULL)
	{
		if (!bAllowEmpty)
		{
			return	FTM_RET_OBJECT_NOT_FOUND;
		}
	}
	else
	{
		strncpy(pActionID, pValue, ulIDLen);
	}	

	return	FTM_RET_OK;
}
