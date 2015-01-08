#include "ftdm.h"
#include "ftm_debug.h"
#include "ftdm_node_info.h"
#include "ftdm_ep_info.h"
#include "ftdm_sqlite.h"

FTM_RET 	FTDM_init(FTDM_CONFIG_PTR pConfig)
{
	if (FTDM_DBIF_init(pConfig->xDatabase.pFileName) != FTM_RET_OK)
	{
		TRACE("FTDM initialization failed.\n");

		return	FTM_RET_ERROR;
	}

	FTDM_initNodeInfo();
	FTDM_initEPInfo();

	TRACE("FTDM initialization completed successfully.\n");

	return	FTM_RET_OK;
}

FTM_RET	FTDM_final(void)
{
	FTDM_finalEPInfo();
	FTDM_finalNodeInfo();

	if (FTDM_DBIF_final() != FTM_RET_OK)
	{
		TRACE("FTDM finalization failed.\n");

		return	FTM_RET_OK;
	}

	TRACE("FTDM finalization completed successfully.\n");

	return	FTM_RET_OK;
}

