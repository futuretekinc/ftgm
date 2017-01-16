#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include "libconfig.h"
#include "ftm.h"
#include "ftdm.h"
#include "ftdm_config.h"
#include "ftdm_ep.h"
#include "ftdm_ep_class.h"
#include "ftdm_node.h"
#include "ftdm_params.h"
#include "ftdm_server_cmds.h"
#include "ftdm_server.h"
#include "ftdm_sqlite.h"
#include "ftdm_trigger.h"
#include "ftdm_action.h"
#include "ftdm_rule.h"
#include "ftdm.h"
#include "ftm_log.h"
#include "ftdm_log.h"

typedef struct FTDM_CONTEXT_STRUCT
{
	FTDM_DBIF_PTR		pDBIF;
	FTM_LIST_PTR		pNodeList;
	FTM_LIST_PTR		pEPList;
	FTM_LIST_PTR		pTriggerList;
	FTM_LIST_PTR		pActionList;
	FTM_LIST_PTR		pRuleList;
	FTDM_LOGGER_PTR		pLogger;
	FTDM_SIS_PTR		pSIS;
} FTDM_CONTEXT;

FTM_RET	FTDM_create
(
	FTDM_CONTEXT_PTR _PTR_	ppFTDM
)
{
	ASSERT(ppFTDM != NULL);
	FTM_RET		xRet;
	FTDM_CONTEXT_PTR	pFTDM;
	
	pFTDM = (FTDM_CONTEXT_PTR)FTM_MEM_malloc(sizeof(FTDM_CONTEXT));
	if (pFTDM == NULL)
	{
		xRet = FTM_RET_NOT_ENOUGH_MEMORY;
		ERROR2(xRet, "Failed to create FTDM!\n");
		goto error;
	}

	xRet = FTDM_DBIF_create(&pFTDM->pDBIF);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create DB interface!\n");
		goto error;
	}

	xRet = FTM_LIST_create(&pFTDM->pNodeList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create node list!\n");	
		goto error;
	}

	FTM_LIST_setSeeker(pFTDM->pNodeList, FTDM_NODE_seeker);
	FTM_LIST_setComparator(pFTDM->pNodeList, FTDM_NODE_comparator);

	xRet = FTM_LIST_create(&pFTDM->pEPList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create ep list!\n");	
		goto error;
	}

	FTM_LIST_setSeeker(pFTDM->pEPList, FTDM_EP_seeker);
	FTM_LIST_setComparator(pFTDM->pEPList, FTDM_EP_comparator);

	xRet = FTM_LIST_create(&pFTDM->pTriggerList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create trigger list!\n");	
		goto error;
	}

	FTM_LIST_setSeeker(pFTDM->pTriggerList, FTDM_TRIGGER_seeker);
	FTM_LIST_setComparator(pFTDM->pTriggerList, FTDM_TRIGGER_comparator);

	xRet = FTM_LIST_create(&pFTDM->pActionList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create action list!\n");	
		goto error;
	}

	FTM_LIST_setSeeker(pFTDM->pActionList, FTDM_ACTION_seeker);
	FTM_LIST_setComparator(pFTDM->pActionList, FTDM_ACTION_comparator);

	xRet = FTM_LIST_create(&pFTDM->pRuleList);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create rule list!\n");	
		goto error;
	}

	FTM_LIST_setSeeker(pFTDM->pRuleList, FTDM_RULE_seeker);
	FTM_LIST_setComparator(pFTDM->pRuleList, FTDM_RULE_comparator);

	xRet = FTDM_LOGGER_create(pFTDM, &pFTDM->pLogger);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create logger!\n");
		goto error;
	}

	xRet = FTDM_SIS_create(pFTDM, &pFTDM->pSIS);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create serice interface server.\n");	
		goto error;
	}

	*ppFTDM = pFTDM;

	return	FTM_RET_OK;

error:
	if (pFTDM != NULL)
	{
		if (pFTDM->pDBIF != NULL)
		{
			FTDM_DBIF_destroy(&pFTDM->pDBIF);
		}

		if (pFTDM->pNodeList != NULL)
		{
			FTM_LIST_destroy(pFTDM->pNodeList);
		}

		if (pFTDM->pEPList != NULL)
		{
			FTM_LIST_destroy(pFTDM->pEPList);
		}

		if (pFTDM->pTriggerList != NULL)
		{
			FTM_LIST_destroy(pFTDM->pTriggerList);
		}

		if (pFTDM->pActionList != NULL)
		{
			FTM_LIST_destroy(pFTDM->pActionList);
		}

		if (pFTDM->pRuleList != NULL)
		{
			FTM_LIST_destroy(pFTDM->pRuleList);
		}

		if (pFTDM->pLogger != NULL)
		{
			FTDM_LOGGER_destroy(&pFTDM->pLogger);	
		}

		if (pFTDM->pSIS != NULL)
		{
			FTDM_SIS_destroy(&pFTDM->pSIS);	
		}

		FTM_MEM_free(pFTDM);	
	}
	return	xRet;
}

FTM_RET	FTDM_destroy
(
	FTDM_CONTEXT_PTR _PTR_ ppFTDM
)
{
	ASSERT(ppFTDM != NULL);

	FTM_RET	xRet;
	FTDM_CONTEXT_PTR	pFTDM = *ppFTDM;

	if (pFTDM->pSIS != NULL)
	{
		xRet = FTDM_SIS_destroy(&pFTDM->pSIS);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to service interface server!\n");	
		}
	}

	if (pFTDM->pDBIF != NULL)
	{
		xRet = FTDM_DBIF_destroy(&(pFTDM)->pDBIF);	
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to destroy DB interface.\n");	
		}
	}

	if ((pFTDM)->pLogger != NULL)
	{
		xRet = FTDM_LOGGER_destroy(&(pFTDM)->pLogger);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to destroy logger.\n");	
		}
	}

	if (pFTDM->pRuleList != NULL)
	{
		FTDM_RULE_PTR	pRule;

		FTM_LIST_iteratorStart(pFTDM->pRuleList);
		while(FTM_LIST_iteratorNext(pFTDM->pRuleList, (FTM_VOID_PTR _PTR_)&pRule) == FTM_RET_OK)
		{
			xRet = FTDM_RULE_destroy(&pRule);	
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to destroy rule!\n");	
			}
		}

		FTM_LIST_destroy(pFTDM->pRuleList);
		pFTDM->pRuleList = NULL;
	}

	if (pFTDM->pActionList != NULL)
	{
		FTDM_ACTION_PTR	pAction;

		FTM_LIST_iteratorStart(pFTDM->pActionList);
		while(FTM_LIST_iteratorNext(pFTDM->pActionList, (FTM_VOID_PTR _PTR_)&pAction) == FTM_RET_OK)
		{
			xRet = FTDM_ACTION_destroy(&pAction);	
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to destroy rule!\n");	
			}
		}

		FTM_LIST_destroy(pFTDM->pActionList);
		pFTDM->pActionList = NULL;
	}

	if (pFTDM->pTriggerList != NULL)
	{
		FTDM_TRIGGER_PTR	pTrigger;

		FTM_LIST_iteratorStart(pFTDM->pTriggerList);
		while(FTM_LIST_iteratorNext(pFTDM->pTriggerList, (FTM_VOID_PTR _PTR_)&pTrigger) == FTM_RET_OK)
		{
			xRet = FTDM_TRIGGER_destroy(&pTrigger);	
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to destroy rule!\n");	
			}
		}

		FTM_LIST_destroy(pFTDM->pTriggerList);
		pFTDM->pTriggerList = NULL;
	}

	if (pFTDM->pEPList != NULL)
	{
		FTDM_EP_PTR	pEP;

		FTM_LIST_iteratorStart(pFTDM->pEPList);
		while(FTM_LIST_iteratorNext(pFTDM->pEPList, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
		{
			xRet = FTDM_EP_destroy(&pEP);	
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to destroy rule!\n");	
			}
		}

		FTM_LIST_destroy(pFTDM->pEPList);
		pFTDM->pEPList = NULL;
	}

	if (pFTDM->pNodeList != NULL)
	{
		FTDM_NODE_PTR	pNode;

		FTM_LIST_iteratorStart(pFTDM->pNodeList);
		while(FTM_LIST_iteratorNext(pFTDM->pNodeList, (FTM_VOID_PTR _PTR_)&pNode) == FTM_RET_OK)
		{
			xRet = FTDM_NODE_destroy(&pNode, FTM_FALSE);	
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to destroy rule!\n");	
			}
		}

		FTM_LIST_destroy(pFTDM->pNodeList);
		pFTDM->pNodeList = NULL;
	}

	FTM_MEM_free(pFTDM);

	*ppFTDM = NULL;

	return	FTM_RET_OK;
}

FTM_RET 	FTDM_init
(
	FTDM_CONTEXT_PTR pFTDM
)
{
	ASSERT(pFTDM != NULL);

	FTM_RET			xRet;
	FTDM_NODE_PTR	pNode;
	FTDM_EP_PTR		pEP;
	FTDM_TRIGGER_PTR	pTrigger;
	FTDM_ACTION_PTR		pAction;
	FTDM_RULE_PTR		pRule;
	
	FTM_LIST_iteratorStart(pFTDM->pNodeList);
	while(FTM_LIST_iteratorNext(pFTDM->pNodeList, (FTM_VOID_PTR _PTR_)&pNode) == FTM_RET_OK)
	{
		xRet = FTDM_NODE_init(pNode);	
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to initialize node!\n");	
		}
	}

	FTM_LIST_iteratorStart(pFTDM->pEPList);
	while(FTM_LIST_iteratorNext(pFTDM->pEPList, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
	{
		xRet = FTDM_EP_init(pEP);	
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to initialize node!\n");	
		}
	}

	FTM_LIST_iteratorStart(pFTDM->pRuleList);
	while(FTM_LIST_iteratorNext(pFTDM->pTriggerList, (FTM_VOID_PTR _PTR_)&pTrigger) == FTM_RET_OK)
	{
		xRet = FTDM_TRIGGER_init(pTrigger);	
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to initialize trigger!\n");	
		}
	}

	while(FTM_LIST_iteratorNext(pFTDM->pActionList, (FTM_VOID_PTR _PTR_)&pAction) == FTM_RET_OK)
	{
		xRet = FTDM_ACTION_init(pAction);	
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to initialize action!\n");	
		}
	}

	FTM_LIST_iteratorStart(pFTDM->pTriggerList);
	while(FTM_LIST_iteratorNext(pFTDM->pRuleList, (FTM_VOID_PTR _PTR_)&pRule) == FTM_RET_OK)
	{
		xRet = FTDM_RULE_init(pRule);	
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to initialize rule!\n");	
		}
	}

	FTM_LIST_iteratorStart(pFTDM->pActionList);
	xRet = FTDM_DBIF_init(pFTDM->pDBIF);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to initialize DB interface.\n");
		return	FTM_RET_OK;
	}

	xRet = FTDM_LOGGER_init(pFTDM->pLogger);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to initialize logger.\n");	
	}

	xRet = FTDM_EP_CLASS_init();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "EP type management initialization failed.\n" );	
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_final
(	
	FTDM_CONTEXT_PTR pFTDM
)
{
	ASSERT(pFTDM != NULL);

	FTM_RET	xRet;
	FTDM_NODE_PTR	pNode;
	FTDM_EP_PTR		pEP;
	FTDM_TRIGGER_PTR	pTrigger;
	FTDM_ACTION_PTR		pAction;
	FTDM_RULE_PTR		pRule;

	xRet = FTDM_SIS_final(pFTDM->pSIS);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Server finalization failed.\n");	
	}

	xRet = FTDM_EP_CLASS_final();
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "EP Class management finalization failed.\n");	
	}
#if 0
	xRet = FTDM_LOGGER_final(pFTDM->pLogger);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to destroy logger.\n");	
	}
#endif

	FTM_LIST_iteratorStart(pFTDM->pRuleList);
	while(FTM_LIST_iteratorNext(pFTDM->pRuleList, (FTM_VOID_PTR _PTR_)&pRule) == FTM_RET_OK)
	{
		xRet = FTDM_RULE_final(pRule);	
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to finalize rule!\n");	
		}
	}

	FTM_LIST_iteratorStart(pFTDM->pActionList);
	while(FTM_LIST_iteratorNext(pFTDM->pActionList, (FTM_VOID_PTR _PTR_)&pAction) == FTM_RET_OK)
	{
		xRet = FTDM_ACTION_final(pAction);	
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to finalize action!\n");	
		}
	}

	FTM_LIST_iteratorStart(pFTDM->pTriggerList);
	while(FTM_LIST_iteratorNext(pFTDM->pTriggerList, (FTM_VOID_PTR _PTR_)&pTrigger) == FTM_RET_OK)
	{
		xRet = FTDM_TRIGGER_final(pTrigger);	
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to finalize trigger!\n");	
		}
	}

	FTM_LIST_iteratorStart(pFTDM->pEPList);
	while(FTM_LIST_iteratorNext(pFTDM->pEPList, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
	{
		xRet = FTDM_EP_final(pEP);	
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to finalize node!\n");	
		}
	}

	FTM_LIST_iteratorStart(pFTDM->pNodeList);
	while(FTM_LIST_iteratorNext(pFTDM->pNodeList, (FTM_VOID_PTR _PTR_)&pNode) == FTM_RET_OK)
	{
		xRet = FTDM_NODE_final(pNode);	
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to finalize node!\n");	
		}
	}

	xRet = FTDM_DBIF_final(pFTDM->pDBIF);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "FTDM finalization failed. [ %08lx ]\n", xRet);
		return	FTM_RET_OK;
	}

	TRACE("FTDM finalization completed successfully.\n");

	return	FTM_RET_OK;
}

FTM_RET 	FTDM_loadConfig
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CONFIG_PTR		pConfig
)
{
	FTM_RET	xRet;
	FTM_CONFIG_ITEM	xItem;

	xRet = FTM_CONFIG_getItem(pConfig, "dbif", &xItem);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_DBIF_loadConfig(pFTDM->pDBIF, &xItem);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to load DB interface configuration!.\n");
			return	xRet;
		}
	}

	xRet = FTM_CONFIG_getItem(pConfig, "ep_class", &xItem);
	if (xRet == FTM_RET_OK)
	{
		xRet = FTDM_EP_CLASS_loadConfig(&xItem);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to load EP class configuration!\n");	
		}
	}

	if (pFTDM->pSIS != NULL)
	{
		xRet = FTM_CONFIG_getItem(pConfig, "sis", &xItem);
		if (xRet != FTM_RET_OK)
		{
			xRet = FTDM_SIS_loadConfig(pFTDM->pSIS, &xItem);
			if (xRet != FTM_RET_OK)
			{
				ERROR2(xRet, "Failed to load service interface server configuration!\n");
			}
		}
	}

	TRACE("FTDM initialization completed successfully.\n");

	return	FTM_RET_OK;
}

FTM_RET 	FTDM_loadObjectFromConfig
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CONFIG_PTR		pConfig
)
{
	FTM_RET	xRet;
	FTM_CONFIG_ITEM	xItem;

	xRet = FTM_CONFIG_getItem(pConfig, "node", &xItem);
	if (xRet == FTM_RET_OK)
	{
		FTM_ULONG		ulItemCount;

		xRet = FTM_CONFIG_LIST_getItemCount(&xItem, &ulItemCount);	
		if (xRet == FTM_RET_OK)
		{
			FTM_ULONG		i;
			FTM_CONFIG_ITEM	xNodeItem;

			for(i = 0 ; i < ulItemCount ; i++)
			{
				xRet = FTM_CONFIG_LIST_getItemAt(&xItem, i, &xNodeItem);
				if (xRet == FTM_RET_OK)
				{
					FTM_NODE		xInfo;
					FTDM_NODE_PTR	pNode;

					xRet = FTM_CONFIG_ITEM_getNode(&xNodeItem, &xInfo);
					if (xRet != FTM_RET_OK)
					{
						ERROR2(xRet, "Failed to read node information.\n");
						continue;
					}

					xRet = FTDM_NODE_create(pFTDM, &xInfo, FTM_TRUE, &pNode);
					if (xRet == FTM_RET_OK)
					{
						FTM_LIST_append(pFTDM->pNodeList, pNode);
					}
					else
					{
						ERROR2(xRet, "Failed to create node[%s].\n", xInfo.pDID);	

					}
				}
			}
		}
		else
		{
			ERROR2(xRet, "Failed to get item count.\n");	
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_loadObjectFromDB
(	
	FTDM_CONTEXT_PTR	pFTDM
)
{
	FTM_RET	xRet;
	FTDM_DBIF_PTR	pDBIF;
	FTM_ULONG	ulMaxCount = 0;

	xRet = FTDM_getDBIF(pFTDM, &pDBIF);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get DB interface!\n");
		return	xRet;
	}

	xRet = FTDM_DBIF_getNodeCount(pDBIF, &ulMaxCount);
	if (xRet != FTM_RET_OK)
	{
		if (ulMaxCount > 0)
		{

			FTM_NODE_PTR	pInfos;
			FTM_ULONG		nNodeCount = 0;

			pInfos = (FTM_NODE_PTR)FTM_MEM_malloc(ulMaxCount * sizeof(FTM_NODE));
			if (pInfos == NULL)
			{
				return	FTM_RET_NOT_ENOUGH_MEMORY;	
			}

			xRet = FTDM_DBIF_getNodeList(pDBIF, pInfos, ulMaxCount, &nNodeCount);
			if (xRet  == FTM_RET_OK)
			{
				FTM_INT	i;

				for(i = 0 ; i < nNodeCount ; i++)
				{
					FTDM_NODE_PTR	pNode;

					xRet = FTDM_NODE_create(pFTDM, &pInfos[i], FTM_FALSE, &pNode);
					if (xRet == FTM_RET_OK)
					{
						xRet = FTM_LIST_append(pFTDM->pNodeList, pNode);
						if (xRet != FTM_RET_OK)
						{
							FTDM_NODE_destroy(&pNode, FTM_FALSE);	
						}
					}
				}
			}

			FTM_MEM_free(pInfos);
		}
	}


	xRet = FTDM_DBIF_getEPCount(pDBIF, &ulMaxCount);
	if (xRet == FTM_RET_OK)
	{
		if (ulMaxCount > 0)
		{

			FTM_EP_PTR	pInfos;
			FTM_ULONG	nEPCount = 0;

			pInfos = (FTM_EP_PTR)FTM_MEM_malloc(ulMaxCount * sizeof(FTM_EP));
			if (pInfos == NULL)
			{
				return	FTM_RET_NOT_ENOUGH_MEMORY;	
			}

			if (FTDM_DBIF_getEPList(pDBIF, pInfos, ulMaxCount, &nEPCount) == FTM_RET_OK)
			{
				FTM_INT	i;

				for(i = 0 ; i < nEPCount ; i++)
				{
					FTDM_EP_PTR	pEP;

					xRet = FTDM_createEP(pFTDM, &pInfos[i], &pEP);
					if (xRet == FTM_RET_OK)
					{
						xRet = FTM_LIST_append(pFTDM->pEPList, pEP);
						if (xRet != FTM_RET_OK)
						{
							WARN("EP append failed[%08x]\n", xRet);	
						}
					}
				}
			}

			FTM_MEM_free(pInfos);
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_saveObjectToDB
(
	FTDM_CONTEXT_PTR	pFTDM
)
{
	ASSERT(pFTDM != NULL);

	FTM_RET	xRet;
	FTM_ULONG		i;
	FTM_ULONG		ulCount;
	FTDM_DBIF_PTR	pDBIF;

	xRet = FTDM_getDBIF(pFTDM, &pDBIF);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get DB interface!\n");
		return	xRet;
	}

	xRet = FTM_LIST_count(pFTDM->pNodeList, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		FTDM_NODE_PTR	pNode;

		for(i = 0 ; i < ulCount ; i++)
		{
			xRet = FTM_LIST_getAt(pFTDM->pNodeList, i, (FTM_VOID_PTR _PTR_)&pNode);
			if (xRet == FTM_RET_OK)
			{
				FTM_NODE	xInfo1;
				FTM_NODE	xInfo2;

				xRet = FTDM_NODE_getInfo(pNode, &xInfo1);
				if (xRet == FTM_RET_OK)
				{
					xRet = FTDM_DBIF_getNode(pDBIF, xInfo1.pDID, &xInfo2);
					switch(xRet)
					{
					case	FTM_RET_OK:
						{
							FTM_BOOL	bChanged;
							xRet = FTDM_NODE_isInfoChanged(pNode, &xInfo2, &bChanged);
							if (xRet  == FTM_RET_OK)
							{
								if (bChanged)
								{
									xRet = FTDM_DBIF_setNode(pDBIF, xInfo1.pDID, &xInfo1);	
									if (xRet != FTM_RET_OK)
									{
										ERROR2(xRet, "Failed to set node on DB!\n");	
									}
								}
							}
							else
							{
								ERROR2(xRet, "Failed to check node info!\n");
							}
						}
						break;

					case	FTM_RET_OBJECT_NOT_FOUND:	
						{
							xRet = FTDM_DBIF_addNode(pDBIF, &xInfo1);	
							if (xRet != FTM_RET_OK)
							{
								ERROR2(xRet, "Failed to create node on DB!\n");	
							}
						}
						break;

					default:
						{
							ERROR2(xRet, "Failed to get node from DB!\n");	
						}
					}
				}
			}
			else
			{
				ERROR2(xRet, "Failed to get node information[%08x]\n", xRet);
			}
		}
	}
	else
	{
		ERROR2(xRet, "Failed to get node count.\n");
	}

	xRet = FTM_LIST_count(pFTDM->pEPList, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		FTDM_EP_PTR	pEP;

		for(i = 0 ; i < ulCount ; i++)
		{
			xRet = FTM_LIST_getAt(pFTDM->pEPList, i, (FTM_VOID_PTR _PTR_)&pEP);
			if (xRet == FTM_RET_OK)
			{
				FTM_EP	xInfo1;
				FTM_EP	xInfo2;

				xRet = FTDM_EP_getInfo(pEP, &xInfo1);
				if (xRet == FTM_RET_OK)
				{
					xRet = FTDM_DBIF_getEP(pDBIF, xInfo1.pDID, &xInfo2);
					switch(xRet)
					{
					case	FTM_RET_OK:
						{
							FTM_BOOL	bChanged;
							xRet = FTDM_EP_isInfoChanged(pEP, &xInfo2, &bChanged);
							if (xRet  == FTM_RET_OK)
							{
								if (bChanged)
								{
									xRet = FTDM_DBIF_setEP(pDBIF, xInfo1.pDID, &xInfo1);	
									if (xRet != FTM_RET_OK)
									{
										ERROR2(xRet, "Failed to set node on DB!\n");	
									}
								}
							}
							else
							{
								ERROR2(xRet, "Failed to check node info!\n");
							}
						}
						break;

					case	FTM_RET_OBJECT_NOT_FOUND:	
						{
							xRet = FTDM_DBIF_addEP(pDBIF, &xInfo1);	
							if (xRet != FTM_RET_OK)
							{
								ERROR2(xRet, "Failed to create node on DB!\n");	
							}
						}
						break;

					default:
						{
							ERROR2(xRet, "Failed to get node from DB!\n");	
						}
					}
				}
			}
			else
			{
				ERROR2(xRet, "Failed to get node information[%08x]\n", xRet);
			}
		}
	}
	else
	{
		ERROR2(xRet, "Failed to get node count.\n");
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_loadObjectFromFile
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR		pFileName
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pFileName != NULL);

	FTM_RET	xRet;
	FTM_CONFIG_PTR	pConfig;

	xRet = FTM_CONFIG_create(pFileName, &pConfig, FTM_FALSE);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create configuration[filename = %s]\n", pFileName);
		return	xRet;
	}

	xRet = FTDM_loadObjectFromConfig(pFTDM, pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed go load object from file[filename = %s]\n", pFileName);	
	}

	xRet = FTM_CONFIG_destroy(&pConfig);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to destroy config!\n");	
	}

	return	xRet;
}

FTM_RET	FTDM_start(FTM_VOID)
{
	return	FTM_RET_OK;
}

FTM_RET	FTDM_setDebugLevel
(
	FTM_ULONG		ulLevel
)
{
	
	FTM_TRACE_LEVEL_print(ulLevel, FTM_TRUE);

	return	FTM_RET_OK;
}

FTM_RET	FTDM_getSIS
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTDM_SIS_PTR _PTR_ ppSIS
)
{
	ASSERT(pFTDM != NULL);

	*ppSIS = pFTDM->pSIS;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_getDBIF
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTDM_DBIF_PTR _PTR_ ppDBIF
)
{
	ASSERT(pFTDM != NULL);

	*ppDBIF = pFTDM->pDBIF;

	return	FTM_RET_OK;
}

FTM_RET	FTDM_removeInvalidData
(
	FTDM_CONTEXT_PTR pFTDM
)
{
	ASSERT(pFTDM != NULL);
	FTM_RET		xRet;
	FTM_ULONG	i, ulCount;

	xRet = FTM_LIST_count(pFTDM->pEPList, &ulCount);
	for(i = 0 ; i < ulCount ; i++)
	{
		FTDM_EP_PTR	pEP;

		xRet = FTM_LIST_getAt(pFTDM->pEPList, i, (FTM_VOID_PTR _PTR_)&pEP);
		if (xRet == FTM_RET_OK)
		{
		
		}
	}
	return	FTM_RET_OK;
}


/******************************************************************
 *	Node
 ******************************************************************/
FTM_RET	FTDM_createNode
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_NODE_PTR		pInfo,
	FTDM_NODE_PTR _PTR_ ppNode
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pInfo != NULL);

	FTM_RET	xRet;
	FTDM_NODE_PTR	pNode;

	xRet = FTDM_NODE_create(pFTDM, pInfo, FTM_TRUE, &pNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create node!\n");
		return	xRet;
	}

	xRet = FTM_LIST_append(pFTDM->pNodeList, pNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create node!\n");
		FTDM_NODE_destroy(&pNode, FTM_TRUE);
	}
	else
	{
		if (ppNode != NULL)
		{
			*ppNode = pNode;	
		}
	}

	return	xRet;
}

FTM_RET	FTDM_deleteNode
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR		pID
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pID != NULL);

	FTM_RET	xRet;
	FTDM_NODE_PTR	pNode;

	xRet = FTM_LIST_get(pFTDM->pNodeList, pID, (FTM_VOID_PTR _PTR_)&pNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete node!\n");
		return	xRet;
	}

	xRet = FTM_LIST_remove(pFTDM->pNodeList, pNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete node!\n");	
		return	xRet;
	}

	xRet = FTDM_NODE_destroy(&pNode, FTM_TRUE);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete node!\n");	
	}

	return	xRet;
}

FTM_RET FTDM_isNodeExist
(
	FTDM_CONTEXT_PTR	pFTDM,
 	FTM_CHAR_PTR	pDID,
	FTM_BOOL_PTR	pExist
)
{
	ASSERT(pDID != NULL);
	ASSERT(pExist != NULL);

	FTM_RET			xRet;

	xRet = FTM_LIST_seek(pFTDM->pNodeList, (FTM_VOID_PTR)pDID);
	if (xRet == FTM_RET_OK)
	{
		*pExist = FTM_TRUE;
	}
	else
	{
		*pExist = FTM_FALSE;
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_getNodeCount
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET	xRet;

	xRet = FTM_LIST_count(pFTDM->pNodeList, pulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get node count!\n");	
	}

	return	xRet;
}

FTM_RET	FTDM_getNode
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR		pID,
	FTDM_NODE_PTR _PTR_	ppNode
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pID != NULL);
	ASSERT(ppNode != NULL);

	FTM_RET	xRet;

	xRet = FTM_LIST_get(pFTDM->pNodeList, pID, (FTM_VOID_PTR _PTR_)ppNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get node!\n");
	}

	return	xRet;
}

FTM_RET	FTDM_getNodeAt
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_ULONG			ulIndex,
	FTDM_NODE_PTR _PTR_	ppNode
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(ppNode != NULL);

	FTM_RET	xRet;

	xRet = FTM_LIST_getAt(pFTDM->pNodeList, ulIndex, (FTM_VOID_PTR _PTR_)ppNode);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get node!\n");
	}

	return	xRet;
}

FTM_RET	FTDM_getNodeIDList
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_DID_PTR		pIDList,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pIDList != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET	xRet;
	FTM_ULONG	ulCount, i;

	xRet = FTM_LIST_count(pFTDM->pNodeList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get node count!\n");
		return	xRet;	
	}

	*pulCount = 0;
	for(i = 0 ; i < ulCount && (*pulCount < ulMaxCount) ; i++)
	{
		FTDM_NODE_PTR	pNode;

		xRet = FTM_LIST_getAt(pFTDM->pNodeList, ulIndex + i, (FTM_VOID_PTR _PTR_)&pNode);
		if (xRet != FTM_RET_OK)
		{
			break;	
		}

		xRet = FTDM_NODE_getID(pNode, pIDList[*pulCount], sizeof(FTM_DID));
		if (xRet == FTM_RET_OK)
		{
			(*pulCount)++;
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_createNodeLog
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR	pDID,
	FTM_RET			xResult
)
{
	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_createObject(pDID, xResult, &pLog);
	if (xRet == FTM_RET_OK)
	{
		pLog->xType = FTM_LOG_TYPE_CREATE_NODE;
		xRet = FTDM_LOGGER_add(pFTDM->pLogger, pLog);
		if (xRet != FTM_RET_OK)
		{
		
			FTM_LOG_destroy(&pLog);	
		}
	}

	return	FTM_RET_OK;
}

/******************************************************************
 *	EP
 ******************************************************************/
FTM_RET	FTDM_createEP
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_EP_PTR		pInfo,
	FTDM_EP_PTR _PTR_ ppEP
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTDM_EP_create(pFTDM, pInfo, &pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create node!\n");
		return	xRet;
	}

	xRet = FTM_LIST_append(pFTDM->pEPList, pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create node!\n");
		FTDM_EP_destroy(&pEP);
	}
	else
	{
		if (ppEP != NULL)
		{
			*ppEP = pEP;	
		}
	}

	return	xRet;
}

FTM_RET	FTDM_deleteEP
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR		pID
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pID != NULL);
	FTM_RET	xRet;
	FTDM_EP_PTR	pEP;

	xRet = FTM_LIST_get(pFTDM->pEPList, pID, (FTM_VOID_PTR _PTR_)&pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete node!\n");
		return	xRet;
	}

	xRet = FTM_LIST_remove(pFTDM->pEPList, pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete node!\n");	
		return	xRet;
	}

	xRet = FTDM_EP_destroy(&pEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete node!\n");	
	}

	return	xRet;
}

FTM_RET	FTDM_getEPCount
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_EP_TYPE			xType,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pulCount != NULL);
	FTM_RET	xRet;

	if (xType == 0)
	{
		xRet = FTM_LIST_count(pFTDM->pEPList, pulCount);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to get node count!\n");	
			return	xRet;
		}
	}
	else
	{
		FTDM_EP_PTR	pEP;
		FTM_EP_TYPE	xEPType;

		FTM_LIST_iteratorStart(pFTDM->pEPList);
		while(FTM_LIST_iteratorNext(pFTDM->pEPList, (FTM_VOID_PTR _PTR_)&pEP) == FTM_RET_OK)
		{
			FTDM_EP_getType(pEP, &xEPType);

			if ((xType & FTM_EP_TYPE_MASK) == (xEPType & FTM_EP_TYPE_MASK))
			{
				(*pulCount)++;	
			}
		}
	}

	return	FTM_RET_OK;
}

FTM_RET	FTDM_getEP
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR		pID,
	FTDM_EP_PTR _PTR_	ppEP
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pID != NULL);
	ASSERT(ppEP != NULL);

	FTM_RET	xRet;

	xRet = FTM_LIST_get(pFTDM->pEPList, pID, (FTM_VOID_PTR _PTR_)ppEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get node!\n");
	}

	return	xRet;
}

FTM_RET	FTDM_getEPAt
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_ULONG			ulIndex,
	FTDM_EP_PTR _PTR_	ppEP
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(ppEP != NULL);

	FTM_RET	xRet;

	xRet = FTM_LIST_getAt(pFTDM->pEPList, ulIndex, (FTM_VOID_PTR _PTR_)ppEP);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get node!\n");
	}

	return	xRet;
}

FTM_RET	FTDM_getEPIDList
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_DID_PTR		pIDList,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pIDList != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET		xRet;
	FTM_ULONG	i, ulCount;

	xRet = FTM_LIST_count(pFTDM->pEPList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get ep count!\n");
		return	xRet;	
	}

	*pulCount = 0;
	for(i = 0 ; i < ulMaxCount && (ulIndex + i) < ulCount ; i++)
	{
		FTDM_EP_PTR	pEP;
		
		xRet = FTM_LIST_getAt(pFTDM->pEPList, ulIndex + i, (FTM_VOID_PTR _PTR_)&pEP);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to get ep at %lu\n", ulIndex + i);
			return	xRet;
		}

		xRet = FTDM_EP_getID(pEP, pIDList[*pulCount], FTM_EPID_LEN + 1);
		if (xRet == FTM_RET_OK)
		{
			(*pulCount)++;	
		}
	}

	return	FTM_RET_OK;
}


FTM_RET	FTDM_createEPLog
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR	pEPID,
	FTM_RET			xResult
)
{
	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_createObject(pEPID, xResult, &pLog);
	if (xRet == FTM_RET_OK)
	{
		pLog->xType = FTM_LOG_TYPE_CREATE_EP;
		xRet = FTDM_LOGGER_add(pFTDM->pLogger, pLog);
		if (xRet != FTM_RET_OK)
		{
		
			FTM_LOG_destroy(&pLog);	
		}
	}

	return	FTM_RET_OK;
}

/******************************************************************
 *	Trigger
 ******************************************************************/
FTM_RET	FTDM_createTrigger
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_TRIGGER_PTR		pInfo,
	FTDM_TRIGGER_PTR _PTR_ ppTrigger
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTDM_TRIGGER_PTR	pTrigger;

	xRet = FTDM_TRIGGER_create(pFTDM, pInfo, &pTrigger);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create node!\n");
		return	xRet;
	}

	xRet = FTM_LIST_append(pFTDM->pTriggerList, pTrigger);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create node!\n");
		FTDM_TRIGGER_destroy(&pTrigger);
	}
	else
	{
		if (ppTrigger != NULL)
		{
			*ppTrigger = pTrigger;	
		}
	}

	return	xRet;
}

FTM_RET	FTDM_deleteTrigger
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR		pID
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pID != NULL);
	FTM_RET	xRet;
	FTDM_TRIGGER_PTR	pTrigger;

	xRet = FTM_LIST_get(pFTDM->pTriggerList, pID, (FTM_VOID_PTR _PTR_)&pTrigger);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete node!\n");
		return	xRet;
	}

	xRet = FTM_LIST_remove(pFTDM->pTriggerList, pTrigger);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete node!\n");	
		return	xRet;
	}

	xRet = FTDM_TRIGGER_destroy(&pTrigger);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete node!\n");	
	}

	return	xRet;
}

FTM_RET	FTDM_getTriggerCount
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pulCount != NULL);
	FTM_RET	xRet;

	xRet = FTM_LIST_count(pFTDM->pTriggerList, pulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get trigger count!\n");	
	}

	return	xRet;
}

FTM_RET	FTDM_getTrigger
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR		pID,
	FTDM_TRIGGER_PTR _PTR_	ppTrigger
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pID != NULL);
	ASSERT(ppTrigger != NULL);
	FTM_RET	xRet;

	xRet = FTM_LIST_get(pFTDM->pTriggerList, pID, (FTM_VOID_PTR _PTR_)ppTrigger);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get node!\n");
	}

	return	xRet;
}

FTM_RET	FTDM_getTriggerAt
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_ULONG			ulIndex,
	FTDM_TRIGGER_PTR _PTR_	ppTrigger
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(ppTrigger != NULL);
	FTM_RET	xRet;

	xRet = FTM_LIST_getAt(pFTDM->pTriggerList, ulIndex, (FTM_VOID_PTR _PTR_)ppTrigger);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get node!\n");
	}

	return	xRet;
}

FTM_RET	FTDM_getTriggerIDList
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_DID_PTR		pIDList,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pIDList != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET	xRet;
	FTM_ULONG	i, ulCount;
	FTDM_TRIGGER_PTR	pTrigger;

	xRet = FTM_LIST_count(pFTDM->pTriggerList, &ulCount);
	if (xRet == FTM_RET_OK)
	{
		*pulCount = 0;
		for(i = 0 ; i < ulMaxCount && (ulIndex + i) < ulCount ; i++)
		{
			xRet = FTM_LIST_getAt(pFTDM->pTriggerList, ulIndex + i, (FTM_VOID_PTR _PTR_)&pTrigger);
			if (xRet == FTM_RET_OK)
			{
				strcpy(pIDList[(*pulCount)++], pTrigger->xInfo.pID);
			}
		}
	}

	return	xRet;
}

FTM_RET	FTDM_createTriggerLog
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR	pID,
	FTM_RET			xResult
)
{
	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_createObject(pID, xResult, &pLog);
	if (xRet == FTM_RET_OK)
	{
		pLog->xType = FTM_LOG_TYPE_CREATE_TRIGGER;
		xRet = FTDM_LOGGER_add(pFTDM->pLogger, pLog);
		if (xRet != FTM_RET_OK)
		{
		
			FTM_LOG_destroy(&pLog);	
		}
	}

	return	FTM_RET_OK;
}


/******************************************************************
 *	Action
 ******************************************************************/
FTM_RET	FTDM_createAction
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_ACTION_PTR		pInfo,
	FTDM_ACTION_PTR _PTR_ ppAction
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTDM_ACTION_PTR	pAction;

	xRet = FTDM_ACTION_create(pFTDM, pInfo, &pAction);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create node!\n");
		return	xRet;
	}

	xRet = FTM_LIST_append(pFTDM->pActionList, pAction);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create node!\n");
		FTDM_ACTION_destroy(&pAction);
	}
	else
	{
		if (ppAction != NULL)
		{
			*ppAction = pAction;	
		}
	}

	return	xRet;
}

FTM_RET	FTDM_deleteAction
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR		pID
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pID != NULL);
	FTM_RET	xRet;
	FTDM_ACTION_PTR	pAction;

	xRet = FTM_LIST_get(pFTDM->pActionList, pID, (FTM_VOID_PTR _PTR_)&pAction);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete node!\n");
		return	xRet;
	}

	xRet = FTM_LIST_remove(pFTDM->pActionList, pAction);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete node!\n");	
		return	xRet;
	}

	xRet = FTDM_ACTION_destroy(&pAction);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete node!\n");	
	}

	return	xRet;
}

FTM_RET	FTDM_getActionCount
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pulCount != NULL);
	FTM_RET	xRet;

	xRet = FTM_LIST_count(pFTDM->pActionList, pulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get action count!\n");	
	}

	return	xRet;
}

FTM_RET	FTDM_getAction
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR		pID,
	FTDM_ACTION_PTR _PTR_	ppAction
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pID != NULL);
	ASSERT(ppAction != NULL);
	FTM_RET	xRet;

	xRet = FTM_LIST_get(pFTDM->pActionList, pID, (FTM_VOID_PTR _PTR_)ppAction);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get node!\n");
	}

	return	xRet;
}

FTM_RET	FTDM_getActionAt
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_ULONG			ulIndex,
	FTDM_ACTION_PTR _PTR_	ppAction
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(ppAction != NULL);
	FTM_RET	xRet;

	xRet = FTM_LIST_getAt(pFTDM->pActionList, ulIndex, (FTM_VOID_PTR _PTR_)ppAction);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get node!\n");
	}

	return	xRet;
}

FTM_RET	FTDM_getActionIDList
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_DID_PTR		pIDList,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pIDList != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET		xRet;
	FTM_ULONG	i, ulCount;

	xRet = FTM_LIST_count(pFTDM->pActionList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get action count!\n");
		return	xRet;	
	}

	*pulCount = 0;
	for(i = 0 ; i < ulMaxCount && (ulIndex + i) < ulCount ; i++)
	{
		FTDM_ACTION_PTR	pAction;
		
		xRet = FTM_LIST_getAt(pFTDM->pActionList, ulIndex + i, (FTM_VOID_PTR _PTR_)&pAction);
		if (xRet != FTM_RET_OK)
		{
			ERROR2(xRet, "Failed to get action at %lu\n", ulIndex + i);
			return	xRet;
		}

		xRet = FTDM_ACTION_getID(pAction, pIDList[*pulCount], sizeof(FTM_DID));
		if (xRet == FTM_RET_OK)
		{
			(*pulCount)++;	
		}
	}

	return	FTM_RET_OK;
}
	
FTM_RET	FTDM_createActionLog
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR	pID,
	FTM_RET			xResult
)
{
	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_createObject(pID, xResult, &pLog);
	if (xRet == FTM_RET_OK)
	{
		pLog->xType = FTM_LOG_TYPE_CREATE_ACTION;
		xRet = FTDM_LOGGER_add(pFTDM->pLogger, pLog);
		if (xRet != FTM_RET_OK)
		{
		
			FTM_LOG_destroy(&pLog);	
		}
	}

	return	FTM_RET_OK;
}

/******************************************************************
 *	Rule
 ******************************************************************/
FTM_RET	FTDM_createRule
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_RULE_PTR		pInfo,
	FTDM_RULE_PTR _PTR_ ppRule
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pInfo != NULL);
	FTM_RET	xRet;
	FTDM_RULE_PTR	pRule;

	xRet = FTDM_RULE_create(pFTDM, pInfo, &pRule);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create node!\n");
		return	xRet;
	}

	xRet = FTM_LIST_append(pFTDM->pRuleList, pRule);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to create node!\n");
		FTDM_RULE_destroy(&pRule);
	}
	else
	{
		if (ppRule != NULL)
		{
			*ppRule = pRule;	
		}
	}

	return	xRet;
}

FTM_RET	FTDM_deleteRule
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR		pID
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pID != NULL);
	FTM_RET	xRet;
	FTDM_RULE_PTR	pRule;

	xRet = FTM_LIST_get(pFTDM->pRuleList, pID, (FTM_VOID_PTR _PTR_)&pRule);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete node!\n");
		return	xRet;
	}

	xRet = FTM_LIST_remove(pFTDM->pRuleList, pRule);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete node!\n");	
		return	xRet;
	}

	xRet = FTDM_RULE_destroy(&pRule);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to delete node!\n");	
	}

	return	xRet;
}

FTM_RET	FTDM_getRuleCount
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_ULONG_PTR		pulCount
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pulCount != NULL);
	FTM_RET	xRet;

	xRet = FTM_LIST_count(pFTDM->pRuleList, pulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get node count!\n");	
	}

	return	xRet;
}

FTM_RET	FTDM_getRule
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR		pID,
	FTDM_RULE_PTR _PTR_	ppRule
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pID != NULL);
	ASSERT(ppRule != NULL);
	FTM_RET	xRet;

	xRet = FTM_LIST_get(pFTDM->pRuleList, pID, (FTM_VOID_PTR _PTR_)ppRule);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get node!\n");
	}

	return	xRet;
}

FTM_RET	FTDM_getRuleAt
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_ULONG			ulIndex,
	FTDM_RULE_PTR _PTR_	ppRule
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(ppRule != NULL);
	FTM_RET	xRet;

	xRet = FTM_LIST_getAt(pFTDM->pRuleList, ulIndex, (FTM_VOID_PTR _PTR_)ppRule);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get node!\n");
	}

	return	xRet;
}

FTM_RET	FTDM_getRuleIDList
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_DID_PTR		pIDList,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pIDList != NULL);
	ASSERT(pulCount != NULL);

	FTM_RET	xRet;
	FTM_ULONG	i, ulCount;
	FTDM_RULE_PTR	pRule;

	xRet = FTM_LIST_count(pFTDM->pRuleList, &ulCount);
	if (xRet != FTM_RET_OK)
	{
		ERROR2(xRet, "Failed to get rule id list!\n");
		return	xRet;	
	}

	*pulCount = 0;
	for(i = 0 ; i < ulMaxCount && (ulIndex + i) < ulCount ; i++)
	{
		xRet = FTM_LIST_getAt(pFTDM->pRuleList, ulIndex + i, (FTM_VOID_PTR _PTR_)&pRule);
		if (xRet -= FTM_RET_OK)
		{
			FTDM_RULE_getID(pRule, pIDList[(*pulCount)++], sizeof(FTM_DID));
		}
	}

	return	xRet;
}

FTM_RET	FTDM_createRuleLog
(
	FTDM_CONTEXT_PTR	pFTDM,
	FTM_CHAR_PTR	pID,
	FTM_RET			xResult
)
{
	FTM_RET		xRet;
	FTM_LOG_PTR	pLog;

	xRet = FTM_LOG_createObject(pID, xResult, &pLog);
	if (xRet == FTM_RET_OK)
	{
		pLog->xType = FTM_LOG_TYPE_CREATE_RULE;
		xRet = FTDM_LOGGER_add(pFTDM->pLogger, pLog);
		if (xRet != FTM_RET_OK)
		{
		
			FTM_LOG_destroy(&pLog);	
		}
	}

	return	FTM_RET_OK;
}

/************************************************
 * Log
 ************************************************/
FTM_RET	FTDM_addLog
(
	FTDM_CONTEXT_PTR pFTDM,
	FTM_LOG_PTR	pLog
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pLog != NULL);
	
	return	FTDM_LOGGER_add(pFTDM->pLogger, pLog);
}

FTM_RET	FTDM_deleteLog
(
	FTDM_CONTEXT_PTR pFTDM,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulDeletedCount
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pulDeletedCount != NULL);

	return	FTDM_LOGGER_del(pFTDM->pLogger, ulIndex, ulCount, pulDeletedCount);
} 

FTM_RET	FTDM_getLogCount
(
	FTDM_CONTEXT_PTR pFTDM,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pulCount != NULL);

	return	FTDM_LOGGER_count(pFTDM->pLogger, pulCount);
}

FTM_RET	FTDM_getLog
(
	FTDM_CONTEXT_PTR pFTDM,
	FTM_ULONG		ulIndex,
	FTM_LOG_PTR		pLogList,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR	pulCount
)
{
	ASSERT(pFTDM != NULL);
	ASSERT(pLogList != NULL);
	ASSERT(pulCount != NULL);

	return	FTDM_LOGGER_get(pFTDM->pLogger, ulIndex, pLogList, ulMaxCount, pulCount);
}

	
