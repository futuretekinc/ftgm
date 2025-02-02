#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm.h"
#include "ftm_list.h"
#include "ftm_mem.h"
#include "ftm_snmp.h"

#undef	__MODULE__
#define	__MODULE__	FTM_TRACE_MODULE_OBJECT

typedef struct
{
	FTM_NODE_TYPE	xType;
	FTM_CHAR_PTR	pName;
} 	FTM_NODE_TYPE_INFO, _PTR_ FTM_NODE_TYPE_INFO_PTR;

FTM_RET	FTM_NODE_create(FTM_NODE_PTR _PTR_ ppNode)
{
	ASSERT(ppNode != NULL);

	FTM_RET	xRet;
	FTM_NODE_PTR	pNode;

	pNode = (FTM_NODE_PTR)FTM_MEM_malloc(sizeof(FTM_NODE));
	if (pNode == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	
	xRet = FTM_NODE_setDefault(pNode);
	if (xRet != FTM_RET_OK)
	{
		FTM_MEM_free(pNode);	
	}
	else
	{
		*ppNode = pNode;	
	}

	return	xRet;
}

FTM_RET	FTM_NODE_destroy(FTM_NODE_PTR _PTR_ ppNode)
{
	ASSERT(ppNode != NULL);

	FTM_MEM_free(*ppNode);

	*ppNode = NULL;

	return	FTM_RET_OK;
}

FTM_RET	FTM_NODE_setDefault(FTM_NODE_PTR pNode)
{
	ASSERT(pNode != NULL);

	memset(pNode, 0, sizeof(FTM_NODE));
	
	pNode->xType			= FTM_NODE_TYPE_SNMP;
	pNode->ulReportInterval	= 60000;
	pNode->ulTimeout		= 10000;
	pNode->xOption.xSNMP.ulVersion = FTM_SNMP_VERSION_2;
	strcpy(pNode->xOption.xSNMP.pURL, "127.0.0.1");
	strcpy(pNode->xOption.xSNMP.pCommunity, "public");
	pNode->xOption.xSNMP.ulMaxRetryCount = 3;

	return	FTM_RET_OK;
}

FTM_RET	FTM_NODE_isValid(FTM_NODE_PTR pNode)
{
	ASSERT(pNode != NULL);

	FTM_RET	xRet;

	xRet = FTM_isValidDID(pNode->pDID);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTM_isValidLocation(pNode->pLocation);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTM_isValidInterval(pNode->ulReportInterval);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet= FTM_NODE_isValidType(pNode->xType);
	if(xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	xRet = FTM_NODE_isValidTimeout(pNode, pNode->ulTimeout);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}
	
	xRet = FTM_NODE_isValidSNMPOpt(pNode, &pNode->xOption.xSNMP);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_RET_OK;
}

FTM_RET FTM_NODE_isValidType(FTM_NODE_TYPE xType)
{
	if ((xType == FTM_NODE_TYPE_SNMP) ||
		(xType == FTM_NODE_TYPE_MODBUS_OVER_TCP) ||
		(xType == FTM_NODE_TYPE_FINS) ||
		(xType == FTM_NODE_TYPE_MODBUS_OVER_SERIAL) ||
		(xType == FTM_NODE_TYPE_VIRTUAL))
	{
		return	FTM_RET_OK;	
	}

	return	FTM_RET_INVALID_TYPE;
}

FTM_RET	FTM_NODE_isValidSNMPOpt(FTM_NODE_PTR pNode, FTM_NODE_OPT_SNMP_PTR pOpts)
{
	ASSERT(pNode != NULL);
	ASSERT(pOpts != NULL);
	
	FTM_RET	xRet;

	if (pOpts->ulVersion >= 3)
	{
		return	FTM_RET_INVALID_VERSION;	
	}

	xRet = FTM_isValidURL(pOpts->pURL);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	xRet = FTM_isValidCommunity(pOpts->pCommunity);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_NODE_isValidTimeout
(
	FTM_NODE_PTR pNode, 
	FTM_ULONG	ulTimeout
)
{
	ASSERT(pNode != NULL);

	if (pNode->ulReportInterval < ulTimeout)
	{
		return	FTM_RET_INVALID_TIMEOUT;	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTM_NODE_isStatic
(
	FTM_NODE_PTR pNode
)
{
#if 0
	ASSERT(pNode != NULL);

	if ((pNode->xFlags & FTM_NODE_FLAG_DYNAMIC) != FTM_NODE_FLAG_DYNAMIC)
	{
		return	FTM_RET_TRUE;
	}
	else
	{
		return	FTM_RET_FALSE;
	}
#else
	return	FTM_RET_TRUE;
#endif
}

FTM_RET	FTM_NODE_isDynamic
(
	FTM_NODE_PTR pNode
)
{
#if 0
	ASSERT(pNode != NULL);

	if ((pNode->xFlags & FTM_NODE_FLAG_DYNAMIC) == FTM_NODE_FLAG_DYNAMIC)
	{
		return	FTM_RET_TRUE;
	}
	else
	{
		return	FTM_RET_FALSE;
	}
#else
		return	FTM_RET_FALSE;
#endif
}

FTM_RET	FTM_NODE_setDID
(
	FTM_NODE_PTR pNode, 
	FTM_CHAR_PTR pDID
)
{
	ASSERT(pNode != NULL);

	if (pDID == NULL)
	{
		memset(pNode->pDID, 0, sizeof(pNode->pDID));
	}

	strcpy(pNode->pDID, pDID);

	return	FTM_RET_OK;
}

FTM_NODE_TYPE_INFO	pNodeTypeInfo[] =
{
	{	.xType = FTM_NODE_TYPE_SNMP, .pName = "SNMP"	},
	{	.xType = FTM_NODE_TYPE_MQTT, .pName = "MQTT"	},
	{	.xType = FTM_NODE_TYPE_MODBUS_OVER_SERIAL, .pName = "MODBUS"	},
	{	.xType = FTM_NODE_TYPE_MODBUS_OVER_TCP, .pName = "MODBUSTCP"	},
	{	.xType = FTM_NODE_TYPE_FINS, .pName = "FINS"	},
	{	.xType = FTM_NODE_TYPE_NONE, .pName = "NONE"	},
	{	.xType = FTM_NODE_TYPE_VIRTUAL, .pName = "VIRTUAL"	},
};

FTM_RET	FTM_NODE_strToType
(
	FTM_CHAR_PTR 		pTypeString, 
	FTM_NODE_TYPE_PTR 	pType
)
{
	FTM_INT	i;

	for(i = 0 ; i < sizeof(pNodeTypeInfo) / sizeof(FTM_NODE_TYPE_INFO) ; i++)
	{
		if (strcasecmp(pTypeString, pNodeTypeInfo[i].pName) == 0)
		{
			*pType = pNodeTypeInfo[i].xType;
			return	FTM_RET_OK;
		}
	}

	return	FTM_RET_INVALID_TYPE;
}

FTM_CHAR_PTR FTM_NODE_typeString
(
	FTM_NODE_TYPE xType
)
{
	FTM_INT	i;

	for(i = 0 ; i < sizeof(pNodeTypeInfo) / sizeof(FTM_NODE_TYPE_INFO) ; i++)
	{
		if (xType == pNodeTypeInfo[i].xType)
		{
			return	pNodeTypeInfo[i].pName;
		}
	}

	return	"UNKNOWN";
}

static	FTM_CHAR_PTR	pFlagName[] =
{
	"STATIC",
	NULL
};

FTM_CHAR_PTR	FTM_NODE_flagString
(
	FTM_NODE_FLAG xFlag
)
{
	FTM_INT	i;

	for(i = 0 ; pFlagName[i] != NULL ; i++)
	{
		if (xFlag & (1 << i))
		{
			return	pFlagName[i];
		}
	}

	return	"";
}

FTM_RET	FTM_NODE_print
(
	FTM_NODE_PTR	pNode
)
{
	ASSERT(pNode != NULL);

	MESSAGE("%16s : %s\n", 	"DID", 		pNode->pDID);
	MESSAGE("%16s : %s\n", 	"Model",		pNode->pModel);
	MESSAGE("%16s : %s\n", 	"Name",		pNode->pName);
	MESSAGE("%16s : %s\n", 	"Type", 		FTM_NODE_typeString(pNode->xType));
	MESSAGE("%16s : %lu\n", "Report Interval", 	pNode->ulReportInterval);
	MESSAGE("%16s : %lu\n", "Timeout", 	pNode->ulTimeout);

	switch(pNode->xType)
	{
	case    FTM_NODE_TYPE_SNMP:
		{   
			MESSAGE("%16s : %s\n",  "Version",  FTM_SNMP_versionString(pNode->xOption.xSNMP.ulVersion));
			MESSAGE("%16s : %s\n",  "URL",      pNode->xOption.xSNMP.pURL);
			MESSAGE("%16s : %s\n",  "Community",pNode->xOption.xSNMP.pCommunity);
			MESSAGE("%16s : %s\n",  "MIB",      pNode->xOption.xSNMP.pMIB);
			MESSAGE("%16s : %lu\n", "Retry Count",pNode->xOption.xSNMP.ulMaxRetryCount);
		}
		break;

	case    FTM_NODE_TYPE_MODBUS_OVER_TCP:
		{   
		}
		break;

	case    FTM_NODE_TYPE_MODBUS_OVER_SERIAL:
		{   
		}
		break;
	}

	return	FTM_RET_OK;
}
