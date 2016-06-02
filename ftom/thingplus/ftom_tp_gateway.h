#ifndef	__FTOM_TP_GATEWAY_H__
#define	__FTOM_TP_GATEWAY_H__

#include "ftm.h"
#include "ftm_list.h"


typedef	struct
{
	FTM_CHAR		pID[FTM_DID_LEN+1];
	FTM_CHAR		pName[FTM_NAME_LEN+1];
	FTM_ULONG		ulReportInterval;
	FTM_ULONG		ulCTime;
	FTM_ULONG		ulMTime;

	FTM_LIST_PTR	pSensorList;
}	FTOM_TP_GATEWAY, _PTR_ FTOM_TP_GATEWAY_PTR;

FTM_RET	FTOM_TP_GATEWAY_create
(
	FTOM_TP_GATEWAY_PTR _PTR_ ppGateway
);

FTM_RET	FTOM_TP_GATEWAY_destroy
(
	FTOM_TP_GATEWAY_PTR _PTR_ ppGateway
);

FTM_RET	FTOM_TP_GATEWAY_init
(
	FTOM_TP_GATEWAY_PTR	pGateway
);

FTM_RET	FTOM_TP_GATEWAY_final
(
	FTOM_TP_GATEWAY_PTR pGateway
);

#endif
