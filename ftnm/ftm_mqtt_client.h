#ifndef	_FTM_MQTT_CLIENT_H_
#define	_FTM_MQTT_CLIENT_H_

#include "ftm.h"
#include "ftnm.h"
#include <pthread.h>

typedef	struct
{
	FTM_CHAR	pClientID[24];

	struct
	{
		FTM_CHAR	pHost[128];
		FTM_UINT16	usPort;
	}	xBroker;

	FTM_ULONG	ulReconnectionTime;
}	FTM_MQTT_CLIENT_CONFIG, _PTR_ FTM_MQTT_CLIENT_CONFIG_PTR;

typedef	struct
{
	FTM_MQTT_CLIENT_CONFIG	xConfig;
	FTM_BOOL				bStop;
	pthread_t				xMain;
	pthread_t				xConnector;
	struct mosquitto		*pMosquitto;
	FTM_MSG_QUEUE_PTR		pMsgQ;
	FTNM_INFO				xFTNMInfo;
}	FTM_MQTT_CLIENT, _PTR_ FTM_MQTT_CLIENT_PTR;

FTM_RET	FTM_MQTT_CLIENT_create(FTM_MQTT_CLIENT_PTR _PTR_ ppClient);
FTM_RET	FTM_MQTT_CLIENT_destroy(FTM_MQTT_CLIENT_PTR _PTR_ ppClient);

FTM_RET	FTM_MQTT_CLIENT_init(FTM_MQTT_CLIENT_PTR pClient);
FTM_RET	FTM_MQTT_CLIENT_final(FTM_MQTT_CLIENT_PTR pClient);

FTM_RET	FTM_MQTT_CLIENT_loadConfig(FTM_MQTT_CLIENT_PTR pClient, FTM_MQTT_CLIENT_CONFIG_PTR pConfig);
FTM_RET	FTM_MQTT_CLIENT_loadFromFile(FTM_MQTT_CLIENT_PTR pClient, FTM_CHAR_PTR pFileName);

FTM_RET	FTM_MQTT_CLIENT_start(FTM_MQTT_CLIENT_PTR pClient);
FTM_RET	FTM_MQTT_CLIENT_stop(FTM_MQTT_CLIENT_PTR pClient);

#endif
