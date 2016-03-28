#ifndef	__FTM_OM_DMC_H__
#define	__FTM_OM_DMC_H__

#include "ftm.h"
#include "ftdm_client.h"
#include "ftm_om_msg.h"
#include "ftm_om_service.h"

typedef	struct	FTM_OM_CONTEXT_STRUCT _PTR_ FTM_OM_CONTEXT_PTR;

typedef	struct
{
	FTM_OM_CONTEXT_PTR		pCTX;
	FTDMC_SESSION			xSession;
	FTDMC_CFG				xConfig;
	pthread_t				xThread;
	FTM_BOOL				bStop;
	FTM_OM_MSG_QUEUE		xMsgQ;
	FTM_OM_SERVICE_ID		xServiceID;
	FTM_OM_SERVICE_CALLBACK	fServiceCB;
}	FTM_OM_DMC, _PTR_	FTM_OM_DMC_PTR;

FTM_RET FTM_OM_DMC_init
(
	FTM_OM_CONTEXT_PTR 	pCTX, 
	FTM_OM_DMC_PTR 		pDMC
);

FTM_RET FTM_OM_DMC_final
(
	FTM_OM_DMC_PTR pDMC
);

FTM_RET FTM_OM_DMC_loadFromFile
(
	FTM_OM_DMC_PTR 	pDMC, 
	FTM_CHAR_PTR 	pConfigFileName
);

FTM_RET	FTM_OM_DMC_showConfig
(
	FTM_OM_DMC_PTR 	pDMC
);

FTM_RET	FTM_OM_DMC_start
(
	FTM_OM_DMC_PTR 	pDMC
);

FTM_RET	FTM_OM_DMC_stop
(
	FTM_OM_DMC_PTR 	pDMC
);

FTM_RET	FTM_OM_DMC_setServiceCallback
(
	FTM_OM_DMC_PTR 			pDMC, 
	FTM_OM_SERVICE_ID 		xID, 
	FTM_OM_SERVICE_CALLBACK pServiceCB
);

FTM_RET	FTM_OM_DMC_notify
(
	FTM_OM_DMC_PTR 	pDMC, 
	FTM_OM_MSG_PTR 	pMsg
);

FTM_RET	FTM_OM_DMC_EP_create
(
	FTM_OM_DMC_PTR 	pDMC, 
	FTM_EP_PTR 		pInfo
);

FTM_RET	FTM_OM_DMC_EP_destroy
(
	FTM_OM_DMC_PTR 	pDMC, 
	FTM_EP_ID 		xEPID
);

FTM_RET FTM_OM_DMC_EP_DATA_get
(
	FTM_OM_DMC_PTR	pDMC,
	FTM_EP_ID 		xEPID, 
	FTM_ULONG		ulStartIndex,
	FTM_EP_DATA_PTR	pData,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTM_OM_DMC_EP_DATA_set
(
	FTM_OM_DMC_PTR 	pDMC, 
	FTM_EP_ID 		xEPID, 
	FTM_EP_DATA_PTR pData
);

FTM_RET FTM_OM_DMC_EP_DATA_setINT
(	FTM_OM_DMC_PTR 	pDMC, 
	FTM_EP_ID 		xEPID, 
	FTM_ULONG 		ulTime, 
	FTM_INT 		nValue
);

FTM_RET FTM_OM_DMC_EP_DATA_setULONG
(
	FTM_OM_DMC_PTR 	pDMC, 
	FTM_EP_ID 		xEPID, 
	FTM_ULONG 		ulTime, 
	FTM_ULONG 		ulValue
);

FTM_RET FTM_OM_DMC_EP_DATA_setFLOAT
(
	FTM_OM_DMC_PTR 	pDMC, 
	FTM_EP_ID 		xEPID, 
	FTM_ULONG 		ulTime, 
	FTM_DOUBLE 		fValue
);

FTM_RET	FTM_OM_DMC_EP_DATA_count
(
	FTM_OM_DMC_PTR 	pDMC, 
	FTM_EP_ID 		xEPID, 
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTM_OM_DMC_EP_DATA_info
(
	FTM_OM_DMC_PTR 	pDMC, 
	FTM_EP_ID 		xEPID, 
	FTM_ULONG_PTR 	pulBeginTime, 
	FTM_ULONG_PTR 	pulEndTime, 
	FTM_ULONG_PTR 	pulCount
);

#endif

