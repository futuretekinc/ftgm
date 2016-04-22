#include "ftom_cgi.h"
#include "ftom_client.h"
#include "mxml.h"

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

