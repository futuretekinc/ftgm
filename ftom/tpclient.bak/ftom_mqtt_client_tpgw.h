#ifndef	_FTOM_MQTT_CLIENT_TPGW_H_

typedef	enum
{
	FTOM_MQTT_TPGW_PUBLISH_TYPE_EP_DATA = 0,
	FTOM_MQTT_TPGW_PUBLISH_TYPE_EP_STATUS
}	FTOM_MQTT_TPGW_PUBLISH_TYPE, _PTR_ FTOM_MQTT_TPGW_PUBLISH_TYPE_PTR;

typedef	struct
{
	FTOM_MQTT_TPGW_PUBLISH_TYPE	xType;
	FTM_CHAR					pEPID[FTM_EPID_LEN+1];
	FTM_ULONG					ulTime;
}	FTOM_MQTT_TPGW_PUBLISH_INFO, _PTR_ FTOM_MQTT_TPGW_PUBLISH_INFO_PTR;

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

FTM_RET	FTOM_MQTT_CLIENT_TPGW_GWStatus
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
