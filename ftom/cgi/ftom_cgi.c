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
	{	"get",		FTOM_CGI_getEP			},
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
	{	"list",	FTOM_CGI_getTriggerList	},
	{	NULL,		NULL					}
};

static 
FTOM_CGI_COMMAND	pActionCmds[] =
{
	{	"list",	FTOM_CGI_getActionList	},
	{	NULL,		NULL					}
};

static 
FTOM_CGI_COMMAND	pRuleCmds[] =
{
	{	"list",		FTOM_CGI_getRuleList	},
	{	NULL,		NULL					}
};

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

FTM_RET	FTOM_CGI_acton
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
	FTM_CHAR_PTR pEPID
)
{
	ASSERT(pReq != NULL);
	ASSERT(pEPID != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "id", false);
	if((pValue == NULL) || (strlen(pValue) > FTM_EPID_LEN))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	strcpy(pEPID, pValue);
	
	return	FTM_RET_OK;
}

FTM_RET FTOM_CGI_getEPType
(
	qentry_t *pReq, 
	FTM_EP_TYPE_PTR pType
)
{
	ASSERT(pReq != NULL);
	ASSERT(pType != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "type", false);
	if (pValue == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	if (strcasecmp(pValue, "temperature") == 0)
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
	FTM_EP_FLAG_PTR	pFlags
)
{
	ASSERT(pReq != NULL);
	ASSERT(pFlags != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getName
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pName
)
{
	ASSERT(pReq != NULL);
	ASSERT(pName != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "name", false);
	if((pValue == NULL) || (strlen(pValue) > FTM_NAME_LEN))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	strcpy(pName, pValue);
	
	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getUnit
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pUnit
)
{
	ASSERT(pReq != NULL);
	ASSERT(pUnit != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "unit", false);
	if((pValue == NULL) || (strlen(pValue) > FTM_UNIT_LEN))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	strcpy(pUnit, pValue);
	
	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getEnable
(
	qentry_t *pReq, 
	FTM_BOOL_PTR		pEnable	
)
{
	ASSERT(pReq != NULL);
	ASSERT(pEnable != NULL);
	
	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "enable", false);
	if (pValue == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	if(strcasecmp(pValue, "true") == 0)
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
	FTM_ULONG_PTR	pTimeout
)
{
	ASSERT(pReq != NULL);
	ASSERT(pTimeout != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "timeout", false);
	if (pValue == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	*pTimeout = strtoul(pValue, 0, 10);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getInterval
(
	qentry_t *pReq, 
	FTM_ULONG_PTR	pInterval
)
{
	ASSERT(pReq != NULL);
	ASSERT(pInterval != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "interval", false);
	if (pValue == NULL)
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	*pInterval = strtoul(pValue, 0, 10);

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getDID
(
	qentry_t *pReq, 
	FTM_CHAR_PTR	pDID
)
{
	ASSERT(pReq != NULL);
	ASSERT(pDID != NULL);

	FTM_CHAR_PTR	pValue;

	pValue = pReq->getstr(pReq, "did", false);
	if((pValue == NULL) || (strlen(pValue) > FTM_DID_LEN))
	{
		return	FTM_RET_INVALID_ARGUMENTS;	
	}

	strcpy(pDID, pValue);
	
	return	FTM_RET_OK;
}

