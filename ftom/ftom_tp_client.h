#ifndef	_FTOM_TP_CLIENT_H_
#define	_FTOM_TP_CLIENT_H_

#include "ftm.h"
#include "ftom.h"
#include <pthread.h>
#include "ftom_mqtt_client.h"
#include "ftom_tp_restapi.h"

#define	FTOM_TP_CLIENT_DEFAULT_BROKER				"dmqtt.thingplus.net"
#define	FTOM_TP_CLIENT_DEFAULT_PORT					8883
#define	FTOM_TP_CLIENT_DEFAULT_REPORT_INTERVAL		60
#define	FTOM_TP_CLIENT_DEFAULT_RETRY_INTERVAL		90

#define	FTOM_TP_CLIENT_DEFAULT_CB_SET				1

typedef	struct
{
	FTM_CHAR	pGatewayID[FTM_GWID_LEN+1];
	FTM_CHAR	pAPIKey[FTM_PASSWD_LEN+1];

	FTM_CHAR	pHost[FTM_HOST_LEN+1];
	FTM_UINT16	usPort;

	FTM_CHAR	pCertFile[FTM_FILE_NAME_LEN+1];

	FTM_ULONG	ulReportInterval;
	FTM_ULONG	ulRetryInterval;
}	FTOM_TP_CLIENT_CONFIG, _PTR_ FTOM_TP_CLIENT_CONFIG_PTR;

typedef	struct FTOM_TP_CLIENT_STRUCT
{
	FTOM_TP_CLIENT_CONFIG	xConfig;

	FTM_BOOL				bStop;
	FTM_BOOL				bConnected;
	pthread_t				xMain;
	pthread_t				xLinkManager;
	FTM_TIMER				xReconnectionTimer;

	FTOM_MSG_QUEUE			xMsgQ;
	FTOM_MQTT_CLIENT		xMQTT;
	FTOM_TP_RESTAPI			xRESTApi;

	FTOM_SERVICE_ID			xServiceID;
	FTOM_SERVICE_CALLBACK	fServiceCB;
}	FTOM_TP_CLIENT, _PTR_ FTOM_TP_CLIENT_PTR;

FTM_RET	FTOM_TP_CLIENT_create
(
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

FTM_RET	FTOM_TP_CLIENT_loadConfig
(
	FTOM_TP_CLIENT_PTR 		pClient, 
	FTOM_TP_CLIENT_CONFIG_PTR 	pConfig
);

FTM_RET	FTOM_TP_CLIENT_loadFromFile
(
	FTOM_TP_CLIENT_PTR pClient, 
	FTM_CHAR_PTR 		pFileName
);

FTM_RET	FTOM_TP_CLIENT_showConfig
(
	FTOM_TP_CLIENT_PTR pClient
);

FTM_RET	FTOM_TP_CLIENT_start
(
	FTOM_TP_CLIENT_PTR pClient
);

FTM_RET	FTOM_TP_CLIENT_stop
(
	FTOM_TP_CLIENT_PTR pClient
);

FTM_RET	FTOM_TP_CLIENT_isRun
(
	FTOM_TP_CLIENT_PTR pClient,
	FTM_BOOL_PTR		pbRun
);

FTM_RET	FTOM_TP_CLIENT_setCallback
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTOM_SERVICE_ID 		xID, 
	FTOM_SERVICE_CALLBACK fCB
);

FTM_RET	FTOM_TP_CLIENT_sendMessage
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTOM_MSG_PTR 			pMsg
);

FTM_RET	FTOM_TP_CLIENT_serverSyncStart
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_BOOL			bAutoRegister
);

FTM_RET	FTOM_TP_CLIENT_reportStatus
(
	FTOM_TP_CLIENT_PTR	pClient
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
#endif