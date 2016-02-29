#ifndef	__FTDM_H__
#define	__FTDM_H__

#include "ftm.h"
#include "ftdm_types.h"
#include "ftdm_cmd.h"
#include "ftdm_config.h"
#include "ftdm_node.h"
#include "ftdm_ep.h"
#include "ftdm_ep_data.h"
#include "ftdm_ep_class.h"

FTM_RET	FTDM_init
(
	FTM_VOID
);

FTM_RET	FTDM_final
(
	FTM_VOID
);

FTM_RET FTDM_loadConfig
(
	FTDM_CFG_PTR pConfig
);
#endif

