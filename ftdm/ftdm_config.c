#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_object.h"
#include "ftdm_config.h"

#define	FTDM_SECTION_DB_STRING				"DATABASE"
#define	FTDM_SECTION_SERVER_STRING			"SERVER"
#define	FTDM_SECTION_NODE_STRING			"NODE"
#define	FTDM_SECTION_EP_STRING				"ENDPOINT"

#define	FTDM_FIELD_NODES_STRING				"nodes"
#define	FTDM_FIELD_FILE_STRING				"file"
#define	FTDM_FIELD_PORT_STRING				"port"
#define	FTDM_FIELD_SESSION_COUNT_STRING		"session_count"
#define	FTDM_FIELD_EP_CLASSES_STRING		"classes"

#define	FTDM_FIELD_EPS_STRING				"eps"

#define	FTDM_FIELD_NODE_DID_STRING			"did"
#define	FTDM_FIELD_NODE_TYPE_STRING			"type"
#define	FTDM_FIELD_NODE_LOCATION_STRING		"location"
#define	FTDM_FIELD_NODE_INTERVAL_STRING		"interval"
#define	FTDM_FIELD_NODE_SNMP_STRING			"snmp"

#define	FTDM_FIELD_SNMP_VERSION_STRING		"version"
#define	FTDM_FIELD_SNMP_URL_STRING			"url"
#define	FTDM_FIELD_SNMP_COMMUNITY_STRING	"community"

#define	FTDM_FIELD_EP_ID_STRING				"epid"
#define	FTDM_FIELD_EP_TYPE_STRING			"type"
#define	FTDM_FIELD_EP_NAME_STRING			"name"
#define	FTDM_FIELD_EP_UNIT_STRING			"unit"
#define	FTDM_FIELD_EP_INTERVAL_STRING		"interval"
#define	FTDM_FIELD_EP_DID_STRING			"did"
#define	FTDM_FIELD_EP_PID_STRING			"pid"

#define	FTDM_FIELD_EP_CLASS_CLASS_STRING	"class"
#define	FTDM_FIELD_EP_CLASS_ID_STRING		"id"
#define	FTDM_FIELD_EP_CLASS_TYPE_STRING		"type"
#define	FTDM_FIELD_EP_CLASS_NAME_STRING		"name"
#define	FTDM_FIELD_EP_CLASS_SN_STRING		"sn"
#define	FTDM_FIELD_EP_CLASS_STATE_STRING	"state"
#define	FTDM_FIELD_EP_CLASS_VALUE_STRING	"value"
#define	FTDM_FIELD_EP_CLASS_TIME_STRING		"time"

static FTM_RET	getDBFileName(config_t *pConfig, FTM_CHAR_PTR pBuff, FTM_ULONG ulLen);

static FTM_RET	getServerInfo(config_t *pConfig, FTM_SERVER_INFO_PTR pInfo);

static FTM_RET	getNodeInfoCount(config_t *pConfig, FTM_ULONG_PTR pCount);
static FTM_RET	getNodeInfoByIndex(config_t *pConfig, FTM_ULONG ulIndex, FTM_NODE_INFO_PTR pInfo);
static int		seekNodeInfo(const void *pItem, const void *pKey);

//static FTM_RET	getEPInfo(config_t *pConfig, FTM_EP_INFO_PTR pInfo);
static FTM_RET	getEPInfoCount(config_t *pConfig, FTM_ULONG_PTR pCount);
static FTM_RET	getEPInfoByIndex(config_t *pConfig, FTM_ULONG ulIndex, FTM_EP_INFO_PTR pInfo);
static int		seekEPInfo(const void *pItem, const void *pKey);

static FTM_RET	getEPClassInfoCount(config_t *pConfig, FTM_ULONG_PTR pCount);
static FTM_RET	getEPClassInfo(config_t *pConfig, FTM_EP_CLASS xClass, FTM_EP_CLASS_INFO_PTR pInfo);
static FTM_RET	getEPClassInfoByIndex(config_t *pConfig, FTM_ULONG ulIndex, FTM_EP_CLASS_INFO_PTR pInfo);
static int		seekEPClassInfo(const void *pItem, const void *pKey);

static FTM_CHAR_PTR	getNodeTypeString(FTM_ULONG ulType);
static FTM_CHAR_PTR	getEPTypeString(FTM_ULONG ulType);
static FTM_CHAR_PTR	getSNMPVersionString(FTM_ULONG ulVersion);

extern char *program_invocation_short_name;


FTM_RET FTDM_CFG_init(FTDM_CFG_PTR pConfig)
{
	ASSERT(pConfig != NULL);

	memset(pConfig, 0, sizeof(FTDM_CFG));

	list_init(&pConfig->xNode.xList);
	list_attributes_seeker(&pConfig->xNode.xList, seekNodeInfo);

	list_init(&pConfig->xEP.xList);
	list_attributes_seeker(&pConfig->xEP.xList, seekEPInfo);

	list_init(&pConfig->xEP.xClassList);
	list_attributes_seeker(&pConfig->xEP.xClassList, seekEPClassInfo);
	return	FTM_RET_OK;
}

FTM_RET FTDM_CFG_final(FTDM_CFG_PTR pConfig)
{
	FTM_NODE_INFO_PTR		pNodeInfo;
	FTM_EP_INFO_PTR			pEPInfo;
	FTM_EP_CLASS_INFO_PTR	pEPClassInfo;

	ASSERT(pConfig != NULL);
	
	list_iterator_start(&pConfig->xNode.xList);
	while( (pNodeInfo = list_iterator_next(&pConfig->xNode.xList)) != NULL)
	{
		free(pNodeInfo);	
	}
	list_destroy(&pConfig->xNode.xList);

	list_iterator_start(&pConfig->xEP.xList);
	while( (pEPInfo = list_iterator_next(&pConfig->xEP.xList)) != NULL)
	{
		free(pEPInfo);	
	}
	list_destroy(&pConfig->xEP.xList);

	list_iterator_start(&pConfig->xEP.xClassList);
	while( (pEPClassInfo = list_iterator_next(&pConfig->xEP.xClassList)) != NULL)
	{
		free(pEPClassInfo);	
	}
	list_destroy(&pConfig->xEP.xClassList);

	if (pConfig->xDB.pFileName != NULL)
	{
		free(pConfig->xDB.pFileName);	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_CFG_load(FTDM_CFG_PTR pConfig, FTM_CHAR_PTR pFileName)
{
	config_t	xConfig;
	FTM_CHAR	pBuff[1024];
	FTM_ULONG	ulCount;
	FTM_SERVER_INFO	xServerInfo;

	ASSERT((pConfig != NULL) && (pFileName != NULL));

	config_init(&xConfig);
	config_read_file(&xConfig, pFileName);

	if (getServerInfo(&xConfig, &xServerInfo) == FTM_RET_OK)
	{
		FTDM_CFG_setServer(pConfig, &xServerInfo);	
	}

	if (getDBFileName(&xConfig, pBuff, sizeof(pBuff)) == FTM_RET_OK)
	{
		FTDM_CFG_setDBFileName(pConfig, pBuff);
	}

	if (getNodeInfoCount(&xConfig, &ulCount) == FTM_RET_OK)
	{
		FTM_ULONG		i;
		FTM_NODE_INFO	xInfo;

		for( i = 0 ; i < ulCount ; i++)
		{
			memset(&xInfo, 0, sizeof(xInfo));
			getNodeInfoByIndex(&xConfig, i, &xInfo);
			FTDM_CFG_addNodeInfo(&pConfig->xNode, &xInfo);
		}
	}

	if (getEPInfoCount(&xConfig, &ulCount) == FTM_RET_OK)
	{
		FTM_ULONG		i;
		FTM_EP_INFO	xInfo;

		for( i = 0 ; i < ulCount ; i++)
		{
			memset(&xInfo, 0, sizeof(xInfo));
			getEPInfoByIndex(&xConfig, i, &xInfo);
			FTDM_CFG_addEPInfo(&pConfig->xEP, &xInfo);
		}
	}

	if (getEPClassInfoCount(&xConfig, &ulCount) == FTM_RET_OK)
	{
		FTM_ULONG			i;
		FTM_EP_CLASS_INFO	xInfo;

		for( i = 0 ; i < ulCount ; i++)
		{
			getEPClassInfoByIndex(&xConfig, i, &xInfo);
			FTDM_CFG_addEPClassInfo(&pConfig->xEP, &xInfo);
		}

	}

	config_destroy(&xConfig);

	return	FTM_RET_OK;
}

FTM_RET FTDM_CFG_show(FTDM_CFG_PTR pConfig)
{
	FTM_ULONG	i, ulCount;

	ASSERT(pConfig != NULL);

	MESSAGE("###################### FTDM Configuration #####################\n");
	MESSAGE("\n[ DB ]\n");
	MESSAGE("\t %-8s : %s\n", "DB FILE", pConfig->xDB.pFileName);

	MESSAGE("\n[ NODE ]\n");
	MESSAGE("\t%-16s %-16s %-16s %-16s %-16s %-16s %-16s\n",
			"DID", "TYPE", "LOCATION", "INTERVAL" "OPT0", "OPT1", "OPT2");
	if (FTDM_CFG_getNodeInfoCount(&pConfig->xNode, &ulCount) == FTM_RET_OK)
	{
		for(i = 0 ; i < ulCount ; i++)
		{
			FTM_NODE_INFO	xNodeInfo;

			FTDM_CFG_getNodeInfoByIndex(&pConfig->xNode, i, &xNodeInfo);
			MESSAGE("\t%-16s %-16s %-16s %8d %-16s %-16s %-16s\n",
				xNodeInfo.pDID,
				getNodeTypeString(xNodeInfo.xType),
				xNodeInfo.pLocation,
				xNodeInfo.ulInterval,
				getSNMPVersionString(xNodeInfo.xOption.xSNMP.nVersion),
				xNodeInfo.xOption.xSNMP.pURL,
				xNodeInfo.xOption.xSNMP.pCommunity);
		}
	}

	MESSAGE("\n[ EP ]\n");
	MESSAGE("# PRE-REGISTERED ENDPOINT\n");
	MESSAGE("\t%-08s %-16s %-16s %-8s %-8s %-16s %-16s\n",
			"EPID", "TYPE", "NAME", "UNIT", "INTERVAL", "DID", "PID");
	if (FTDM_CFG_getEPInfoCount(&pConfig->xEP, &ulCount) == FTM_RET_OK)
	{
		for(i = 0 ; i < ulCount ; i++)
		{
			FTM_EP_INFO	xEPInfo;

			FTDM_CFG_getEPInfoByIndex(&pConfig->xEP, i, &xEPInfo);
			MESSAGE("\t%08lx %-16s %-16s %-8s %8lu %-16s %-16s\n",
				xEPInfo.xEPID,
				getEPTypeString(xEPInfo.xType),
				xEPInfo.pName,
				xEPInfo.pUnit,
				xEPInfo.nInterval,
				xEPInfo.pDID,
				xEPInfo.pPID);
		}
	}


	MESSAGE("\n#CLASS INFORMATION\n");
	MESSAGE("\t%-08s %-16s %-16s %-16s %-16s %-16s %-16s %-16s\n",
			"CLASS", "ID", "TYPE", "NAME", "S/N", "STATE", "VALUE", "TIME");
	if (FTDM_CFG_getEPClassInfoCount(&pConfig->xEP, &ulCount) == FTM_RET_OK)
	{
		for(i = 0 ; i < ulCount ; i++)
		{
			FTM_EP_CLASS_INFO	xEPClassInfo;

			FTDM_CFG_getEPClassInfoByIndex(&pConfig->xEP, i, &xEPClassInfo);
			MESSAGE("\t%08lx %-16s %-16s %-16s %-16s %-16s %-16s %-16s\n",
				xEPClassInfo.xClass, 
				xEPClassInfo.xOIDs.pID,
				xEPClassInfo.xOIDs.pType,
				xEPClassInfo.xOIDs.pName,
				xEPClassInfo.xOIDs.pSN,
				xEPClassInfo.xOIDs.pState,
				xEPClassInfo.xOIDs.pValue,
				xEPClassInfo.xOIDs.pTime);
		}
	}
	return	FTM_RET_OK;
}

/************************************************************************
 * DB Configuration 
 ************************************************************************/
FTM_RET FTDM_CFG_setDBFileName(FTDM_CFG_PTR pConfig, FTM_CHAR_PTR pFileName)
{
	ASSERT((pConfig != NULL) && (pFileName != NULL));

	if (pConfig->xDB.pFileName != NULL)
	{
		free(pConfig->xDB.pFileName);	
	}

	pConfig->xDB.pFileName = strdup(pFileName);

	return	FTM_RET_OK;
}

/************************************************************************
 * Server Configuration 
 ************************************************************************/
FTM_RET	FTDM_CFG_setServer(FTDM_CFG_PTR pConfig, FTM_SERVER_INFO_PTR pInfo)
{
	ASSERT((pConfig != NULL) && (pInfo != NULL));

	pConfig->xServer.usPort = pInfo->usPort;
	pConfig->xServer.ulMaxSession = pInfo->ulSessionCount;

	return	FTM_RET_OK;
}

/************************************************************************
 * Node Configuraton
 ************************************************************************/
FTM_RET	FTDM_CFG_addNodeInfo(FTDM_CFG_NODE_PTR pConfig, FTM_NODE_INFO_PTR pInfo)
{
	FTM_NODE_INFO_PTR	pNewInfo;

	ASSERT((pConfig != NULL) && (pInfo != NULL));

	pNewInfo = (FTM_NODE_INFO_PTR)calloc(1, sizeof(FTM_NODE_INFO));
	if (pNewInfo == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNewInfo, pInfo, sizeof(FTM_NODE_INFO));

	list_append(&pConfig->xList, pNewInfo);

	return	FTM_RET_OK;
}

FTM_RET FTDM_CFG_getNodeInfoCount(FTDM_CFG_NODE_PTR pConfig, FTM_ULONG_PTR pCount)
{
	ASSERT((pConfig != NULL) && (pCount != NULL));

	*pCount = list_size(&pConfig->xList);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_CFG_getNodeInfoByIndex(FTDM_CFG_NODE_PTR pConfig, FTM_ULONG ulIndex, FTM_NODE_INFO_PTR pInfo)
{
	FTM_NODE_INFO_PTR	pNodeInfo;

	ASSERT((pConfig != NULL) && (pInfo != NULL));

	pNodeInfo = list_get_at(&pConfig->xList, ulIndex);
	if (pNodeInfo == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	memcpy(pInfo, pNodeInfo, sizeof(FTM_NODE_INFO));

	return	FTM_RET_OK;
}

/************************************************************************
 * EndPoint Configuraton
 ************************************************************************/
FTM_RET	FTDM_CFG_addEPInfo(FTDM_CFG_EP_PTR pConfig, FTM_EP_INFO_PTR pInfo)
{
	FTM_EP_INFO_PTR	pNewInfo;

	ASSERT((pConfig != NULL) && (pInfo != NULL));

	pNewInfo = (FTM_EP_INFO_PTR)calloc(1, sizeof(FTM_EP_INFO));
	if (pNewInfo == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNewInfo, pInfo, sizeof(FTM_EP_INFO));

	list_append(&pConfig->xList, pNewInfo);

	return	FTM_RET_OK;
}

FTM_RET FTDM_CFG_getEPInfoCount(FTDM_CFG_EP_PTR pConfig, FTM_ULONG_PTR pCount)
{
	ASSERT((pConfig != NULL) && (pCount != NULL));

	*pCount = list_size(&pConfig->xList);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_CFG_getEPInfoByIndex(FTDM_CFG_EP_PTR pConfig, FTM_ULONG ulIndex, FTM_EP_INFO_PTR pInfo)
{
	FTM_EP_INFO_PTR	pEPInfo;

	ASSERT((pConfig != NULL) && (pInfo != NULL));

	pEPInfo = list_get_at(&pConfig->xList, ulIndex);
	if (pEPInfo == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	memcpy(pInfo, pEPInfo, sizeof(FTM_EP_INFO));

	return	FTM_RET_OK;
}

/************************************************************************
 * EndPoint Class Configuraton
 ************************************************************************/
FTM_RET FTDM_CFG_addEPClassInfo(FTDM_CFG_EP_PTR pConfig, FTM_EP_CLASS_INFO_PTR pInfo)
{
	FTM_EP_CLASS_INFO_PTR	pNewInfo;	
	
	ASSERT((pConfig != NULL) && (pInfo != NULL));

	pNewInfo = (FTM_EP_CLASS_INFO_PTR)calloc(1, sizeof(FTM_EP_CLASS_INFO));
	if (pNewInfo == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNewInfo, pInfo, sizeof(FTM_EP_CLASS_INFO));

	list_append(&pConfig->xClassList, pNewInfo);

	return	FTM_RET_OK;
}

FTM_RET FTDM_CFG_getEPClassInfoCount(FTDM_CFG_EP_PTR pConfig, FTM_ULONG_PTR pCount)
{
	ASSERT((pConfig != NULL) && (pCount != NULL));

	*pCount = list_size(&pConfig->xClassList);

	return	FTM_RET_OK;
}

FTM_RET FTDM_CFG_getEPClassInfo(FTDM_CFG_EP_PTR pConfig, FTM_EP_CLASS xClass, FTM_EP_CLASS_INFO_PTR pInfo)
{
	ASSERT((pConfig != NULL) && (pInfo != NULL));

	return	FTM_RET_OK;
}

FTM_RET FTDM_CFG_getEPClassInfoByIndex(FTDM_CFG_EP_PTR pConfig, FTM_ULONG ulIndex, FTM_EP_CLASS_INFO_PTR pInfo)
{
	FTM_EP_CLASS_INFO_PTR	pEPClassInfo;

	ASSERT((pConfig != NULL) && (pInfo != NULL));

	pEPClassInfo = list_get_at(&pConfig->xClassList, ulIndex);
	if (pEPClassInfo == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	memcpy(pInfo, pEPClassInfo, sizeof(FTM_EP_CLASS_INFO));

	return	FTM_RET_OK;
}

/************************************************************************
 * Internal functions
 ************************************************************************/
FTM_RET	getDBFileName(config_t *pConfig, FTM_CHAR_PTR pBuff, FTM_ULONG ulLen)
{
	config_setting_t *pSection;
	config_setting_t *pField;

	ASSERT(pBuff != NULL);

	if (pConfig == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	pSection = config_lookup(pConfig, FTDM_SECTION_DB_STRING);
	if (pSection == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;
	}

	pField = config_setting_get_member(pSection, FTDM_FIELD_FILE_STRING);
	if (pField == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	strncpy(pBuff, config_setting_get_string(pField), ulLen);
	return	FTM_RET_OK;	
}

FTM_RET	getServerInfo(config_t *pConfig, FTM_SERVER_INFO_PTR pInfo)
{
	config_setting_t *pSection;
	config_setting_t *pField;

	ASSERT(pInfo != NULL);

	if (pConfig == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	pSection = config_lookup(pConfig, FTDM_SECTION_SERVER_STRING);
	if (pSection == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	pField = config_setting_get_member(pSection, FTDM_FIELD_PORT_STRING);
	if (pField == NULL)
	{
		pInfo->usPort = FTDM_SERVER_DEFAULT_PORT;
	}
	else
	{
		pInfo->usPort = (FTM_USHORT)config_setting_get_int(pField);
	}

	pField = config_setting_get_member(pSection, FTDM_FIELD_SESSION_COUNT_STRING);
	if (pField == NULL)
	{
		pInfo->ulSessionCount = FTDM_SERVER_DEFAULT_MAX_SESSION;
	}
	else
	{
		pInfo->ulSessionCount = (FTM_ULONG)config_setting_get_int(pField);
	}

	return	FTM_RET_OK;
}

FTM_RET	getNodeInfoCount(config_t *pConfig, FTM_ULONG_PTR pCount)
{
	config_setting_t *pSection;
	config_setting_t *pField;
	
	ASSERT(pCount != NULL);

	if (pConfig == NULL)
	{
		TRACE("pConfig is NULL!\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	pSection = config_lookup(pConfig, FTDM_SECTION_NODE_STRING);
	if (pSection == NULL)
	{
		TRACE("Section [%s] not found.\n", FTDM_SECTION_NODE_STRING);
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	pField = config_setting_get_member(pSection, FTDM_FIELD_NODES_STRING);

	if ((pField != NULL) && (config_setting_is_list(pField) == CONFIG_TRUE))
	{
		*pCount	= config_setting_length(pField);
	}
	else
	{
		*pCount = 0;
	}

	return	FTM_RET_OK;
}

FTM_RET	getNodeInfoByIndex(config_t *pConfig, FTM_ULONG ulIndex,  FTM_NODE_INFO_PTR pInfo)
{
	config_setting_t *pSection;
	config_setting_t *pDefault;

	ASSERT(pInfo != NULL);

	if (pConfig == NULL)
	{
		TRACE("pConfig is NULL!\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	pSection = config_lookup(pConfig, FTDM_SECTION_NODE_STRING);
	if (pSection == NULL)
	{
		TRACE("Section [%s] not found.\n", FTDM_SECTION_NODE_STRING);
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	pDefault = config_setting_get_member(pSection, FTDM_FIELD_NODES_STRING);
	if ((pDefault != NULL) && (config_setting_is_list(pDefault) == CONFIG_TRUE))
	{
		FTM_NODE_INFO	xNodeInfo;
		config_setting_t	*pNode;
		config_setting_t	*pField;

		memset(&xNodeInfo, 0, sizeof(xNodeInfo));

		pNode = config_setting_get_elem(pDefault, ulIndex);
		if (pNode == NULL)
		{
			TRACE("Object not found\n");
			return	FTM_RET_OBJECT_NOT_FOUND;
		}

		pField = config_setting_get_member(pNode, FTDM_FIELD_NODE_DID_STRING);	
		if(pField != NULL)
		{
			strncpy(xNodeInfo.pDID, config_setting_get_string(pField), FTM_DID_LEN);
		}

		pField = config_setting_get_member(pNode, FTDM_FIELD_NODE_TYPE_STRING);	
		if(pField != NULL)
		{
			xNodeInfo.xType = (FTM_ULONG)config_setting_get_int(pField);
		}

		pField = config_setting_get_member(pNode, FTDM_FIELD_NODE_LOCATION_STRING);	
		if(pField != NULL)
		{
			strncpy(xNodeInfo.pLocation, config_setting_get_string(pField), FTM_LOCATION_LEN);
		}

		pField = config_setting_get_member(pNode, FTDM_FIELD_NODE_INTERVAL_STRING);	
		if(pField != NULL)
		{
			xNodeInfo.ulInterval = (FTM_ULONG)config_setting_get_int(pField);
		}

		switch(xNodeInfo.xType)
		{
		case	FTM_NODE_TYPE_SNMP:
			{
				config_setting_t	*pSNMP;

				pSNMP = config_setting_get_member(pNode, FTDM_FIELD_NODE_SNMP_STRING);	
				if(pSNMP != NULL)
				{
					pField = config_setting_get_member(pSNMP, FTDM_FIELD_SNMP_VERSION_STRING);
					if (pField != NULL)
					{
						xNodeInfo.xOption.xSNMP.nVersion = (FTM_ULONG)config_setting_get_int(pField);
					}

					pField = config_setting_get_member(pSNMP, FTDM_FIELD_SNMP_URL_STRING);
					if (pField != NULL)
					{
						strncpy(xNodeInfo.xOption.xSNMP.pURL, config_setting_get_string(pField), FTM_URL_LEN);
					}

					pField = config_setting_get_member(pSNMP, FTDM_FIELD_SNMP_COMMUNITY_STRING);
					if (pField != NULL)
					{
						strncpy(xNodeInfo.xOption.xSNMP.pCommunity, config_setting_get_string(pField), FTM_SNMP_COMMUNITY_LEN);
					}

				}
			}
			break;
		}

		memcpy(pInfo, &xNodeInfo, sizeof(xNodeInfo));
	}

	return	FTM_RET_OK;
}

int	seekNodeInfo(const void *pItem, const void *pKey)
{
	FTM_NODE_INFO_PTR	pInfo = (FTM_NODE_INFO_PTR)pItem;
	FTM_CHAR_PTR		pDID = (FTM_CHAR_PTR)pKey;

	return (strcmp(pInfo->pDID, pDID) == 0);
}

/****************************************************************************
 *
 ****************************************************************************/

FTM_RET	getEPInfoCount(config_t *pConfig, FTM_ULONG_PTR pCount)
{
	config_setting_t *pSection;
	config_setting_t *pField;
	
	ASSERT(pCount != NULL);

	if (pConfig == NULL)
	{
		TRACE("pConfig is NULL!\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	pSection = config_lookup(pConfig, FTDM_SECTION_EP_STRING);
	if (pSection == NULL)
	{
		TRACE("Section [%s] not found.\n", FTDM_SECTION_EP_STRING);
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	pField = config_setting_get_member(pSection, FTDM_FIELD_EPS_STRING);

	if ((pField != NULL) && (config_setting_is_list(pField) == CONFIG_TRUE))
	{
		*pCount	= config_setting_length(pField);
	}
	else
	{
		*pCount = 0;
	}

	return	FTM_RET_OK;
}

FTM_RET	getEPInfoByIndex(config_t *pConfig, FTM_ULONG ulIndex, FTM_EP_INFO_PTR pInfo)
{
	config_setting_t *pSection;
	config_setting_t *pDefault;

	ASSERT(pInfo != NULL);

	if (pConfig == NULL)
	{
		TRACE("pConfig is NULL!\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	pSection = config_lookup(pConfig, FTDM_SECTION_EP_STRING);
	if (pSection == NULL)
	{
		TRACE("Section [%s] not found.\n", FTDM_SECTION_EP_STRING);
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	pDefault = config_setting_get_member(pSection, FTDM_FIELD_EPS_STRING);
	if ((pDefault != NULL) && (config_setting_is_list(pDefault) == CONFIG_TRUE))
	{
		FTM_EP_INFO			xEPInfo;
		config_setting_t	*pEP;
		config_setting_t	*pField;

		pEP = config_setting_get_elem(pDefault, ulIndex);
		if (pEP == NULL)
		{
			TRACE("Object not found\n");
			return	FTM_RET_OBJECT_NOT_FOUND;
		}

		pField = config_setting_get_member(pEP, FTDM_FIELD_EP_ID_STRING);	
		if(pField != NULL)
		{
			xEPInfo.xEPID = (FTM_ULONG)config_setting_get_int(pField);
		}

		pField = config_setting_get_member(pEP, FTDM_FIELD_EP_TYPE_STRING);	
		if(pField != NULL)
		{
			xEPInfo.xType = (FTM_ULONG)config_setting_get_int(pField);
		}

		pField = config_setting_get_member(pEP, FTDM_FIELD_EP_NAME_STRING);	
		if(pField != NULL)
		{
			strncpy(xEPInfo.pName, config_setting_get_string(pField), FTM_NAME_LEN);
		}

		pField = config_setting_get_member(pEP, FTDM_FIELD_EP_UNIT_STRING);	
		if(pField != NULL)
		{
			strncpy(xEPInfo.pUnit, config_setting_get_string(pField), FTM_UNIT_LEN);
		}

		pField = config_setting_get_member(pEP, FTDM_FIELD_EP_INTERVAL_STRING);	
		if(pField != NULL)
		{
			xEPInfo.nInterval = (FTM_ULONG)config_setting_get_int(pField);
		}

		pField = config_setting_get_member(pEP, FTDM_FIELD_EP_DID_STRING);	
		if(pField != NULL)
		{
			strncpy(xEPInfo.pDID, config_setting_get_string(pField), FTM_DID_LEN);
		}

		pField = config_setting_get_member(pEP, FTDM_FIELD_EP_PID_STRING);	
		if(pField != NULL)
		{
			strncpy(xEPInfo.pPID, config_setting_get_string(pField), FTM_DID_LEN);
		}

		memcpy(pInfo, &xEPInfo, sizeof(xEPInfo));
	}

	return	FTM_RET_OK;
}

FTM_RET	getEPClassInfoCount(config_t *pConfig, FTM_ULONG_PTR pCount)
{
	config_setting_t *pSection;
	config_setting_t *pField;
	
	ASSERT(pCount != NULL);

	if (pConfig == NULL)
	{
		TRACE("pConfig is NULL!\n");
		return	FTM_RET_NOT_INITIALIZED;	
	}

	pSection = config_lookup(pConfig, FTDM_SECTION_EP_STRING);
	if (pSection == NULL)
	{
		TRACE("Section [%s] not found.\n", FTDM_SECTION_EP_STRING);
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	pField = config_setting_get_member(pSection, FTDM_FIELD_EP_CLASSES_STRING);

	if ((pField != NULL) && (config_setting_is_list(pField) == CONFIG_TRUE))
	{
		*pCount	= config_setting_length(pField);
	}
	else
	{
		*pCount = 0;
	}

	return	FTM_RET_OK;
}

FTM_RET	getEPClassInfo(config_t *pConfig, FTM_EP_CLASS xClass, FTM_EP_CLASS_INFO_PTR pInfo)
{
	config_setting_t *pSection;
	config_setting_t *pItemArray;
	config_setting_t *pItem;
	config_setting_t *pField;
	
	ASSERT(pCount != NULL);

	if (pConfig == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	pSection = config_lookup(pConfig, FTDM_SECTION_EP_STRING);
	if (pSection == NULL)
	{
		TRACE("Section [%s] not found.\n", FTDM_SECTION_EP_STRING);
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	pItemArray = config_setting_get_member(pSection, FTDM_FIELD_EP_CLASSES_STRING);
	if (pItemArray != NULL)
	{
		FTM_ULONG	i;

		for(i = 0 ; i < config_setting_length(pItemArray) ; i++)
		{
			pItem = config_setting_get_elem(pItemArray, i);
			if (pItem != NULL)
			{
				pField = config_setting_get_member(pItem, FTDM_FIELD_EP_CLASS_CLASS_STRING);
				if ((pField == NULL) || (xClass != (FTM_ULONG)config_setting_get_int(pField)))
				{
					continue;
				}

				pInfo->xClass = xClass;

				pField = config_setting_get_member(pItem, FTDM_FIELD_EP_CLASS_ID_STRING);
				if (pField != NULL)
				{
					strcpy(pInfo->xOIDs.pID, config_setting_get_string(pField));
				}

				pField = config_setting_get_member(pItem, FTDM_FIELD_EP_CLASS_TYPE_STRING);
				if (pField != NULL)
				{
					strcpy(pInfo->xOIDs.pType, config_setting_get_string(pField));
				}

				pField = config_setting_get_member(pItem, FTDM_FIELD_EP_CLASS_NAME_STRING);
				if (pField != NULL)
				{
					strcpy(pInfo->xOIDs.pName, config_setting_get_string(pField));
				}

				pField = config_setting_get_member(pItem, FTDM_FIELD_EP_CLASS_SN_STRING);
				if (pField != NULL)
				{
					strcpy(pInfo->xOIDs.pSN, config_setting_get_string(pField));
				}

				pField = config_setting_get_member(pItem, FTDM_FIELD_EP_CLASS_STATE_STRING);
				if (pField != NULL)
				{
					strcpy(pInfo->xOIDs.pState, config_setting_get_string(pField));
				}

				pField = config_setting_get_member(pItem, FTDM_FIELD_EP_CLASS_VALUE_STRING);
				if (pField != NULL)
				{
					strcpy(pInfo->xOIDs.pValue, config_setting_get_string(pField));
				}

				pField = config_setting_get_member(pItem, FTDM_FIELD_EP_CLASS_TIME_STRING);
				if (pField != NULL)
				{
					strcpy(pInfo->xOIDs.pTime, config_setting_get_string(pField));
				}

			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	getEPClassInfoByIndex(config_t *pConfig, FTM_ULONG ulIndex, FTM_EP_CLASS_INFO_PTR pInfo)
{
	config_setting_t *pSection;
	config_setting_t *pItemArray;
	config_setting_t *pItem;
	config_setting_t *pField;
	
	ASSERT(pCount != NULL);

	if (pConfig == NULL)
	{
		return	FTM_RET_NOT_INITIALIZED;	
	}

	pSection = config_lookup(pConfig, FTDM_SECTION_EP_STRING);
	if (pSection == NULL)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	pItemArray = config_setting_get_member(pSection, FTDM_FIELD_EP_CLASSES_STRING);
	if (pItemArray != NULL)
	{
		pItem = config_setting_get_elem(pItemArray, ulIndex);
		if (pItem != NULL)
		{
			pField = config_setting_get_member(pItem, FTDM_FIELD_EP_CLASS_CLASS_STRING);
			if (pField != NULL)
			{ 
				pInfo->xClass = (FTM_ULONG)config_setting_get_int(pField);
			}

			pField = config_setting_get_member(pItem, FTDM_FIELD_EP_CLASS_ID_STRING);
			if (pField != NULL)
			{
				strcpy(pInfo->xOIDs.pID, config_setting_get_string(pField));
			}

			pField = config_setting_get_member(pItem, FTDM_FIELD_EP_CLASS_TYPE_STRING);
			if (pField != NULL)
			{
				strcpy(pInfo->xOIDs.pType, config_setting_get_string(pField));
			}

			pField = config_setting_get_member(pItem, FTDM_FIELD_EP_CLASS_NAME_STRING);
			if (pField != NULL)
			{
				strcpy(pInfo->xOIDs.pName, config_setting_get_string(pField));
			}

			pField = config_setting_get_member(pItem, FTDM_FIELD_EP_CLASS_SN_STRING);
			if (pField != NULL)
			{
				strcpy(pInfo->xOIDs.pSN, config_setting_get_string(pField));
			}

			pField = config_setting_get_member(pItem, FTDM_FIELD_EP_CLASS_STATE_STRING);
			if (pField != NULL)
			{
				strcpy(pInfo->xOIDs.pState, config_setting_get_string(pField));
			}

			pField = config_setting_get_member(pItem, FTDM_FIELD_EP_CLASS_VALUE_STRING);
			if (pField != NULL)
			{
				strcpy(pInfo->xOIDs.pValue, config_setting_get_string(pField));
			}

			pField = config_setting_get_member(pItem, FTDM_FIELD_EP_CLASS_TIME_STRING);
			if (pField != NULL)
			{
				strcpy(pInfo->xOIDs.pTime, config_setting_get_string(pField));
			}

		}
	}

	return	FTM_RET_OK;
}

int	seekEPInfo(const void *pItem, const void *pKey)
{
	FTM_EP_INFO_PTR	pInfo = (FTM_EP_INFO_PTR)pItem;
	FTM_EPID_PTR	pEPID = (FTM_EPID_PTR)pKey;

	return	(pInfo->xEPID == *pEPID);
}

int	seekEPClassInfo(const void *pItem, const void *pKey)
{
	FTM_EP_CLASS_INFO_PTR	pInfo = (FTM_EP_CLASS_INFO_PTR)pItem;
	FTM_EP_CLASS_PTR		pClass= (FTM_EP_CLASS_PTR)pKey;

	return	(pInfo->xClass == *pClass);
}

FTM_CHAR_PTR	getNodeTypeString(FTM_ULONG ulType)
{
	switch(ulType)
	{
	case	FTM_NODE_TYPE_SNMP:	return	"SNMP";
	case	FTM_NODE_TYPE_MODBUS_OVER_TCP: return	"MODBUS/TCP";
	case	FTM_NODE_TYPE_MODBUS_OVER_SERIAL: return	"MODBUS/SERIAL";
	}

	return	"UNKNOWN";
}

FTM_CHAR_PTR	getEPTypeString(FTM_ULONG ulType)
{
	static	FTM_CHAR	pBuff[16];
	switch(ulType)
	{
	case	FTM_EP_CLASS_TEMPERATURE: 	return	"TEMPERATURE";
	case	FTM_EP_CLASS_HUMIDITY:		return	"HUMIDITY";	
	case	FTM_EP_CLASS_VOLTAGE:		return	"VOLTAGE";
	case	FTM_EP_CLASS_CURRENT:		return	"CURRENT";
	case	FTM_EP_CLASS_DI:			return	"DIGITAL INPUT";	
	case	FTM_EP_CLASS_DO:			return	"DIGITAL OUTPUT";
	case	FTM_EP_CLASS_GAS:			return	"GAS";
	case	FTM_EP_CLASS_POWER:			return	"POWER";
	case	FTM_EP_CLASS_SRF:			return	"SRF";
	case	FTM_EP_CLASS_AI:			return	"ANALOG INPUT";
	case	FTM_EP_CLASS_MULTI:			return	"MULTI-FUNCTION";
	}

	sprintf(pBuff, "%08lx", ulType);
	return	pBuff;
}

FTM_CHAR_PTR	getSNMPVersionString(FTM_ULONG ulVersion)
{
	switch(ulVersion)
	{
	case	0:	return	"SNMPv1";
	case	1: return	"SNMPv2c";
	case	2: return	"SNMPv3";
	}

	return	"SNMP";
}

