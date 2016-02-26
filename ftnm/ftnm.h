#ifndef	__FTNM_H__
#define	__FTNM_H__

#include "ftm_types.h"
#include "ftm_debug.h"
#include "ftm_error.h"
#include "ftm_mem.h"
#include "ftm_value.h"
#include "ftnm_types.h"
#include "ftnm_dmc.h"
#include "ftnm_server.h"
#include "ftnm_snmpc.h"
#include "ftnm_snmptrapd.h"

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
	FTM_LIST		xEPList;
	pthread_t		xPThread;

	FTDMC_SESSION		xDMCSession;
	FTNM_SERVER_PTR		pServer;
	FTNM_SNMPC_PTR		pSNMPC;
	FTNM_SNMPTRAPD_PTR	pSNMPTrapd;
}	FTNM_CONTEXT, _PTR_ FTNM_CONTEXT_PTR;


FTM_RET	FTNM_init(void);
FTM_RET	FTNM_final(void);
FTM_RET	FTNM_loadConfig(FTM_CHAR_PTR pConfigFileName);

FTM_RET	FTNM_showConfig(void);
FTM_RET FTNM_run(void);
FTM_RET FTNM_waitingForFinished(void);

FTM_RET	FTNM_setEPData(FTM_EPID xEPID, FTM_EP_DATA_PTR pData);
FTM_RET	FTNM_getEPDataInfo(FTM_EPID xEPID, FTM_ULONG_PTR pulBeginTime, FTM_ULONG_PTR pulEndTime, FTM_ULONG_PTR pulCount);
FTM_RET	FTNM_getEPDataCount(FTM_EPID xEPID, FTM_ULONG_PTR ulCount);
#endif

