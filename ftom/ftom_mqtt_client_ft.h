#ifndef	_FTOM_MQTT_CLIENT_FT_H_
#define	_FTOM_MQTT_CLIENT_FT_H_

#include "ftm.h"
#include "ftom.h"
#include "ftom_mqtt_client.h"

FTM_VOID FTOM_MQTT_CLIENT_FT_connectCB
(
	struct mosquitto 	*mosq, 
	void 				*pObj, 
	int 				nResult
);

FTM_VOID FTOM_MQTT_CLIENT_FT_disconnectCB
(
	struct mosquitto 	*mosq, 
	void 				*pObj, 
	int 				nResult
);

FTM_VOID FTOM_MQTT_CLIENT_FT_publishCB
(
	struct mosquitto 	*mosq, 
	void 				*pObj, 
	int 				nResult
);

FTM_VOID FTOM_MQTT_CLIENT_FT_messageCB
(
	struct mosquitto 	*mosq, 
	void 				*pObj, 
	const struct mosquitto_message *message
);

FTM_VOID FTOM_MQTT_CLIENT_FT_subscribeCB
(
	struct mosquitto 	*mosq, 
	void 				*pObj, 
	int 				nMID, 
	int 				nQoS, 
	const int 			*pGrantedQoS
);

FTM_RET	FTOM_MQTT_CLIENT_FT_topicParser
(
	FTM_CHAR_PTR		pTopic, 
	FTM_CHAR_PTR		pArgv[], 
	FTM_INT 			nMaxArgc, 
	FTM_INT_PTR			pnArgc
);

FTM_RET	FTOM_MQTT_CLIENT_FT_requestMessageParser
(
	FTM_CHAR_PTR 		pMessage, 
	FTOM_MSG_PTR _PTR_	pMsg
);

FTM_RET	FTOM_MQTT_CLIENT_FT_publishEPDataINT
(
	FTOM_MQTT_CLIENT_PTR pClient, 
	FTM_EP_ID 			xEPID, 
	FTM_ULONG 			ulTime, 
	FTM_INT 			nValue, 
	FTM_INT 			nAverage, 
	FTM_INT 			nCount, 
	FTM_INT 			nMax, 
	FTM_INT 			nMin
);

FTM_RET	FTOM_MQTT_CLIENT_FT_publishEPDataULONG
(
	FTOM_MQTT_CLIENT_PTR pClient, 
	FTM_EP_ID 			xEPID, 
	FTM_ULONG 			ulTime, 
	FTM_ULONG 			ulValue, 
	FTM_ULONG 			nAverage, 
	FTM_INT 			nCount, 
	FTM_ULONG 			ulMax, 
	FTM_ULONG 			ulMin
);

FTM_RET	FTOM_MQTT_CLIENT_FT_publishEPDataFLOAT
(
	FTOM_MQTT_CLIENT_PTR pClient, 
	FTM_EP_ID 			xEPID, 
	FTM_ULONG 			ulTime, 
	FTM_FLOAT 			fValue, 
	FTM_FLOAT 			nAverage, 
	FTM_INT 			nCount, 
	FTM_FLOAT 			fMax, 
	FTM_FLOAT 			fMin
);

FTM_RET	FTOM_MQTT_CLIENT_FT_publishEPDataBOOL
(
	FTOM_MQTT_CLIENT_PTR pClient, 
	FTM_EP_ID 			xEPID, 
	FTM_ULONG 			ulTime, 
	FTM_BOOL 			bValue
);

#endif
