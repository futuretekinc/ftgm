#ifndef	__FTOM_DMC_H__
#define	__FTOM_DMC_H__

#include "ftm.h"
#include "ftdm_client.h"
#include "ftom_msg.h"
#include "ftom_service.h"

typedef	struct	FTOM_STRUCT _PTR_ FTOM_PTR;

typedef	struct
{
	FTOM_PTR				pOM;
	FTDMC_SESSION			xSession;
	FTDMC_CFG				xConfig;
	pthread_t				xThread;
	FTM_BOOL				bStop;
	FTOM_MSG_QUEUE		xMsgQ;
	FTOM_SERVICE_ID		xServiceID;
	FTOM_SERVICE_CALLBACK	fServiceCB;
}	FTOM_DMC, _PTR_	FTOM_DMC_PTR;

FTM_RET	FTOM_DMC_create
(
	FTOM_PTR pOM,
	FTOM_DMC_PTR _PTR_ 	ppDMC
);

FTM_RET FTOM_DMC_init
(
	FTOM_DMC_PTR 	pDMC,
	FTOM_PTR 		pOM 
);

FTM_RET FTOM_DMC_final
(
	FTOM_DMC_PTR pDMC
);

FTM_RET FTOM_DMC_loadFromFile
(
	FTOM_DMC_PTR 	pDMC, 
	FTM_CHAR_PTR 	pConfigFileName
);

FTM_RET	FTOM_DMC_showConfig
(
	FTOM_DMC_PTR 	pDMC
);

FTM_RET	FTOM_DMC_start
(
	FTOM_DMC_PTR 	pDMC
);

FTM_RET	FTOM_DMC_stop
(
	FTOM_DMC_PTR 	pDMC
);

FTM_RET	FTOM_DMC_setServiceCallback
(
	FTOM_DMC_PTR 			pDMC, 
	FTOM_SERVICE_ID 		xID, 
	FTOM_SERVICE_CALLBACK pServiceCB
);

FTM_RET	FTOM_DMC_notify
(
	FTOM_DMC_PTR 	pDMC, 
	FTOM_MSG_PTR 	pMsg
);

FTM_RET	FTOM_DMC_EP_create
(
	FTOM_DMC_PTR 	pDMC, 
	FTM_EP_PTR 		pInfo
);

FTM_RET	FTOM_DMC_EP_destroy
(
	FTOM_DMC_PTR 	pDMC, 
	FTM_CHAR_PTR	pEPID
);

FTM_RET	FTOM_DMC_setEP
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pEPID,
	FTM_EP_PTR		pInfo
);

FTM_RET FTOM_DMC_getEPData
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulStartIndex,
	FTM_EP_DATA_PTR	pData,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTOM_DMC_appendEPData
(
	FTOM_DMC_PTR 	pDMC, 
	FTM_CHAR_PTR	pEPID,
	FTM_EP_DATA_PTR pData
);

FTM_RET	FTOM_DMC_EP_DATA_del
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulIndex,
	FTM_ULONG		ulCount,
	FTM_ULONG_PTR	pulDeletedCount
);

FTM_RET	FTOM_DMC_EP_DATA_delWithTime
(
	FTOM_DMC_PTR	pDMC,
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG		ulBegin,
	FTM_ULONG		ulEnd,
	FTM_ULONG_PTR	pulDeletedCount
);

FTM_RET	FTOM_DMC_EP_DATA_count
(
	FTOM_DMC_PTR 	pDMC, 
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTOM_DMC_EP_DATA_info
(
	FTOM_DMC_PTR 	pDMC, 
	FTM_CHAR_PTR	pEPID,
	FTM_ULONG_PTR 	pulBeginTime, 
	FTM_ULONG_PTR 	pulEndTime, 
	FTM_ULONG_PTR 	pulCount
);

#endif

