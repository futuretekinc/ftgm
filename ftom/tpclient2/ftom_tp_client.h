#ifndef	_FTOM_TP_CLIENT_H_
#define	_FTOM_TP_CLIENT_H_

#include "ftm.h"
#include "ftom.h"
#include <pthread.h>
#include "ftom_mqttc.h"
#include "ftom_tp_restapi.h"
#include "ftom_client.h"

#define	FTOM_TP_CLIENT_DEFAULT_BROKER				"dmqtt.thingplus.net"
#define	FTOM_TP_CLIENT_DEFAULT_PORT					8883
#define	FTOM_TP_CLIENT_DEFAULT_REPORT_INTERVAL		60
#define	FTOM_TP_CLIENT_DEFAULT_RETRY_INTERVAL		90

#define	FTOM_TP_CLIENT_DEFAULT_CB_SET				1

typedef	struct
{
	FTM_CHAR	pGatewayID[FTM_GWID_LEN+1];
	FTM_CHAR	pAPIKey[FTM_PASSWD_LEN+1];
	FTM_CHAR	pCertFile[FTM_FILE_NAME_LEN+1];
	FTM_CHAR	pUserID[FTM_USER_ID_LEN+1];
	FTM_CHAR	pPasswd[FTM_PASSWD_LEN+1];
	FTM_ULONG	ulReportInterval;

	struct
	{
		FTM_CHAR	pHost[FTM_URL_LEN+1];
		FTM_USHORT	usPort;
	}	xFTOMC;

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

	FTM_BOOL				bStop;
	FTM_BOOL				bConnected;
	pthread_t				xMain;
	pthread_t				xLinkManager;
	FTM_TIMER				xReportTimer;
	FTM_TIMER				xRetryTimer;

	FTOM_MSG_QUEUE			xMsgQ;
	FTOM_MQTT_CLIENT		xMQTT;
	FTOM_TP_RESTAPI			xRESTApi;

	FTOM_SERVICE_ID			xServiceID;
	FTOM_SERVICE_CB			fServiceCB;

	FTOM_CLIENT_PTR			pFTOMC;
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

FTM_RET	FTOM_TP_CLIENT_setConfig
(
	FTOM_TP_CLIENT_PTR 		pClient, 
	FTOM_TP_CLIENT_CONFIG_PTR 	pConfig
);

FTM_RET	FTOM_TP_CLIENT_loadConfig
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTM_CONFIG_PTR		pConfig
);

FTM_RET	FTOM_TP_CLIENT_loadConfigFromFile
(
	FTOM_TP_CLIENT_PTR pClient, 
	FTM_CHAR_PTR 		pFileName
);

FTM_RET	FTOM_TP_CLIENT_saveConfig
(
	FTOM_TP_CLIENT_PTR 	pClient, 
	FTM_CONFIG_PTR		pConfig
);

FTM_RET	FTOM_TP_CLIENT_saveConfigToFile
(
	FTOM_TP_CLIENT_PTR 	pClient, 
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

FTM_RET	FTOM_TP_CLIENT_waitingForFinished
(
	FTOM_TP_CLIENT_PTR 	pClient
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

FTM_RET	FTOM_TP_CLIENT_respose
(
	FTOM_TP_CLIENT_PTR	pClient,
	FTM_CHAR_PTR		pMsgID,
	FTM_INT				nErrorCode,
	FTM_CHAR_PTR		pMessage
);
#endif
