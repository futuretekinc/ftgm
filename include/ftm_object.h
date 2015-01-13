#ifndef	__FTM_OBJECT_H__
#define __FTM_OBJECT_H__

#define	FTM_DID_LEN							32
#define	FTM_URL_LEN							256
#define	FTM_LOCATION_LEN					256
#define	FTM_SNMP_COMMUNITY_LEN				128

#define	FTM_NAME_LEN						256
#define	FTM_UNIT_LEN						32

typedef	unsigned long	FTM_NODE_TYPE, _PTR_ FTM_NODE_TYPE_PTR;

#define	FTM_NODE_TYPE_SNMP 		 			0x00000101
#define	FTM_NODE_TYPE_MODBUS_OVER_TCP		0x00000102
#define	FTM_NODE_TYPE_MODBUS_OVER_SERIAL	0x00000202

typedef	unsigned long	FTM_IP_ADDR, _PTR_ FTM_IP_ADDR_PTR;

typedef	unsigned long	FTM_CMD, _PTR_ FTM_CMD_PTR;

typedef	struct
{
	FTM_ULONG			nVersion;
	FTM_CHAR			pURL[FTM_URL_LEN + 1];
	FTM_CHAR			pCommunity[FTM_SNMP_COMMUNITY_LEN + 1];
}	FTM_SNMP_NODE_INFO, _PTR_ FTM_SNMP_NODE_INFO_PTR;

typedef	struct 
{
	FTM_CHAR			pDID[FTM_DID_LEN + 1];
	FTM_NODE_TYPE		xType;
	FTM_CHAR			pLocation[FTM_LOCATION_LEN + 1];
	FTM_ULONG			ulInterval;
	union 
	{
		FTM_SNMP_NODE_INFO	xSNMP;
	}					xOption;
}	FTM_NODE_INFO, _PTR_ FTM_NODE_INFO_PTR;

typedef	unsigned long	FTM_EP_CLASS, _PTR_ FTM_EP_CLASS_PTR;
typedef	unsigned long	FTM_EPID, _PTR_ FTM_EPID_PTR;
typedef	unsigned long	FTM_EP_TYPE, _PTR_ FTM_EP_TYPE_PTR;

#define	FTM_EP_CLASS_MASK			0x7F000000
#define	FTM_EP_CLASS_TEMPERATURE	0x01000000
#define	FTM_EP_CLASS_HUMIDITY		0x02000000
#define	FTM_EP_CLASS_VOLTAGE		0x03000000
#define	FTM_EP_CLASS_CURRENT		0x04000000
#define	FTM_EP_CLASS_DI				0x05000000
#define	FTM_EP_CLASS_DO				0x06000000
#define	FTM_EP_CLASS_GAS			0x07000000
#define	FTM_EP_CLASS_POWER			0x08000000
#define	FTM_EP_CLASS_SRF			0x09000000
#define	FTM_EP_CLASS_AI				0x0A000000
#define	FTM_EP_CLASS_MULTI			0x7F000000

typedef	struct
{
	FTM_EP_CLASS	xClass;	
	struct
	{
		FTM_CHAR	pID[32];
		FTM_CHAR	pType[32];
		FTM_CHAR	pName[32];
		FTM_CHAR	pSN[32];
		FTM_CHAR	pState[32];
		FTM_CHAR	pValue[32];
		FTM_CHAR	pTime[32];
	}	xOIDs;
}	FTM_EP_CLASS_INFO, _PTR_ FTM_EP_CLASS_INFO_PTR;

#define	FTM_EP_TYPE_MASK			0x7FFF0000

typedef	struct
{
	FTM_EPID		xEPID;
	FTM_EP_TYPE		xType;
	FTM_CHAR		pName[FTM_NAME_LEN+1];
	FTM_CHAR		pUnit[FTM_UNIT_LEN+1];
	FTM_ULONG		nInterval;	
	FTM_CHAR		pDID[FTM_DID_LEN+1];
	FTM_CHAR		pPID[FTM_DID_LEN+1];
}	FTM_EP_INFO, _PTR_ FTM_EP_INFO_PTR;

FTM_RET			FTM_initEPTypeString(void);
FTM_RET			FTM_finalEPTypeString(void);
FTM_RET			FTM_appendEPTypeString(FTM_EP_TYPE xType, FTM_CHAR_PTR pTypeString);
FTM_CHAR_PTR	FTM_getEPTypeString(FTM_EP_TYPE xType);

typedef	struct
{
	FTM_ULONG		ulClass;
	FTM_CHAR		pID[32];
	FTM_CHAR		pType[32];
	FTM_CHAR		pName[32];
	FTM_CHAR		pSN[32];
	FTM_CHAR		pState[32];
	FTM_CHAR		pValue[32];
	FTM_CHAR		pTime[32];
}	FTM_EP_OID_INFO, _PTR_ FTM_EP_OID_INFO_PTR;

FTM_RET	FTM_initEPOIDInfo(void);
FTM_RET	FTM_finalEPOIDInfo(void);
FTM_RET	FTM_addEPOIDInfo(FTM_EP_OID_INFO_PTR pOIDInfo);
FTM_RET	FTM_getEPOIDInfo(FTM_EPID xEPID, FTM_EP_OID_INFO_PTR _PTR_ ppOIDInfo);

typedef	unsigned long	FTM_EP_DATA_TYPE, _PTR_ FTM_EP_DATA_TYPE_PTR;

#define	FTM_EP_DATA_TYPE_INT	0
#define	FTM_EP_DATA_TYPE_ULONG	1
#define	FTM_EP_DATA_TYPE_FLOAT	2

typedef	struct
{
	FTM_ULONG			nTime;
	FTM_EP_DATA_TYPE	xType;
	union 
	{
		FTM_LONG	nValue;
		FTM_ULONG	ulValue;
		FTM_DOUBLE	fValue;
	}	xValue;
}	FTM_EP_DATA, _PTR_ FTM_EP_DATA_PTR;

FTM_CHAR_PTR FTM_nodeTypeString(FTM_NODE_TYPE nType);


typedef	struct
{
	FTM_USHORT			usPort;
	FTM_ULONG			ulSessionCount;
}	FTM_SERVER_INFO, _PTR_ FTM_SERVER_INFO_PTR;

typedef	struct
{
	FTM_CHAR_PTR		pFileName;
}	FTM_DB_INFO, _PTR_ FTM_DB_INFO_PTR;

#endif

