#ifndef	__FTNM_DMC_H__
#define	__FTNM_DMC_H__

#include "ftm.h"
#include "ftdm_client.h"
#include "ftnm_msg.h"
#include "ftnm_service.h"

typedef	struct	FTNM_CONTEXT_STRUCT _PTR_ FTNM_CONTEXT_PTR;

typedef	struct
{
	FTNM_CONTEXT_PTR		pCTX;
	FTDMC_SESSION			xSession;
	FTDMC_CFG				xConfig;
	pthread_t				xThread;
	FTM_BOOL				bStop;
	FTNM_MSG_QUEUE			xMsgQ;
	FTNM_SERVICE_ID			xServiceID;
	FTNM_SERVICE_CALLBACK	fServiceCB;
}	FTNM_DMC, _PTR_	FTNM_DMC_PTR;

FTM_RET FTNM_DMC_init(FTNM_CONTEXT_PTR pCTX, FTNM_DMC_PTR pDMC);
FTM_RET FTNM_DMC_final(FTNM_DMC_PTR pDMC);

FTM_RET FTNM_DMC_loadFromFile(FTNM_DMC_PTR pDMC, FTM_CHAR_PTR pConfigFileName);
FTM_RET	FTNM_DMC_showConfig(FTNM_DMC_PTR pDMC);

FTM_RET	FTNM_DMC_start(FTNM_DMC_PTR pDMC);
FTM_RET	FTNM_DMC_stop(FTNM_DMC_PTR pDMC);

FTM_RET	FTNM_DMC_setServiceCallback(FTNM_DMC_PTR pDMC, FTNM_SERVICE_ID xID, FTNM_SERVICE_CALLBACK pServiceCB);

FTM_RET	FTNM_DMC_EP_create(FTNM_DMC_PTR pDMC, FTM_EP_PTR pInfo);
FTM_RET	FTNM_DMC_EP_destroy(FTNM_DMC_PTR pDMC, FTM_EP_ID xEPID);

FTM_RET FTNM_DMC_EP_DATA_get
(
	FTNM_DMC_PTR	pDMC,
	FTM_EP_ID 		xEPID, 
	FTM_ULONG		ulStartIndex,
	FTM_EP_DATA_PTR	pData,
	FTM_ULONG		ulMaxCount,
	FTM_ULONG_PTR 	pulCount
);

FTM_RET	FTNM_DMC_EP_DATA_set(FTNM_DMC_PTR pDMC, FTM_EP_ID xEPID, FTM_EP_DATA_PTR pData);
FTM_RET FTNM_DMC_EP_DATA_setINT(FTNM_DMC_PTR pDMC, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_INT nValue);
FTM_RET FTNM_DMC_EP_DATA_setULONG(FTNM_DMC_PTR pDMC, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_ULONG ulValue);
FTM_RET FTNM_DMC_EP_DATA_setFLOAT(FTNM_DMC_PTR pDMC, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_DOUBLE fValue);
FTM_RET	FTNM_DMC_EP_DATA_count(FTNM_DMC_PTR pDMC, FTM_EP_ID xEPID, FTM_ULONG_PTR pulCount);
FTM_RET	FTNM_DMC_EP_DATA_info(FTNM_DMC_PTR pDMC, FTM_EP_ID xEPID, FTM_ULONG_PTR pulBeginTime, FTM_ULONG_PTR pulEndTime, FTM_ULONG_PTR pulCount);

#endif

