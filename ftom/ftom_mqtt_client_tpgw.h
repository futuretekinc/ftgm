#ifndef	_FTOM_MQTT_CLIENT_TPGW_H_

FTM_VOID FTOM_MQTT_CLIENT_TPGW_connectCB
(
	struct mosquitto 	*mosq, 
	void 				*pObj, 
	int 				nResult
);

FTM_VOID FTOM_MQTT_CLIENT_TPGW_disconnectCB
(
	struct mosquitto 	*mosq, 
	void 				*pObj, 
	int 				nResult
);

FTM_VOID FTOM_MQTT_CLIENT_TPGW_publishCB
(
	struct mosquitto 	*mosq, 
	void 				*pObj, 
	int 				nResult
);

FTM_VOID FTOM_MQTT_CLIENT_TPGW_messageCB
(
	struct mosquitto 	*mosq, 
	void 				*pObj, 
	const struct mosquitto_message *message
);

FTM_VOID FTOM_MQTT_CLIENT_TPGW_subscribeCB
(
	struct mosquitto 	*mosq, 
	void 				*pObj, 
	int 				nMID, 
	int 				nQoS, 
	const int 			*pGrantedQoS
);

FTM_RET	FTOM_MQTT_CLIENT_TPGW_publishEPData
(
	FTOM_MQTT_CLIENT_PTR pClient, 
	FTM_EP_ID 			xEPID, 
	FTM_EP_DATA_PTR 	pData,
	FTM_ULONG			ulCount
);

#endif