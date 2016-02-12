#include "ftdm.h"
#include "ftm_debug.h"
#include "ftdm_node_info.h"
#include "ftdm_ep_info.h"
#include "ftdm_ep_class_info.h"
#include "ftdm_sqlite.h"

FTM_RET 	FTDM_init(FTDM_CFG_PTR pConfig)
{
	FTM_RET	nRet;

	nRet = FTDM_DBIF_init(&pConfig->xDB);
	if (nRet != FTM_RET_OK)
	{
		ERROR("FTDM initialization failed. [ %08lx ]\n", nRet);
		return	nRet;
	}

	nRet = FTDM_NODE_INFO_init(&pConfig->xNode);
	if (nRet != FTM_RET_OK)
	{
		ERROR("FTDM_initNodeInfo failed\n");	
	}

	nRet = FTDM_EP_INFO_init(&pConfig->xEP);
	if (nRet != FTM_RET_OK)
	{
		ERROR("FTDM_initEPInfo failed\n");	
	}

	nRet = FTDM_EP_CLASS_INFO_init(&pConfig->xEP);
	if (nRet != FTM_RET_OK)
	{
		ERROR("FTDM_initEPClassInfo failed\n");	
	}

	FTM_DEBUG_configSet(&pConfig->xDebug);


	TRACE("FTDM initialization completed successfully.\n");

	return	FTM_RET_OK;
}

FTM_RET	FTDM_final(void)
{
	FTM_RET	nRet;

	nRet = FTDM_EP_CLASS_INFO_final();
	if (nRet != FTM_RET_OK)
	{
		ERROR("FTDM_finalNodeInfo failed\n");	
	}

	nRet = FTDM_EP_INFO_final();
	if (nRet != FTM_RET_OK)
	{
		ERROR("FTDM_finalNodeInfo failed\n");	
	}

	nRet = FTDM_NODE_INFO_final();
	if (nRet != FTM_RET_OK)
	{
		ERROR("FTDM_finalNodeInfo failed\n");	
	}


	nRet = FTDM_DBIF_final();
	if (nRet != FTM_RET_OK)
	{
		ERROR("FTDM finalization failed. [ %08lx ]\n", nRet);

		return	FTM_RET_OK;
	}

	TRACE("FTDM finalization completed successfully.\n");

	return	FTM_RET_OK;
}

