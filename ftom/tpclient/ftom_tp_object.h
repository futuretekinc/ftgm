#ifndef	__FTOM_TP_GATEWAY_H__
#define	__FTOM_TP_GATEWAY_H__

#include "ftm.h"
#include "ftm_list.h"


typedef	struct
{
	FTM_CHAR		pID[FTM_EPID_LEN+1];
	FTM_CHAR		pName[FTM_NAME_LEN+1];
	FTM_CHAR		pOwnerID[FTM_DID_LEN+1];
	FTM_CHAR		pDeviceID[FTM_DID_LEN+1];
	FTM_EP_TYPE		xType;
	FTM_ULONG		ulAddress;
	FTM_ULONG		ulSequence;
	FTM_UINT64		ullCTime;
	FTM_UINT64		ullMTime;
	FTM_ULONG		ulServerDataTime;

	struct
	{
		FTM_UINT64	ulStart;
		FTM_UINT64	ulEnd;
	}	xUnsyncDataTime;
}	FTOM_TP_SENSOR, _PTR_ FTOM_TP_SENSOR_PTR;

typedef	struct
{
	FTM_CHAR		pID[FTM_EPID_LEN+1];
	FTM_CHAR		pName[FTM_NAME_LEN+1];
	FTM_CHAR		pModel[FTM_NAME_LEN+1];
	FTM_CHAR		pOwnerID[FTM_DID_LEN+1];
	FTM_UINT64		ullCTime;
	FTM_UINT64		ullMTime;
}	FTOM_TP_DEVICE, _PTR_ FTOM_TP_DEVICE_PTR;

typedef	struct
{
	FTM_CHAR		pID[FTM_DID_LEN+1];
	FTM_CHAR		pName[FTM_NAME_LEN+1];
	FTM_ULONG		ulReportInterval;
	FTM_UINT64		ullCTime;
	FTM_UINT64		ullMTime;
	FTM_BOOL		bAutoCreateCoverable;

	FTM_LIST_PTR	pDeviceList;
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
