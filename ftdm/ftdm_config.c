#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ftm.h"
#include "ftdm_config.h"

static int		FTDM_CFG_EP_seeker(const void *pItem, const void *pKey);
static int		FTDM_CFG_EP_CLASS_seeker(const void *pItem, const void *pKey);

extern char *program_invocation_short_name;


FTM_RET FTDM_CFG_init(FTDM_CFG_PTR pConfig)
{
	ASSERT(pConfig != NULL);

	memset(pConfig, 0, sizeof(FTDM_CFG));

	FTM_LIST_init(&pConfig->xEP.xList);
	FTM_LIST_setSeeker(&pConfig->xEP.xList, FTDM_CFG_EP_seeker);

	FTM_LIST_init(&pConfig->xEP.xTypeList);
	FTM_LIST_setSeeker(&pConfig->xEP.xTypeList, FTDM_CFG_EP_CLASS_seeker);
	return	FTM_RET_OK;
}

FTM_RET FTDM_CFG_final(FTDM_CFG_PTR pConfig)
{
	FTM_EP_PTR			pEPInfo;
	FTM_EP_CLASS_PTR	pEPClassInfo;

	ASSERT(pConfig != NULL);
	
	FTM_LIST_iteratorStart(&pConfig->xEP.xList);
	while( FTM_LIST_iteratorNext(&pConfig->xEP.xList, (FTM_VOID_PTR _PTR_)&pEPInfo) == FTM_RET_OK)
	{
		FTM_MEM_free(pEPInfo);	
	}
	FTM_LIST_final(&pConfig->xEP.xList);

	FTM_LIST_iteratorStart(&pConfig->xEP.xTypeList);
	while( FTM_LIST_iteratorNext(&pConfig->xEP.xTypeList, (FTM_VOID_PTR _PTR_)&pEPClassInfo) == FTM_RET_OK)
	{
		FTM_MEM_free(pEPClassInfo);	
	}
	FTM_LIST_final(&pConfig->xEP.xTypeList);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_CFG_readFromFile(FTDM_CFG_PTR pConfig, FTM_CHAR_PTR pFileName)
{
	FTM_RET			xRet;
	FTM_CONFIG_PTR	pRoot;
	FTM_CONFIG_ITEM	xServer;
	FTM_CONFIG_ITEM	xDB;
	FTM_CONFIG_ITEM	xDebug;
	FTM_CONFIG_ITEM	xEPSection;

	ASSERT(pConfig != NULL);
	ASSERT(pFileName != NULL);

	xRet = FTM_CONFIG_create(pFileName, &pRoot, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	xRet = FTM_CONFIG_getItem(pRoot, "database", &xDB);
	if (xRet == FTM_RET_OK)
	{
		FTM_CHAR	pDBFileName[FTM_FILE_NAME_LEN+1];

		xRet = FTM_CONFIG_ITEM_getItemString(&xDB, "file", pDBFileName, sizeof(pDBFileName));
		if (xRet == FTM_RET_OK)
		{
			strcpy(pConfig->xDB.pFileName, pDBFileName);
		}
	}

	xRet = FTM_CONFIG_getItem(pRoot, "server", &xServer);
	if (xRet == FTM_RET_OK)
	{
		FTM_USHORT	usPort;
		FTM_ULONG	ulSession;

		xRet = FTM_CONFIG_ITEM_getItemUSHORT(&xServer, "port", &usPort);
		if (xRet == FTM_RET_OK)
		{
			pConfig->xServer.usPort = usPort;
		}

		xRet = FTM_CONFIG_ITEM_getItemULONG(&xServer, "session_count", &ulSession);
		if (xRet == FTM_RET_OK)
		{
			pConfig->xServer.ulMaxSession = ulSession;
		}
	}

	xRet = FTM_CONFIG_getItem(pRoot, "ep", &xEPSection);
	if (xRet == FTM_RET_OK)
	{
		FTM_CONFIG_ITEM	xTypeItemList;

		xRet = FTM_CONFIG_ITEM_getChildItem(&xEPSection, "types", &xTypeItemList);
		if (xRet == FTM_RET_OK)
		{
			FTM_ULONG		ulItemCount;

			xRet = FTM_CONFIG_LIST_getItemCount(&xTypeItemList, &ulItemCount);	
			if (xRet == FTM_RET_OK)
			{
				FTM_ULONG		i;
				FTM_CONFIG_ITEM	xTypeItem;

				for(i = 0 ; i < ulItemCount ; i++)
				{
					xRet = FTM_CONFIG_LIST_getItemAt(&xTypeItemList, i, &xTypeItem);
					if (xRet == FTM_RET_OK)
					{
						FTM_EP_CLASS	xEPClass;
						xRet = FTM_CONFIG_ITEM_getEPClass(&xTypeItem, &xEPClass);
						if (xRet != FTM_RET_OK)
						{
							continue;
						}

						xRet = FTDM_CFG_EP_CLASS_append(&pConfig->xEP, &xEPClass);
						if (xRet != FTM_RET_OK)
						{
							ERROR("Cant not append EP Type[%08x]\n", xEPClass.xType);
						}
					}
				}
			}
		}
	}

	xRet = FTM_CONFIG_getItem(pRoot, "debug", &xDebug);
	if (xRet == FTM_RET_OK)
	{
		FTM_ULONG	ulLevel;

		xRet = FTM_CONFIG_ITEM_getItemULONG(&xServer, "level", &ulLevel);
		if (xRet == FTM_RET_OK)
		{
			pConfig->xPrint.ulLevel = ulLevel;
		}
	}

	FTM_CONFIG_destroy(&pRoot);

	return	FTM_RET_OK;
}

FTM_RET FTDM_CFG_show(FTDM_CFG_PTR pConfig)
{
	FTM_ULONG	i, ulCount;

	ASSERT(pConfig != NULL);

	MESSAGE("###################### FTDM Configuration #####################\n");
	MESSAGE("\n[ DB ]\n");
	MESSAGE("\t %-8s : %s\n", "DB FILE", pConfig->xDB.pFileName);


	MESSAGE("\n#CLASS INFORMATION\n");
	MESSAGE("\t%8s %16s %16s %16s %16s %16s %16s\n", "CLASS", "ID", "NAME", "S/N", "STATE", "VALUE", "TIME");
	if (FTDM_CFG_EP_CLASS_count(&pConfig->xEP, &ulCount) == FTM_RET_OK)
	{
		for(i = 0 ; i < ulCount ; i++)
		{
			FTM_EP_CLASS	xEPClassInfo;

			FTDM_CFG_EP_CLASS_getAt(&pConfig->xEP, i, &xEPClassInfo);
			MESSAGE("\t%08lx %-16s %-16s %-16s %-16s %-16s %-16s\n",
				xEPClassInfo.xType, 
				xEPClassInfo.pID,
				xEPClassInfo.pName,
				xEPClassInfo.pSN,
				xEPClassInfo.pState,
				xEPClassInfo.pValue,
				xEPClassInfo.pTime);
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

	if (strlen(pFileName) > FTM_FILE_NAME_LEN)
	{
		return	FTM_RET_INVALID_ARGUMENTS;
	}	
	strcpy(pConfig->xDB.pFileName, pFileName);

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
FTM_RET	FTDM_CFG_NODE_append(FTDM_CFG_NODE_PTR pConfig, FTM_NODE_PTR pInfo)
{
	FTM_NODE_PTR	pNewInfo;

	ASSERT(pConfig != NULL);
	ASSERT(pInfo != NULL);

	pNewInfo = (FTM_NODE_PTR)FTM_MEM_malloc(sizeof(FTM_NODE));
	if (pNewInfo == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNewInfo, pInfo, sizeof(FTM_NODE));

	FTM_LIST_append(&pConfig->xList, pNewInfo);

	return	FTM_RET_OK;
}

FTM_RET FTDM_CFG_NODE_count(FTDM_CFG_NODE_PTR pConfig, FTM_ULONG_PTR pCount)
{
	ASSERT(pConfig != NULL);
	ASSERT(pCount != NULL);

	return	FTM_LIST_count(&pConfig->xList, pCount);
}

FTM_RET	FTDM_CFG_NODE_getAt(FTDM_CFG_NODE_PTR pConfig, FTM_ULONG ulIndex, FTM_NODE_PTR pInfo)
{
	FTM_RET				xRet;
	FTM_NODE_PTR	pNodeInfo;

	ASSERT((pConfig != NULL) && (pInfo != NULL));

	xRet = FTM_LIST_getAt(&pConfig->xList, ulIndex, (FTM_VOID_PTR _PTR_)&pNodeInfo);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;
	}

	memcpy(pInfo, pNodeInfo, sizeof(FTM_NODE));

	return	FTM_RET_OK;
}

/************************************************************************
 * EndPoint Configuraton
 ************************************************************************/
FTM_RET	FTDM_CFG_EP_append(FTDM_CFG_EP_PTR pConfig, FTM_EP_PTR pInfo)
{
	FTM_EP_PTR	pNewInfo;

	ASSERT((pConfig != NULL) && (pInfo != NULL));

	pNewInfo = (FTM_EP_PTR)FTM_MEM_malloc(sizeof(FTM_EP));
	if (pNewInfo == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNewInfo, pInfo, sizeof(FTM_EP));

	FTM_LIST_append(&pConfig->xList, pNewInfo);

	return	FTM_RET_OK;
}

FTM_RET FTDM_CFG_EP_count(FTDM_CFG_EP_PTR pConfig, FTM_ULONG_PTR pCount)
{
	ASSERT((pConfig != NULL) && (pCount != NULL));

	return	FTM_LIST_count(&pConfig->xList, pCount);
}

FTM_RET	FTDM_CFG_EP_getAt(FTDM_CFG_EP_PTR pConfig, FTM_ULONG ulIndex, FTM_EP_PTR pInfo)
{
	ASSERT(pConfig != NULL);
	ASSERT(pInfo != NULL);

	FTM_EP_PTR	pEPInfo;


	if (FTM_LIST_getAt(&pConfig->xList, ulIndex, (FTM_VOID_PTR _PTR_)&pEPInfo) != FTM_RET_OK)
	{
		return	FTM_RET_OBJECT_NOT_FOUND;	
	}

	memcpy(pInfo, pEPInfo, sizeof(FTM_EP));

	return	FTM_RET_OK;
}

/************************************************************************
 * EndPoint Class Configuraton
 ************************************************************************/
FTM_RET FTDM_CFG_EP_CLASS_append(FTDM_CFG_EP_PTR pConfig, FTM_EP_CLASS_PTR pInfo)
{
	FTM_EP_CLASS_PTR	pNewInfo;	
	
	ASSERT((pConfig != NULL) && (pInfo != NULL));

	pNewInfo = (FTM_EP_CLASS_PTR)FTM_MEM_malloc(sizeof(FTM_EP_CLASS));
	if (pNewInfo == NULL)
	{
		return	FTM_RET_NOT_ENOUGH_MEMORY;	
	}

	memcpy(pNewInfo, pInfo, sizeof(FTM_EP_CLASS));

	FTM_LIST_append(&pConfig->xTypeList, pNewInfo);

	return	FTM_RET_OK;
}

FTM_RET FTDM_CFG_EP_CLASS_count(FTDM_CFG_EP_PTR pConfig, FTM_ULONG_PTR pCount)
{
	ASSERT((pConfig != NULL) && (pCount != NULL));

	FTM_LIST_count(&pConfig->xTypeList, pCount);

	return	FTM_RET_OK;
}

FTM_RET FTDM_CFG_EP_CLASS_get(FTDM_CFG_EP_PTR pConfig, FTM_EP_TYPE xType, FTM_EP_CLASS_PTR pInfo)
{
	ASSERT((pConfig != NULL) && (pInfo != NULL));

	return	FTM_RET_OK;
}

FTM_RET FTDM_CFG_EP_CLASS_getAt(FTDM_CFG_EP_PTR pConfig, FTM_ULONG ulIndex, FTM_EP_CLASS_PTR pInfo)
{
	FTM_RET					xRet;
	FTM_EP_CLASS_PTR	pEPClassInfo;

	ASSERT((pConfig != NULL) && (pInfo != NULL));

	xRet = FTM_LIST_getAt(&pConfig->xTypeList, ulIndex, (FTM_VOID_PTR _PTR_)&pEPClassInfo);
	if (xRet != FTM_RET_OK)
	{
		return	xRet;	
	}

	memcpy(pInfo, pEPClassInfo, sizeof(FTM_EP_CLASS));

	return	FTM_RET_OK;
}

/****************************************************************************
 *
 ****************************************************************************/
FTM_BOOL	FTDM_CFG_EP_seeker
(
	const FTM_VOID_PTR pItem, 
	const FTM_VOID_PTR pKey
)
{
	FTM_EP_PTR		pInfo = (FTM_EP_PTR)pItem;
	FTM_CHAR_PTR	pEPID = (FTM_CHAR_PTR)pKey;

	return	strncpy(pInfo->pEPID, pEPID, FTM_EPID_LEN) == 0;
}

FTM_BOOL	FTDM_CFG_EP_CLASS_seeker
(
	const FTM_VOID_PTR pItem, 
	const FTM_VOID_PTR pKey
)
{
	FTM_EP_CLASS_PTR	pInfo = (FTM_EP_CLASS_PTR)pItem;
	FTM_EP_TYPE_PTR		pClass= (FTM_EP_TYPE_PTR)pKey;

	return	(pInfo->xType == *pClass);
}

FTM_CHAR_PTR	FTDM_CFG_SNMP_getVersionString
(
	FTM_ULONG ulVersion
)
{
	switch(ulVersion)
	{
	case	0:	return	"SNMPv1";
	case	1: return	"SNMPv2c";
	case	2: return	"SNMPv3";
	}

	return	"SNMP";
}

