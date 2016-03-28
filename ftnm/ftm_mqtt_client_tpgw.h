#ifndef	_FTM_MQTT_CLIENT_TPGW_H_

FTM_VOID FTM_MQTT_CLIENT_TPGW_connectCB(struct mosquitto *mosq, void *pObj, int nResult);
FTM_VOID FTM_MQTT_CLIENT_TPGW_disconnectCB(struct mosquitto *mosq, void *pObj, int nResult);
FTM_VOID FTM_MQTT_CLIENT_TPGW_publishCB(struct mosquitto *mosq, void *pObj, int nResult);
FTM_VOID FTM_MQTT_CLIENT_TPGW_messageCB(struct mosquitto *mosq, void *pObj, const struct mosquitto_message *message);
FTM_VOID FTM_MQTT_CLIENT_TPGW_subscribeCB(struct mosquitto *mosq, void *pObj, int nMID, int nQoS, const int *pGrantedQoS);

FTM_RET	FTM_MQTT_CLIENT_TPGW_topicParser(FTM_CHAR_PTR	pTopic, FTM_CHAR_PTR	pArgv[], FTM_INT nMaxArgc, FTM_INT_PTR	pnArgc);
FTM_RET	FTM_MQTT_CLIENT_TPGW_requestMessageParser(FTM_CHAR_PTR pMessage, FTNM_MSG_PTR _PTR_	pMsg);
FTM_RET	FTM_MQTT_CLIENT_TPGW_publishEPDataINT(FTM_MQTT_CLIENT_PTR pClient, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_INT nValue, FTM_INT nAverage, FTM_INT nCount, FTM_INT nMax, FTM_INT nMin);
FTM_RET	FTM_MQTT_CLIENT_TPGW_publishEPDataULONG(FTM_MQTT_CLIENT_PTR pClient, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_ULONG ulValue, FTM_ULONG nAverage, FTM_INT nCount, FTM_ULONG ulMax, FTM_ULONG ulMin);
FTM_RET	FTM_MQTT_CLIENT_TPGW_publishEPDataFLOAT(FTM_MQTT_CLIENT_PTR pClient, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_FLOAT fValue, FTM_FLOAT nAverage, FTM_INT nCount, FTM_FLOAT fMax, FTM_FLOAT fMin);
FTM_RET	FTM_MQTT_CLIENT_TPGW_publishEPDataBOOL(FTM_MQTT_CLIENT_PTR pClient, FTM_EP_ID xEPID, FTM_ULONG ulTime, FTM_BOOL bValu);

#endif
