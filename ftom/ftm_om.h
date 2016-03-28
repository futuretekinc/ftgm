#ifndef	__FTM_OM_H__
#define	__FTM_OM_H__

#include "ftm.h"
#include "ftm_om_types.h"
#include "ftm_om_dmc.h"
#include "ftm_om_client.h"
#include "ftm_om_server.h"
#include "ftm_om_snmpc.h"
#include "ftm_om_snmptrapd.h"
#include "ftm_om_trigger.h"
#include "ftm_om_action.h"
#include "ftm_om_rule.h"
#include "ftm_om_msg.h"

#define	FTM_OM_DEFAULT_SERVER_SESSION_COUNT	10

typedef	FTM_ULONG	FTM_OM_STATE;

#define	FTM_OM_STATE_CREATING 			0x00000001
#define	FTM_OM_STATE_CREATED				0x00000002
#define	FTM_OM_STATE_INITIALIZING			0x00000003
#define	FTM_OM_STATE_INITIALIZED			0x00000004
#define	FTM_OM_STATE_SYNCING				0x00000005
#define	FTM_OM_STATE_SYNCHRONIZED			0x00000006
#define	FTM_OM_STATE_CALLED				0x00000007
#define	FTM_OM_STATE_WAITING				0x00000008
#define	FTM_OM_STATE_CALL_FOR_PROCESSING	0x00000009
#define	FTM_OM_STATE_PROCESSING			0x0000000A
#define	FTM_OM_STATE_STOPED				0x0000000B
#define	FTM_OM_STATE_PROCESS_FINISHED		0x0000000C
#define	FTM_OM_STATE_PAUSED				0x0000000D
#define	FTM_OM_STATE_INACTIVATED			0x0000000E
#define	FTM_OM_STATE_FINISHING			0x0000000F
#define	FTM_OM_STATE_FINISHED				0x00000012
#define	FTM_OM_STATE_CONNECTED			0x00000010
#define	FTM_OM_STATE_INITIALIZE			0x00000011

typedef	struct
{
	FTM_CHAR		pDID[FTM_DID_LEN + 1];
}	FTM_OM_CONFIG, _PTR_ FTM_OM_CONFIG_PTR;

typedef	struct FTM_OM_EPM_STRUCT _PTR_ FTM_OM_EPM_PTR;

typedef	struct FTM_OM_STRUCT
{
	FTM_OM_CONFIG		xConfig;

	FTM_OM_STATE			xState;
	pthread_t			xThread;

	FTM_BOOL			bStop;

	FTM_OM_EPM_PTR		pEPM;
	FTM_OM_MSG_QUEUE_PTR	pMsgQ;
	FTM_OM_TRIGGERM_PTR	pTriggerM;
	FTM_OM_ACTIONM_PTR	pActionM;
	FTM_OM_RULEM_PTR		pRuleM;
}	FTM_OM, _PTR_ FTM_OM_PTR;


FTM_RET	FTM_OM_create
(
	FTM_OM_PTR _PTR_ ppOM
);

FTM_RET	FTM_OM_destroy
(
	FTM_OM_PTR _PTR_ ppOM
);

FTM_RET	FTM_OM_init
(
	FTM_OM_PTR pOM
);

FTM_RET	FTM_OM_final
(
	FTM_OM_PTR pOM
);

FTM_RET	FTM_OM_loadFromFile
(
	FTM_OM_PTR 	pOM, 
	FTM_CHAR_PTR 	pConfigFileName
);

FTM_RET	FTM_OM_showConfig
(
	FTM_OM_PTR 	pOM
);

FTM_RET FTM_OM_start
(
	FTM_OM_PTR 	pOM
);

FTM_RET FTM_OM_stop
(
	FTM_OM_PTR 	pOM
);

FTM_RET FTM_OM_waitingForFinished
(
	FTM_OM_PTR 	pOM
);

FTM_RET	FTM_OM_getDID
(
	FTM_OM_PTR 		pOM, 
	FTM_CHAR_PTR 	pBuff, 
	FTM_ULONG 		ulBuffLen
);

FTM_RET	FTM_OM_createEP
(
	FTM_OM_PTR pOM, 
	FTM_EP_PTR pInfo
);

FTM_RET	FTM_OM_destroyEP
(
	FTM_OM_PTR 	pOM, 
	FTM_EP_ID 	xEPID
);

FTM_RET	FTM_OM_getEPDataList
(
	FTM_OM_PTR 		pOM, 
	FTM_EP_ID 		xEPID, 
	FTM_ULONG 		ulStart, 
	FTM_EP_DATA_PTR pDataList, 
	FTM_ULONG 		ulMaxCount, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTM_OM_getEPDataInfo
(
	FTM_OM_PTR 		pOM, 
	FTM_EP_ID 		xEPID, 
	FTM_ULONG_PTR 	pulBeginTime, 
	FTM_ULONG_PTR 	pulEndTime, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTM_OM_getEPDataCount
(
	FTM_OM_PTR 		pOM, 
	FTM_EP_ID 		xEPID, 
	FTM_ULONG_PTR 	ulCount
);

FTM_RET	FTM_OM_NOTIFY_SNMPTrap
(
	FTM_OM_PTR 	pOM, 
	FTM_CHAR_PTR 		pTrapMsg
);

FTM_RET FTM_OM_NOTIFY_EPChanged
(	
	FTM_OM_PTR 	pOM, 
	FTM_EP_ID 			xEPID, 
	FTM_EP_DATA_PTR 	pData
);

FTM_RET	FTM_OM_NOTIFY_EPUpdated
(
	FTM_OM_PTR 	pOM, 
	FTM_EP_ID 			xEPID, 
	FTM_EP_DATA_PTR 	pData
);

FTM_RET	FTM_OM_NOTIFY_EPDataSaveToDB
(
	FTM_OM_PTR 	pOM, 
	FTM_EP_ID 			xEPID, 
	FTM_EP_DATA_PTR 	pData
);

FTM_RET	FTM_OM_NOTIFY_quit
(
	FTM_OM_PTR 	pOM
);

FTM_RET		FTM_OM_NOTIFY_EPDataTransINT
(
	FTM_OM_PTR	pOM,
	FTM_EP_ID	xEPID, 
	FTM_INT		nValue,
	FTM_INT 	nAverage, 
	FTM_INT 	nCount, 
	FTM_INT 	nMax, 
	FTM_INT 	nMin
);

FTM_RET		FTM_OM_NOTIFY_EPDataTransULONG
(
	FTM_OM_PTR	pOM,
	FTM_EP_ID 	xEPID, 
	FTM_ULONG 	ulValue, 
	FTM_ULONG 	ulAverage, 
	FTM_INT 	nCount, 
	FTM_ULONG 	ulMax, 
	FTM_ULONG 	ulMin
);

FTM_RET		FTM_OM_NOTIFY_EPDataTransFLOAT
(
	FTM_OM_PTR	pOM,
	FTM_EP_ID 	xEPID, 
	FTM_FLOAT 	fValue, 
	FTM_FLOAT 	fAverage, 
	FTM_INT 	nCount, 
	FTM_FLOAT 	fMax, 
	FTM_FLOAT 	fMin
);

FTM_RET		FTM_OM_NOTIFY_EPDataTransBOOL
(
	FTM_OM_PTR	pOM,
	FTM_EP_ID 	xEPID, 
	FTM_BOOL 	bValue
);

#endif

