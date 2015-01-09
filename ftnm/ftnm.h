#ifndef	__FTNM_H__
#define	__FTNM_H__

#include "ftm_types.h"
#include "ftm_debug.h"
#include "ftm_error.h"
#include "ftnm_types.h"
#include "ftnm_config.h"
#include "ftnm_node.h"
#include "ftnm_ep.h"

typedef	struct
{
	FTNM_CFG_PTR	pConfig;

	FTDMC_SESSION	xSession;

}	FTNM_CONTEXT, _PTR_ FTNM_CONTEXT_PTR;

FTM_RET	FTNM_init(FTNM_CONTEXT_PTR pContext, FTNM_CFG_PTR pConfig);
FTM_RET	FTNM_final(FTNM_CONTEXT_PTR pContext);

FTM_RET	FTNM_run(FTNM_CONTEXT_PTR pConext);

#endif

