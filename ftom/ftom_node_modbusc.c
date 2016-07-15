#include <stdlib.h>
#include "ftom.h"
#include "ftom_dmc.h"
#include "ftom_ep.h"
#include "ftom_node_class.h"
#include "ftom_node_modbus_client_hhtw.h"

static
FTOM_NODE_CLASS_PTR	pClasses[] =
{
	&xNodeModbusClientHHTW
};

FTM_RET	FTOM_NODE_MBC_getClass
(
	FTM_CHAR_PTR	pModel,
	FTOM_NODE_CLASS_PTR	_PTR_ ppClass
)
{
	FTM_INT	i;

	for(i = 0 ; i < sizeof(pClasses) / sizeof(FTOM_NODE_CLASS_PTR) ; i++)
	{
		if(strcasecmp(pInfo->pModel, pClasses[i]->pModel) == 0)
		{
			*ppClass = pClasses[i];	
			return	FTM_RET_OK;
		}
	}

	return	FTM_RET_OBJECT_NOT_FOUND;
}

FTM_RET	FTOM_NODE_MBC_create
(
	FTM_NODE_PTR pInfo, 
	FTOM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pInfo != NULL);
	ASSERT(ppNode != NULL);

	FTM_RET	xRet;
	FTOM_NODE_MBC_PTR	pNode;
	FTOM_NODE_CLASS_PTR	pClass = NULL;

	xRet = FTOM_NODE_MBC_getClass(pInfo->pModel, &pClass);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	pNode = (FTOM_NODE_MBC_PTR)FTM_MEM_malloc(sizeof(FTOM_NODE_MBC));
	if (pNode == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR2(xRet, "Not enough memory!\n");
		return	xRet;
	}

	memcpy(&pNode->xCommon.xInfo, pInfo, sizeof(FTM_NODE));

	pNode->xCommon.pClass = pClass;

	*ppNode = (FTOM_NODE_PTR)pNode;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_MBC_destroy
(
	FTOM_NODE_MBC_PTR _PTR_ ppNode
)
{
	ASSERT(ppNode != NULL);

	FTM_MEM_free(*ppNode);

	*ppNode = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_MBC_set
(
	FTOM_NODE_MBC_PTR		pNode,
	FTM_NODE_FIELD			xFields,
	FTM_NODE_PTR			pInfo
)
{
	ASSERT(pNode != NULL);
	ASSERT(pInfo != NULL);

	if (xFields & FTM_NODE_FIELD_FLAGS)
	{
		pNode->xInfo.xFlags = pInfo->xFlags;
	}

	if (xFields & FTM_NODE_FIELD_NAME)
	{
		strcpy(pNode->xInfo.pName, pInfo->pName);
	}

	if (xFields & FTM_NODE_FIELD_LOCATION)
	{
		strcpy(pNode->xInfo.pLocation, pInfo->pLocation);
	}

	if (xFields & FTM_NODE_FIELD_INTERVAL)
	{
		pNode->xInfo.ulReportInterval = pInfo->ulReportInterval;
	}

	if (xFields & FTM_NODE_FIELD_TIMEOUT)
	{
		pNode->xInfo.ulTimeout = pInfo->ulTimeout;
	}

	if (xFields & FTM_NODE_FIELD_MQTT_VERSION)
	{
		pNode->xInfo.xOption.xMQTT.ulVersion = pInfo->xOption.xMQTT.ulVersion;
	}

	if (xFields & FTM_NODE_FIELD_MQTT_URL)
	{
		strcpy(pNode->xInfo.xOption.xMQTT.pURL, pInfo->xOption.xMQTT.pURL);
	}

	if (xFields & FTM_NODE_FIELD_MQTT_TOPIC)
	{
		strcpy(pNode->xInfo.xOption.xMQTT.pTopic, pInfo->xOption.xMQTT.pTopic);
	}

	return	FTM_RET_OK;
}

FTM_RET	FTOM_NODE_MBC_printOpts
(
	FTOM_NODE_MBC_PTR	pNode
)
{
	ASSERT(pNode != NULL);

	MESSAGE("%16s   %10s - %d\n", "", "Version", 	pNode->xCommon.xInfo.xOption.xMB.ulVersion);	
	MESSAGE("%16s   %10s - %s\n", "", "URL", 		pNode->xCommon.xInfo.xOption.xMB.pURL);
	MESSAGE("%16s   %10s - %lu\n","", "Port", 		pNode->xCommon.xInfo.xOption.xMB.ulPort);
	MESSAGE("%16s   %10s - %lu\n","", "SlaveID", 	pNode->xCommon.xInfo.xOption.xMB.ulSlaveID);

	return	FTM_RET_OK;
}
