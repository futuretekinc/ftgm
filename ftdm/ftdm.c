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

	FTDM_initNodeInfo(&pConfig->xNode);
	FTDM_initEPInfo(&pConfig->xEP);
	FTDM_initEPClassInfo(&pConfig->xEP);

	TRACE("FTDM initialization completed successfully.\n");

	return	FTM_RET_OK;
}

FTM_RET	FTDM_final(void)
{
	FTM_RET	nRet;

	FTDM_finalEPClassInfo();
	FTDM_finalEPInfo();
	FTDM_finalNodeInfo();

	nRet = FTDM_DBIF_final();
	if (nRet != FTM_RET_OK)
	{
		ERROR("FTDM finalization failed. [ %08lx ]\n", nRet);

		return	FTM_RET_OK;
	}

	TRACE("FTDM finalization completed successfully.\n");

	return	FTM_RET_OK;
}

