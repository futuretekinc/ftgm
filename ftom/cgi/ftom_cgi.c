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

static FTOM_CGI_COMMAND	pCmds[] =
{
	{	"ep",			FTOM_CGI_getEP			},
	{	"eplist",		FTOM_CGI_getEPList		},
	{	"data",			FTOM_CGI_getEPData		},
	{	"datalast",		FTOM_CGI_getEPDataLast	},
	{	"node",			FTOM_CGI_getNode		},
	{	"nodelist",		FTOM_CGI_getNodeList	},
	{	"triggerlist",	FTOM_CGI_getTriggerList	},
	{	"actionlist",	FTOM_CGI_getActionList	},
	{	"rulelist",		FTOM_CGI_getRuleList	},
	{	NULL,		NULL					}
};


FTM_RET	FTOM_CGI_main(FTOM_CLIENT_PTR pClient, qentry_t *pReq)
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
