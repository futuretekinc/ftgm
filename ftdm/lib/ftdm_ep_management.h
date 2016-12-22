#ifndef	__FTDM_EP_MANAGEMENT_H__
#define	__FTDM_EP_MANAGEMENT_H__

#include "ftdm_config.h"
#include "ftdm_ep.h"
#include "ftdm_dbif.h"

typedef	struct FTDM_EPM_STRUCT
{
	FTDM_DBIF_PTR	pDBIF;
	FTM_LIST_PTR	pList;
}	FTDM_EPM, _PTR_ FTDM_EPM_PTR;
	
FTM_RET	FTDM_EPM_init
(
	FTDM_EPM_PTR	pEPM
);

FTM_RET FTDM_EPM_final
(
	FTDM_EPM_PTR	pEPM
);

FTM_RET	FTDM_EPM_create
(
	FTDM_DBIF_PTR	pDBIF,
	FTDM_EPM_PTR _PTR_ ppEPM
);

FTM_RET	FTDM_EPM_destroy
(
	FTDM_EPM_PTR _PTR_ ppEPM
);

FTM_RET	FTDM_EPM_loadConfig
(
	FTDM_EPM_PTR	pEPM,
	FTM_CONFIG_PTR	pConfig
);

FTM_RET	FTDM_EPM_loadFromFile

(
	FTDM_EPM_PTR	pEPM,
	FTM_CHAR_PTR	pFileName
);

FTM_RET	FTDM_EPM_loadFromDB
(
	FTDM_EPM_PTR	pEPM
);

FTM_RET	FTDM_EPM_saveToDB
(
	FTDM_EPM_PTR	pEPM
);


FTM_RET	FTDM_EPM_count
(
	FTDM_EPM_PTR	pEPM,
	FTM_EP_TYPE		xType,
	FTM_ULONG_PTR	pnCount
);

FTM_RET	FTDM_EPM_get
(
	FTDM_EPM_PTR		pEPM,
	FTM_CHAR_PTR		pEPID,
	FTDM_EP_PTR	_PTR_ 	ppEP
);

FTM_RET	FTDM_EPM_getAt
(
	FTDM_EPM_PTR		pEPM,
	FTM_ULONG			ulIndex,
	FTDM_EP_PTR	_PTR_ 	ppEP
);

FTM_RET	FTDM_EPM_getEPIDList
(
	FTDM_EPM_PTR		pEPM,
	FTM_EPID_PTR		pEPIDs,
	FTM_ULONG			ulIndex,
	FTM_ULONG			ulMaxCount,
	FTM_ULONG_PTR		pulCount
);

FTM_RET	FTDM_EPM_append
(
	FTDM_EPM_PTR	pEPM,
	FTDM_EP_PTR		pEP
);

FTM_RET	FTDM_EPM_remove
(
	FTDM_EPM_PTR	pEPM,
	FTDM_EP_PTR		pEP
);

FTM_RET	FTDM_EPM_showList
(
	FTDM_EPM_PTR	pEPM
);

#endif
