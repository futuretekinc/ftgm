#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <mosquitto.h>
#include "ftm_mqtt_client.h"

static FTM_VOID FTM_MQTT_CLIENT_connectCB(struct mosquitto *mosq, void *obj, int result);
static FTM_VOID	FTM_MQTT_CLIENT_messageCB(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message);
static FTM_VOID_PTR FTM_MQTT_CLIENT_process(FTM_VOID_PTR pData);
static FTM_VOID_PTR FTM_MQTT_CLIENT_receiver(FTM_VOID_PTR pData);

FTM_RET	FTM_MQTT_CLIENT_init(FTM_MQTT_CLIENT_PTR pClient)
{
	ASSERT(pClient != NULL);

	memset(pClient, 0, sizeof(FTM_MQTT_CLIENT));

	return	FTM_RET_OK;
}

FTM_RET	FTM_MQTT_CLIENT_final(FTM_MQTT_CLIENT_PTR pClient)
{
	ASSERT(pClient != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTM_MQTT_CLIENT_loadConfig(FTM_MQTT_CLIENT_PTR pClient, FTM_MQTT_CLIENT_CONFIG_PTR pConfig)
{
	ASSERT(pClient != NULL);
	ASSERT(pConfig != NULL);

	memcpy(&pClient->xConfig, pConfig, sizeof(FTM_MQTT_CLIENT_CONFIG));

	return	FTM_RET_OK;
}


FTM_RET	FTM_MQTT_CLIENT_loadFromFile(FTM_MQTT_CLIENT_PTR pClient, FTM_CHAR_PTR pFileName)
{
	ASSERT(pClient != NULL);
	ASSERT(pFileName != NULL);

	return	FTM_RET_OK;
}

FTM_RET	FTM_MQTT_CLIENT_start(FTM_MQTT_CLIENT_PTR pClient)
{
	ASSERT(pClient != NULL);

	FTM_MSGQ_create(&pClient->pMsgQ);

	pthread_create(&pClient->xMain, NULL, FTM_MQTT_CLIENT_process, pClient);

	return	FTM_RET_OK;
}

FTM_RET	FTM_MQTT_CLIENT_stop(FTM_MQTT_CLIENT_PTR pClient)
{
	pClient->bStop = FTM_TRUE;

	pthread_join(pClient->xMain, NULL);

	FTM_MSGQ_destroy(pClient->pMsgQ);
	pClient->pMsgQ = NULL;

	return	FTM_RET_OK;
}

FTM_VOID_PTR FTM_MQTT_CLIENT_process(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);

	FTM_MQTT_CLIENT_PTR	pClient = (FTM_MQTT_CLIENT_PTR)pData;

	TRACE("CLIENT main started.\n");

	mosquitto_lib_init();

	pClient->pMosquitto = mosquitto_new(pClient->xConfig.pClientID, true, NULL);
	if(pClient->pMosquitto)
	{
		mosquitto_connect_callback_set(pClient->pMosquitto, FTM_MQTT_CLIENT_connectCB);
		mosquitto_message_callback_set(pClient->pMosquitto, FTM_MQTT_CLIENT_messageCB);

		pthread_create(&pClient->xReceiver, NULL, FTM_MQTT_CLIENT_receiver, pClient);

		while(!pClient->bStop)
		{
			usleep(1000);
		}

		pthread_join(pClient->xReceiver, NULL);

	}

	if (pClient->pMosquitto != NULL)
	{
		mosquitto_destroy(pClient->pMosquitto);
		pClient->pMosquitto = NULL;
	}

	mosquitto_lib_cleanup();

	TRACE("CLIENT main stopped.\n");

	return 0;
}

FTM_VOID_PTR FTM_MQTT_CLIENT_receiver(FTM_VOID_PTR pData)
{
	ASSERT(pData != NULL);

	FTM_MQTT_CLIENT_PTR	pClient = (FTM_MQTT_CLIENT_PTR)pData;
	FTM_INT	nRet;


	TRACE("CLIENT receiver started.\n");
	mosquitto_connect(pClient->pMosquitto, pClient->xConfig.xBroker.pHost, pClient->xConfig.xBroker.usPort, 60);
	mosquitto_subscribe(pClient->pMosquitto, NULL, "#", 0);

	while(!pClient->bStop)
	{
		nRet = mosquitto_loop(pClient->pMosquitto, -1, 1);
		if(!pClient->bStop && nRet)
		{
			sleep(20);
			mosquitto_reconnect(pClient->pMosquitto);
		}
	}
	TRACE("CLIENT receiver stopped.\n");

	return 0;
}

FTM_VOID FTM_MQTT_CLIENT_connectCB(struct mosquitto *mosq, void *obj, int result)
{
}

FTM_VOID FTM_MQTT_CLIENT_messageCB(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
{
	MESSAGE("message->topic = %s\n", message->topic);
	MESSAGE("message->payload= %s\n", (char *)message->payload);
}

