#ifndef	_FTOM_MQTT_CLIENT_H_
#define	_FTOM_MQTT_CLIENT_H_

#include "ftm.h"
#include "ftom.h"
#include <pthread.h>
#include <mosquitto.h>

#define	FTOM_MQTT_CLIENT_TOPIC_LENGTH				128
#define	FTOM_MQTT_CLIENT_MESSAGE_LENGTH				1024
#define	FTOM_MQTT_CLIENT_DEFAULT_BROKER				"127.0.0.1"
#define	FTOM_MQTT_CLIENT_DEFAULT_PORT				1883
#define	FTOM_MQTT_CLIENT_DEFAULT_RECONNECTION_TIME	5

#define	FTOM_MQTT_METHOD_REQ_TIME_SYNC				1
#define	FTOM_MQTT_METHOD_REQ_CONTROL_ACTUATOR		2
#define	FTOM_MQTT_METHOD_REQ_SET_PROPERTY			3
#define	FTOM_MQTT_METHOD_REQ_POWER_OFF				4
#define	FTOM_MQTT_METHOD_REQ_REBOOT					5
#define	FTOM_MQTT_METHOD_REQ_RESTART					6
#define	FTOM_MQTT_METHOD_REQ_SW_UPDATE				7
#define	FTOM_MQTT_METHOD_REQ_SW_INFO					8

#define	FTOM_MQTT_CLIENT_DEFAULT_CB_SET				1

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
}	FTOM_MQTT_CLIENT_CONFIG, _PTR_ FTOM_MQTT_CLIENT_CONFIG_PTR;

typedef	struct
{
	FTOM_MQTT_CLIENT_CONFIG	xConfig;

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
}	FTOM_MQTT_CLIENT, _PTR_ FTOM_MQTT_CLIENT_PTR;

typedef	FTM_VOID (*FTOM_MQTT_CLIENT_CONNECT_CB)(struct mosquitto *mosq, void *pObj, int nResult);
typedef	FTM_VOID (*FTOM_MQTT_CLIENT_DISCONNECT_CB)(struct mosquitto *mosq, void *pObj, int nResult);
typedef	FTM_VOID (*FTOM_MQTT_CLIENT_PUBLISH_CB)(struct mosquitto *mosq, void *pObj, int nResult);
typedef	FTM_VOID (*FTOM_MQTT_CLIENT_MESSAGE_CB)(struct mosquitto *mosq, void *pObj, const struct mosquitto_message *message);
typedef FTM_VOID (*FTOM_MQTT_CLIENT_SUBSCRIBE_CB)(struct mosquitto *mosq, void *pObj, int nMID, int nQoS, const int *pGrantedQoS);
typedef FTM_VOID (*FTOM_MQTT_CLIENT_TIMER_CB)(struct mosquitto *mosq, void *pObj);

typedef FTM_RET	 (*FTOM_MQTT_CLIENT_PUBLISH_EP_DATA)
(
	FTOM_MQTT_CLIENT_PTR	pClient, 
	FTM_CHAR_PTR			pEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount
);

typedef	struct
{
	FTOM_MQTT_CLIENT_CONNECT_CB				fConnect;
	FTOM_MQTT_CLIENT_DISCONNECT_CB			fDisconnect;
	FTOM_MQTT_CLIENT_PUBLISH_CB				fPublish;
	FTOM_MQTT_CLIENT_MESSAGE_CB				fMessage;
	FTOM_MQTT_CLIENT_SUBSCRIBE_CB			fSubscribe;
	FTOM_MQTT_CLIENT_TIMER_CB				fTimer;
	FTOM_MQTT_CLIENT_PUBLISH_EP_DATA			fPublishEPData;
}	FTOM_MQTT_CLIENT_CALLBACK_SET, _PTR_ FTOM_MQTT_CLIENT_CALLBACK_SET_PTR;

FTM_RET	FTOM_MQTT_CLIENT_create
(
	FTOM_PTR 		pOM,
	FTOM_MQTT_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTOM_MQTT_CLIENT_destroy
(
	FTOM_MQTT_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTOM_MQTT_CLIENT_init
(
	FTOM_MQTT_CLIENT_PTR pClient,
	FTOM_PTR 			pOM
);

FTM_RET	FTOM_MQTT_CLIENT_final
(
	FTOM_MQTT_CLIENT_PTR pClient
);

FTM_RET	FTOM_MQTT_CLIENT_loadConfig
(
	FTOM_MQTT_CLIENT_PTR 		pClient, 
	FTOM_MQTT_CLIENT_CONFIG_PTR 	pConfig
);

FTM_RET	FTOM_MQTT_CLIENT_loadFromFile
(
	FTOM_MQTT_CLIENT_PTR pClient, 
	FTM_CHAR_PTR 		pFileName
);

FTM_RET	FTOM_MQTT_CLIENT_showConfig
(
	FTOM_MQTT_CLIENT_PTR pClient
);

FTM_RET	FTOM_MQTT_CLIENT_start
(
	FTOM_MQTT_CLIENT_PTR pClient
);

FTM_RET	FTOM_MQTT_CLIENT_stop
(
	FTOM_MQTT_CLIENT_PTR pClient
);

FTM_RET	FTOM_MQTT_CLIENT_setCallback
(
	FTOM_MQTT_CLIENT_PTR 	pClient, 
	FTOM_SERVICE_ID 		xID, 
	FTOM_SERVICE_CALLBACK fCB
);

FTM_RET	FTOM_MQTT_CLIENT_notify
(
	FTOM_MQTT_CLIENT_PTR 	pClient, 
	FTOM_MSG_PTR 			pMsg
);

FTM_RET	FTOM_MQTT_CLIENT_publish
(
	FTOM_MQTT_CLIENT_PTR 	pClient, 
	FTM_CHAR_PTR			pTopic,
	FTM_CHAR_PTR			pMessage,
	FTM_ULONG				ulMessageLen
);

FTM_RET	FTOM_MQTT_CLIENT_publishEPData
(
	FTOM_MQTT_CLIENT_PTR pClient,
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR		pData,
	FTM_ULONG			ulCount
);

FTM_RET	FTOM_MQTT_CLIENT_pushMsg
(
	FTOM_MQTT_CLIENT_PTR pClient,
	FTOM_MSG_PTR		pMsg	
);

#endif
