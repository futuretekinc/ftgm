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

#define	FTNM_DEFAULT_SERVER_PORT			8889
#define	FTNM_DEFAULT_SERVER_SESSION_COUNT	10

typedef	FTM_ULONG	FTNM_STATE;

#define	FTNM_STATE_CREATING 			0x00000001
#define	FTNM_STATE_CREATED				0x00000002
#define	FTNM_STATE_INITIALIZING			0x00000003
#define	FTNM_STATE_INITIALIZED			0x00000004
#define	FTNM_STATE_SYNCING				0x00000005
#define	FTNM_STATE_SYNCHRONIZED			0x00000006
#define	FTNM_STATE_CALLED				0x00000007
#define	FTNM_STATE_WAITING				0x00000008
#define	FTNM_STATE_CALL_FOR_PROCESSING	0x00000009
#define	FTNM_STATE_PROCESSING			0x0000000A
#define	FTNM_STATE_PROCESS_FINISHED		0x0000000B
#define	FTNM_STATE_PAUSED				0x0000000C
#define	FTNM_STATE_INACTIVATED			0x0000000D
#define	FTNM_STATE_FINISHING			0x0000000E
#define	FTNM_STATE_FINISHED				0x0000000F
#define	FTNM_STATE_CONNECTED			0x00000010

typedef	struct
{
	FTNM_STATE		xState;
	pthread_t		xThread;
	FTDMC_SESSION	xSession;

}	FTNM_DMC, _PTR_	FTNM_DMC_PTR;

typedef	struct
{
	FTNM_CFG		xConfig;
	FTNM_DMC		xDMC;	
}	FTNM_CONTEXT, _PTR_ FTNM_CONTEXT_PTR;

FTM_RET	FTNM_init(FTM_CHAR_PTR pConfigFileName);
FTM_RET	FTNM_final(FTM_VOID);

FTM_RET	FTNM_showConfig(FTM_VOID);

FTM_RET FTNM_run(FTM_VOID);


FTM_RET	FTNM_DMC_setEPData(FTNM_EP_PTR pEP);
FTM_RET FTNM_DMC_setEPDataINT(FTM_EPID xEPID, FTM_ULONG ulTime, FTM_INT nValue);
FTM_RET FTNM_DMC_setEPDataULONG(FTM_EPID xEPID, FTM_ULONG ulTime, FTM_ULONG ulValue);
FTM_RET FTNM_DMC_setEPDataFLOAT(FTM_EPID xEPID, FTM_ULONG ulTime, FTM_DOUBLE fValue);
#endif

