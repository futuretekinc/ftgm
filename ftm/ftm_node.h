#ifndef	__FTM_NODE_H__
#define __FTM_NODE_H__

#define	FTM_DID_LEN							32
#define	FTM_URL_LEN							256
#define	FTM_LOCATION_LEN					256
#define	FTM_SNMP_COMMUNITY_LEN				128
#define	FTM_SNMP_MIB_LEN					128

#define	FTM_MQTT_TOPIC_LEN					128

#define	FTM_NAME_LEN						256
#define	FTM_UNIT_LEN						32

typedef	unsigned long	FTM_NODE_TYPE, _PTR_ FTM_NODE_TYPE_PTR;

#define	FTM_NODE_TYPE_SNMP 		 			0x00000101
#define	FTM_NODE_TYPE_MODBUS_OVER_TCP		0x00000102
#define	FTM_NODE_TYPE_MODBUS_OVER_SERIAL	0x00000202


typedef	struct 
{
	FTM_CHAR			pDID[FTM_DID_LEN + 1];
	FTM_NODE_TYPE		xType;
	FTM_CHAR			pLocation[FTM_LOCATION_LEN + 1];
	FTM_ULONG			ulInterval;
	FTM_ULONG			ulTimeout;
	union 
	{
		struct
		{
			FTM_ULONG	ulVersion;
			FTM_CHAR	pURL[FTM_URL_LEN + 1];
			FTM_CHAR	pCommunity[FTM_SNMP_COMMUNITY_LEN + 1];
			FTM_CHAR	pMIB[FTM_SNMP_MIB_LEN + 1];
		} xSNMP;

		struct
		{
			FTM_ULONG	ulVersion;
			FTM_CHAR	pURL[FTM_URL_LEN + 1];
			FTM_CHAR	pTopic[FTM_MQTT_TOPIC_LEN + 1];
		} xMQTT;
	}					xOption;
}	FTM_NODE_INFO, _PTR_ FTM_NODE_INFO_PTR;

#endif

