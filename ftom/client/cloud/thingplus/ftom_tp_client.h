#ifndef	_FTOM_TP_CLIENT_H_
#define	_FTOM_TP_CLIENT_H_

#include "ftm.h"
#include "ftom.h"
#include <pthread.h>
#include "ftom_mqtt_client.h"
#include "ftom_tp_restapi.h"
#include "ftom_client.h"

#define	FTOM_TP_CLIENT_DEFAULT_BROKER				"dmqtt.thingplus.net"
#define	FTOM_TP_CLIENT_DEFAULT_PORT					8883
#define	FTOM_TP_CLIENT_DEFAULT_REPORT_INTERVAL		60
#define	FTOM_TP_CLIENT_DEFAULT_RETRY_INTERVAL		90

#define	FTOM_TP_CLIENT_DEFAULT_CB_SET				1

typedef	struct
{
	FTM_CHAR	pName[FTM_NAME_LEN + 1];
	FTM_CHAR	pGatewayID[FTM_GWID_LEN+1];
	FTM_CHAR	pAPIKey[FTM_PASSWD_LEN+1];
	FTM_CHAR	pCertFile[FTM_FILE_NAME_LEN+1];
	FTM_CHAR	pUserID[FTM_USER_ID_LEN+1];
	FTM_CHAR	pPasswd[FTM_PASSWD_LEN+1];
	FTM_ULONG	ulReportInterval;

	struct
	{
		FTM_CHAR	pHost[FTM_HOST_LEN+1];
		FTM_UINT16	usPort;
	
		FTM_BOOL	bSecure;
	
		FTM_ULONG	ulRetryInterval;
	}	xMQTT;

	struct	
	{
		FTM_CHAR	pBaseURL[FTM_URL_LEN+1];
	
		FTM_BOOL	bSecure;
	}	xRESTApi;


}	FTOM_TP_CLIENT_CONFIG, _PTR_ FTOM_TP_CLIENT_CONFIG_PTR;

typedef	struct FTOM_TP_CLIENT_STRUCT
{
	FTOM_TP_CLIENT_CONFIG	xConfig;

	pthread_t				xThreadMain;

	FTOM_CLIENT_NOTIFY_CB	fNotifyCB;
	FTM_VOID_PTR			pNotifyData;

	FTOM_MSG_QUEUE_PTR		pMsgQ;

	FTM_BOOL				bStop;
	FTM_BOOL				bConnected;
	FTM_BOOL				bReportON;

	FTM_TIMER				xReportTimer;
	FTM_TIMER				xRetryTimer;
	FTM_TIMER				xServerSyncTimer;

	FTOM_TP_GATEWAY_PTR		pGateway;
	FTM_LOCK_PTR			pGatewayLock;

	FTOM_MQTT_CLIENT		xMQTT;
	FTOM_TP_RESTAPI			xRESTApi;
}	FTOM_TP_CLIENT, _PTR_ FTOM_TP_CLIENT_PTR;

FTM_RET	FTOM_TP_CLIENT_create
(
	FTM_CHAR_PTR	pName,
	FTOM_TP_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTOM_TP_CLIENT_destroy
(
	FTOM_TP_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTOM_TP_CLIENT_init
(
	FTOM_TP_CLIENT_PTR pClient
);

FTM_RET	FTOM_TP_CLIENT_final
(
	FTOM_TP_CLIENT_PTR pClient
);

FTM_RET	FTOM_TP_CLIENT_CONFIG_load
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTM_CONFIG_PTR		pConfig
);

FTM_RET	FTOM_TP_CLIENT_CONFIG_save
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTM_CONFIG_PTR		pConfig
);

FTM_RET	FTOM_TP_CLIENT_CONFIG_show
(
	FTOM_TP_CLIENT_PTR pClient
);

FTM_RET	FTOM_TP_CLIENT_setNotifyCB
(
	FTOM_TP_CLIENT_PTR pClient,
	FTOM_CLIENT_NOTIFY_CB	fNotifyCB,
	FTM_VOID_PTR		pNotidyData
);

FTM_RET	FTOM_TP_CLIENT_MESSAGE_send
(
	FTOM_TP_CLIENT_PTR pClient,
	FTOM_MSG_PTR		pMsg
);

FTM_RET	FTOM_TP_CLIENT_start
(
	FTOM_TP_CLIENT_PTR pClient
);

FTM_RET	FTOM_TP_CLIENT_stop
(
	FTOM_TP_CLIENT_PTR pClient
);

FTM_RET	FTOM_TP_CLIENT_isRunning
(
	FTOM_TP_CLIENT_PTR pClient,
	FTM_BOOL_PTR		pbRun
);

FTM_RET	FTOM_TP_CLIENT_waitingForFinished
(
	FTOM_TP_CLIENT_PTR 	pClient
);

FTM_RET	FTOM_TP_CLIENT_MESSAGE_send
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTOM_MSG_PTR 			pMsg
);

FTM_RET	FTOM_TP_CLIENT_MESSAGE_process
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTOM_MSG_PTR		pBaseMsg
);

FTM_RET	FTOM_TP_CLIENT_GATEWAY_sync
(
	FTOM_TP_CLIENT_PTR	pClient
);

FTM_RET	FTOM_TP_CLIENT_GATEWAY_updateStatus
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_BOOL			bStatus
);

FTM_RET	FTOM_TP_CLIENT_NODE_getCount
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_ULONG_PTR		pulCount
);

FTM_RET	FTOM_TP_CLIENT_NODE_register
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_NODE_PTR		pNode
);

FTM_RET	FTOM_TP_CLIENT_NODE_isExist
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pID
);

FTM_RET	FTOM_TP_CLIENT_EP_getCount
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_ULONG_PTR		pulCount
);

FTM_RET	FTOM_TP_CLIENT_EP_getEPIDAt
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_ULONG			ulIndex,
	FTM_CHAR_PTR		pBuff,
	FTM_ULONG			ulBuffLen
);

FTM_RET	FTOM_TP_CLIENT_EP_register
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_EP_PTR			pEP
);

FTM_RET	FTOM_TP_CLIENT_EP_isExist
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pEPID
);

FTM_RET	FTOM_TP_CLIENT_EP_updateStatus
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pEPID,
	FTM_BOOL			bStatus
);

FTM_RET	FTOM_TP_CLIENT_EP_DATA_getLastTime
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pEPID,
	FTM_ULONG_PTR		pulLastTime
);

FTM_RET	FTOM_TP_CLIENT_EP_DATA_send
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pDatas,
	FTM_ULONG			ulCount
);

FTM_RET	FTOM_TP_CLIENT_DISCOVERY_getEPInfoCount
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_EP_PTR			pEPInfos,
	FTM_ULONG_PTR		pulCount
);

FTM_RET	FTOM_TP_CLIENT_serverSyncStart
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_BOOL			bAutoRegister
);

FTM_RET	FTOM_TP_CLIENT_reportGWStatus
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pGatewayID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout
);

FTM_RET	FTOM_TP_CLIENT_sendEPStatus
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pEPID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout
);

FTM_RET	FTOM_TP_CLIENT_sendEPData
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pDatas, 
	FTM_ULONG			ulCount
);

FTM_RET	FTOM_TP_CLIENT_respose
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pMsgID,
	FTM_INT				nErrorCode,
	FTM_CHAR_PTR		pMessage
);
#endif