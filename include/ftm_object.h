#ifndef	__FTM_OBJECT_H__
#define __FTM_OBJECT_H__

#define	FTM_DID_LEN							32
#define	FTM_URL_LEN							256
#define	FTM_LOCATION_LEN					256
#define	FTM_SNMP_COMMUNITY_LEN				128

#define	FTM_NAME_LEN						256
#define	FTM_UNIT_LEN						32

typedef	unsigned long	FTM_NODE_TYPE, _PTR_ FTM_NODE_TYPE_PTR;

#define	FTM_NODE_TYPE_ETH_SNMP 		 0x00000101
#define	FTM_NODE_TYPE_ETH_MODBUS	 0x00000102
#define	FTM_NODE_TYPE_SERIAL_MODBUS	 0x00000202

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
	union 
	{
		FTM_SNMP_NODE_INFO	xSNMP;
	}					xOption;
}	FTM_NODE_INFO, _PTR_ FTM_NODE_INFO_PTR;

typedef	unsigned long	FTM_EPID, _PTR_ FTM_EPID_PTR;
typedef	unsigned long	FTM_EP_TYPE, _PTR_ FTM_EP_TYPE_PTR;

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

typedef	struct
{
	FTM_EPID		xEPID;
	FTM_ULONG		nTime;
	FTM_ULONG		nValue;
}	FTM_EP_DATA, _PTR_ FTM_EP_DATA_PTR;
#endif

