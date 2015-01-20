#ifndef	__FTDM_H__
#define	__FTDM_H__

#include "ftm_types.h"
#include "ftm_debug.h"
#include "ftm_object.h"
#include "ftdm_type.h"
#include "ftdm_cmd.h"
#include "ftdm_config.h"
#include "ftdm_node_info.h"
#include "ftdm_ep_info.h"
#include "ftdm_ep_data.h"
#include "ftdm_ep_class_info.h"

FTM_RET	FTDM_init
(
	FTDM_CFG_PTR pConfig
);

FTM_RET	FTDM_final
(
	void
);

#endif

