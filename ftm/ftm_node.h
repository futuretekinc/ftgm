#ifndef	__FTM_NODE_H__
#define __FTM_NODE_H__

#include "ftm_types.h"

typedef	unsigned long	FTM_NODE_TYPE, _PTR_ FTM_NODE_TYPE_PTR;

#define	FTM_NODE_TYPE_NONE					0x00000000
#define	FTM_NODE_TYPE_SNMP 		 			0x00000001
#define	FTM_NODE_TYPE_MODBUS_OVER_TCP		0x00000002
#define	FTM_NODE_TYPE_MQTT					0x00000003
#define	FTM_NODE_TYPE_MODBUS_OVER_SERIAL	0x00000004
#define	FTM_NODE_TYPE_FINS					0x00000005

typedef	unsigned long	FTM_NODE_FLAG, _PTR_ FTM_NODE_FLAG_PTR;
#define	FTM_NODE_FLAG_DYNAMIC	(1 << 0)
#define	FTM_NODE_FLAG_LAST		(1 << 0)

typedef	FTM_ULONG	FTM_NODE_FIELD, _PTR_ FTM_NODE_FIELD_PTR;
#define	FTM_NODE_FIELD_DID				(1 << 0)	
#define	FTM_NODE_FIELD_TYPE				(1 << 1)	
#define	FTM_NODE_FIELD_FLAGS			(1 << 2)	
#define	FTM_NODE_FIELD_LOCATION			(1 << 3)	
#define	FTM_NODE_FIELD_INTERVAL			(1 << 4)	
#define	FTM_NODE_FIELD_TIMEOUT			(1 << 5)	
#define	FTM_NODE_FIELD_SNMP_VERSION		(1 << 6)	
#define	FTM_NODE_FIELD_SNMP_URL			(1 << 7)	
#define	FTM_NODE_FIELD_SNMP_COMMUNITY	(1 << 8)	
#define	FTM_NODE_FIELD_SNMP_MIB			(1 << 9)	
#define	FTM_NODE_FIELD_SNMP_MAX_RETRY	(1 << 10)	
#define	FTM_NODE_FIELD_SNMP				(FTM_NODE_FIELD_SNMP_VERSION | FTM_NODE_FIELD_SNMP_URL | FTM_NODE_FIELD_SNMP_COMMUNITY | FTM_NODE_FIELD_SNMP_MIB)
#define	FTM_NODE_FIELD_MQTT_VERSION		(1 << 11)	
#define	FTM_NODE_FIELD_MQTT_URL			(1 << 12)	
#define	FTM_NODE_FIELD_MQTT_TOPIC		(1 << 13)	
#define	FTM_NODE_FIELD_MQTT				(FTM_NODE_FIELD_MQTT_VERSION | FTM_NODE_FIELD_MQTT_URL | FTM_NODE_FIELD_MQTT_TOPIC)
#define	FTM_NODE_FIELD_LORA_VERION		(1 << 14)	
#define	FTM_NODE_FIELD_LORA_DEVICE		(1 << 15)	
#define	FTM_NODE_FIELD_LORA				(FTM_NODE_FIELD_LORA_VERION | FTM_NODE_FIELD_LORA_DEVICE)
#define	FTM_NODE_FIELD_OPTION			(FTM_NODE_FIELD_SNMP | FTM_NODE_FIELD_MQTT | FTM_NODE_FIELD_LORA)
#define	FTM_NODE_FIELD_ALL				(0xFFFFFFFF)

typedef	struct
{
	FTM_ULONG	ulVersion;
	FTM_CHAR	pURL[FTM_URL_LEN + 1];
	FTM_CHAR	pCommunity[FTM_SNMP_COMMUNITY_LEN + 1];
	FTM_CHAR	pMIB[FTM_SNMP_MIB_LEN + 1];
	FTM_ULONG	ulMaxRetryCount;
}	FTM_NODE_OPT_SNMP, _PTR_ FTM_NODE_OPT_SNMP_PTR;

typedef	struct
{
	FTM_ULONG	ulVersion;
	FTM_CHAR	pURL[FTM_URL_LEN + 1];
	FTM_ULONG	ulPort;
	FTM_ULONG	ulSlaveID;
}	FTM_NODE_OPT_MODBUS_OVER_TCP, _PTR_ FTM_NODE_OPT_MODBUS_OVER_TCP_PTR;

typedef	struct
{
	FTM_ULONG	ulVersion;
	FTM_CHAR	pDIP[FTM_URL_LEN + 1];
	FTM_ULONG	ulSP;
	FTM_ULONG	ulDP;
	FTM_ULONG	ulDA;
	FTM_ULONG	ulSA;
	FTM_ULONG	ulServerID;
	FTM_ULONG	ulRetryCount;
}	FTM_NODE_OPT_FINS, _PTR_ FTM_NODE_OPT_FINS_PTR;

typedef struct
{
	FTM_ULONG	ulVersion;
	FTM_CHAR	pURL[FTM_URL_LEN + 1];
	FTM_CHAR	pTopic[FTM_MQTT_TOPIC_LEN + 1];
} 	FTM_NODE_OPT_MQTT, _PTR_ FTM_NODE_OPT_MQTT_PTR;

typedef struct
{
	FTM_ULONG	ulVersion;	
	FTM_CHAR	pDevice[FTM_DEVICE_NAME_LEN + 1];
} 	FTM_NODE_OPT_LORA, _PTR_ FTM_NODE_OPT_LORA_PTR;

typedef	struct 
{
	FTM_CHAR			pDID[FTM_DID_LEN + 1];
	FTM_NODE_TYPE		xType;
	FTM_CHAR			pModel[FTM_NAME_LEN + 1];
	FTM_NODE_FLAG		xFlags;
	FTM_CHAR			pLocation[FTM_LOCATION_LEN + 1];
	FTM_ULONG			ulInterval;
	FTM_ULONG			ulTimeout;
	union 
	{
		FTM_NODE_OPT_SNMP				xSNMP;
		FTM_NODE_OPT_MODBUS_OVER_TCP	xMB;
		FTM_NODE_OPT_FINS				xFINS;
		FTM_NODE_OPT_MQTT				xMQTT;
		FTM_NODE_OPT_LORA				xLoRa;
	}					xOption;
}	FTM_NODE, _PTR_ FTM_NODE_PTR;

FTM_RET	FTM_NODE_create
(
	FTM_NODE_PTR _PTR_ ppNode
);

FTM_RET	FTM_NODE_destroy
(
	FTM_NODE_PTR _PTR_ ppNode
);

FTM_RET	FTM_NODE_isValid
(
	FTM_NODE_PTR pNode
);

FTM_RET FTM_NODE_isValidType
(
	FTM_ULONG	xType
);

FTM_RET	FTM_NODE_isValidTimeout
(
	FTM_NODE_PTR pNode, 
	FTM_ULONG 	ulTimeout
);

FTM_RET	FTM_NODE_isValidSNMPOpt
(
	FTM_NODE_PTR pNode, 
	FTM_NODE_OPT_SNMP_PTR pOpts
);

FTM_RET	FTM_NODE_isStatic
(
	FTM_NODE_PTR pNode
);

FTM_RET	FTM_NODE_isDynamic
(
	FTM_NODE_PTR pNode
);

FTM_RET	FTM_NODE_setDefault
(
	FTM_NODE_PTR pNode
);

FTM_RET	FTM_NODE_setDID
(
	FTM_NODE_PTR	pNode, 
	FTM_CHAR_PTR 	pDID
);

FTM_RET	FTM_NODE_strToType
(
	FTM_CHAR_PTR 		pTypeString, 
	FTM_NODE_TYPE_PTR 	pType
);

FTM_CHAR_PTR	FTM_NODE_typeString
(
	FTM_NODE_TYPE 	xType
);

FTM_CHAR_PTR	FTM_NODE_flagString
(
	FTM_NODE_FLAG 	xFlag
);
#endif

