#ifndef	__FTNM_H__
#define	__FTNM_H__

#include "ftm_types.h"
#include "ftm_debug.h"
#include "ftm_error.h"
#include "ftm_mem.h"
#include "ftm_value.h"
#include "ftnm_types.h"
#include "ftnm_config.h"
#include "ftnm_node.h"
#include "ftnm_ep.h"
#include "ftnm_ep_class.h"

typedef	FTM_ULONG	FTNM_DMC_STATE;

#define	FTNM_DMC_STATE_CREATING 			0x00000001
#define	FTNM_DMC_STATE_CREATED				0x00000002
#define	FTNM_DMC_STATE_INITIALIZING			0x00000003
#define	FTNM_DMC_STATE_INITIALIZED			0x00000004
#define	FTNM_DMC_STATE_SYNCING				0x00000005
#define	FTNM_DMC_STATE_SYNCHRONIZED			0x00000006
#define	FTNM_DMC_STATE_CALLED				0x00000007
#define	FTNM_DMC_STATE_WAITING				0x00000008
#define	FTNM_DMC_STATE_CALL_FOR_PROCESSING	0x00000009
#define	FTNM_DMC_STATE_PROCESSING			0x0000000A
#define	FTNM_DMC_STATE_PROCESS_FINISHED		0x0000000B
#define	FTNM_DMC_STATE_PAUSED				0x0000000C
#define	FTNM_DMC_STATE_INACTIVATED			0x0000000D
#define	FTNM_DMC_STATE_FINISHING			0x0000000E
#define	FTNM_DMC_STATE_FINISHED				0x0000000F
#define	FTNM_DMC_STATE_CONNECTED			0x00000010

typedef	struct
{
	FTNM_DMC_STATE	xState;
	pthread_t		xThread;
	FTDMC_SESSION	xSession;

}	FTNM_DMC, _PTR_	FTNM_DMC_PTR;

typedef	struct
{
	FTNM_CFG_PTR	pConfig;

	FTNM_DMC		xDMC;	
}	FTNM_CONTEXT, _PTR_ FTNM_CONTEXT_PTR;

FTM_RET	FTNM_init(FTNM_CONTEXT_PTR pContext, FTNM_CFG_PTR pConfig);
FTM_RET	FTNM_final(FTNM_CONTEXT_PTR pContext);

FTM_RET FTNM_DMC_run(FTNM_CONTEXT_PTR pContext);

#endif

