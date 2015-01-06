#include "ftdm.h"
#include "ftm_debug.h"
#include "ftdm_device.h"
#include "ftdm_ep.h"
#include "sqlite_if.h"

FTM_RET 	FTDM_init(void)
{
	if ((FTDM_DBIF_init() != FTM_RET_OK) ||
		(FTDM_DBIF_initNodeInfoTable() != FTM_RET_OK) ||
		(FTDM_DBIF_initEPInfoTable() != FTM_RET_OK))
	{
		TRACE("FTDM initialization failed.\n");

		return	FTM_RET_ERROR;
	
	}

	FTDM_initNode();
	FTDM_initEP();

	TRACE("FTDM initialization completed successfully.\n");

	return	FTM_RET_OK;
}

FTM_RET	FTDM_final(void)
{
	FTDM_finalNode();

	if (FTDM_DBIF_final() != FTM_RET_OK)
	{
		TRACE("FTDM finalization failed.\n");

		return	FTM_RET_OK;
	}

	TRACE("FTDM finalization completed successfully.\n");

	return	FTM_RET_OK;
}

