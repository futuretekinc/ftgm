#include "ftdm.h"
#include "sqlite_if.h"
#include "debug.h"

FTDM_RET 	FTDM_init(void)
{
	if ((FTDM_DBIF_init() != FTDM_RET_OK) ||
		(FTDM_DBIF_initDeviceInfoTable() != FTDM_RET_OK) ||
		(FTDM_DBIF_initEPInfoTable() != FTDM_RET_OK) ||
		(FTDM_DBIF_initEPDataTable() != FTDM_RET_OK))
	{
		TRACE("FTDM initialization failed.\n");

		return	FTDM_RET_ERROR;
	
	}

	TRACE("FTDM initialization completed successfully.\n");

	return	FTDM_RET_OK;
}

FTDM_RET	FTDM_final(void)
{
	if (FTDM_DBIF_final() != FTDM_RET_OK)
	{
		TRACE("FTDM finalization failed.\n");

		return	FTDM_RET_OK;
	}

	TRACE("FTDM finalization completed successfully.\n");

	return	FTDM_RET_OK;
}

