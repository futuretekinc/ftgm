#ifndef	__FTNM_H__
#define	__FTNM_H__

#include "ftm.h"
#include "ftnm_types.h"
#include "ftnm_dmc.h"
#include "ftnm_client.h"
#include "ftnm_server.h"
#include "ftnm_snmpc.h"
#include "ftnm_snmptrapd.h"
#include "ftnm_trigger.h"
#include "ftnm_action.h"
#include "ftnm_rule.h"

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
#define	FTNM_STATE_STOPED				0x0000000B
#define	FTNM_STATE_PROCESS_FINISHED		0x0000000C
#define	FTNM_STATE_PAUSED				0x0000000D
#define	FTNM_STATE_INACTIVATED			0x0000000E
#define	FTNM_STATE_FINISHING			0x0000000F
#define	FTNM_STATE_FINISHED				0x00000012
#define	FTNM_STATE_CONNECTED			0x00000010
#define	FTNM_STATE_INITIALIZE			0x00000011

typedef	struct
{
	FTM_CHAR		pDID[FTM_DID_LEN + 1];
}	FTNM_INFO, _PTR_ FTNM_INFO_PTR;

typedef	struct
{
	FTNM_STATE		xState;
	FTM_LIST		xEPList;
	pthread_t		xThread;

	FTM_BOOL		bStop;
}	FTNM_CONTEXT, _PTR_ FTNM_CONTEXT_PTR;


FTM_RET	FTNM_init(FTM_VOID);
FTM_RET	FTNM_final(FTM_VOID);
FTM_RET	FTNM_loadFromFile(FTM_CHAR_PTR pConfigFileName);

FTM_RET	FTNM_showConfig(FTM_VOID);
FTM_RET FTNM_start(FTM_VOID);
FTM_RET FTNM_stop(FTM_VOID);
FTM_RET FTNM_waitingForFinished(FTM_VOID);

FTM_RET	FTNM_getDMC(FTNM_DMC_PTR _PTR_ ppDMC);

FTM_RET	FTNM_setEPData(FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData);
FTM_RET	FTNM_getEPDataInfo(FTM_EP_ID xEPID, FTM_ULONG_PTR pulBeginTime, FTM_ULONG_PTR pulEndTime, FTM_ULONG_PTR pulCount);
FTM_RET	FTNM_getEPDataCount(FTM_EP_ID xEPID, FTM_ULONG_PTR ulCount);

FTM_RET	FTNM_NOTIFY_quit(FTM_VOID);

#endif

