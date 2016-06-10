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

FTM_RET	FTOM_MQTT_CLIENT_TPGW_reportGWStatus
(
	FTOM_MQTT_CLIENT_PTR pClient, 
	FTM_CHAR_PTR		pGatewayID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout
);

FTM_RET	FTOM_MQTT_CLIENT_TPGW_publishEPStatus
(
	FTOM_MQTT_CLIENT_PTR pClient, 
	FTM_CHAR_PTR		pEPID,
	FTM_BOOL			bStatus,
	FTM_ULONG			ulTimeout
);

FTM_RET	FTOM_MQTT_CLIENT_TPGW_publishEPData
(
	FTOM_MQTT_CLIENT_PTR pClient, 
	FTM_CHAR_PTR		pEPID,
	FTM_EP_DATA_PTR 	pData,
	FTM_ULONG			ulCount
);

FTM_RET	FTOM_MQTT_CLIENT_TPGW_response
(
	FTOM_MQTT_CLIENT_PTR pClient, 
	FTM_CHAR_PTR		pMsgID,
	FTM_INT				nCode,
	FTM_CHAR_PTR		pMessage
);
#endif
