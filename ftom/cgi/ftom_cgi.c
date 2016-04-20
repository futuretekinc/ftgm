#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "ftom_cgi.h"
#include "ftom_client.h"
#include "mxml.h"


static const 
char *FTOM_CGI_whitespaceCB
(
	mxml_node_t *node,
	int			where
);
FTM_RET	FTOM_CGI_getNodeList(FTOM_CLIENT_PTR pClient, qentry_t *pReq);

static FTOM_CGI_COMMAND	pCmds[] =
{
	{	"nodelist",	FTOM_CGI_getNodeList	},
	{	NULL,		NULL			}
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
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_CGI_getNodeList(FTOM_CLIENT_PTR pClient, qentry_t *pReq)
{
	ASSERT(pClient != NULL);
	ASSERT(pReq != NULL);

	mxml_node_t	*pXML = NULL;
	mxml_node_t	*pSensorNode = NULL;
	mxml_node_t	*pIndexNode = NULL;
	mxml_node_t	*pFieldNode = NULL;
	mxml_node_t	*pValueListNode = NULL;
	FTM_RET		xRet;
	FTM_CHAR	pBuff[1024];
	FTM_EP		xEPInfo;
	FTM_EP_DATA_PTR	pData;
	FTM_ULONG	ulCount = 0;

	FTM_CHAR_PTR	pMAC  = pReq->getstr(pReq, "mac", false);
	FTM_CHAR_PTR	pEPID = pReq->getstr(pReq, "id", false);


	xRet = FTOM_CLIENT_EP_get(pClient, pEPID, &xEPInfo);
	if (xRet != FTM_RET_OK)
	{
		goto finish;
	}

	pXML = mxmlNewXML("1.0");
	pSensorNode = mxmlNewElement(pXML, "SENSOR");
	pFieldNode = mxmlNewElement(pSensorNode, "MAC");
	mxmlNewText(pFieldNode, 0, pMAC);
	pFieldNode = mxmlNewElement(pSensorNode, "ID");
	mxmlNewText(pFieldNode, 0, pEPID);
	pFieldNode = mxmlNewElement(pSensorNode, "TYPED");
	mxmlNewTextf(pFieldNode, 0, "%s", FTM_EP_typeString(xEPInfo.xType));
	pFieldNode = mxmlNewElement(pSensorNode, "NAME");
	mxmlNewTextf(pFieldNode, 0, "%s", xEPInfo.pName);
	pFieldNode = mxmlNewElement(pSensorNode, "SN");
	mxmlNewTextf(pFieldNode, 0, "%s", xEPInfo.pUnit);

	pData = (FTM_EP_DATA_PTR)FTM_MEM_malloc(sizeof(FTM_EP_DATA) * 10);
	xRet = FTOM_CLIENT_EP_DATA_getList(pClient, pEPID, 0, pData, 10, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		goto finish;	
	}

	pValueListNode = mxmlNewElement(pSensorNode, "VALUES");

	FTM_INT	i;
	for(i = 0 ; i < ulCount ; i++)
	{
		FTM_CHAR	pValueString[64];

		pIndexNode = mxmlNewElement(pValueListNode, "INDEX");
		pFieldNode = mxmlNewElement(pIndexNode, "NUMBER");
		mxmlNewInteger(pFieldNode, 1);
		pFieldNode = mxmlNewElement(pIndexNode, "STATE");
		mxmlNewInteger(pFieldNode, 1);
		FTM_EP_DATA_snprint(pValueString, sizeof(pValueString), &pData[i]);
		pFieldNode = mxmlNewElement(pIndexNode, "VALUE");
		mxmlNewTextf(pFieldNode, 0, "%s", pValueString);
		pFieldNode = mxmlNewElement(pIndexNode, "LASTVALUE");
		mxmlNewTextf(pFieldNode, 0, "%s", pValueString);
		pFieldNode = mxmlNewElement(pIndexNode, "INTERVAL");
		mxmlNewInteger(pFieldNode, 1);
		pFieldNode = mxmlNewElement(pIndexNode, "TIME");
		mxmlNewInteger(pFieldNode, 1);
	}

	mxmlSaveString(pXML, pBuff, sizeof(pBuff), FTOM_CGI_whitespaceCB);

	qcgires_setcontenttype(pReq, "text/xml");
	printf("%s", pBuff);

	xRet = FTM_RET_OK;

finish:
	if (pData != NULL)
	{
		FTM_MEM_free(pData);
	}

	if (pXML)
	{
		mxmlRelease(pXML);
	}

	return	xRet;
}

/*
 * 'whitespace_cb()' - Let the mxmlSaveFile() function know when to insert
 *                     newlines and tabs...
 */

static const 
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
