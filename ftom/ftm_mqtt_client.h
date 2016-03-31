#ifndef	_FTM_MQTT_CLIENT_H_
#define	_FTM_MQTT_CLIENT_H_

#include "ftm.h"
#include "ftm_om.h"
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

typedef	struct FTM_OM_STRUCT _PTR_ FTM_OM_PTR;

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

	FTM_OM_PTR				pOM;

	FTM_BOOL				bStop;
	FTM_BOOL				bConnected;
	pthread_t				xMain;
	pthread_t				xConnector;
	FTM_TIMER				xReconnectionTimer;

	FTM_CHAR				pDID[FTM_DID_LEN + 1];
	struct mosquitto		*pMosquitto;
	FTM_OM_MSG_QUEUE_PTR	pMsgQ;
	FTM_LIST_PTR			pPublishList;
	FTM_OM_SERVICE_ID		xServiceID;
	FTM_OM_SERVICE_CALLBACK	fServiceCB;
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
	FTM_EP_DATA_PTR		pData
);

typedef FTM_RET	 (*FTM_MQTT_CLIENT_PUBLISH_EP_DATA_INT)
(
	FTM_MQTT_CLIENT_PTR	pClient, 
	FTM_EP_ID 			xEPID, 
	FTM_ULONG 			ulTime, 
	FTM_INT 			nValue, 
	FTM_INT 			nAverage, 
	FTM_INT 			nCount, 
	FTM_INT 			nMax, 
	FTM_INT 			nMin
);

typedef FTM_RET	 (*FTM_MQTT_CLIENT_PUBLISH_EP_DATA_ULONG)
(
	FTM_MQTT_CLIENT_PTR pClient, 
	FTM_EP_ID 			xEPID, 
	FTM_ULONG 			ulTime, 
	FTM_ULONG 			ulValue,
	FTM_ULONG 			nAverage, 
	FTM_INT 			nCount, 
	FTM_ULONG 			ulMax, 
	FTM_ULONG 			ulMin
);

typedef FTM_RET	 (*FTM_MQTT_CLIENT_PUBLISH_EP_DATA_FLOAT)
(
	FTM_MQTT_CLIENT_PTR pClient, 
	FTM_EP_ID 			xEPID, 
	FTM_ULONG 			ulTime, 
	FTM_FLOAT 			fValue, 
	FTM_FLOAT 			fAverage, 
	FTM_INT 			nCount, 
	FTM_FLOAT 			fMax, 
	FTM_FLOAT 			fMin
);

typedef FTM_RET	 (*FTM_MQTT_CLIENT_PUBLISH_EP_DATA_BOOL)
(
	FTM_MQTT_CLIENT_PTR pClient, 
	FTM_EP_ID 			xEPID, 
	FTM_ULONG 			ulTime, 
	FTM_BOOL 			bValue
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
	FTM_MQTT_CLIENT_PUBLISH_EP_DATA_INT		fPublishEPDataINT;
	FTM_MQTT_CLIENT_PUBLISH_EP_DATA_ULONG	fPublishEPDataULONG;
	FTM_MQTT_CLIENT_PUBLISH_EP_DATA_FLOAT	fPublishEPDataFLOAT;
	FTM_MQTT_CLIENT_PUBLISH_EP_DATA_BOOL	fPublishEPDataBOOL;
}	FTM_MQTT_CLIENT_CALLBACK_SET, _PTR_ FTM_MQTT_CLIENT_CALLBACK_SET_PTR;

FTM_RET	FTM_MQTT_CLIENT_create
(
	FTM_OM_PTR 		pOM,
	FTM_MQTT_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTM_MQTT_CLIENT_destroy
(
	FTM_MQTT_CLIENT_PTR _PTR_ ppClient
);

FTM_RET	FTM_MQTT_CLIENT_init
(
	FTM_MQTT_CLIENT_PTR pClient,
	FTM_OM_PTR 			pOM
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
	FTM_OM_SERVICE_ID 		xID, 
	FTM_OM_SERVICE_CALLBACK fCB
);

FTM_RET	FTM_MQTT_CLIENT_notify
(
	FTM_MQTT_CLIENT_PTR 	pClient, 
	FTM_OM_MSG_PTR 			pMsg
);

FTM_RET	FTM_MQTT_CLIENT_publish
(
	FTM_MQTT_CLIENT_PTR 	pClient, 
	FTM_CHAR_PTR			pTopic,
	FTM_CHAR_PTR			pMessage,
	FTM_ULONG				ulMessageLen
);
#endif
