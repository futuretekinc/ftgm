#ifndef	_FTM_MQTT_CLIENT_H_
#define	_FTM_MQTT_CLIENT_H_

#include "ftm.h"
#include "ftom.h"
#include <pthread.h>
#include <mosquitto.h>

#define	FTM_MQTT_CLIENT_TOPIC_LENGTH				128
#define	FTM_MQTT_CLIENT_MESSAGE_LENGTH				1024
#define	FTM_MQTT_CLIENT_DEFAULT_BROKER				"127.0.0.1"
#define	FTM_MQTT_CLIENT_DEFAULT_PORT				1883
#define	FTM_MQTT_CLIENT_DEFAULT_RECONNECTION_TIME	5

#define	FTM_MQTT_METHOD_REQ_TIME_SYNC				1
#define	FTM_MQTT_METHOD_REQ_CONTROL_ACTUATOR		2
#define	FTM_MQTT_METHOD_REQ_SET_PROPERTY			3
#define	FTM_MQTT_METHOD_REQ_POWER_OFF				4
#define	FTM_MQTT_METHOD_REQ_REBOOT					5
#define	FTM_MQTT_METHOD_REQ_RESTART					6
#define	FTM_MQTT_METHOD_REQ_SW_UPDATE				7
#define	FTM_MQTT_METHOD_REQ_SW_INFO					8

#define	FTM_MQTT_CLIENT_DEFAULT_CB_SET				1

typedef	struct FTOM_STRUCT _PTR_ FTOM_PTR;

typedef	struct
{
	FTM_CHAR	pClientID[24];

	struct
	{
		FTM_CHAR	pHost[128];
		FTM_UINT16	usPort;
	}	xBroker;

	FTM_ULONG	ulReconnectionTime;
	FTM_ULONG	ulCBSet;
}	FTM_MQTT_CLIENT_CONFIG, _PTR_ FTM_MQTT_CLIENT_CONFIG_PTR;

typedef	struct
{
	FTM_MQTT_CLIENT_CONFIG	xConfig;

	FTOM_PTR				pOM;

	FTM_BOOL				bStop;
	FTM_BOOL				bConnected;
	pthread_t				xMain;
	pthread_t				xConnector;
	FTM_TIMER				xReconnectionTimer;

	FTM_CHAR				pDID[FTM_DID_LEN + 1];
	struct mosquitto		*pMosquitto;
	FTOM_MSG_QUEUE_PTR	pMsgQ;
	FTM_LIST_PTR			pPublishList;
	FTOM_SERVICE_ID		xServiceID;
	FTOM_SERVICE_CALLBACK	fServiceCB;
}	FTM_MQTT_CLIENT, _PTR_ FTM_MQTT_CLIENT_PTR;

typedef	FTM_VOID (*FTM_MQTT_CLIENT_CONNECT_CB)(struct mosquitto *mosq, void *pObj, int nResult);
typedef	FTM_VOID (*FTM_MQTT_CLIENT_DISCONNECT_CB)(struct mosquitto *mosq, void *pObj, int nResult);
typedef	FTM_VOID (*FTM_MQTT_CLIENT_PUBLISH_CB)(struct mosquitto *mosq, void *pObj, int nResult);
typedef	FTM_VOID (*FTM_MQTT_CLIENT_MESSAGE_CB)(struct mosquitto *mosq, void *pObj, const struct mosquitto_message *message);
typedef FTM_VOID (*FTM_MQTT_CLIENT_SUBSCRIBE_CB)(struct mosquitto *mosq, void *pObj, int nMID, int nQoS, const int *pGrantedQoS);
typedef FTM_VOID (*FTM_MQTT_CLIENT_TIMER_CB)(struct mosquitto *mosq, void *pObj);

typedef FTM_RET	 (*FTM_MQTT_CLIENT_PUBLISH_EP_DATA)
(
	FTM_MQTT_CLIENT_PTR	pClient, 
	FTM_EP_ID 			xEPID, 
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount
);

typedef	struct
{
	FTM_MQTT_CLIENT_CONNECT_CB				fConnect;
	FTM_MQTT_CLIENT_DISCONNECT_CB			fDisconnect;
	FTM_MQTT_CLIENT_PUBLISH_CB				fPublish;
	FTM_MQTT_CLIENT_MESSAGE_CB				fMessage;
	FTM_MQTT_CLIENT_SUBSCRIBE_CB			fSubscribe;
	FTM_MQTT_CLIENT_TIMER_CB				fTimer;
	FTM_MQTT_CLIENT_PUBLISH_EP_DATA			fPublishEPData;
}	FTM_MQTT_CLIENT_CALLBACK_SET, _PTR_ FTM_MQTT_CLIENT_CALLBACK_SET_PTR;

FTM_RET	FTM_MQTT_CLIENT_create
(
	FTOM_PTR 		pOM,
	FTM_MQTT_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTM_MQTT_CLIENT_destroy
(
	FTM_MQTT_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTM_MQTT_CLIENT_init
(
	FTM_MQTT_CLIENT_PTR pClient,
	FTOM_PTR 			pOM
);

FTM_RET	FTM_MQTT_CLIENT_final
(
	FTM_MQTT_CLIENT_PTR pClient
);

FTM_RET	FTM_MQTT_CLIENT_loadConfig
(
	FTM_MQTT_CLIENT_PTR 		pClient, 
	FTM_MQTT_CLIENT_CONFIG_PTR 	pConfig
);

FTM_RET	FTM_MQTT_CLIENT_loadFromFile
(
	FTM_MQTT_CLIENT_PTR pClient, 
	FTM_CHAR_PTR 		pFileName
);

FTM_RET	FTM_MQTT_CLIENT_showConfig
(
	FTM_MQTT_CLIENT_PTR pClient
);

FTM_RET	FTM_MQTT_CLIENT_start
(
	FTM_MQTT_CLIENT_PTR pClient
);

FTM_RET	FTM_MQTT_CLIENT_stop
(
	FTM_MQTT_CLIENT_PTR pClient
);

FTM_RET	FTM_MQTT_CLIENT_setCallback
(
	FTM_MQTT_CLIENT_PTR 	pClient, 
	FTOM_SERVICE_ID 		xID, 
	FTOM_SERVICE_CALLBACK fCB
);

FTM_RET	FTM_MQTT_CLIENT_notify
(
	FTM_MQTT_CLIENT_PTR 	pClient, 
	FTOM_MSG_PTR 			pMsg
);

FTM_RET	FTM_MQTT_CLIENT_publish
(
	FTM_MQTT_CLIENT_PTR 	pClient, 
	FTM_CHAR_PTR			pTopic,
	FTM_CHAR_PTR			pMessage,
	FTM_ULONG				ulMessageLen
);

FTM_RET	FTM_MQTT_CLIENT_publishEPData
(
	FTM_MQTT_CLIENT_PTR pClient,
	FTM_EP_ID			xEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount
);

FTM_RET	FTM_MQTT_CLIENT_pushMsg
(
	FTM_MQTT_CLIENT_PTR pClient,
	FTOM_MSG_PTR		pMsg	
);

#endif
