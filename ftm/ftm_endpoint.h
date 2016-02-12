#ifndef	__FTM_EP_H__
#define __FTM_EP_H__

#define	FTM_EP_NAME_LEN						256
#define	FTM_EP_UNIT_LEN						32

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
#define	FTM_EP_CLASS_AI				0x0A000000
#define	FTM_EP_CLASS_COUNT			0x0B000000
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
	FTM_CHAR		pName[FTM_EP_NAME_LEN+1];
	FTM_CHAR		pUnit[FTM_EP_UNIT_LEN+1];
	FTM_ULONG		ulTimeout;
	FTM_ULONG		ulInterval;	
	FTM_CHAR		pDID[FTM_DID_LEN+1];
	FTM_EPID		xDEPID;
	FTM_CHAR		pPID[FTM_DID_LEN+1];
	FTM_EPID		xPEPID;
}	FTM_EP_INFO, _PTR_ FTM_EP_INFO_PTR;

FTM_RET			FTM_initEPTypeString(void);
FTM_RET			FTM_finalEPTypeString(void);
FTM_RET			FTM_appendEPTypeString(FTM_EP_TYPE xType, FTM_CHAR_PTR pTypeString);
FTM_CHAR_PTR	FTM_getEPTypeString(FTM_EP_TYPE xType);

typedef	unsigned long	FTM_EP_DATA_TYPE, _PTR_ FTM_EP_DATA_TYPE_PTR;

#define	FTM_EP_DATA_TYPE_INT	0
#define	FTM_EP_DATA_TYPE_ULONG	1
#define	FTM_EP_DATA_TYPE_FLOAT	2

typedef	struct
{
	FTM_ULONG			ulTime;
	FTM_EP_DATA_TYPE	xType;
	union 
	{
		FTM_INT		nValue;
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

FTM_RET	FTM_EP_DATA_snprint(FTM_CHAR_PTR pBuff, FTM_ULONG ulMaxLen, FTM_EP_DATA_PTR pData);

#endif

