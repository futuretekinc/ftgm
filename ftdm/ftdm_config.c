#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm_error.h"
#include "ftm_debug.h"
#include "ftm_object.h"
#include "ftm_mem.h"
#include "ftdm_config.h"

#define	FTDM_SECTION_DB_STRING				"DATABASE"
#define	FTDM_SECTION_SERVER_STRING			"SERVER"
#define	FTDM_SECTION_NODE_STRING			"NODE"
#define	FTDM_SECTION_EP_STRING				"ENDPOINT"
#define	FTDM_SECTION_DEBUG_STRING			"DEBUG"

#define	FTDM_FIELD_NODES_STRING				"nodes"
#define	FTDM_FIELD_FILE_STRING				"file"
#define	FTDM_FIELD_PORT_STRING				"port"
#define	FTDM_FIELD_SESSION_COUNT_STRING		"session_count"
#define	FTDM_FIELD_EP_CLASSES_STRING		"class_info"

#define	FTDM_FIELD_EPS_STRING				"eps"

#define	FTDM_FIELD_NODE_DID_STRING			"did"
#define	FTDM_FIELD_NODE_TYPE_STRING			"type"
#define	FTDM_FIELD_NODE_LOCATION_STRING		"location"
#define	FTDM_FIELD_NODE_INTERVAL_STRING		"interval"
#define	FTDM_FIELD_NODE_TIMEOUT_STRING		"timeout"
#define	FTDM_FIELD_NODE_SNMP_STRING			"snmp"

#define	FTDM_FIELD_SNMP_VERSION_STRING		"version"
#define	FTDM_FIELD_SNMP_URL_STRING			"url"
#define	FTDM_FIELD_SNMP_COMMUNITY_STRING	"community"
#define	FTDM_FIELD_SNMP_MIB_STRING			"mib"

#define	FTDM_FIELD_EP_ID_STRING				"epid"
#define	FTDM_FIELD_EP_TYPE_STRING			"type"
#define	FTDM_FIELD_EP_NAME_STRING			"name"
#define	FTDM_FIELD_EP_UNIT_STRING			"unit"
#define	FTDM_FIELD_EP_ENABLE_STRING			"enable"
#define	FTDM_FIELD_EP_INTERVAL_STRING		"interval"
#define	FTDM_FIELD_EP_TIMEOUT_STRING		"timeout"
#define	FTDM_FIELD_EP_DID_STRING			"did"
#define	FTDM_FIELD_EP_DEPID_STRING			"depid"
#define	FTDM_FIELD_EP_PID_STRING			"pid"
#define	FTDM_FIELD_EP_PEPID_STRING			"pepid"

#define	FTDM_FIELD_EP_CLASS_CLASS_STRING	"class"
#define	FTDM_FIELD_EP_CLASS_ID_STRING		"id"
#define	FTDM_FIELD_EP_CLASS_TYPE_STRING		"type"
#define	FTDM_FIELD_EP_CLASS_NAME_STRING		"name"
#define	FTDM_FIELD_EP_CLASS_SN_STRING		"sn"
#define	FTDM_FIELD_EP_CLASS_STATE_STRING	"state"
#define	FTDM_FIELD_EP_CLASS_VALUE_STRING	"value"
#define	FTDM_FIELD_EP_CLASS_TIME_STRING		"time"

#define	FTDM_FIELD_DEBUG_MODE_STRING				"mode"
#define	FTDM_FIELD_DEBUG_TRACE_PATH_STRING			"path"
#define	FTDM_FIELD_DEBUG_TRACE_PREFIX_STRING		"prefix"
#define	FTDM_FIELD_DEBUG_TRACE_TO_FILE_STRING		"to_file"
#define	FTDM_FIELD_DEBUG_TRACE_PRINT_LINE_STRING	"print_tline"
#define	FTDM_FIELD_DEBUG_ERROR_PATH_STRING			"path"
#define	FTDM_FIELD_DEBUG_ERROR_PREFIX_STRING		"prefix"
#define	FTDM_FIELD_DEBUG_ERROR_TO_FILE_STRING		"to_file"
#define	FTDM_FIELD_DEBUG_ERROR_PRINT_LINE_STRING	"print_tline"

static FTM_RET	CFG_DBFileNameGet(config_t *pConfig, FTM_CHAR_PTR pBuff, FTM_ULONG ulLen);

static FTM_RET	CFG_serverInfoGet(config_t *pConfig, FTM_SERVER_INFO_PTR pInfo);

static FTM_RET	CFG_NODE_INFO_count(config_t *pConfig, FTM_ULONG_PTR pCount);
static FTM_RET	CFG_NODE_INFO_getAt(config_t *pConfig, FTM_ULONG ulIndex, FTM_NODE_INFO_PTR pInfo);
static int		CFG_NODE_INFO_seeker(const void *pItem, const void *pKey);

//static FTM_RET	getEPInfo(config_t *pConfig, FTM_EP_INFO_PTR pInfo);
static FTM_RET	CFG_EP_INFO_count(config_t *pConfig, FTM_ULONG_PTR pCount);
static FTM_RET	CFG_EP_INFO_getAt(config_t *pConfig, FTM_ULONG ulIndex, FTM_EP_INFO_PTR pInfo);
static int		CFG_EP_INFO_seeker(const void *pItem, const void *pKey);

static FTM_RET	CFG_EP_CLASS_INFO_count(config_t *pConfig, FTM_ULONG_PTR pCount);
static FTM_RET	CFG_EP_CLASS_INFO_get(config_t *pConfig, FTM_EP_CLASS xClass, FTM_EP_CLASS_INFO_PTR pInfo);
static FTM_RET	CFG_EP_CLASS_INFO_getAt(config_t *pConfig, FTM_ULONG ulIndex, FTM_EP_CLASS_INFO_PTR pInfo);
static int		CFG_EP_CLASS_INFO_seeker(const void *pItem, const void *pKey);

static FTM_CHAR_PTR	CFG_nodeTypeString(FTM_ULONG ulType);
static FTM_CHAR_PTR	CFG_EPTypeString(FTM_ULONG ulType);
static FTM_CHAR_PTR	CFG_SNMPVersionString(FTM_ULONG ulVersion);

extern char *program_invocation_short_name;


FTM_RET FTDM_CFG_init(FTDM_CFG_PTR pConfig)
{
	ASSERT(pConfig != NULL);

	memset(pConfig, 0, sizeof(FTDM_CFG));

	FTM_LIST_init(&pConfig->xNode.xList);
	FTM_LIST_setSeeker(&pConfig->xNode.xList, CFG_NODE_INFO_seeker);

	FTM_LIST_init(&pConfig->xEP.xList);
	FTM_LIST_setSeeker(&pConfig->xEP.xList, CFG_EP_INFO_seeker);

	FTM_LIST_init(&pConfig->xEP.xClassList);
	FTM_LIST_setSeeker(&pConfig->xEP.xClassList, CFG_EP_CLASS_INFO_seeker);
	return	FTM_RET_OK;
}

FTM_RET FTDM_CFG_final(FTDM_CFG_PTR pConfig)
{
	FTM_NODE_INFO_PTR		pNodeInfo;
	FTM_EP_INFO_PTR			pEPInfo;
	FTM_EP_CLASS_INFO_PTR	pEPClassInfo;

	ASSERT(pConfig != NULL);
	
	FTM_LIST_iteratorStart(&pConfig->xNode.xList);
	while(FTM_LIST_iteratorNext(&pConfig->xNode.xList, (FTM_VOID_PTR _PTR_)&pNodeInfo) == FTM_RET_OK)
	{
		FTM_MEM_free(pNodeInfo);	
	}
	FTM_LIST_final(&pConfig->xNode.xList);

	FTM_LIST_iteratorStart(&pConfig->xEP.xList);
	while( FTM_LIST_iteratorNext(&pConfig->xEP.xList, (FTM_VOID_PTR _PTR_)&pEPInfo) == FTM_RET_OK)
	{
		FTM_MEM_free(pEPInfo);	
	}
	FTM_LIST_final(&pConfig->xEP.xList);

	FTM_LIST_iteratorStart(&pConfig->xEP.xClassList);
	while( FTM_LIST_iteratorNext(&pConfig->xEP.xClassList, (FTM_VOID_PTR _PTR_)&pEPClassInfo) == FTM_RET_OK)
	{
		FTM_MEM_free(pEPClassInfo);	
	}
	FTM_LIST_final(&pConfig->xEP.xClassList);

	if (pConfig->xDB.pFileName != NULL)
	{
		FTM_MEM_free(pConfig->xDB.pFileName);	
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

	FTM_DEBUG_configLoad(&pConfig->xDebug, pFileName);	
	FTM_DEBUG_configSet(&pConfig->xDebug);

	config_init(&xConfig);
	config_read_file(&xConfig, pFileName);

	if (CFG_serverInfoGet(&xConfig, &xServerInfo) == FTM_RET_OK)
	{
		FTDM_CFG_setServer(pConfig, &xServerInfo);	
	}

	if (CFG_DBFileNameGet(&xConfig, pBuff, sizeof(pBuff)) == FTM_RET_OK)
	{
		FTDM_CFG_setDBFileName(pConfig, pBuff);
	}


	if (CFG_NODE_INFO_count(&xConfig, &ulCount) == FTM_RET_OK)
	{
		FTM_ULONG		i;
		FTM_NODE_INFO	xInfo;

		for( i = 0 ; i < ulCount ; i++)
		{
			memset(&xInfo, 0, sizeof(xInfo));
			CFG_NODE_INFO_getAt(&xConfig, i, &xInfo);
			FTDM_CFG_NODE_INFO_append(&pConfig->xNode, &xInfo);
		}
	}

	if (CFG_EP_INFO_count(&xConfig, &ulCount) == FTM_RET_OK)
	{
		FTM_ULONG		i;
		FTM_EP_INFO	xInfo;

		for( i = 0 ; i < ulCount ; i++)
		{
			memset(&xInfo, 0, sizeof(xInfo));
			CFG_EP_INFO_getAt(&xConfig, i, &xInfo);
			FTDM_CFG_EP_INFO_append(&pConfig->xEP, &xInfo);
		}
	}

	if (CFG_EP_CLASS_INFO_count(&xConfig, &ulCount) == FTM_RET_OK)
	{
		FTM_ULONG			i;
		FTM_EP_CLASS_INFO	xInfo;

		TRACE("EP Class : %d\n", ulCount);
		for( i = 0 ; i < ulCount ; i++)
		{
			CFG_EP_CLASS_INFO_getAt(&xConfig, i, &xInfo);
			FTDM_CFG_EP_CLASS_INFO_append(&pConfig->xEP, &xInfo);
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
	MESSAGE("\t%-16s %-16s %-16s %-8s %-8s %-16s %-16s %-16s %-16s\n",
			"DID", "TYPE", "LOCATION", "INTERVAL", "TIMEOUT", "OPT0", "OPT1", "OPT2", "OPT3");
	if (FTDM_CFG_NODE_INFO_count(&pConfig->xNode, &ulCount) == FTM_RET_OK)
	{
		for(i = 0 ; i < ulCount ; i++)
		{
			FTM_NODE_INFO	xNodeInfo;

			FTDM_CFG_NODE_INFO_getAt(&pConfig->xNode, i, &xNodeInfo);
			MESSAGE("\t%-16s %-16s %-16s %-8d %-8d %-16s %-16s %-16s %-16s\n",
				xNodeInfo.pDID,
				CFG_nodeTypeString(xNodeInfo.xType),
				xNodeInfo.pLocation,
				xNodeInfo.ulInterval,
				xNodeInfo.ulTimeout,
				CFG_SNMPVersionString(xNodeInfo.xOption.xSNMP.ulVersion),
				xNodeInfo.xOption.xSNMP.pURL,
				xNodeInfo.xOption.xSNMP.pCommunity,
				xNodeInfo.xOption.xSNMP.pMIB);
		}
	}

	MESSAGE("\n[ EP ]\n");
	MESSAGE("# PRE-REGISTERED ENDPOINT\n");
	MESSAGE("\t%-08s %-16s %-16s %-8s %-8s %-8s %-16s %-08s %-16s %-08s\n",
			"EPID", "TYPE", "NAME", "UNIT", "STATE", "INTERVAL", "TIMEOUT", "DID", "DEPID", "PID", "PEPID");
	if (FTDM_CFG_EP_INFO_count(&pConfig->xEP, &ulCount) == FTM_RET_OK)
	{
		for(i = 0 ; i < ulCount ; i++)
		{
			FTM_EP_INFO	xEPInfo;

			FTDM_CFG_EP_INFO_getAt(&pConfig->xEP, i, &xEPInfo);
			MESSAGE("\t%08lx %-16s %-16s %-8s ",
				xEPInfo.xEPID,
				CFG_EPTypeString(xEPInfo.xType),
				xEPInfo.pName,
				xEPInfo.pUnit);

			switch(xEPInfo.xState)
			{
			case	FTM_EP_STATE_DISABLE: 	MESSAGE("%-8s ", "DISABLE");  	break; 
			case	FTM_EP_STATE_RUN: 		MESSAGE("%-8s ", "RUN"); 		break; 
			case	FTM_EP_STATE_STOP: 		MESSAGE("%-8s ", "STOP"); 		break;
			case	FTM_EP_STATE_ERROR: 	MESSAGE("%-8s ", "ERROR"); 		break;
			default: MESSAGE("%-8s ", "UNKNOWN");
			}

			MESSAGE("%-8lu %-8lu %-16s %08lx %-16s %08lx\n",
				xEPInfo.ulInterval,
				xEPInfo.ulTimeout,
				xEPInfo.pDID,
				xEPInfo.xDEPID,
				xEPInfo.pPID,
				xEPInfo.xPEPID);
		}
	}


	MESSAGE("\n#CLASS INFORMATION\n");
	MESSAGE("\t%-08s %-16s %-16s %-16s %-16s %-16s %-16s %-16s\n",
			"CLASS", "ID", "TYPE", "NAME", "S/N", "STATE", "VALUE", "TIME");
	if (FTDM_CFG_EP_CLASS_INFO_count(&pConfig->xEP, &ulCount) == FTM_RET_OK)
	{
		for(i = 0 ; i < ulCount ; i++)
		{
			FTM_EP_CLASS_INFO	xEPClassInfo;

			FTDM_CFG_EP_CLASS_INFO_getAt(&pConfig->xEP, i, &xEPClassInfo);
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
FTM_RET	FTDM_CFG_NODE_INFO_append(FTDM_CFG_NODE_PTR pConfig, FTM_NODE_INFO_PTR pInfo)
{
	FTM_NODE_INFO_PTR	pNewInfo;

	ASSERT(pConfig != NULL);
	ASSERT(pInfo != NULL);

	pNewInfo = (FTM_NODE_INFO_PTR)calloc(1, sizeof(FTM_NODE_INFO));
	if (pNewInfo == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNewInfo, pInfo, sizeof(FTM_NODE_INFO));

	FTM_LIST_append(&pConfig->xList, pNewInfo);

	return	FTM_RET_OK;
}

FTM_RET FTDM_CFG_NODE_INFO_count(FTDM_CFG_NODE_PTR pConfig, FTM_ULONG_PTR pCount)
{
	ASSERT(pConfig != NULL);
	ASSERT(pCount != NULL);

	return	FTM_LIST_count(&pConfig->xList, pCount);
}

FTM_RET	FTDM_CFG_NODE_INFO_getAt(FTDM_CFG_NODE_PTR pConfig, FTM_ULONG ulIndex, FTM_NODE_INFO_PTR pInfo)
{
	FTM_RET				xRet;
	FTM_NODE_INFO_PTR	pNodeInfo;

	ASSERT((pConfig != NULL) && (pInfo != NULL));

	xRet = FTM_LIST_getAt(&pConfig->xList, ulIndex, (FTM_VOID_PTR _PTR_)&pNodeInfo);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	memcpy(pInfo, pNodeInfo, sizeof(FTM_NODE_INFO));

	return	FTM_RET_OK;
}

/************************************************************************
 * EndPoint Configuraton
 ************************************************************************/
FTM_RET	FTDM_CFG_EP_INFO_append(FTDM_CFG_EP_PTR pConfig, FTM_EP_INFO_PTR pInfo)
{
	FTM_EP_INFO_PTR	pNewInfo;

	ASSERT((pConfig != NULL) && (pInfo != NULL));

	pNewInfo = (FTM_EP_INFO_PTR)calloc(1, sizeof(FTM_EP_INFO));
	if (pNewInfo == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNewInfo, pInfo, sizeof(FTM_EP_INFO));

	FTM_LIST_append(&pConfig->xList, pNewInfo);

	return	FTM_RET_OK;
}

FTM_RET FTDM_CFG_EP_INFO_count(FTDM_CFG_EP_PTR pConfig, FTM_ULONG_PTR pCount)
{
	ASSERT((pConfig != NULL) && (pCount != NULL));

	return	FTM_LIST_count(&pConfig->xList, pCount);
}

FTM_RET	FTDM_CFG_EP_INFO_getAt(FTDM_CFG_EP_PTR pConfig, FTM_ULONG ulIndex, FTM_EP_INFO_PTR pInfo)
{
	ASSERT(pConfig != NULL);
	ASSERT(pInfo != NULL);

	FTM_EP_INFO_PTR	pEPInfo;


	if (FTM_LIST_getAt(&pConfig->xList, ulIndex, (FTM_VOID_PTR _PTR_)&pEPInfo) != FTM_RET_OK)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	memcpy(pInfo, pEPInfo, sizeof(FTM_EP_INFO));

	return	FTM_RET_OK;
}

/************************************************************************
 * EndPoint Class Configuraton
 ************************************************************************/
FTM_RET FTDM_CFG_EP_CLASS_INFO_append(FTDM_CFG_EP_PTR pConfig, FTM_EP_CLASS_INFO_PTR pInfo)
{
	FTM_EP_CLASS_INFO_PTR	pNewInfo;	
	
	ASSERT((pConfig != NULL) && (pInfo != NULL));

	pNewInfo = (FTM_EP_CLASS_INFO_PTR)calloc(1, sizeof(FTM_EP_CLASS_INFO));
	if (pNewInfo == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNewInfo, pInfo, sizeof(FTM_EP_CLASS_INFO));

	FTM_LIST_append(&pConfig->xClassList, pNewInfo);

	return	FTM_RET_OK;
}

FTM_RET FTDM_CFG_EP_CLASS_INFO_count(FTDM_CFG_EP_PTR pConfig, FTM_ULONG_PTR pCount)
{
	ASSERT((pConfig != NULL) && (pCount != NULL));

	FTM_LIST_count(&pConfig->xClassList, pCount);

	return	FTM_RET_OK;
}

FTM_RET FTDM_CFG_EP_CLASS_INFO_get(FTDM_CFG_EP_PTR pConfig, FTM_EP_CLASS xClass, FTM_EP_CLASS_INFO_PTR pInfo)
{
	ASSERT((pConfig != NULL) && (pInfo != NULL));

	return	FTM_RET_OK;
}

FTM_RET FTDM_CFG_EP_CLASS_INFO_getAt(FTDM_CFG_EP_PTR pConfig, FTM_ULONG ulIndex, FTM_EP_CLASS_INFO_PTR pInfo)
{
	FTM_RET					xRet;
	FTM_EP_CLASS_INFO_PTR	pEPClassInfo;

	ASSERT((pConfig != NULL) && (pInfo != NULL));

	xRet = FTM_LIST_getAt(&pConfig->xClassList, ulIndex, (FTM_VOID_PTR _PTR_)&pEPClassInfo);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	memcpy(pInfo, pEPClassInfo, sizeof(FTM_EP_CLASS_INFO));

	return	FTM_RET_OK;
}

/************************************************************************
 * Internal functions
 ************************************************************************/
FTM_RET	CFG_DBFileNameGet(config_t *pConfig, FTM_CHAR_PTR pBuff, FTM_ULONG ulLen)
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

FTM_RET	CFG_serverInfoGet(config_t *pConfig, FTM_SERVER_INFO_PTR pInfo)
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

FTM_RET	CFG_NODE_INFO_count(config_t *pConfig, FTM_ULONG_PTR pCount)
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

FTM_RET	CFG_NODE_INFO_getAt(config_t *pConfig, FTM_ULONG ulIndex,  FTM_NODE_INFO_PTR pInfo)
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

		pField = config_setting_get_member(pNode, FTDM_FIELD_NODE_TIMEOUT_STRING);	
		if(pField != NULL)
		{
			xNodeInfo.ulTimeout = (FTM_ULONG)config_setting_get_int(pField);
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
						xNodeInfo.xOption.xSNMP.ulVersion = (FTM_ULONG)config_setting_get_int(pField);
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

					pField = config_setting_get_member(pSNMP, FTDM_FIELD_SNMP_MIB_STRING);
					if (pField != NULL)
					{
						strncpy(xNodeInfo.xOption.xSNMP.pMIB, config_setting_get_string(pField), FTM_SNMP_MIB_LEN);
					}

				}
			}
			break;
		}

		memcpy(pInfo, &xNodeInfo, sizeof(xNodeInfo));
	}

	return	FTM_RET_OK;
}

int	CFG_NODE_INFO_seeker(const void *pItem, const void *pKey)
{
	FTM_NODE_INFO_PTR	pInfo = (FTM_NODE_INFO_PTR)pItem;
	FTM_CHAR_PTR		pDID = (FTM_CHAR_PTR)pKey;

	return (strcmp(pInfo->pDID, pDID) == 0);
}

/****************************************************************************
 *
 ****************************************************************************/

FTM_RET	CFG_EP_INFO_count(config_t *pConfig, FTM_ULONG_PTR pCount)
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

FTM_RET	CFG_EP_INFO_getAt(config_t *pConfig, FTM_ULONG ulIndex, FTM_EP_INFO_PTR pInfo)
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

		pField = config_setting_get_member(pEP, FTDM_FIELD_EP_ENABLE_STRING);	
		if(pField != NULL)
		{
			if (config_setting_get_int(pField) == 0)
			{
				xEPInfo.xState = FTM_EP_STATE_DISABLE;	
			}
			else
			{
				xEPInfo.xState = FTM_EP_STATE_RUN;	
			}
		}

		pField = config_setting_get_member(pEP, FTDM_FIELD_EP_INTERVAL_STRING);	
		if(pField != NULL)
		{
			xEPInfo.ulInterval = (FTM_ULONG)config_setting_get_int(pField);
		}

		pField = config_setting_get_member(pEP, FTDM_FIELD_EP_TIMEOUT_STRING);	
		if(pField != NULL)
		{
			xEPInfo.ulTimeout = (FTM_ULONG)config_setting_get_int(pField);
		}

		pField = config_setting_get_member(pEP, FTDM_FIELD_EP_DID_STRING);	
		if(pField != NULL)
		{
			strncpy(xEPInfo.pDID, config_setting_get_string(pField), FTM_DID_LEN);
		}

		pField = config_setting_get_member(pEP, FTDM_FIELD_EP_DEPID_STRING);	
		if(pField != NULL)
		{
			xEPInfo.xDEPID = (FTM_ULONG)config_setting_get_int(pField);
		}

		pField = config_setting_get_member(pEP, FTDM_FIELD_EP_PID_STRING);	
		if(pField != NULL)
		{
			strncpy(xEPInfo.pPID, config_setting_get_string(pField), FTM_DID_LEN);
		}

		pField = config_setting_get_member(pEP, FTDM_FIELD_EP_PEPID_STRING);	
		if(pField != NULL)
		{
			xEPInfo.xPEPID = (FTM_ULONG)config_setting_get_int(pField);
		}

		memcpy(pInfo, &xEPInfo, sizeof(xEPInfo));
	}

	return	FTM_RET_OK;
}

FTM_RET	CFG_EP_CLASS_INFO_count(config_t *pConfig, FTM_ULONG_PTR pCount)
{
	config_setting_t *pSection;
	config_setting_t *pField;
	
	ASSERT(pConfig != NULL);

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
	
	TRACE("pField = %08lx\n", pField);
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

FTM_RET	CFG_EP_CLASS_INFO_get(config_t *pConfig, FTM_EP_CLASS xClass, FTM_EP_CLASS_INFO_PTR pInfo)
{
	config_setting_t *pSection;
	config_setting_t *pItemArray;
	config_setting_t *pItem;
	config_setting_t *pField;
	
	ASSERT((pConfig != NULL) && (pInfo != NULL));

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

FTM_RET	CFG_EP_CLASS_INFO_getAt(config_t *pConfig, FTM_ULONG ulIndex, FTM_EP_CLASS_INFO_PTR pInfo)
{
	config_setting_t *pSection;
	config_setting_t *pItemArray;
	config_setting_t *pItem;
	config_setting_t *pField;
	
	ASSERT((pConfig != NULL) && (pInfo!= NULL));

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

int	CFG_EP_INFO_seeker(const void *pItem, const void *pKey)
{
	FTM_EP_INFO_PTR	pInfo = (FTM_EP_INFO_PTR)pItem;
	FTM_EPID_PTR	pEPID = (FTM_EPID_PTR)pKey;

	return	(pInfo->xEPID == *pEPID);
}

int	CFG_EP_CLASS_INFO_seeker(const void *pItem, const void *pKey)
{
	FTM_EP_CLASS_INFO_PTR	pInfo = (FTM_EP_CLASS_INFO_PTR)pItem;
	FTM_EP_CLASS_PTR		pClass= (FTM_EP_CLASS_PTR)pKey;

	return	(pInfo->xClass == *pClass);
}

FTM_CHAR_PTR	CFG_nodeTypeString(FTM_ULONG ulType)
{
	switch(ulType)
	{
	case	FTM_NODE_TYPE_SNMP:	return	"SNMP";
	case	FTM_NODE_TYPE_MODBUS_OVER_TCP: return	"MODBUS/TCP";
	case	FTM_NODE_TYPE_MODBUS_OVER_SERIAL: return	"MODBUS/SERIAL";
	}

	return	"UNKNOWN";
}

FTM_CHAR_PTR	CFG_EPTypeString(FTM_ULONG ulType)
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
	case	FTM_EP_CLASS_AI:			return	"ANALOG INPUT";
	case	FTM_EP_CLASS_MULTI:			return	"MULTI-FUNCTION";
	}

	sprintf(pBuff, "%08lx", ulType);
	return	pBuff;
}

FTM_CHAR_PTR	CFG_SNMPVersionString(FTM_ULONG ulVersion)
{
	switch(ulVersion)
	{
	case	0:	return	"SNMPv1";
	case	1: return	"SNMPv2c";
	case	2: return	"SNMPv3";
	}

	return	"SNMP";
}

